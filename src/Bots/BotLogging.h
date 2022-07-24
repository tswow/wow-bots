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
#pragma once

#include <string>

enum class LogLevel : uint8_t
{
    LOG_TRACE = 1,
    LOG_DEBUG = 2,
    LOG_INFO  = 3,
    LOG_WARN  = 4,
    LOG_ERROR = 5
};

void BotLog(LogLevel level, std::string const& category, char const* fmt, ...);

namespace sol
{
    struct variadic_args;
}
void LBotLog(LogLevel level, std::string const& category, std::string message, sol::variadic_args args);
void LBotPrint(sol::variadic_args args);

#define BOT_LOG_TRACE(category,message,...) BotLog(LogLevel::LOG_TRACE,category,message,__VA_ARGS__)
#define BOT_LOG_DEBUG(category,message,...) BotLog(LogLevel::LOG_DEBUG,category,message,__VA_ARGS__)
#define BOT_LOG_INFO(category,message,...)  BotLog(LogLevel::LOG_INFO,category,message,__VA_ARGS__)
#define BOT_LOG_WARN(category,message,...)  BotLog(LogLevel::LOG_WARN,category,message,__VA_ARGS__)
#define BOT_LOG_ERROR(category,message,...) BotLog(LogLevel::LOG_ERROR,category,message,__VA_ARGS__)
