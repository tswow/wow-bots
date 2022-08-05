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
#include "BotAuth.h"
#include "Bot.h"
#include "BotPacket.h"
#include "BotMutable.h"
#include "BotProfile.h"
#include "BotLogging.h"

#include "BigNumber.h"
#include "CryptoHash.h"
#include "CryptoRandom.h"

#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

enum class AuthResult : uint8_t
{
    SUCCESS = 0,
    FAILURE = 0x01,
    UNKNOWN1 = 0x02,
    ACCOUNT_BANNED = 0x03,
    NO_MATCH = 0x04,
    UNKNOWN2 = 0x05,
    ACCOUNT_IN_USE = 0x06,
    PREPAID_TIME_LIMIT = 0x07,
    SERVER_FULL = 0x08,
    WRONG_BUILD_NUMBER = 0x09,
    UPDATE_CLIENT = 0x0a,
    UNKNOWN3 = 0x0b,
    ACCOUNT_FREEZED = 0x0c,
    UNKNOWN4 = 0x0d,
    UNKNOWN5 = 0x0e,
    PARENTAL_CONTROL = 0x0f
};

std::string AuthResultString(AuthResult result)
{
    switch (result)
    {
    case AuthResult::SUCCESS: return "SUCCESS";
    case AuthResult::FAILURE: return "FAILURE";
    case AuthResult::UNKNOWN1: return "UNKNOWN1";
    case AuthResult::ACCOUNT_BANNED: return "ACCOUNT_BANNED";
    case AuthResult::NO_MATCH: return "NO_MATCH";
    case AuthResult::UNKNOWN2: return "UNKNOWN2";
    case AuthResult::ACCOUNT_IN_USE: return "ACCOUNT_IN_USE";
    case AuthResult::PREPAID_TIME_LIMIT: return "PREPAID_TIME_LIMIT";
    case AuthResult::SERVER_FULL: return "SERVER_FULL";
    case AuthResult::WRONG_BUILD_NUMBER: return "WRONG_BUILD_NUMBER";
    case AuthResult::UPDATE_CLIENT: return "UPDATE_CLIENT";
    case AuthResult::UNKNOWN3: return "UNKNOWN3";
    case AuthResult::ACCOUNT_FREEZED: return "ACCOUNT_FREEZED";
    case AuthResult::UNKNOWN4: return "UNKNOWN4";
    case AuthResult::UNKNOWN5: return "UNKNOWN5";
    case AuthResult::PARENTAL_CONTROL: return "PARENTAL_CONTROL";
    default:
        return "INVALID_AUTH_RESULT";
    }
}

enum class AuthCommand : uint8_t
{
    LOGON_CHALLENGE   = 0x00,
    LOGON_PROOF       = 0x01,
    REALM_LIST        = 0x10,
    TRANSFER_INITIATE = 0x30,
    TRANSFER_DATA     = 0x31,
    TRANSFER_ACCEPT   = 0x32,
    TRANSFER_RESUME   = 0x33,
    TRANSFER_CANCEL   = 0x34
};

std::string AuthCommandString(AuthCommand result)
{
    switch (result)
    {
    case AuthCommand::LOGON_CHALLENGE: return "LOGON_CHALLENGE";
    case AuthCommand::LOGON_PROOF: return "LOGON_PROOF";
    case AuthCommand::REALM_LIST: return "REALM_LIST";
    case AuthCommand::TRANSFER_INITIATE: return "TRANSFER_INITIATE";
    case AuthCommand::TRANSFER_DATA: return "TRANSFER_DATA";
    case AuthCommand::TRANSFER_ACCEPT: return "TRANSFER_ACCEPT";
    case AuthCommand::TRANSFER_RESUME: return "TRANSFER_RESUME";
    case AuthCommand::TRANSFER_CANCEL: return "TRANSFER_CANCEL";
    default:
        return "INVALID_AUTH_COMMAND";
    }
}

#pragma pack(push,1)
struct ClientAuthChallenge
{
    ClientAuthChallenge(std::string const& username, uint32_t ip)
        : m_packetSize(30 + username.size())
        , m_ip(ip)
        , m_nameLen(username.size())
    {}
    AuthCommand m_command = AuthCommand::LOGON_CHALLENGE;
    uint8_t m_commandValue = 6;
    uint16_t m_packetSize;
    uint8_t m_service[4] = { 'W','o','W', '\0' };
    uint8_t m_version[3] = { '3','3','5' };
    uint16_t m_build = 12340;
    uint8_t m_arch[4] = { '6','8','x', '\0' };
    uint8_t m_os[4] = { 'n','i','W', '\0' };
    uint8_t m_loc[4] = { 'S','U','n','e' };
    uint32_t unk = 0x3c;
    uint32_t m_ip;
    uint8_t m_nameLen;
};

struct ServerAuthProof
{
    std::array<uint8_t, 20> M2;
    uint32_t unk1;
    uint32_t unk2;
    uint16_t unk3;
};

struct ClientAuthProof
{
    std::vector<uint8_t> packet;
    std::vector<uint8_t> m2;
    BigNumber key;
};

struct ClientRequestRealmlist
{
    AuthCommand command = AuthCommand::REALM_LIST;
    std::array<uint8_t,4> unk;
};

struct ServerRealmlistHeader
{
    std::array<uint8_t, 7> unk;
    uint16_t size;
};
#pragma pack(pop)

static void AssertAuthCommand(AuthCommand expected, AuthCommand com, AuthResult res)
{
    if (res != AuthResult::SUCCESS)
    {
        throw std::runtime_error("Auth failure: " + AuthResultString(res) + " (" + std::to_string(uint32_t(res)) + ")");
    }

    if (expected != com)
    {
        std::string exprs = std::string(AuthCommandString(expected));
        std::string coms = std::string(AuthCommandString(com));
        throw std::runtime_error("Invalid auth command: Expected " + exprs + " but got " + coms + "(" + std::to_string(uint32_t(com)) + ")");
    }
}

static boost::asio::awaitable<void> AssertAuthCommand(AuthCommand expected, BotSocket& socket)
{
    AuthCommand com = co_await socket.Read<AuthCommand>();
    if (com == AuthCommand::LOGON_CHALLENGE)
    {
        co_await socket.Read<uint8_t>();
    }
    AuthResult res = co_await socket.Read<AuthResult>();
    AssertAuthCommand(expected, com, res);
}

template <int64_t I>
static BigNumber CreateBigNumber(uint8_t* arr)
{
    BigNumber n;
    n.SetBinary(arr, I, true);
    return n;
}

static BigNumber CreateBigNumber(std::vector<uint8_t> const& arr)
{
    BigNumber n;
    n.SetBinary(arr.data(), arr.size(), true);
    return n;
}

template <int64_t I>
static BigNumber CreateBigNumber(std::vector<uint8_t> const& arr)
{
    BigNumber n;
    n.SetBinary(arr.data(), I, true);
    return n;
}

static BigNumber CreateBigNumber(uint32_t arr)
{
    return BigNumber(arr);
}

template <int64_t I>
static BigNumber CreateBigNumber(std::array<uint8_t, I> arr)
{
    BigNumber n;
    n.SetBinary(arr.data(), I, true);
    return n;
}

template <typename F>
std::vector<uint8_t> MergeVec(F first) {
    std::vector<uint8_t> vec(sizeof(F));
    memcpy(vec.data(), &first, sizeof(F));
    return vec;
}

std::vector<uint8_t> MergeVec(std::string const& str)
{
    std::vector<uint8_t> vec(str.size());
    memcpy(vec.data(), str.data(), str.size());
    return vec;
}

std::vector<uint8_t> MergeVec(std::vector<uint8_t> const& vec)
{
    return vec;
}

std::vector<uint8_t> MergeVec(BigNumber first)
{
    std::vector<uint8_t> vec(first.GetNumBytes());
    first.GetBytes(vec.data(), first.GetNumBytes());

    // BigNumber vectors should be cleaned
    if (vec[vec.size() - 1] == 0)
    {
        vec.resize(vec.size() - 1);
    }
    return vec;
}

std::vector<uint8_t> MergeVec(uint8_t value)
{
    return { value };
}

template <typename F, typename ... Rest>
std::vector<uint8_t> MergeVec(F first, Rest ... args)
{
    std::vector<uint8_t> v1 = MergeVec(first);
    std::vector<uint8_t> v2 = MergeVec(args...);
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
}

template <typename ... Args>
std::array<uint8_t,20> SHA1(Args ... vecs)
{
    std::vector<uint8_t> vec = MergeVec(vecs...);
    return Trinity::Crypto::SHA1::GetDigestOf(MergeVec(vecs...));
}

template <int64_t T>
std::array<uint8_t, T> getRandomBytes()
{
    std::array<uint8_t, T> arr;
    for (uint32_t i = 0; i < T; ++i)
    {
        arr[i] = uint8_t(i);
    }
    return arr;
}

// This function can serve as a general document for how to connect to an authserver -> worldserver on 3.3.5
boost::asio::awaitable<void> AuthMgr::AuthenticateBot(boost::asio::any_io_executor & exec, std::string const& authServerIp, Bot& bot)
{
    // Step 1: Establish tcp connection with authserver
    BotSocket& authSocket = bot.m_authSocket.emplace(BotSocket(exec, authServerIp, "3724"));
    co_await authSocket.Connect(exec);

    // Step 2: Send client auth challenge
    AuthPacket pkt(MergeVec(ClientAuthChallenge(bot.GetUsername(), authSocket.address()), bot.GetUsername()));
    bool cancelAuthChallenge = false;
    FIRE(OnAuthChallenge, bot.GetEvents(), {}, bot, pkt, BotMutable<bool>(&cancelAuthChallenge))
    if (cancelAuthChallenge)
    {
        co_return;
    }
    co_await pkt.Send(bot);

    // Step 3: Read server auth challenge and generate proof
    co_await AssertAuthCommand(AuthCommand::LOGON_CHALLENGE, authSocket);
    ServerAuthChallenge serverChallenge = co_await authSocket.Read<ServerAuthChallenge>();
    std::string authString = bot.GetUsername() + ":" + bot.GetPassword();
    std::transform(authString.begin(), authString.end(), authString.begin(), [](uint8_t c) {return std::toupper(c); });
    // this is just a bunch of math
    BigNumber k(3);
    BigNumber B = CreateBigNumber<32>(serverChallenge.m_B);
    BigNumber g(serverChallenge.m_g);
    BigNumber N = CreateBigNumber<32>(serverChallenge.m_N);
    BigNumber salt = CreateBigNumber<32>(serverChallenge.m_salt);
    BigNumber unk1 = CreateBigNumber<16>(serverChallenge.m_unk3);
    BigNumber x = CreateBigNumber(SHA1(serverChallenge.m_salt, SHA1(authString)));
    BigNumber A;
    BigNumber a;
    do
    {
        a = BigNumber(getRandomBytes<19>());
        A = g.ModExp(a, N);
    } while (A.ModExp(1, N) == 0);
    BigNumber u = CreateBigNumber(SHA1(A, B));
    BigNumber S = ((B + k * (N - g.ModExp(x, N))) % N).ModExp(a + (u * x), N);
    std::vector<uint8_t> sData = MergeVec(S);
    if (sData.size() < 32)
        sData.resize(32);
    std::array<uint8_t, 40> keyData;
    std::array<uint8_t, 16> temp;
    for (int i = 0; i < 16; ++i)
        temp[i] = sData[i * 2];
    std::array<uint8_t, 20> keyHash = SHA1(temp);
    for (int i = 0; i < 20; ++i)
        keyData[int64_t(i) * 2] = keyHash[i];
    for (int i = 0; i < 16; ++i)
        temp[i] = sData[int64_t(i) * 2 + 1];
    keyHash = SHA1(temp);
    for (int i = 0; i < 20; ++i)
        keyData[int64_t(i) * 2 + 1] = keyHash[i];
    BigNumber key(keyData);
    std::vector<uint8_t> gnHash(20);
    auto nHash = SHA1(N);
    for (int i = 0; i < 20; ++i)
        gnHash[i] = nHash[i];
    auto gHash = SHA1(g);
    for (int i = 0; i < 20; ++i)
        gnHash[i] ^= gHash[i];
    auto m1Hash = SHA1(gnHash,SHA1(bot.GetUsername()),serverChallenge.m_salt,A,B,key);
    auto m2Hash = SHA1(A, m1Hash, keyData);
    auto packet = AuthPacket(MergeVec(uint8_t(AuthCommand::LOGON_PROOF), A, m1Hash, std::vector<uint8_t>(22)));
    bool cancelLogonProof = false;
    FIRE(OnAuthProof, bot.GetEvents(), {}, bot, serverChallenge, packet, BotMutable<bool>(&cancelLogonProof));
    if (cancelLogonProof)
    {
        co_return;
    }
    co_await packet.Send(bot);

    // Step 4: Read and verify server auth proof
    co_await AssertAuthCommand(AuthCommand::LOGON_PROOF, authSocket);
    ServerAuthProof serverProof = co_await authSocket.Read<ServerAuthProof>();
    if (serverProof.M2 != m2Hash)
    {
        throw std::runtime_error("Server proof mismatch");
    }

    // Step 5: Request and read realmlist
    AuthPacket realmlistRequest(MergeVec(ClientRequestRealmlist({})));
    bool cancelRealmlistRequest = false;
    FIRE(OnRequestRealms, bot.GetEvents(), { } , bot, realmlistRequest, BotMutable<bool>(&cancelRealmlistRequest));
    if (cancelRealmlistRequest)
    {
        co_return;
    }
    co_await realmlistRequest.Send(bot);
    ServerRealmlistHeader header = co_await authSocket.Read<ServerRealmlistHeader>();
    std::vector<RealmInfo> realms(header.size);
    for (RealmInfo& realm : realms)
    {
        realm.m_type = co_await authSocket.Read<uint8_t>();
        realm.m_locked = co_await authSocket.Read<uint8_t>();
        realm.m_flags = co_await authSocket.Read<uint8_t>();
        realm.m_name = co_await authSocket.ReadCString();
        std::string tokens = co_await authSocket.ReadCString();
        size_t off = tokens.find(':');
        if (off == std::string::npos)
        {
            realm.m_port = 8085;
            realm.m_address = tokens;
        }
        else
        {
            realm.m_address = tokens.substr(0, off);
            realm.m_port = std::stoi(tokens.substr(off + 1));
        }
        realm.m_population = co_await authSocket.Read<float>();
        realm.m_load = co_await authSocket.Read<uint8_t>();
        realm.m_timezone = co_await authSocket.Read<uint8_t>();
        realm.m_id = co_await authSocket.Read<uint8_t>();
        if ((realm.m_flags & 4) != 0)
        {
            realm.m_major_version = co_await authSocket.Read<uint8_t>();
            realm.m_minor_version = co_await authSocket.Read<uint8_t>();
            realm.m_bugfix_version = co_await authSocket.Read<uint8_t>();
            realm.m_build = co_await authSocket.Read<uint8_t>();
        }
    }
    RealmInfo selectedRealm = realms[0];
    FIRE(OnSelectRealm, bot.GetEvents(), {}, bot, realms, BotMutable<RealmInfo>(&selectedRealm));

    // Step 6: Close auth connection and connect to worldserver
    bool shouldCloseAuthSocket = true;
    bool shouldCancelAuthSocketClose = false;
    FIRE(OnCloseAuthConnection, bot.GetEvents(), {}, bot, BotMutable<bool>(&shouldCloseAuthSocket), BotMutable<bool>(&shouldCancelAuthSocketClose));
    if (shouldCloseAuthSocket)
    {
        authSocket.Close();
    }
    if (shouldCancelAuthSocketClose)
    {
        co_return;
    }
    if (bot.m_worldSocket.has_value())
    {
        bot.m_worldSocket.value().Close();
    }
    BotSocket& worldSocket = bot.m_worldSocket.emplace(BotSocket(exec, selectedRealm.m_address, selectedRealm.m_port));
    co_await worldSocket.Connect(exec);

    // Step 7: Read world auth challenge and generate session key
    WorldPacket worldAuthChallenge = co_await WorldPacket::ReadWorldPacket(bot);
    if (worldAuthChallenge.GetOpcode() != Opcodes::SMSG_AUTH_CHALLENGE)
    {
        throw std::runtime_error("Got wrong opcode " + std::to_string(uint32_t(worldAuthChallenge.GetOpcode())));
    }
    uint32_t one = worldAuthChallenge.Read<uint32_t>();
    uint32_t seed = worldAuthChallenge.Read<uint32_t>();
    BigNumber seed1 = CreateBigNumber(worldAuthChallenge.ReadBytes(16));
    BigNumber seed2 = CreateBigNumber(worldAuthChallenge.ReadBytes(16));
    BigNumber ourSeed = getRandomBytes<4>();
    WorldPacket authSession = WorldPacket(Opcodes::CMSG_AUTH_SESSION);
    authSession
        .Write<uint32_t>(12340)
        .Write<uint32_t>(0)
        .WriteCString(bot.GetUsername())
        .Write<uint32_t>(0)
        .Write<uint32_t>(ourSeed.AsDword())
        .Write<uint32_t>(0)
        .Write<uint32_t>(0)
        .Write<uint32_t>(selectedRealm.m_id)
        .Write<uint64_t>(0)
        .WriteBytes(SHA1(bot.GetUsername(), uint32_t(0), ourSeed.AsDword(), uint32_t(seed), keyData))
        .Write<uint32_t>(0)
        ;
    bool cancelWorldAuthSession = false;
    FIRE(OnWorldAuthChallenge, bot.GetEvents(), { worldAuthChallenge.Reset(); }, bot, worldAuthChallenge, authSession, BotMutable<bool>(&cancelWorldAuthSession));
    if (cancelWorldAuthSession)
    {
        co_return;
    }
    co_await authSession.Send(bot);

    // Step 7: Verify world authentication
    bot.SetEncryptionKey(keyData);
    WorldPacket authResponsePacket = co_await WorldPacket::ReadWorldPacket(bot);
    if (authResponsePacket.GetOpcode() != Opcodes::SMSG_AUTH_RESPONSE)
    {
        throw std::runtime_error("Got wrong opcode " + std::to_string(uint32_t(worldAuthChallenge.GetOpcode())));
    }

    WorldAuthResponse resp = authResponsePacket.Read<WorldAuthResponse>();
    if (resp.m_detail != CommandDetail::AUTH_OK)
    {
        throw std::runtime_error(std::string("World authentication failed: ") + std::to_string(uint32_t(resp.m_detail)));
    }

    // Step 8: Send initial char enum request, then enter main loop
    bool cancelWorldAuthResponse = false;
    WorldPacket charEnumPacket = WorldPacket(Opcodes::CMSG_CHAR_ENUM);
    FIRE(OnWorldAuthResponse, bot.GetEvents(), { }, bot, resp, charEnumPacket, BotMutable<bool>(&cancelWorldAuthResponse))
    if (cancelWorldAuthResponse)
    {
        co_return;
    }
}

AuthMgr* AuthMgr::instance()
{
    static AuthMgr mgr;
    return &mgr;
}
