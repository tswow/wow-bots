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

#include "BotOpcodes.h"

#include <boost/asio/awaitable.hpp>

#include <vector>
#include <string>

class Bot;
namespace promise { class Promise; }

class PacketBase
{
protected:
    std::vector<uint8_t> m_data;
    uint32_t m_read_ctr = 0;
public:
    std::vector<uint8_t> ReadBytes(uint32_t size);
    std::string ReadString(uint32_t size);
    std::string ReadCString();

    uint8_t ReadUInt8();
    int8_t ReadInt8();
    uint16_t ReadUInt16();
    int16_t ReadInt16();
    uint32_t ReadUInt32();
    int32_t ReadInt32();
    uint64_t ReadUInt64();
    int64_t ReadInt64();
    float ReadFloat();
    double ReadDouble();
    
    template <typename T>
    T Read()
    {
        T value;
        memcpy(&value, m_data.data() + m_read_ctr, sizeof(T));
        m_read_ctr += sizeof(T);
        return value;
    }

    PacketBase(std::vector<uint8_t> const& data);
};

#define PACKET_WRITE_DECL(type)\
    template <typename T>\
    type& WriteBytes(T value)\
    {\
        m_data.insert(m_data.end(), value.begin(), value.end());\
        return *this;\
    }\
    template <typename T>\
    type& Write(T value)\
    {\
        size_t size = m_data.size();\
        m_data.resize(size + sizeof(T));\
        memcpy(m_data.data() + size, &value, sizeof(T));\
        return *this;\
    }\
    type& WritePadding(uint32_t padding);\
    type& WriteUInt8(uint8_t value);\
    type& WriteInt8(int8_t value);\
    type& WriteUInt16(uint16_t value);\
    type& WriteInt16(int16_t value);\
    type& WriteUInt32(uint32_t value);\
    type& WriteInt32(int32_t value);\
    type& WriteUInt64(uint64_t value);\
    type& WriteInt64(int64_t value);\
    type& WriteFloat(float value);\
    type& WriteDouble(double value);\
    type& WriteString(std::string const& str);\
    type& WriteCString(std::string const& str);\

class WorldPacket: public PacketBase
{
public:
    WorldPacket(std::vector<uint8_t> const& packet);
    WorldPacket(Opcodes opcode, size_t initialSize = 0);
    uint64_t ReadPackedGUID();
    void WritePackedGUID(uint64_t guid);
    Opcodes GetOpcode() const;
    void SetOpcode(Opcodes opcode);
    uint16_t GetPayloadSize();
    void Reserve(uint32_t amount);
    void Reset();
    void Seek(uint32_t offset);
    boost::asio::awaitable<int64_t> Send(Bot& bot);
    void SendNoWait(Bot& bot);
    static boost::asio::awaitable<WorldPacket> ReadWorldPacket(Bot& bot);
    static promise::Promise ReadWorldPacket2(Bot& bot);
    PACKET_WRITE_DECL(WorldPacket)
private:
    void Prepare(Bot& bot);
};

class AuthPacket: public PacketBase
{
public:
    AuthPacket(std::vector<uint8_t> const& packet);
    AuthPacket(size_t initialSize = 0);
    void Reserve(uint32_t amount);
    void Reset();
    void Seek(uint32_t offset);
    boost::asio::awaitable<uint64_t> Send(Bot& bot);
    void SendNoWait(Bot& bot);
    promise::Promise Send2(Bot& bot);
    PACKET_WRITE_DECL(AuthPacket)
};