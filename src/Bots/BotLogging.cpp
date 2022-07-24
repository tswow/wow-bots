/*
 * This file is part of the wotlk-bots project <https://github.com/tswow/wotlk-bots>.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "BotLogging.h"

#include "Config.h"

#include <sol/sol.hpp>

#include <stdarg.h>
#include <mutex>
#include <iostream>

std::mutex log_mutex;

static bool should_log(LogLevel level, std::string const& category)
{
    uint8_t blevel = static_cast<uint8_t>(level);
    int logLevel = sConfigMgr->GetIntDefault("Logger." + category, -1);
    logLevel = logLevel < 0 ? sConfigMgr->GetIntDefault("Logger.*", 1) : logLevel;
    return ((logLevel > 0 && logLevel <= blevel));
}

static void log_header(LogLevel level, std::string const& category)
{
    switch (level)
    {
    case LogLevel::LOG_TRACE: std::cout << "[TRACE]"; break;
    case LogLevel::LOG_DEBUG: std::cout << "[DEBUG]"; break;
    case LogLevel::LOG_INFO: std::cout << "[INFO]"; break;
    case LogLevel::LOG_WARN: std::cout << "[WARN]"; break;
    case LogLevel::LOG_ERROR: std::cout << "[ERROR]"; break;
    }
    std::cout << "[" << category << "]: ";
}

void BotLog(LogLevel level, std::string const& category, char const* fmt, ...)
{
    if (!should_log(level,category))
    {
        return;
    }
    std::scoped_lock lock(log_mutex);
    log_header(level, category);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

void LBotPrint(sol::variadic_args args)
{
    LogLevel level = LogLevel::LOG_INFO;
    std::string category = "print";
    if (!should_log(level, category))
    {
        return;
    }

    std::scoped_lock lock(log_mutex);
    log_header(level, category);

    std::string message = "";
    for (sol::stack_proxy value : args)
    {
        switch (value.get_type())
        {
        case sol::type::boolean:
            message += std::to_string(value.as<bool>()) + " ";
            break;
        case sol::type::number:
        {
            double db = value.as<double>();
            if (std::floor(db) == db)
            {
                int64_t i = int64_t(db);
                message += std::to_string(i) + " ";
            }
            else
            {
                message += std::to_string(db) + " ";
            }
            break;
        }
        case sol::type::string:
            message += value.as<std::string>() + " ";
            break;
        case sol::type::table:
            // todo: hex
            message += "Table at " + std::to_string(uint64_t(value.as<sol::table>().pointer())) + " ";
            break;
        case sol::type::function:
            message += "Table at " + std::to_string(uint64_t(value.as<sol::function>().pointer())) + " ";
            break;
        case sol::type::lightuserdata:
            message += "Light userdata at " + std::to_string(uint64_t(value.as<sol::lightuserdata>().pointer())) + " ";
            break;
        case sol::type::lua_nil:
            message += "nil ";
            break;
        case sol::type::userdata:
            message += "Userdata at " + std::to_string(uint64_t(value.as<sol::userdata>().pointer())) + " ";
            break;
        default:
            message += "(unhandled value) ";
        }
    }
    std::cout << message << "\n";
}

void LBotLog(LogLevel level, std::string const& category, std::string message, sol::variadic_args args)
{
    if (!should_log(level, category))
    {
        return;
    }
    std::scoped_lock lock(log_mutex);
    log_header(level, category);

    bool isCmd = false;
    int argc = 0;
    for (int i = 0; i < message.size(); ++i)
    {
        if (message[i] == '%')
        {
            isCmd = true;
        }
        else if (isCmd)
        {
            switch (message[i])
            {
            case 'c':
            case 's':
                std::cout << args[argc++].as<std::string>();
                break;
            case 'u':
                std::cout << args[argc++].as<unsigned int>();
                break;
            case 'i':
                std::cout << args[argc++].as<int>();
                break;
            case 'd':
                std::cout << args[argc++].as<double>();
                break;
            }
            isCmd = false;
        }
        else
        {
            std::cout << message[i];
        }
    }
    std::cout << "\n";
}
