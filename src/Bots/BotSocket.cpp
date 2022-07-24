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
#include "BotSocket.h"

using boost::asio::ip::tcp;

BotSocket::BotSocket(boost::asio::any_io_executor& ctx, std::string const& ip, std::string const& port)
    : m_worldSocket(ctx)
    , m_ip(ip)
    , m_port(port)
{}

BotSocket::BotSocket(boost::asio::any_io_executor& ctx, std::string const& ip, uint32_t port)
: BotSocket(ctx,ip,std::to_string(port))
{}

boost::asio::ip::tcp::socket& BotSocket::GetSocket()
{
    return m_worldSocket;
}

void BotSocket::Close()
{
    m_worldSocket.close();
}

boost::asio::awaitable<boost::asio::ip::tcp::endpoint> BotSocket::Connect(boost::asio::any_io_executor& ctx)
{
    tcp::resolver resolver(ctx);
    auto endpoints = resolver.resolve(m_ip, m_port);
    return boost::asio::async_connect(m_worldSocket, endpoints, boost::asio::use_awaitable);
}

uint32_t BotSocket::address()
{
    return m_worldSocket.local_endpoint().address().to_v4().to_uint();
}

boost::asio::awaitable<uint64_t> BotSocket::WriteVector(std::vector<uint8_t> const& value)
{
    co_return co_await boost::asio::async_write(m_worldSocket, boost::asio::buffer(value), boost::asio::use_awaitable);
}

boost::asio::awaitable<uint64_t> BotSocket::WriteCString(std::string value)
{
    std::vector<uint8_t> vec(value.size()+1);
    memcpy(vec.data(), value.data(), value.size() + 1);
    return WriteVector(vec);
}

boost::asio::awaitable<uint64_t> BotSocket::WriteString(std::string value)
{
    return boost::asio::async_write(m_worldSocket, boost::asio::buffer(value.c_str(), value.size()), boost::asio::use_awaitable);
}

boost::asio::awaitable<std::string> BotSocket::ReadCString()
{
    std::string value = "";
    for (;;)
    {
        char chr = co_await Read<char>();
        if (chr == 0)
            break;
        else
            value += std::string{ chr };
    }
    co_return value;
}

boost::asio::awaitable<std::vector<uint8_t>> BotSocket::ReadVector(uint32_t size)
{
    std::vector<uint8_t> vec(size);
    co_await boost::asio::async_read(m_worldSocket, boost::asio::buffer(vec.data(), vec.size()), boost::asio::use_awaitable);
    co_return vec;
}
