// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The CyprusCoin Developers
// Copyright (c) 2018-2020, The WrkzCoin developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common/Util.h"

#include <config/CryptoNoteConfig.h>
#include <logging/ILogger.h>
#include <rapidjson/document.h>
#include <thread>

using namespace rapidjson;

namespace DaemonConfig
{
    struct DaemonConfiguration
    {
        bool help;
        bool version;
        bool osVersion;
        bool resync;
        uint32_t rewindToHeight;

        bool importChain;
        bool exportChain;
        uint32_t exportNumBlocks;
        bool exportCheckPoints;

        bool printGenesisTx;
        // std::vector<std::string> genesisAwardAddresses;

        std::string configFile;
        std::string dataDirectory;
        bool dumpConfig;
        std::string checkPoints;
        std::string logFile;
        int logLevel;
        bool noConsole;
        std::string outputFile;

        bool enableBlockExplorer;
        bool enableBlockExplorerDetailed;
        bool enableMining;
        std::string enableCors;
        bool enableTrtlRpc;
        std::string feeAddress;
        int feeAmount;

        bool localIp;
        bool hideMyPort;
        std::string p2pInterface;
        int p2pPort;
        int p2pExternalPort;
        bool p2pResetPeerstate;
        std::string rpcInterface;
        int rpcPort;

        std::vector<std::string> exclusiveNodes;
        std::vector<std::string> peers;
        std::vector<std::string> priorityNodes;
        std::vector<std::string> seedNodes;

        bool enableLevelDB;
        bool enableDbCompression;
        uint64_t dbMaxOpenFiles;
        uint64_t dbReadCacheSizeMB;
        uint64_t dbThreads;
        uint64_t dbWriteBufferSizeMB;
        uint64_t dbMaxFileSizeMB;
        bool dbOptimize;
        bool dbPurge;

        uint32_t transactionValidationThreads;

        DaemonConfiguration()
        {
            std::stringstream logfile;
            logfile << CryptoNote::CRYPTONOTE_NAME << "d.log";

            dataDirectory = Tools::getDefaultDataDirectory();
            checkPoints = "default";
            logFile = logfile.str();
            logLevel = Logging::WARNING;
            rewindToHeight = 0;
            p2pInterface = "0.0.0.0";
            p2pPort = CryptoNote::P2P_DEFAULT_PORT;
            p2pExternalPort = 0;
            transactionValidationThreads = std::thread::hardware_concurrency();
            rpcInterface = "127.0.0.1";
            rpcPort = CryptoNote::RPC_DEFAULT_PORT;
            noConsole = false;
            enableBlockExplorer = false;
            enableBlockExplorerDetailed = false;
            enableMining = false;
            localIp = false;
            hideMyPort = false;
            p2pResetPeerstate = false;
            help = false;
            version = false;
            osVersion = false;
            printGenesisTx = false;
            dumpConfig = false;
            enableDbCompression = true;
            resync = false;
            enableLevelDB = false;
            importChain = false;
            exportChain = false;
            exportNumBlocks = 0;
            exportCheckPoints = false;
            feeAmount = 0;
            dbOptimize = false;
            enableTrtlRpc = false;
        }
    };

    DaemonConfiguration initConfiguration(const char *path);

    bool updateConfigFormat(const std::string &configFile, DaemonConfiguration &config);

    void handleSettings(int argc, char *argv[], DaemonConfiguration &config);

    void handleSettings(const std::string &configFile, DaemonConfiguration &config);

    void asFile(const DaemonConfiguration &config, const std::string &filename);

    std::string asString(const DaemonConfiguration &config);

    Document asJSON(const DaemonConfiguration &config);
} // namespace DaemonConfig
