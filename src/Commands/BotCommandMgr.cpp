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
#include "BotCommandMgr.h"
#include "cxxopts.hpp"
#include "BotCommands.h"
#include "BotLuaCommand.h"
#include "Config.h"

BotCommand::BotCommand(std::string const& name)
    : m_name(name)
{}

BotCommandParameter::BotCommandParameter(std::string const& name, BotCommandParamType type)
    : m_name(name)
    , m_type(type)
    , m_has_def(false)
    , m_def("")
{}

BotCommandParameter::BotCommandParameter(std::string const& name, std::string const& def)
    : m_name(name)
    , m_type(BotCommandParamType::BOOLEAN)
    , m_has_def(true)
    , m_def(def)
{}

BotCommandParameter::BotCommandParameter(std::string const& name, double def)
    : m_name(name)
    , m_type(BotCommandParamType::NUMBER)
    , m_has_def(true)
    , m_def(std::to_string(def))
{}

BotCommandParameter::BotCommandParameter(std::string const& name, bool def)
    : m_name(name)
    , m_type(BotCommandParamType::STRING)
    , m_has_def(true)
    , m_def(def ? "true" : "false")
{}

BotCommandParameter::BotCommandParameter(Positional positional)
    : m_name("")
    , m_type(BotCommandParamType::POSITIONAL)
    , m_has_def(false)
    , m_def("")
{}

BotCommandBuilder::BotCommandBuilder(BotCommand* command)
    : m_command(command)
{}

BotCommandBuilder& BotCommandBuilder::AddStringParam(std::string const& name, std::string const& def)
{
    m_command->m_params.push_back(BotCommandParameter(name, def));
    return *this;
}

BotCommandBuilder& BotCommandBuilder::AddStringParam(std::string const& name)
{
    m_command->m_params.push_back(BotCommandParameter(name, BotCommandParamType::STRING));
    return *this;
}
BotCommandBuilder& BotCommandBuilder::AddNumberParam(std::string const& name, double def)
{
    m_command->m_params.push_back(BotCommandParameter(name, def));
    return *this;
}
BotCommandBuilder& BotCommandBuilder::AddNumberParam(std::string const& name)
{
    m_command->m_params.push_back(BotCommandParameter(name, BotCommandParamType::NUMBER));
    return *this;
}

BotCommandBuilder& BotCommandBuilder::AddBoolParam(std::string const& name, bool def)
{
    m_command->m_params.push_back(BotCommandParameter(name, def));
    return *this;
}

BotCommandBuilder& BotCommandBuilder::AddBoolParam(std::string const& name)
{
    m_command->m_params.push_back(BotCommandParameter(name, BotCommandParamType::BOOLEAN));
    return *this;
}

BotCommandBuilder& BotCommandBuilder::SetDescription(std::string const& description)
{
    m_command->m_description = description;
    return *this;
}

void BotCommandBuilder::SetCallback(std::function<void(BotCommandArguments)> callback)
{
    m_command->m_callback = callback;
}

double BotCommandArguments::get_number(std::string const& name) const
{
    return std::stod(m_args.find(name)->second);
}

std::string BotCommandArguments::get_string(std::string const& name) const
{
    return m_args.find(name)->second;
}

bool BotCommandArguments::get_bool(std::string const& name) const
{
    std::string str = m_args.find(name)->second;
    return (str == "false" || str == "0") ? false : true;
}

static void pushString(std::vector<char*>& vec, std::string & value)
{
    if (value.size() > 0)
    {
        char* c = new char[value.size() + 1];
        memcpy(c, value.data(), value.size() + 1);
        vec.push_back(c);
    }
    value = "";
}

void BotCommand::fire(std::string const& input)
{
    BotCommandArguments args;
    int curArg = 0;
    cxxopts::Options options("", "");

    std::vector<std::string> positionals;
    bool passedPositional = false;
    for (BotCommandParameter const& param : m_params)
    {
        switch (param.m_type)
        {
        case BotCommandParamType::POSITIONAL:
            passedPositional = true;
            continue;
        case BotCommandParamType::BOOLEAN:
            if(param.m_has_def)
                options.add_options()(param.m_name,"", cxxopts::value<bool>());
            else
                options.add_options()(param.m_name,"", cxxopts::value<bool>()->default_value(param.m_def));
            break;
        case BotCommandParamType::NUMBER:
            if(param.m_has_def)
                options.add_options()(param.m_name,"", cxxopts::value<double>());
            else
                options.add_options()(param.m_name,"", cxxopts::value<double>()->default_value(param.m_def));
            break;
        case BotCommandParamType::STRING:
            if(param.m_has_def)
                options.add_options()(param.m_name,"", cxxopts::value<std::string>());
            else
                options.add_options()(param.m_name,"", cxxopts::value<std::string>()->default_value(param.m_def));
            break;
        }
        if (!passedPositional)
            positionals.push_back(param.m_name);
    }
    if (positionals.size() > 0)
    {
        options.parse_positional(positionals);
    }

    std::vector<char*> splitInput;
    std::string cur = "";
    bool inStr = false;
    for(int i=0;i<input.size();++i)
    {
        if (input[i] == '"')
        {
            inStr = !inStr;
            pushString(splitInput, cur);
        }
        else if (!inStr && (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] == '\n'))
        {
            pushString(splitInput, cur);
            continue;
        }
        else
        {
            cur.push_back(input[i]);
        }
    }

    if (inStr)
    {
        throw std::runtime_error("Uneven quotes");
    }
    pushString(splitInput, cur);
    cxxopts::ParseResult optRes = options.parse(splitInput.size(), splitInput.data());
    for (auto const& arg : optRes.arguments())
    {
        args.m_args[arg.key()] = arg.value();
    }

    for (auto const& param : m_params)
    {
        auto itr = args.m_args.find(param.m_name);
        if (itr == args.m_args.end())
        {
            if (param.m_has_def)
            {
                args.m_args[param.m_name] = param.m_def;
            }
            else
            {
                throw std::runtime_error("Missing required argument " + param.m_name);
            }
        }
    }

    m_callback(args);
    for (char* str : splitInput)
        delete str;
}

BotCommandBuilder BotCommandMgr::CreateCommand(std::string const& name)
{
    auto h = (m_commands[name] = std::unique_ptr<BotCommand>( new BotCommand(name) )).get();
    return BotCommandBuilder(h);
}

void BotCommandMgr::Fire(std::string const& input)
{
    BotCommand* cur_command = nullptr;
    for (auto & [name,value] : m_commands)
    {
        if (input.starts_with(name) && (!cur_command || name.size() > cur_command->m_name.size()))
        {
            cur_command = value.get();
        }
    }
    if (cur_command)
    {
        cur_command->fire(input);
    }
}

BotCommandMgr* BotCommandMgr::instance()
{
    static BotCommandMgr mgr;
    return &mgr;
}

void BotCommandMgr::Reload()
{
    m_commands.clear();
    RegisterBaseCommands();
    if (sConfigMgr->GetBoolDefault("Lua.Enabled", true))
    {
        m_lua.Reload();
    }
}

BotCommandMgr::~BotCommandMgr()
{
    // ensure no lua callbacks before we destroy lua state
    m_commands.clear();
}
