// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The CyprusCoin Developers
// Copyright (c) 2018-2020, The WrkzCoin developers
//
// Please see the included LICENSE file for more information.

#include "DaemonConfiguration.h"

#include "common/PathTools.h"
#include "common/Util.h"
#include "rapidjson/stringbuffer.h"

#include <config/CliHeader.h>
#include <config/CryptoNoteConfig.h>
#include <cxxopts.hpp>
#include <fstream>
#include <logging/ILogger.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

using namespace rapidjson;

namespace DaemonConfig
{
    DaemonConfiguration initConfiguration(const char *path)
    {
        DaemonConfiguration config;
        config.logFile = Common::ReplaceExtenstion(Common::NativePathToGeneric(path), ".log");
        return config;
    }

    void handleSettings(const int argc, char *argv[], DaemonConfiguration &config)
    {
        cxxopts::Options options(argv[0], CryptoNote::getProjectCLIHeader());

        options.add_options("Core")
            ("help", "Display this help message.", cxxopts::value<bool>(config.help))
            ("version", "Output daemon version information.", cxxopts::value<bool>(config.version))
            ("os-version", "Output Operating System version information.", cxxopts::value<bool>(config.osVersion))
            ("resync", "Forces the daemon to delete the blockchain data and start resyncing.", cxxopts::value<bool>(config.resync))
            ("rewind-to-height", "Rewinds the local blockchain cache to the specified height.", cxxopts::value<uint32_t>(config.rewindToHeight), "<height>");

        options.add_options("Import / Export")
            ("import-blockchain", "Import blockchain from dump file.", cxxopts::value<bool>(config.importChain))
            ("export-blockchain", "Export blockchain to a dump file.", cxxopts::value<bool>(config.exportChain))
            ("max-export-blocks", "Maximum number of blocks for export to dump file.", cxxopts::value<uint32_t>(config.exportNumBlocks), "<blocks>")
            ("export-checkpoints", "Export blockchain checkpoints.", cxxopts::value<bool>(config.exportCheckPoints));

        options.add_options("Genesis Block")
            ("print-genesis-tx", "Print the genesis block transaction hex and exits.", cxxopts::value<bool>(config.printGenesisTx));

        options.add_options("Daemon")
            ("c,config-file", "Specify the <path> to a configuration file", cxxopts::value<std::string>(config.configFile), "<path>")
            ("data-dir", "Specify the <path> to the Blockchain data directory", cxxopts::value<std::string>(config.dataDirectory), "<path>")
            ("dump-config", "Prints the current configuration to the screen", cxxopts::value<bool>(config.dumpConfig))
            ("load-checkpoints", "Specify a file <path> containing a CSV of Blockchain checkpoints for faster sync. A value of 'default' uses the built-in checkpoints.", cxxopts::value<std::string>(config.checkPoints), "<path>")
            ("log-file", "Specify the <path> to the log file", cxxopts::value<std::string>(config.logFile), "<path>")
            ("log-level", "Specify log level", cxxopts::value<int>(config.logLevel))
            ("no-console", "Disable daemon console commands", cxxopts::value<bool>(config.noConsole))
            ("save-config", "Save the configuration to the specified <file>", cxxopts::value<std::string>(config.outputFile), "<file>");

        options.add_options("RPC")
            ("enable-blockexplorer", "Enable the Blockchain Explorer RPC", cxxopts::value<bool>(config.enableBlockExplorer))
            ("enable-blockexplorer-detailed", "Enable the Blockchain Explorer Detailed RPC", cxxopts::value<bool>(config.enableBlockExplorerDetailed))
            ("enable-mining", "Enable Mining RPC", cxxopts::value<bool>(config.enableMining))
            ("enable-cors", "Adds header 'Access-Control-Allow-Origin' to the RPC responses using the <domain>. Uses the value specified as the domain. Use * for all.", cxxopts::value<std::string>(config.enableCors), "<domain>")
            ("enable-trtl-rpc", "Enable the turtlecoin RPC API", cxxopts::value<bool>(config.enableTrtlRpc))
            ("fee-address", "Sets the convenience charge <address> for light wallets that use the daemon", cxxopts::value<std::string>(config.feeAddress), "<address>")
            ("fee-amount", "Sets the convenience charge amount for light wallets that use the daemon", cxxopts::value<int>(config.feeAmount));

        options.add_options("Network")
            ("allow-local-ip", "Allow the local IP to be added to the peer list", cxxopts::value<bool>(config.localIp))
            ("hide-my-port", "Do not announce yourself as a peerlist candidate", cxxopts::value<bool>(config.hideMyPort))
            ("p2p-bind-ip", "Interface IP address for the P2P service", cxxopts::value<std::string>(config.p2pInterface), "<ip>")
            ("p2p-bind-port", "TCP port for the P2P service", cxxopts::value<int>(config.p2pPort), "#")
            ("p2p-external-port", "External TCP port for the P2P service (NAT port forward)", cxxopts::value<int>(config.p2pExternalPort), "#")
            ("p2p-reset-peerstate", "Generate a new peer ID and remove known peers saved previously", cxxopts::value<bool>(config.p2pResetPeerstate))
            ("rpc-bind-ip", "Interface IP address for the RPC service", cxxopts::value<std::string>(config.rpcInterface), "<ip>")
            ("rpc-bind-port", "TCP port for the RPC service", cxxopts::value<int>(config.rpcPort), "#");

        options.add_options("Peer")
            ("add-exclusive-node", "Manually add a peer to the local peer list ONLY attempt connections to it. [ip:port]", cxxopts::value<std::vector<std::string>>(config.exclusiveNodes), "<ip:port>")
            ("add-peer", "Manually add a peer to the local peer list", cxxopts::value<std::vector<std::string>>(config.peers), "<ip:port>")
            ("add-priority-node", "Manually add a peer to the local peer list and attempt to maintain a connection to it [ip:port]", cxxopts::value<std::vector<std::string>>(config.priorityNodes), "<ip:port>")
            ("seed-node", "Connect to a node to retrieve the peer list and then disconnect", cxxopts::value<std::vector<std::string>>(config.seedNodes), "<ip:port>");

        const std::string maxOpenFiles = 
            "(default: " + std::to_string(CryptoNote::ROCKSDB_MAX_OPEN_FILES) 
            + " (ROCKSDB), " + std::to_string(CryptoNote::LEVELDB_MAX_OPEN_FILES)
            + " (LEVELDB))";

        const std::string readCache = 
            "(default: " + std::to_string(CryptoNote::ROCKSDB_READ_BUFFER_MB) 
            + " (ROCKSDB), " + std::to_string(CryptoNote::LEVELDB_READ_BUFFER_MB)
            + " (LEVELDB))";

        const std::string writeBuffer = 
            "(default: " + std::to_string(CryptoNote::ROCKSDB_WRITE_BUFFER_MB) 
            + " (ROCKSDB), " + std::to_string(CryptoNote::LEVELDB_WRITE_BUFFER_MB)
            + " (LEVELDB))";

        options.add_options("Database")
            ("db-enable-level-db", "Use LevelDB instead of RocksDB", cxxopts::value<bool>(config.enableLevelDB))
            ("db-enable-compression", "Enable database compression", cxxopts::value<bool>(config.enableDbCompression)->default_value(config.enableDbCompression ? "true" : "false"))
            ("db-max-open-files", "Number of files that can be used by the database at one time " + maxOpenFiles, cxxopts::value<int>())
            ("db-read-buffer-size", "Size of the database read cache in megabytes (MB) " + readCache, cxxopts::value<int>())
            ("db-threads", "Number of background threads used for compaction and flush operations (RocksDB only)", cxxopts::value<int>(config.dbThreads))
            ("db-write-buffer-size", "Size of the database write buffer in megabytes (MB) " + writeBuffer, cxxopts::value<int>())
            ("db-max-file-size", "Max file size of database files in megabytes (MB) (LevelDB only)", cxxopts::value<int>()->default_value(std::to_string(CryptoNote::LEVELDB_MAX_FILE_SIZE_MB)))
            ("db-optimize", "Optimize database and close", cxxopts::value<bool>(config.dbOptimize))
            ("db-purge", "Purge unwanted data in the database and close", cxxopts::value<bool>(config.dbPurge));

        options.add_options("Syncing")
            ("transaction-validation-threads", "Number of threads to use to validate a transaction's inputs in parallel.", cxxopts::value<uint32_t>(config.transactionValidationThreads));

        try
        {
            const auto cli = options.parse(argc, argv);

            if (cli.count("rewind-to-height") > 0 && config.rewindToHeight == 0)
            {
                std::cout << CryptoNote::getProjectCLIHeader()
                          << "Please use the `--resync` option instead of `--rewind-to-height 0` to completely "
                             "reset the synchronization state."
                          << std::endl;
                exit(1);
            }

            if (cli.count("max-export-blocks") > 0 && config.exportNumBlocks == 0)
            {
                std::cout << CryptoNote::getProjectCLIHeader()
                          << "`--max-export-blocks` can not be 0."
                          << std::endl;
                exit(1);
            }

            /* Using levelDB, lets set the level DB defaults. Will overwrite with
             * passed in values later if present. */
            if (cli.count("db-enable-level-db") > 0 && config.enableLevelDB)
            {
                config.dbMaxOpenFiles = cli.count("db-max-open-files") > 0 ? cli["db-max-open-files"].as<int>() : CryptoNote::LEVELDB_MAX_OPEN_FILES;
                config.dbReadCacheSizeMB = cli.count("db-read-buffer-size") > 0 ? cli["db-read-buffer-size"].as<int>() : CryptoNote::LEVELDB_READ_BUFFER_MB;
                config.dbWriteBufferSizeMB = cli.count("db-write-buffer-size") > 0 ? cli["db-write-buffer-size"].as<int>() : CryptoNote::LEVELDB_WRITE_BUFFER_MB;
                config.dbMaxFileSizeMB = cli.count("db-max-file-size") > 0 ? cli["db-max-file-size"].as<int>() : CryptoNote::LEVELDB_MAX_FILE_SIZE_MB;
            }
            else
            {
                config.dbMaxOpenFiles = cli.count("db-max-open-files") > 0 ? cli["db-max-open-files"].as<int>() : CryptoNote::ROCKSDB_MAX_OPEN_FILES;
                config.dbReadCacheSizeMB = cli.count("db-read-buffer-size") > 0 ? cli["db-read-buffer-size"].as<int>() : CryptoNote::ROCKSDB_READ_BUFFER_MB;
                config.dbWriteBufferSizeMB = cli.count("db-write-buffer-size") > 0 ? cli["db-write-buffer-size"].as<int>() : CryptoNote::ROCKSDB_WRITE_BUFFER_MB;
            }

            if (cli.count("db-max-file-size") > 0)
            {
                config.dbMaxFileSizeMB = cli["db-max-file-size"].as<int>();
            }

            if (config.help) // Do we want to display the help message?
            {
                std::cout << options.help() << std::endl;
                exit(0);
            }

            if (config.version) // Do we want to display the software version?
            {
                std::cout << CryptoNote::getProjectCLIHeader() << std::endl;
                exit(0);
            }

            if (config.osVersion) // Do we want to display the OS version information?
            {
                std::cout << CryptoNote::getProjectCLIHeader() << "OS: " << Tools::get_os_version_string() << std::endl;
                exit(0);
            }
        }
        catch (const cxxopts::exceptions::exception &e)
        {
            std::cout << "Error: Unable to parse command line argument options: " << e.what() << std::endl
                      << std::endl
                      << options.help()
                      << std::endl;
            exit(1);
        }
    }

    void handleSettings(const std::string& configFile, DaemonConfiguration &config)
    {
        std::ifstream data(configFile);

        if (!data.good())
        {
            throw std::runtime_error("The --config-file you specified does not exist, please check the filename and try again.");
        }

        IStreamWrapper isw(data);

        Document j;
        j.ParseStream(isw);

        // Daemon Options

        if (j.HasMember("data-dir"))
        {
            config.dataDirectory = j["data-dir"].GetString();
        }

        if (j.HasMember("load-checkpoints"))
        {
            config.checkPoints = j["load-checkpoints"].GetString();
        }

        if (j.HasMember("log-file"))
        {
            config.logFile = j["log-file"].GetString();
        }

        if (j.HasMember("log-level"))
        {
            config.logLevel = j["log-level"].GetInt();
        }

        if (j.HasMember("no-console"))
        {
            config.noConsole = j["no-console"].GetBool();
        }

        // RPC Options

        if (j.HasMember("enable-blockexplorer"))
        {
            config.enableBlockExplorer = j["enable-blockexplorer"].GetBool();
        }

        if (j.HasMember("enable-blockexplorer-detailed"))
        {
            config.enableBlockExplorerDetailed = j["enable-blockexplorer-detailed"].GetBool();
        }

        if (j.HasMember("enable-mining"))
        {
            config.enableMining = j["enable-mining"].GetBool();
        }

        if (j.HasMember("enable-cors"))
        {
            config.enableCors = j["enable-cors"].GetString();
        }

        if (j.HasMember("enable-trtl-api"))
        {
            config.enableTrtlRpc = j["enable-trtl-api"].GetBool();
        }

        if (j.HasMember("fee-address"))
        {
            config.feeAddress = j["fee-address"].GetString();
        }

        if (j.HasMember("fee-amount"))
        {
            config.feeAmount = j["fee-amount"].GetInt();
        }

        // Network Options

        if (j.HasMember("allow-local-ip"))
        {
            config.localIp = j["allow-local-ip"].GetBool();
        }

        if (j.HasMember("hide-my-port"))
        {
            config.hideMyPort = j["hide-my-port"].GetBool();
        }

        if (j.HasMember("p2p-bind-ip"))
        {
            config.p2pInterface = j["p2p-bind-ip"].GetString();
        }

        if (j.HasMember("p2p-bind-port"))
        {
            config.p2pPort = j["p2p-bind-port"].GetInt();
        }

        if (j.HasMember("p2p-external-port"))
        {
            config.p2pExternalPort = j["p2p-external-port"].GetInt();
        }

        if (j.HasMember("p2p-reset-peerstate"))
        {
            config.p2pResetPeerstate = j["p2p-reset-peerstate"].GetBool();
        }

        if (j.HasMember("rpc-bind-ip"))
        {
            config.rpcInterface = j["rpc-bind-ip"].GetString();
        }

        if (j.HasMember("rpc-bind-port"))
        {
            config.rpcPort = j["rpc-bind-port"].GetInt();
        }

        // Peer Options

        if (j.HasMember("add-exclusive-node"))
        {
            const Value &va = j["add-exclusive-node"];

            for (auto &v : va.GetArray())
            {
                config.exclusiveNodes.emplace_back(v.GetString());
            }
        }

        if (j.HasMember("add-peer"))
        {
            const Value &va = j["add-peer"];

            for (auto &v : va.GetArray())
            {
                config.peers.emplace_back(v.GetString());
            }
        }

        if (j.HasMember("add-priority-node"))
        {
            const Value &va = j["add-priority-node"];

            for (auto &v : va.GetArray())
            {
                config.priorityNodes.emplace_back(v.GetString());
            }
        }

        if (j.HasMember("seed-node"))
        {
            const Value &va = j["seed-node"];

            for (auto &v : va.GetArray())
            {
                config.seedNodes.emplace_back(v.GetString());
            }
        }

        // Database Options

        /* Using levelDB, lets set the level DB defaults. Will overwrite with
         * passed in values later if present. */
        if (j.HasMember("db-enable-level-db") && j["db-enable-level-db"].GetBool())
        {
            config.enableLevelDB = true;
            config.dbMaxOpenFiles = CryptoNote::LEVELDB_MAX_OPEN_FILES;
            config.dbReadCacheSizeMB = CryptoNote::LEVELDB_READ_BUFFER_MB;
            config.dbWriteBufferSizeMB = CryptoNote::LEVELDB_WRITE_BUFFER_MB;
            config.dbMaxFileSizeMB = CryptoNote::LEVELDB_MAX_FILE_SIZE_MB;
        }
        else
        {
            config.dbMaxOpenFiles = CryptoNote::ROCKSDB_MAX_OPEN_FILES;
            config.dbReadCacheSizeMB = CryptoNote::ROCKSDB_READ_BUFFER_MB;
            config.dbWriteBufferSizeMB = CryptoNote::ROCKSDB_WRITE_BUFFER_MB;
            config.dbThreads = CryptoNote::ROCKSDB_BACKGROUND_THREADS;
        }

        if (j.HasMember("db-enable-compression"))
        {
            config.enableDbCompression = j["db-enable-compression"].GetBool();
        }

        if (j.HasMember("db-max-open-files"))
        {
            config.dbMaxOpenFiles = j["db-max-open-files"].GetInt();
        }

        if (j.HasMember("db-read-buffer-size"))
        {
            config.dbReadCacheSizeMB = j["db-read-buffer-size"].GetInt();
        }

        if (j.HasMember("db-threads"))
        {
            config.dbThreads = j["db-threads"].GetInt();
        }

        if (j.HasMember("db-write-buffer-size"))
        {
            config.dbWriteBufferSizeMB = j["db-write-buffer-size"].GetInt();
        }

        if (j.HasMember("db-max-file-size"))
        {
            config.dbMaxFileSizeMB = j["db-max-file-size"].GetInt();
        }

        // Syncing Options

        if (j.HasMember("transaction-validation-threads"))
        {
            config.transactionValidationThreads = j["transaction-validation-threads"].GetInt();
        }
    }

    Document asJSON(const DaemonConfiguration &config)
    {
        Document j;
        Document::AllocatorType &alloc = j.GetAllocator();

        j.SetObject();

        j.AddMember("data-dir", config.dataDirectory, alloc);
        j.AddMember("load-checkpoints", config.checkPoints, alloc);
        j.AddMember("log-file", config.logFile, alloc);
        j.AddMember("log-level", config.logLevel, alloc);
        j.AddMember("no-console", config.noConsole, alloc);

        j.AddMember("enable-blockexplorer", config.enableBlockExplorer, alloc);
        j.AddMember("enable-blockexplorer-detailed", config.enableBlockExplorerDetailed, alloc);
        j.AddMember("enable-mining", config.enableMining, alloc);
        j.AddMember("enable-cors", config.enableCors, alloc);
        j.AddMember("enable-trtl-api", config.enableTrtlRpc, alloc);
        j.AddMember("fee-address", config.feeAddress, alloc);
        j.AddMember("fee-amount", config.feeAmount, alloc);

        j.AddMember("allow-local-ip", config.localIp, alloc);
        j.AddMember("hide-my-port", config.hideMyPort, alloc);
        j.AddMember("p2p-bind-ip", config.p2pInterface, alloc);
        j.AddMember("p2p-bind-port", config.p2pPort, alloc);
        j.AddMember("p2p-external-port", config.p2pExternalPort, alloc);
        j.AddMember("p2p-reset-peerstate", config.p2pResetPeerstate, alloc);
        j.AddMember("rpc-bind-ip", config.rpcInterface, alloc);
        j.AddMember("rpc-bind-port", config.rpcPort, alloc);

        {
            Value arr(rapidjson::kArrayType);
            for (const auto& v : config.exclusiveNodes)
            {
                arr.PushBack(Value().SetString(StringRef(v.c_str())), alloc);
            }
            j.AddMember("add-exclusive-node", arr, alloc);
        }

        {
            Value arr(rapidjson::kArrayType);
            for (const auto& v : config.peers)
            {
                arr.PushBack(Value().SetString(StringRef(v.c_str())), alloc);
            }
            j.AddMember("add-peer", arr, alloc);
        }

        {
            Value arr(rapidjson::kArrayType);
            for (const auto& v : config.priorityNodes)
            {
                arr.PushBack(Value().SetString(StringRef(v.c_str())), alloc);
            }
            j.AddMember("add-priority-node", arr, alloc);
        }

        {
            Value arr(rapidjson::kArrayType);
            for (const auto& v : config.seedNodes)
            {
                arr.PushBack(Value().SetString(StringRef(v.c_str())), alloc);
            }
            j.AddMember("seed-node", arr, alloc);
        }

        j.AddMember("db-enable-level-db", config.enableLevelDB, alloc);
        j.AddMember("db-enable-compression", config.enableDbCompression, alloc);
        j.AddMember("db-max-open-files", config.dbMaxOpenFiles, alloc);
        j.AddMember("db-read-buffer-size", config.dbReadCacheSizeMB, alloc);
        j.AddMember("db-threads", config.dbThreads, alloc);
        j.AddMember("db-write-buffer-size", config.dbWriteBufferSizeMB, alloc);
        j.AddMember("db-max-file-size", config.dbMaxFileSizeMB, alloc);

        j.AddMember("transaction-validation-threads", config.transactionValidationThreads, alloc);

        return j;
    }

    std::string asString(const DaemonConfiguration &config)
    {
        StringBuffer stringBuffer;
        PrettyWriter writer(stringBuffer);
        asJSON(config).Accept(writer);
        return stringBuffer.GetString();
    }

    void asFile(const DaemonConfiguration &config, const std::string &filename)
    {
        std::ofstream data(filename);
        OStreamWrapper osw(data);
        PrettyWriter writer(osw);
        asJSON(config).Accept(writer);
    }
} // namespace DaemonConfig
