// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright 2014-2018, The Monero Developers
// Copyright 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "CryptoNote.h"
#include "errors/Errors.h"

#include <tuple>
#include <vector>

namespace Mnemonics
{
    std::tuple<Error, Crypto::SecretKey> MnemonicToPrivateKey(std::string words);

    std::tuple<Error, Crypto::SecretKey> MnemonicToPrivateKey(std::vector<std::string> words);

    std::string PrivateKeyToMnemonic(Crypto::SecretKey privateKey);

    bool HasValidChecksum(std::vector<std::string> words);

    std::string GetChecksumWord(std::vector<std::string> words);

    std::vector<int> GetWordIndexes(std::vector<std::string> words);
} // namespace Mnemonics
