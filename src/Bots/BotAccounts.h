#pragma once

#include <string>
#include <vector>

class BotAccount
{
public:
    BotAccount(std::string const& username, std::string const& password);
    std::string GetPassword();
    std::string GetUsername();
private:
    std::string m_password;
    std::string m_username;
};

void ReloadAccounts();
BotAccount GetBotAccount(uint32_t bot);
std::vector<BotAccount> GetBotAccounts(std::vector<uint32_t> bot);
