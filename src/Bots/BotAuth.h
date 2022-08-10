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

#include "BotMgr.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/any_io_executor.hpp>

#include <cstdint>
#include <string>

#pragma pack(push,1)
enum class CommandDetail : uint8_t
{
    AUTH_OK = 12,
    AUTH_FAILED = 13,
    AUTH_REJECT = 14,
    AUTH_BAD_SERVER_PROOF = 15,
    AUTH_UNAVAILABLE = 16,
    AUTH_SYSTEM_ERROR = 17,
    AUTH_BILLING_ERROR = 18,
    AUTH_BILLING_EXPIRED = 19,
    AUTH_VERSION_MISMATCH = 20,
    AUTH_UNKNOWN_ACCOUNT = 21,
    AUTH_INCORRECT_PASSWORD = 22,
    AUTH_SESSION_EXPIRED = 23,
    AUTH_SERVER_SHUTTING_DOWN = 24,
    AUTH_ALREADY_LOGGING_IN = 25,
    AUTH_LOGIN_SERVER_NOT_FOUND = 26,
    AUTH_WAIT_QUEUE = 27,
    AUTH_BANNED = 28,
    AUTH_ALREADY_ONLINE = 29,
    AUTH_NO_TIME = 30,
    AUTH_DB_BUSY = 31,
    AUTH_SUSPENDED = 32,
    AUTH_PARENTAL_CONTROL = 33,
    AUTH_LOCKED_ENFORCED = 34,

    CHAR_CREATE_IN_PROGRESS = 46,
    CHAR_CREATE_SUCCESS = 47,
    CHAR_CREATE_ERROR = 48,
    CHAR_CREATE_FAILED = 49,
    CHAR_CREATE_NAME_IN_USE = 50,
    CHAR_CREATE_DISABLED = 51,
    CHAR_CREATE_PVP_TEAMS_VIOLATION = 52,
    CHAR_CREATE_SERVER_LIMIT = 53,
    CHAR_CREATE_ACCOUNT_LIMIT = 54,
    CHAR_CREATE_SERVER_QUEUE = 55,
    CHAR_CREATE_ONLY_EXISTING = 56,
    CHAR_CREATE_EXPANSION = 57,
    CHAR_CREATE_EXPANSION_CLASS = 58,
    CHAR_CREATE_LEVEL_REQUIREMENT = 59,
    CHAR_CREATE_UNIQUE_CLASS_LIMIT = 60,
    CHAR_CREATE_CHARACTER_IN_GUILD = 61,
    CHAR_CREATE_RESTRICTED_RACECLASS = 62,
    CHAR_CREATE_CHARACTER_CHOOSE_RACE = 63,
    CHAR_CREATE_CHARACTER_ARENA_LEADER = 64,
    CHAR_CREATE_CHARACTER_DELETE_MAIL = 65,
    CHAR_CREATE_CHARACTER_SWAP_FACTION = 66,
    CHAR_CREATE_CHARACTER_RACE_ONLY = 67,
    CHAR_CREATE_CHARACTER_GOLD_LIMIT = 68,
    CHAR_CREATE_FORCE_LOGIN = 69,

    CHAR_NAME_SUCCESS = 87,
    CHAR_NAME_FAILURE = 88,
    CHAR_NAME_NO_NAME = 89,
    CHAR_NAME_TOO_SHORT = 90,
    CHAR_NAME_TOO_LONG = 91,
    CHAR_NAME_INVALID_CHARACTER = 92,
    CHAR_NAME_MIXED_LANGUAGES = 93,
    CHAR_NAME_PROFANE = 94,
    CHAR_NAME_RESERVED = 95,
    CHAR_NAME_INVALID_APOSTROPHE = 96,
    CHAR_NAME_MULTIPLE_APOSTROPHES = 97,
    CHAR_NAME_THREE_CONSECUTIVE = 98,
    CHAR_NAME_INVALID_SPACE = 99,
    CHAR_NAME_CONSECUTIVE_SPACES = 100,
    CHAR_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS = 101,
    CHAR_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END = 102,
    CHAR_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME = 103
};

struct ServerAuthChallenge
{
    std::array<uint8_t, 32> m_B;
    uint8_t m_gLen;
    uint8_t m_g;
    uint8_t m_nLen;
    std::array<uint8_t, 32> m_N;
    std::array<uint8_t, 32> m_salt;
    std::array<uint8_t, 16> m_unk3;
    uint8_t m_securityFlags;
};

struct WorldAuthResponse
{
    CommandDetail m_detail;
    uint32_t billingTimeRemaining;
    uint8_t billingFlags;
    uint32_t billingTimeRested;
    uint8_t expansion;
};
#pragma pack(pop)
