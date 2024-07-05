// Copyright (c) 2018-2024, The DeroGold Developers
// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <string>
#include <system_error>

namespace CryptoNote::error
{
    enum class DataBaseErrorCodes : int
    {
        NOT_INITIALIZED = 1,
        ALREADY_INITIALIZED,
        INTERNAL_ERROR,
        IO_ERROR
    };

    class DataBaseErrorCategory final : public std::error_category
    {
    public:
        static DataBaseErrorCategory INSTANCE;

        [[nodiscard]] const char *name() const noexcept override
        {
            return "DataBaseErrorCategory";
        }

        [[nodiscard]] std::error_condition default_error_condition(const int ev) const noexcept override
        {
            return std::error_condition {ev, *this};
        }

        [[nodiscard]] std::string message(const int ev) const override
        {
            switch (ev)
            {
                case static_cast<int>(DataBaseErrorCodes::NOT_INITIALIZED): return "Object was not initialized";
                case static_cast<int>(DataBaseErrorCodes::ALREADY_INITIALIZED):
                    return "Object has been already initialized";
                case static_cast<int>(DataBaseErrorCodes::INTERNAL_ERROR): return "Internal error";
                case static_cast<int>(DataBaseErrorCodes::IO_ERROR): return "IO error";
                default: return "Unknown error";
            }
        }

    private:
        DataBaseErrorCategory() {}
    };
} // namespace CryptoNote::error

inline std::error_code make_error_code(CryptoNote::error::DataBaseErrorCodes e)
{
    return std::error_code {static_cast<int>(e), CryptoNote::error::DataBaseErrorCategory::INSTANCE};
}
