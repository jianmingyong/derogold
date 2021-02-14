// Copyright (c) 2018-2021, The DeroGold Developers
// Copyright (c) 2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <string>

namespace Utilities
{
    bool confirm(const std::string &msg);

    bool confirm(const std::string &msg, const bool defaultReturn);
} // namespace Utilities
