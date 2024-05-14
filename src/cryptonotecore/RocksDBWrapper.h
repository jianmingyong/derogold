// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2018-2020, The WrkzCoin developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "IDataBase.h"
#include "rocksdb/db.h"

#include <atomic>
#include <logging/LoggerRef.h>
#include <memory>
#include <string>

namespace CryptoNote
{
    class RocksDBWrapper : public IDataBase
    {
      public:
        RocksDBWrapper(
            std::shared_ptr<Logging::ILogger> logger,
            DataBaseConfig config);

        ~RocksDBWrapper() override;

        RocksDBWrapper(const RocksDBWrapper &) = delete;

        RocksDBWrapper(RocksDBWrapper &&) = delete;

        RocksDBWrapper &operator=(const RocksDBWrapper &) = delete;

        RocksDBWrapper &operator=(RocksDBWrapper &&) = delete;

        void init() override;

        void shutdown() override;

        void destroy() override; // Be careful with this method!

        std::error_code write(IWriteBatch &batch) override;

        std::error_code read(IReadBatch &batch) override;

        std::error_code readThreadSafe(IReadBatch &batch) override;
        
        void recreate() override;

      private:
        std::error_code write(IWriteBatch &batch, bool sync);

        static rocksdb::Options getDBOptions(const DataBaseConfig &config);

        static std::string getDataDir(const DataBaseConfig &config);

        void optimize() const;

        enum State
        {
            NOT_INITIALIZED,
            INITIALIZED
        };

        Logging::LoggerRef logger;

        std::unique_ptr<rocksdb::DB> db;

        std::atomic<State> state;
        
        const DataBaseConfig m_config;
    };
} // namespace CryptoNote
