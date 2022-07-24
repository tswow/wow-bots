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
#include <functional>
#include <map>
#include <optional>

#include "BotLuaCommand.h"

enum class BotCommandParamType
{
    NUMBER,
    STRING,
    BOOLEAN,
    POSITIONAL
};

enum class Positional
{
    POSITIONAL
};

struct BotCommandParameter
{
    BotCommandParameter(std::string const& name, BotCommandParamType type);
    BotCommandParameter(std::string const& name, std::string const& def);
    BotCommandParameter(std::string const& name, double def);
    BotCommandParameter(std::string const& name, bool def);
    BotCommandParameter(Positional positional);

    bool m_has_def;
    BotCommandParamType m_type;
    std::string m_name;
    std::string m_def;
};

#define NAMED_ARGUMENTS BotCommandParameter(Positional::POSITIONAL)

class BotCommandArguments
{
    std::map<std::string, std::string> m_args;
public:
    double get_number(std::string const& name) const;
    std::string get_string(std::string const& name) const;
    bool get_bool(std::string const& name) const;
    friend class BotCommand;
};

class BotCommand
{
    std::string m_name;
    std::string m_description = "";
    std::vector<BotCommandParameter> m_params;
    std::function<void(BotCommandArguments)> m_callback;
public:
    BotCommand(std::string const& name);
    BotCommand() = default;
    void fire(std::string const& input);
    friend class BotCommandMgr;
    friend class BotCommandBuilder;
};

class BotCommandBuilder
{
    BotCommand* m_command;
public:
    BotCommandBuilder(BotCommand* command);
    BotCommandBuilder& SetDescription(std::string const& description);
    BotCommandBuilder& AddStringParam(std::string const& name, std::string const& def);
    BotCommandBuilder& AddStringParam(std::string const& name);
    BotCommandBuilder& AddNumberParam(std::string const& name, double def);
    BotCommandBuilder& AddNumberParam(std::string const& name);
    BotCommandBuilder& AddBoolParam(std::string const& name, bool def);
    BotCommandBuilder& AddBoolParam(std::string const& name);
    void SetCallback(std::function<void(BotCommandArguments)> callback);
};

class BotCommandMgr
{
public:
    static BotCommandMgr* instance();
    BotCommandBuilder CreateCommand(std::string const& m_name);
    void Reload();
    void RegisterBaseCommands();
    void Fire(std::string const& input);
    ~BotCommandMgr();
private:
    BotLuaCommand m_lua;
    std::map<std::string, std::unique_ptr<BotCommand>> m_commands;
};

#define sBotCommandMgr BotCommandMgr::instance()