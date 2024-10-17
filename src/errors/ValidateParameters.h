// Copyright (c) 2018-2021, The DeroGold Developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <errors/Errors.h>
#include <memory>
#include <string>
#include <subwallets/SubWallets.h>
#include <vector>

Error validateFusionTransaction(uint64_t mixin,
                                std::vector<std::string> subWalletsToTakeFrom,
                                std::string destinationAddress,
                                std::shared_ptr<SubWallets> subWallets,
                                uint64_t currentHeight);

Error validateTransaction(std::vector<std::pair<std::string, uint64_t>> destinations,
                          uint64_t mixin,
                          uint64_t fee,
                          std::string paymentID,
                          std::vector<std::string> subWalletsToTakeFrom,
                          std::string changeAddress,
                          std::shared_ptr<SubWallets> subWallets,
                          uint64_t unlockTime,
                          uint64_t currentHeight);

Error validateIntegratedAddresses(std::vector<std::pair<std::string, uint64_t>> destinations,
    std::string paymentID);

Error validatePaymentID(std::string paymentID);

Error validateHash(std::string hash);

Error validatePrivateKey(const Crypto::SecretKey &privateViewKey);

Error validatePublicKey(const Crypto::PublicKey &publicKey);

Error validateMixin(uint64_t mixin, uint64_t height);

Error validateAmount(std::vector<std::pair<std::string, uint64_t>> destinations,
                     uint64_t fee,
                     std::vector<std::string> subWalletsToTakeFrom,
                     std::shared_ptr<SubWallets> subWallets,
                     uint64_t currentHeight);

Error validateDestinations(std::vector<std::pair<std::string, uint64_t>> destinations);

Error validateAddresses(std::vector<std::string> addresses, bool integratedAddressesAllowed);

Error validateOurAddresses(std::vector<std::string> addresses, std::shared_ptr<SubWallets> subWallets);
