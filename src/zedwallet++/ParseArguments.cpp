// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

///////////////////////////////////////
#include <zedwallet++/ParseArguments.h>
///////////////////////////////////////

#include "version.h"

#include <config/CliHeader.h>
#include <config/Config.h>
#include <config/CryptoNoteConfig.h>
#include <config/WalletConfig.h>
#include <cxxopts.hpp>
#include <utilities/Utilities.h>
#include <zedwallet++/Utilities.h>

ZedConfig parseArguments(int argc, char **argv)
{
    ZedConfig config;

    std::string defaultRemoteDaemon = "127.0.0.1:" + std::to_string(CryptoNote::RPC_DEFAULT_PORT);

    cxxopts::Options options(argv[0], CryptoNote::getProjectCLIHeader());

    bool help, version, scanCoinbaseTransactions;

    std::string remoteDaemon;

    int logLevel;

    unsigned int threads;

    std::string logFilePath;

    options.add_options("Core")
		("h,help", "Display this help message", cxxopts::value<bool>(help)->default_value("false"))
        ("v,version", "Output software version information", cxxopts::value<bool>(version)->default_value("false"));

    options.add_options("Daemon")(
        "r,remote-daemon",
        "The daemon <host:port> combination to use for node operations.",
        cxxopts::value<std::string>(remoteDaemon)->default_value(defaultRemoteDaemon),
        "<host:port>")

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        ("ssl",
         "Use SSL when connecting to the daemon.",
         cxxopts::value<bool>(config.ssl)->default_value("false")->implicit_value("true"))
#endif
        ;

    options.add_options("Wallet")
        ("w,wallet-file",
         "Open the wallet <file>",
         cxxopts::value<std::string>(config.walletFile),
         "<file>")

        ("p,password",
         "Use the password <pass> to open the wallet",
         cxxopts::value<std::string>(config.walletPass),
         "<pass>")

        ("log-level",
         "Specify log level",
         cxxopts::value<int>(logLevel)->default_value(std::to_string(config.logLevel)),
         "#")

        ("log-file",
         "Specify filepath to log to. Logging to file is disabled by default",
         cxxopts::value<std::string>(logFilePath),
         "<file>")

        ("threads",
         "Specify number of wallet sync threads",
         cxxopts::value<unsigned int>(threads)->default_value(
         std::to_string(std::max(1u, std::thread::hardware_concurrency()))),
         "#")

        ("scan-coinbase-transactions",
         "Scan miner/coinbase transactions",
         cxxopts::value<bool>(scanCoinbaseTransactions)->default_value("false")->implicit_value("true"));

    try
    {
        const auto result = options.parse(argc, argv);

        config.walletGiven = result.count("wallet-file") != 0;

        /* We could check if the string is empty, but an empty password is valid */
        config.passGiven = result.count("password") != 0;
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        std::cout << "Error: Unable to parse command line argument options: " << e.what() << std::endl << std::endl;
        std::cout << options.help({}) << std::endl;
        exit(1);
    }

    if (help) // Do we want to display the help message?
    {
        std::cout << options.help({}) << std::endl;
        exit(0);
    }
    else if (version) // Do we want to display the software version?
    {
        std::cout << CryptoNote::getProjectCLIHeader() << std::endl;
        exit(0);
    }

    if (logLevel < Logger::DISABLED || logLevel > Logger::TRACE)
    {
        std::cout << "Log level must be between " << Logger::DISABLED << " and " << Logger::TRACE << "!" << std::endl;
        exit(1);
    }
    else
    {
        config.logLevel = static_cast<Logger::LogLevel>(logLevel);
    }

    if (logFilePath != "")
    {
        config.loggingFilePath = logFilePath;

        std::ofstream logFile(logFilePath, std::ios_base::app);

        if (!logFile)
        {
            std::cout << "Failed to open log file. Please ensure you specified "
                      << "a valid filepath and have permissions to create files "
                      << "in this directory. Error: " << strerror(errno) << std::endl;

            exit(1);
        }
    }

    if (threads == 0)
    {
        std::cout << "Thread count must be at least 1" << std::endl;
        exit(1);
    }
    else
    {
        config.threads = threads;
    }

    if (!remoteDaemon.empty())
    {
        if (!Utilities::parseDaemonAddressFromString(config.host, config.port, remoteDaemon))
        {
            std::cout << "There was an error parsing the --remote-daemon you specified" << std::endl;
            exit(1);
        }
    }

    if (scanCoinbaseTransactions)
    {
        Config::config.wallet.skipCoinbaseTransactions = false;
    }

    return config;
}
