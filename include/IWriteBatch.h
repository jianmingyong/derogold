// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CryptoNote
{
    class IWriteBatch
    {
    public:
        virtual ~IWriteBatch() = default;

        virtual std::vector<std::pair<std::string, std::string>> extractRawDataToInsert() = 0;
        virtual std::vector<std::string> extractRawKeysToRemove() = 0;

        virtual std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>
        extractRawDataToInsertWithCF() { return {}; }
        virtual std::unordered_map<std::string, std::vector<std::string>> extractRawKeysToRemoveWithCF() { return {}; }
    };

} // namespace CryptoNote
