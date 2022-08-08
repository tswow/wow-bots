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
#include "BotPacket.h"
#include "Bot.h"

#define PACKET_WRITE_DEF(type)\
    type& type::WriteString(std::string const& str)\
    {\
        m_data.insert(m_data.end(), str.begin(), str.end());\
        return *this;\
    }\
    type& type::WriteCString(std::string const& str)\
    {\
        m_data.reserve(m_data.size() + str.size() + 1);\
        WriteString(str);\
        m_data.push_back('\0');\
        return *this;\
    }\
    type& type::WriteUInt8(uint8_t value) { return Write<uint8_t>(value);}\
    type& type::WriteInt8(int8_t value) { return Write<int8_t>(value);}\
    type& type::WriteUInt16(uint16_t value) { return Write<uint16_t>(value);}\
    type& type::WriteInt16(int16_t value) { return Write<int16_t>(value);}\
    type& type::WriteUInt32(uint32_t value) { return Write<uint32_t>(value);}\
    type& type::WriteInt32(int32_t value) { return Write<int32_t>(value);}\
    type& type::WriteUInt64(uint64_t value) { return Write<uint64_t>(value);}\
    type& type::WriteInt64(int64_t value) { return Write<int64_t>(value);}\
    type& type::WriteFloat(float value) { return Write<float>(value);}\
    type& type::WriteDouble(double value) { return Write<double>(value);}\
    type& type::WritePadding(uint32_t padding) { m_data.resize(m_data.size() + padding); return *this;}\


PacketBase::PacketBase(std::vector<uint8_t> const& data)
    : m_data(data)
{}

uint8_t PacketBase::ReadUInt8() { return Read<uint8_t>(); }\
int8_t PacketBase::ReadInt8() { return Read<int8_t>(); }\
uint16_t PacketBase::ReadUInt16() { return Read<uint16_t>(); }\
int16_t PacketBase::ReadInt16() { return Read<int16_t>(); }\
uint32_t PacketBase::ReadUInt32() { return Read<uint32_t>(); }\
int32_t PacketBase::ReadInt32() { return Read<int32_t>(); }\
uint64_t PacketBase::ReadUInt64() { return Read<uint64_t>(); }\
int64_t PacketBase::ReadInt64() { return Read<int64_t>(); }\
float PacketBase::ReadFloat() { return Read<float>(); }\
double PacketBase::ReadDouble() { return Read<double>(); }\

std::vector<uint8_t> PacketBase::ReadBytes(uint32_t size)
{
    std::vector<uint8_t> bytes(size);
    memcpy(bytes.data(), m_data.data() + m_read_ctr, size);
    m_read_ctr += size;
    return bytes;
}

std::string PacketBase::ReadString(uint32_t size)
{
    std::string str;
    str.resize(size);
    memcpy(str.data(), m_data.data() + m_read_ctr, size);
    m_read_ctr += size;
    return str;
}

std::string PacketBase::ReadCString()
{
    std::string str = "";
    for (int i = 0;; ++i)
    {
        char c = m_data[i];
        if (c == '\0')
            break;
        else
            str.push_back(c);
    }
    m_read_ctr += str.size();
    return str;
}

Opcodes WorldPacket::GetOpcode() const
{
    return *reinterpret_cast<Opcodes const*>(m_data.data() + 2);
}

void WorldPacket::SetOpcode(Opcodes opcode)
{
    *reinterpret_cast<Opcodes*>(m_data.data() + 2) = opcode;
}


uint16_t WorldPacket::GetPayloadSize()
{
    return m_data.size() - 6;
}

void WorldPacket::Reserve(uint32_t amount)
{
    m_data.reserve(amount + 6);
}

void WorldPacket::Reset()
{
    m_read_ctr = 6;
}
void WorldPacket::Seek(uint32_t offset)
{
    m_read_ctr = offset + 6;
}

void WorldPacket::Prepare(Bot& bot)
{
    uint16_t size = m_data.size() - 2;
    memcpy(m_data.data(), &size, sizeof(uint16_t));
    std::reverse(m_data.begin(), m_data.begin() + 2);
    if (bot.m_encrypt.has_value())
    {
        bot.m_encrypt->UpdateData(m_data.data(), 6);
    }
}

boost::asio::awaitable<int64_t> WorldPacket::Send(Bot& bot)
{
    // need to do this so the m_data array is in scope
    Prepare(bot);
    co_return co_await bot.GetWorldSocket().WriteVector(m_data);
}

void WorldPacket::SendNoWait(Bot& bot)
{
    Prepare(bot);
    bot.GetWorldSocket().WriteVectorNoWait(m_data);
}


WorldPacket::WorldPacket(std::vector<uint8_t> const& packet)
    : PacketBase{ packet }
{
    m_read_ctr = 6;
}

WorldPacket::WorldPacket(Opcodes opcode, size_t initialSize)
    : PacketBase({})
{
    m_data.resize(sizeof(Opcodes) + sizeof(uint16_t));
    memcpy(m_data.data() + 2, &opcode, sizeof(Opcodes));
    m_data.reserve(sizeof(Opcodes) + sizeof(uint16_t) + initialSize);
    m_read_ctr = 6;
}

boost::asio::awaitable<WorldPacket> WorldPacket::ReadWorldPacket(Bot& bot)
{
    BotSocket& socket = bot.GetWorldSocket();
    // todo: read 6 bytes right away, we don't need to split it like this
    uint8_t firstByte = co_await socket.Read<uint8_t>();
    if (bot.m_decrypt.has_value())
    {
        bot.m_decrypt.value().UpdateData(&firstByte, sizeof(firstByte));
    }

    Opcodes command;
    uint32_t size;
    if (firstByte & 0x80)
    {
        std::array<uint8_t, 2> arr = co_await socket.ReadArray<2>();
        if (bot.m_decrypt.has_value())
        {
            bot.m_decrypt.value().UpdateData(arr);
        }
        size = (uint32_t)((((firstByte) & 0x7F) << 16) | ((arr[0] << 8) | arr[1]));
    }
    else
    {
        firstByte &= 0x7f;
        uint8_t size2 = co_await socket.Read<uint8_t>();
        if (bot.m_decrypt.has_value())
        {
            bot.m_decrypt.value().UpdateData(&size2, sizeof(uint8_t));
        }
        size = (uint32_t)((firstByte) << 8 | size2);
    }
    
    uint16_t cmdBytes = co_await socket.Read<uint16_t>();
    if (bot.m_decrypt.has_value())
    {
        bot.m_decrypt.value().UpdateData(reinterpret_cast<uint8_t*>(&cmdBytes), sizeof(uint16_t));
    }
    command = static_cast<Opcodes>(cmdBytes);
    size -= 2;
    std::vector<uint8_t> data = co_await socket.ReadVector(size);
    std::vector<uint8_t> dataFull(data.size() + 6);
    memcpy(dataFull.data() + 2, &command, sizeof(Opcodes));
    memcpy(dataFull.data() + 6, data.data(), data.size());
    co_return WorldPacket(dataFull);
}

AuthPacket::AuthPacket(std::vector<uint8_t> const& packet)
    : PacketBase(packet)
{}

void AuthPacket::Reserve(uint32_t amount)
{
    m_data.reserve(amount);
}

void AuthPacket::Reset()
{
    m_read_ctr = 0;
}

void AuthPacket::Seek(uint32_t offset)
{
    m_read_ctr = offset;
}

AuthPacket::AuthPacket(size_t initialSize)
    : PacketBase({})
{
    Reserve(initialSize);
}

boost::asio::awaitable<uint64_t> AuthPacket::Send(Bot& bot)
{
    return bot.GetAuthSocket().WriteVector(m_data);
}

void AuthPacket::SendNoWait(Bot& bot)
{
    bot.GetAuthSocket().WriteVectorNoWait(m_data);
}

uint64_t WorldPacket::ReadPackedGUID()
{
    uint8 mask = ReadUInt8();
    if (mask == 0)
        return 0;
    uint64 res = 0;
    int i = 0;
    while (i < 8)
    {
        if ((mask & 1 << i) != 0)
            res += (uint64_t(ReadUInt8())) << (i * 8);
        i++;
    }
    return res;
}


PACKET_WRITE_DEF(WorldPacket)
PACKET_WRITE_DEF(AuthPacket)
