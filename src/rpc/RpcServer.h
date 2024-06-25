// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "httplib.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <cryptonotecore/Core.h>
#include <cryptonoteprotocol/CryptoNoteProtocolHandlerCommon.h>
#include <errors/Errors.h>
#include <memory>
#include <optional>
#include <p2p/NetNode.h>
#include <string>

enum class RpcMode
{
    Default = 0,
    BlockExplorerEnabled = 1,
    AllMethodsEnabled = 2,
};

class RpcServer
{
  public:
    ////////////////////////////////
    /* Constructors / Destructors */
    ////////////////////////////////
    RpcServer(
        uint16_t bindPort,
        std::string rpcBindIp,
        std::string corsHeader,
        std::string feeAddress,
        uint64_t feeAmount,
        RpcMode rpcMode,
        std::shared_ptr<CryptoNote::Core> core,
        std::shared_ptr<CryptoNote::NodeServer> p2p,
        std::shared_ptr<CryptoNote::ICryptoNoteProtocolHandler> syncManager,
        bool useTrtlApi);

    ~RpcServer();

    /////////////////////////////
    /* Public member functions */
    /////////////////////////////

    /* Starts the server. */
    void start();

    /* Stops the server. */
    void stop();

    /* Gets the IP/port combo the server is running on */
    std::tuple<std::string, uint16_t> getConnectionInfo();

  private:
    //////////////////////////////
    /* Private member functions */
    //////////////////////////////

    /* Starts listening for requests on the server */
    void listen();

    std::optional<rapidjson::Document>
        getJsonBody(const httplib::Request &req, httplib::Response &res, bool bodyRequired);

    /* Handles stuff like parsing json and then forwards onto the handler */
    void middleware(
        const httplib::Request &req,
        httplib::Response &res,
        RpcMode routePermissions,
        bool bodyRequired,
        bool syncRequired,
        const std::function<std::tuple<Error, uint16_t>(
            const httplib::Request &req,
            httplib::Response &res,
            const rapidjson::Document &body)>& handler);

    void failRequest(int errorCode, const std::string& body, httplib::Response &res);

    void failRequest(const Error& error, httplib::Response &res);

    void failJsonRpcRequest(int64_t errorCode, const std::string &errorMessage, httplib::Response &res);

    uint64_t calculateTotalFeeAmount(const std::vector<Crypto::Hash> &transactionHashes);

    void generateBlockHeader(
        const Crypto::Hash &blockHash,
        rapidjson::Writer<rapidjson::StringBuffer> &writer,
        bool headerOnly = false);

    void generateTransactionPrefix(
        const CryptoNote::Transaction &transaction,
        rapidjson::Writer<rapidjson::StringBuffer> &writer);

    /////////////////////
    /* OPTION REQUESTS */
    /////////////////////

    void handleOptions(const httplib::Request &req, httplib::Response &res) const;

    //////////////////
    /* GET REQUESTS */
    //////////////////

    std::tuple<Error, uint16_t>
        getBlockHeaderByHashTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getBlockHeaderByHeightTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRawBlockByHashTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRawBlockByHeightTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getBlockCountTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getBlocksByHeightTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getLastBlockHeaderTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        feeTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        heightTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getGlobalIndexesTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        infoTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        peersTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getTransactionDetailsByHashTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRawTransactionByHashTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getTransactionsInPoolTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRawTransactionsInPoolTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        info(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        fee(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        height(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        peers(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    ///////////////////
    /* POST REQUESTS */
    ///////////////////

    std::tuple<Error, uint16_t>
        submitBlockTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getBlockTemplateTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRandomOutsTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getWalletSyncDataTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRawBlocksTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        sendTransactionTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getPoolChangesTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getTransactionsStatusTrtlApi(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        sendTransaction(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRandomOuts(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getWalletSyncData(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getGlobalIndexes(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        queryBlocksLite(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getTransactionsStatus(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getPoolChanges(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        queryBlocksDetailed(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    /* Deprecated. Use getGlobalIndexes instead. */
    std::tuple<Error, uint16_t> getGlobalIndexesDeprecated(
        const httplib::Request &req,
        httplib::Response &res,
        const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getRawBlocks(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    ///////////////////////
    /* JSON RPC REQUESTS */
    ///////////////////////

    std::tuple<Error, uint16_t>
        getBlockTemplateJsonRpc(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        submitBlockJsonRpc(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getBlockCountJsonRpc(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t> getBlockHashForHeightJsonRpc(
        const httplib::Request &req,
        httplib::Response &res,
        const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getLastBlockHeaderJsonRpc(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t> getBlockHeaderByHashJsonRpc(
        const httplib::Request &req,
        httplib::Response &res,
        const rapidjson::Document &body);

    std::tuple<Error, uint16_t> getBlockHeaderByHeightJsonRpc(
        const httplib::Request &req,
        httplib::Response &res,
        const rapidjson::Document &body);

    std::tuple<Error, uint16_t>
        getBlocksByHeightJsonRpc(const httplib::Request &req, httplib::Response &res, const rapidjson::Document &body);

    std::tuple<Error, uint16_t> getBlockDetailsByHashJsonRpc(
        const httplib::Request &req,
        httplib::Response &res,
        const rapidjson::Document &body);

    std::tuple<Error, uint16_t> getTransactionDetailsByHashJsonRpc(
        const httplib::Request &req,
        httplib::Response &res,
        const rapidjson::Document &body);

    std::tuple<Error, uint16_t> getTransactionsInPoolJsonRpc(
        const httplib::Request &req,
        httplib::Response &res,
        const rapidjson::Document &body);

    //////////////////////////////
    /* Private member variables */
    //////////////////////////////

    /* Our server instance */
    httplib::Server m_server;

    /* The server host */
    const std::string m_host;

    /* The server port */
    const uint16_t m_port;

    /* The header to use with 'Access-Control-Allow-Origin'. If empty string,
     * header is not added. */
    const std::string m_corsHeader;

    /* The thread running the server */
    std::thread m_serverThread;

    /* The address to return from the /fee endpoint */
    const std::string m_feeAddress;

    /* The amount to return from the /fee endpoint */
    const uint64_t m_feeAmount;

    /* RPC methods that are enabled */
    const RpcMode m_rpcMode;

    /* A pointer to our CryptoNoteCore instance */
    const std::shared_ptr<CryptoNote::Core> m_core;

    /* A pointer to our P2P stack */
    const std::shared_ptr<CryptoNote::NodeServer> m_p2p;

    const std::shared_ptr<CryptoNote::ICryptoNoteProtocolHandler> m_syncManager;

    /* Use turtle api instead of xmr variant */
    const bool m_useTrtlApi;
};
