// Copyright (c) 2018-2021, The DeroGold Developers
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cstdint>

namespace CryptoNote
{
    class ICoreInformation
    {
      public:
        virtual ~ICoreInformation() = default;

        virtual size_t getPoolTransactionCount() const = 0;

        virtual size_t getBlockchainTransactionCount() const = 0;

        virtual size_t getAlternativeBlockCount() const = 0;

        virtual std::vector<Transaction> getPoolTransactions() const = 0;
    };

} // namespace CryptoNote
