// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2018-2020, The WrkzCoin developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "IDataBase.h"
#include "leveldb/db.h"

#include <atomic>
#include <logging/LoggerRef.h>
#include <memory>
#include <string>

namespace CryptoNote
{
    class LevelDBWrapper final : public IDataBase
    {
        enum State
        {
            NOT_INITIALIZED,
            INITIALIZED
        };

        const Logging::LoggerRef logger;
        const DataBaseConfig m_config;

        std::atomic<State> state;

        std::unique_ptr<leveldb::DB> db;

    public:
        inline static const std::string DB_NAME = "LevelDB";

        LevelDBWrapper(const std::shared_ptr<Logging::ILogger> &logger, DataBaseConfig config);

        LevelDBWrapper(const LevelDBWrapper &) = delete;
        LevelDBWrapper(LevelDBWrapper &&) = delete;

        LevelDBWrapper &operator=(const LevelDBWrapper &) = delete;
        LevelDBWrapper &operator=(LevelDBWrapper &&) = delete;

        void init() override;

        void shutdown() override;

        void destroy() override;

        std::error_code write(IWriteBatch &batch) override;

        std::error_code read(IReadBatch &batch) override;

        std::error_code readThreadSafe(IReadBatch &batch) override;

        void recreate() override;

        void optimize() override;

    private:
        std::error_code write(IWriteBatch &batch, bool sync);

        leveldb::Options getDBOptions(const DataBaseConfig &config);

        std::string getDataDir(const DataBaseConfig &config);
    };
} // namespace CryptoNote