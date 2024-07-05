// Copyright (c) 2018-2024, The DeroGold Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "IDataBase.h"
#include "lmdb.h"
#include "logging/LoggerRef.h"

#include <memory>

#define THROW_IF_FAIL(function)                         \
    {                                                   \
        const int status = function;                    \
                                                        \
        if (status != 0)                                \
        {                                               \
            throw std::exception(mdb_strerror(status)); \
        }                                               \
    }

namespace CryptoNote
{
    class LMDBWrapper final : public IDataBase
    {
        enum State
        {
            NOT_INITIALIZED,
            INITIALIZED
        };

        inline static const std::string DB_NAME = "LMDB";

        const Logging::LoggerRef logger;
        const DataBaseConfig config;

        std::atomic<State> state;

        MDB_env *dbEnv = nullptr;
        MDB_txn *dbTxn = nullptr;
        MDB_dbi *dbDbi = nullptr;

    public:
        LMDBWrapper(const std::shared_ptr<Logging::ILogger> &logger, DataBaseConfig config) :
            logger(logger, "LMDBWrapper"),
            config(std::move(config)),
            state(NOT_INITIALIZED)
        {
        }

        ~LMDBWrapper() override
        {
            // Failsave for freeing memory as shutdown might not be called.

            if (dbEnv != nullptr)
            {
                mdb_env_close(dbEnv);
            }
        }

        void init() override;
        void shutdown() override;
        void destroy() override;
        void recreate() override;

        std::error_code write(IWriteBatch &batch) override;
        std::error_code read(IReadBatch &batch) override;
        std::error_code readThreadSafe(IReadBatch &batch) override;

    private:
        static std::string getDataDir(const DataBaseConfig &config)
        {
            return config.dataDir + "/" + DB_NAME;
        }
    };
} // namespace CryptoNote
