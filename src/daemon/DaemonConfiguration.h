// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The CyprusCoin Developers
// Copyright (c) 2018-2020, The WrkzCoin developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common/Util.h"
#include "config/CryptoNoteConfig.h"
#include "logging/ILogger.h"

#include <rapidjson/document.h>
#include <thread>

namespace DaemonConfig
{
    struct DaemonConfiguration
    {
        bool help = false;
        bool version = false;
        bool osVersion = false;
        bool resync = false;
        uint32_t rewindToHeight = 0;

        bool importChain = false;
        bool exportChain = false;
        uint32_t exportNumBlocks = 0;
        bool exportCheckPoints = false;

        bool printGenesisTx = false;

        std::string configFile;
        std::string dataDirectory = Tools::getDefaultDataDirectory();
        bool dumpConfig = false;
        std::string checkPoints = "default";
        std::string logFile;
        int logLevel = Logging::WARNING;
        bool noConsole = false;
        std::string outputFile;

        bool enableBlockExplorer = false;
        bool enableBlockExplorerDetailed = false;
        bool enableMining = false;
        std::string enableCors;
        bool enableTrtlRpc = false;
        std::string feeAddress;
        int feeAmount = 0;

        bool localIp = false;
        bool hideMyPort = false;
        std::string p2pInterface = "0.0.0.0";
        int p2pPort = CryptoNote::P2P_DEFAULT_PORT;
        int p2pExternalPort = 0;
        bool p2pResetPeerstate = false;
        std::string rpcInterface = "127.0.0.1";
        int rpcPort = CryptoNote::RPC_DEFAULT_PORT;

        std::vector<std::string> exclusiveNodes;
        std::vector<std::string> peers;
        std::vector<std::string> priorityNodes;
        std::vector<std::string> seedNodes;

        bool enableLevelDB = false;
        bool enableDbCompression = true;
        uint64_t dbMaxOpenFiles = CryptoNote::ROCKSDB_MAX_OPEN_FILES;
        uint64_t dbReadCacheSizeMB = CryptoNote::ROCKSDB_READ_BUFFER_MB;
        int dbThreads = static_cast<int>(std::thread::hardware_concurrency());
        uint64_t dbWriteBufferSizeMB = CryptoNote::ROCKSDB_WRITE_BUFFER_MB;
        uint64_t dbMaxFileSizeMB = CryptoNote::LEVELDB_MAX_FILE_SIZE_MB;
        bool dbOptimize = false;
        bool dbUseExperimentalSerializer = false;

        uint32_t transactionValidationThreads = std::thread::hardware_concurrency();

        DaemonConfiguration()
        {
            std::stringstream logfile;
            logfile << CryptoNote::CRYPTONOTE_NAME << "d.log";
            logFile = logfile.str();
        }
    };

    DaemonConfiguration initConfiguration(const char *path);

    void handleSettings(int argc, char *argv[], DaemonConfiguration &config);

    void handleSettings(const std::string &configFile, DaemonConfiguration &config);

    void asFile(const DaemonConfiguration &config, const std::string &filename);

    std::string asString(const DaemonConfiguration &config);

    rapidjson::Document asJSON(const DaemonConfiguration &config);
} // namespace DaemonConfig
