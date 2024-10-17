// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <boost/serialization/map.hpp>
#include <cryptonotecore/BlockchainCache.h>

namespace CryptoNote
{
    struct KeyOutputInfo
    {
        Crypto::PublicKey publicKey;
        Crypto::Hash transactionHash;
        uint64_t unlockTime;
        uint16_t outputIndex;

        void serialize(CryptoNote::ISerializer &s);

        template<class Archive> void serialize(Archive &ar, const unsigned int version)
        {
            // clang-format off
            ar & BOOST_NVP(publicKey);
            ar & BOOST_NVP(transactionHash);
            ar & BOOST_NVP(unlockTime);
            ar & BOOST_NVP(outputIndex);
            // clang-format on
        }
    };

    // inherit here to avoid breaking IBlockchainCache interface
    struct ExtendedTransactionInfo : CachedTransactionInfo
    {
        // CachedTransactionInfo tx;
        std::map<IBlockchainCache::Amount, std::vector<IBlockchainCache::GlobalOutputIndex>>
            amountToKeyIndexes; // global key output indexes spawned in this transaction
        void serialize(ISerializer &s);

        template<class Archive> void serialize(Archive &ar, const unsigned int version)
        {
            // clang-format off
            ar & boost::serialization::base_object<CachedTransactionInfo>(*this);
            ar & BOOST_NVP(amountToKeyIndexes);
            // clang-format on
        }
    };

} // namespace CryptoNote
