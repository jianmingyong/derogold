// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "LoggerRef.h"

#include <utility>

namespace Logging
{
    LoggerRef::LoggerRef(std::shared_ptr<ILogger> logger, std::string category):
        logger(std::move(logger)),
        category(std::move(category))
    {
    }

    LoggerMessage LoggerRef::operator()(Level level, const std::string &color) const
    {
        return {logger, category, level, color};
    }

    std::shared_ptr<ILogger> LoggerRef::getLogger() const
    {
        return logger;
    }

} // namespace Logging
