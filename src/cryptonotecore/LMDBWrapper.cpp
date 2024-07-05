// Copyright (c) 2018-2024, The DeroGold Developers
//
// Please see the included LICENSE file for more information.

#include "LMDBWrapper.h"

#include "DataBaseErrors.h"

#include <filesystem>

namespace CryptoNote
{
    void LMDBWrapper::init()
    {
        if (state.load() != NOT_INITIALIZED)
        {
            throw std::system_error(make_error_code(error::DataBaseErrorCodes::ALREADY_INITIALIZED));
        }

        const std::filesystem::path dbPath = getDataDir(config);

        if (!is_directory(dbPath))
        {
            std::error_code errorCode;

            if (!create_directory(dbPath, errorCode))
            {
                throw std::system_error(errorCode);
            }
        }

        THROW_IF_FAIL(mdb_env_create(&dbEnv));

        // Set about 1 TB of space.
        THROW_IF_FAIL(mdb_env_set_mapsize(dbEnv, 1 * 1024 * 1024 * 1024 * 1024));

        THROW_IF_FAIL(mdb_env_open(dbEnv, dbPath.generic_string().c_str(), 0, 644));

        THROW_IF_FAIL(mdb_txn_begin(dbEnv, nullptr, 0, &dbTxn));

        THROW_IF_FAIL(mdb_dbi_open(dbTxn, nullptr, MDB_CREATE, dbDbi));
    }

    void LMDBWrapper::shutdown()
    {
        if (state.load() != INITIALIZED)
        {
            throw std::system_error(make_error_code(CryptoNote::error::DataBaseErrorCodes::NOT_INITIALIZED));
        }

        logger(Logging::INFO) << "Closing DB.";

        mdb_env_close(dbEnv);

        state.store(NOT_INITIALIZED);
    }
} // CryptoNote