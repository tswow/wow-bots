#include "BotAccounts.h"

#include "Config.h"
#include "BotLogging.h"
#include <nlohmann/json.hpp>

#include <optional>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

BotAccount::BotAccount(std::string const& username, std::string const& password)
    : m_username(username)
    , m_password(password)
{}

std::string BotAccount::GetPassword()
{
    return m_password;
}
std::string BotAccount::GetUsername()
{
    return m_username;
}

static std::optional<nlohmann::json> json;
void ReloadAccounts()
{
    fs::path accountsJson = sConfigMgr->GetStringDefault("Accounts.Path","./accounts.json");
    json.reset();
    if (fs::exists(accountsJson))
    {
        json.emplace(nlohmann::json::parse(std::ifstream(accountsJson)));
    }
    else
    {
        BOT_LOG_WARN("Accounts","No accounts.json at %s", accountsJson.string().c_str());
    }
}

BotAccount GetBotAccount(uint32_t bot)
{
    if (!json.has_value())
    {
        throw std::runtime_error("Unable to load bots: No accounts.json");
    }
    auto itr = json.value().find(std::to_string(bot));
    if (itr == json.value().end())
    {
        throw std::runtime_error("Could not find bot " + std::to_string(bot));
    }
    auto obj = *itr;
    std::string username = obj["username"];
    std::string password = obj["password"];
    return { username,password };
}

std::vector<BotAccount> GetBotAccounts(std::vector<uint32_t> bot)
{
    std::vector<BotAccount> accounts;
    accounts.reserve(bot.size());
    for (uint32_t id : bot)
    {
        accounts.push_back(GetBotAccount(id));
    }
    return accounts;
}
