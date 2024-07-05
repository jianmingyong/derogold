// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2018-2020, The WrkzCoin developers
//
// Please see the included LICENSE file for more information.

#include "RocksDBWrapper.h"

#include "DataBaseErrors.h"
#include "rocksdb/table.h"
#include "rocksdb/utilities/options_util.h"

#include <utility>

namespace CryptoNote
{
    void RocksDBWrapper::init()
    {
        if (state.load() != NOT_INITIALIZED)
        {
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::ALREADY_INITIALIZED));
        }

        const std::string dataDir = getDataDir(config);

        logger(Logging::INFO) << "Opening DB in " << dataDir;

        rocksdb::DB *dbPtr;
        const rocksdb::Options dbOptions = getDBOptions(config);

        if (const rocksdb::Status status = rocksdb::DB::Open(dbOptions, dataDir, &dbPtr); status.ok())
        {
            logger(Logging::INFO) << "DB opened in " << dataDir;
        }
        else if (status.IsIOError())
        {
            logger(Logging::ERROR) << "DB Error. DB can't be opened in " << dataDir << ". Error: " << status.ToString();
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::IO_ERROR));
        }
        else
        {
            logger(Logging::ERROR) << "DB Error. DB can't be opened in " << dataDir << ". Error: " << status.ToString();
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::INTERNAL_ERROR));
        }

        db.reset(dbPtr);
        state.store(INITIALIZED);
    }

    void RocksDBWrapper::shutdown()
    {
        if (state.load() != INITIALIZED)
        {
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::NOT_INITIALIZED));
        }

        logger(Logging::INFO) << "Closing DB.";
        db->Flush(rocksdb::FlushOptions());
        db->SyncWAL();
        db.reset();
        state.store(NOT_INITIALIZED);
    }

    void RocksDBWrapper::destroy()
    {
        if (state.load() != NOT_INITIALIZED)
        {
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::ALREADY_INITIALIZED));
        }

        const std::string dataDir = getDataDir(config);

        logger(Logging::WARNING) << "Destroying DB in " << dataDir;

        const rocksdb::Options dbOptions = getDBOptions(config);

        if (const rocksdb::Status status = DestroyDB(dataDir, dbOptions); status.ok())
        {
            logger(Logging::WARNING) << "DB destroyed in " << dataDir;
        }
        else
        {
            logger(Logging::ERROR) << "DB Error. DB can't be destroyed in " << dataDir
                                   << ". Error: " << status.ToString();
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::INTERNAL_ERROR));
        }
    }

    std::error_code RocksDBWrapper::write(IWriteBatch &batch)
    {
        if (state.load() != INITIALIZED)
        {
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::NOT_INITIALIZED));
        }

        rocksdb::WriteBatch rocksdbBatch;

        const std::vector rawData(batch.extractRawDataToInsert());

        for (const auto &[key, value] : rawData)
        {
            rocksdbBatch.Put(rocksdb::Slice(key), rocksdb::Slice(value));
        }

        const std::vector rawKeys(batch.extractRawKeysToRemove());

        for (const std::string &key : rawKeys)
        {
            rocksdbBatch.Delete(rocksdb::Slice(key));
        }

        if (const rocksdb::Status status = db->Write(rocksdb::WriteOptions(), &rocksdbBatch); !status.ok())
        {
            logger(Logging::ERROR) << "Can't write to DB. " << status.ToString();
            return make_error_code(error::DataBaseErrorCodes::INTERNAL_ERROR);
        }

        return std::error_code {};
    }

    std::error_code RocksDBWrapper::read(IReadBatch &batch)
    {
        if (state.load() != INITIALIZED)
        {
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::NOT_INITIALIZED));
        }

        const std::vector rawKeys(batch.getRawKeys());

        if (rawKeys.empty())
        {
            logger(Logging::ERROR) << "RocksDBWrapper::read: detected rawKeys.size() == 0!!!";
            return make_error_code(error::DataBaseErrorCodes::INTERNAL_ERROR);
        }

        std::vector<rocksdb::Slice> keySlices;
        keySlices.reserve(rawKeys.size());

        for (const std::string &key : rawKeys)
        {
            keySlices.emplace_back(key);
        }

        std::vector<std::string> values;
        values.reserve(rawKeys.size());

        const std::vector<rocksdb::Status> statuses = db->MultiGet(rocksdb::ReadOptions(), keySlices, &values);

        std::vector<bool> resultStates;
        resultStates.reserve(rawKeys.size());

        for (const rocksdb::Status &status : statuses)
        {
            if (!status.ok() && !status.IsNotFound())
            {
                return make_error_code(error::DataBaseErrorCodes::INTERNAL_ERROR);
            }

            resultStates.push_back(status.ok());
        }

        batch.submitRawResult(values, resultStates);
        return std::error_code {};
    }

    std::error_code RocksDBWrapper::readThreadSafe(IReadBatch &batch)
    {
        if (state.load() != INITIALIZED)
        {
            throw std::runtime_error("Not initialized.");
        }

        const std::vector rawKeys(batch.getRawKeys());
        std::vector<std::string> values(rawKeys.size());
        std::vector<bool> resultStates;

        size_t i = 0;

        for (const std::string &key : rawKeys)
        {
            if (const rocksdb::Status status = db->Get(rocksdb::ReadOptions(), rocksdb::Slice(key), &values[i++]);
                status.ok())
            {
                resultStates.push_back(true);
            }
            else
            {
                if (!status.IsNotFound())
                {
                    return make_error_code(error::DataBaseErrorCodes::INTERNAL_ERROR);
                }

                resultStates.push_back(false);
            }
        }

        batch.submitRawResult(values, resultStates);
        return std::error_code {};
    }

    void RocksDBWrapper::recreate()
    {
        if (state.load() == INITIALIZED)
        {
            shutdown();
        }

        destroy();
        init();
    }

    void RocksDBWrapper::optimize()
    {
        const std::string dbData = getDataDir(config);
        const rocksdb::Options dbOptions = getDBOptions(config);
        rocksdb::DB *rocksDb;

        if (rocksdb::DB::Open(dbOptions, dbData, &rocksDb).ok())
        {
            rocksdb::CompactRangeOptions compactRangeOptions;
            compactRangeOptions.exclusive_manual_compaction = true;
            compactRangeOptions.change_level = true;

            logger(Logging::INFO) << "Preparing to optimize DB for reading... This may take a long time.";
            logger(Logging::INFO) << "Please do not close the program abruptly to prevent DB corruption.";

            rocksDb->CompactRange(compactRangeOptions, nullptr, nullptr);

            auto waitForCompactOptions = rocksdb::WaitForCompactOptions();
            waitForCompactOptions.flush = true;
            waitForCompactOptions.close_db = true;
            rocksDb->WaitForCompact(waitForCompactOptions);

            logger(Logging::INFO) << "Optimized DeroGold DB.";
        }
    }

    rocksdb::Options RocksDBWrapper::getDBOptions(const DataBaseConfig &config)
    {
        rocksdb::DBOptions dbOptions;
        dbOptions.create_if_missing = true;
        dbOptions.info_log_level = rocksdb::InfoLogLevel::INFO_LEVEL;
        dbOptions.keep_log_file_num = 1;
        dbOptions.IncreaseParallelism(static_cast<int>(config.backgroundThreadsCount));
        dbOptions.max_open_files = static_cast<int>(config.maxOpenFiles);
        dbOptions.skip_stats_update_on_db_open = true;
        dbOptions.compaction_readahead_size = 2 * 1024 * 1024;

        rocksdb::ColumnFamilyOptions fOptions;

        // sets the size of a single memtable. Once memtable exceeds this size, it is marked immutable and a new one is
        // created.
        fOptions.write_buffer_size = config.writeBufferSize / 4;
        // merge two memtables when flushing to L0
        fOptions.min_write_buffer_number_to_merge = 2;
        // this means we'll use 50% extra memory in the worst case, but will reduce
        // write stalls.
        fOptions.max_write_buffer_number = 4;
        // start flushing L0->L1 as soon as possible. each file on level0 is
        // (memtable_memory_budget / 2). This will flush level 0 when it's bigger than
        // memtable_memory_budget.
        fOptions.level0_file_num_compaction_trigger = 2;

        // doesn't really matter much, but we don't want to create too many files
        fOptions.target_file_size_base = config.writeBufferSize / 4;
        // make Level1 size equal to Level0 size, so that L0->L1 compactions are fast
        fOptions.max_bytes_for_level_base = config.writeBufferSize;

        fOptions.num_levels = 10;

        // level style compaction
        fOptions.compaction_style = rocksdb::kCompactionStyleLevel;
        fOptions.compression_per_level.resize(fOptions.num_levels);

        const auto compressionLevel = config.compressionEnabled ? rocksdb::kZSTD : rocksdb::kNoCompression;

        for (int i = 0; i < fOptions.num_levels; ++i)
        {
            // don't compress l0 & l1
            fOptions.compression_per_level[i] = i < 2 ? rocksdb::kNoCompression : compressionLevel;
        }

        fOptions.bottommost_compression = compressionLevel;

        rocksdb::BlockBasedTableOptions tableOptions;
        tableOptions.block_cache = rocksdb::NewLRUCache(config.readCacheSize);
        const std::shared_ptr<rocksdb::TableFactory> tableFactory(NewBlockBasedTableFactory(tableOptions));
        fOptions.table_factory = tableFactory;

        return rocksdb::Options {dbOptions, fOptions};
    }
} // namespace CryptoNote
