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

#include "ARC4.h"

#include <boost/asio.hpp>
#include <promise.hpp>

#include <string>
#include <vector>
#include <optional>
#include <iostream>

class BotSocket2
{
public:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver m_resolver;
    BotSocket2(boost::asio::io_context& ctx);
    promise::Promise Connect(std::string const& ip, std::string const& port);
    promise::Promise WriteVector(std::vector<uint8_t> const& value);
    promise::Promise ReadCString();
    promise::Promise ReadString(uint32_t size);
    template <typename T>
    promise::Promise WritePOD(T& value)
    {
        unsigned char* chr = new unsigned char[sizeof(T)];
        memcpy(chr, &value, sizeof(T));
        return promise::newPromise([this, chr, value](promise::Defer& defer) {
            boost::asio::async_write(m_socket, boost::asio::buffer(chr, sizeof(T)), [chr,defer](const boost::system::error_code& ec, size_t amount) {
                delete[] chr;
                if (ec.failed())
                {
                    defer.reject(ec);
                }
                else
                {
                    defer.resolve();
                }
            });
        });
    }

    template <typename T>
    promise::Promise ReadPOD()
    {
        return promise::newPromise([this](promise::Defer& defer) {
            T* value = new T();
            boost::asio::async_read(m_socket, boost::asio::buffer(value, sizeof(T)), [defer, value](const boost::system::error_code& ec, std::size_t len)
            {
                if (ec.failed())
                {
                    delete value;
                    return defer.reject(ec);
                }
                else
                {
                    T v = *value;
                    delete value;
                    return defer.resolve(v);
                }
            });
        });
    }
};

class BotSocket
{
    boost::asio::ip::tcp::socket m_worldSocket;
    std::string m_ip;
    std::string m_port;
public:
    BotSocket(boost::asio::any_io_executor& ctx, std::string const& ip, std::string const& port);
    BotSocket(boost::asio::any_io_executor& ctx, std::string const& ip, uint32_t port);

    boost::asio::ip::tcp::socket& GetSocket();

    void Close();
    boost::asio::awaitable<boost::asio::ip::tcp::endpoint> Connect(boost::asio::any_io_executor& ctx);
    uint32_t address();

    void WriteVectorNoWait(std::vector<uint8_t> const& value);
    boost::asio::awaitable<uint64_t> WriteVector(std::vector<uint8_t> const& value);
    boost::asio::awaitable<uint64_t> WriteCString(std::string value);
    boost::asio::awaitable<uint64_t> WriteString(std::string value);
    boost::asio::awaitable<std::string> ReadCString();
    boost::asio::awaitable<std::vector<uint8_t>> ReadVector(uint32_t size);

    template<typename T>
    auto Write(T value)
    {
        return boost::asio::async_write(m_worldSocket, boost::asio::buffer((char*)&value, sizeof(value)), boost::asio::use_awaitable);
    }

    template<int64_t C>
    boost::asio::awaitable<std::array<uint8_t, C>> ReadArray()
    {
        std::array<uint8_t, C> arr;
        co_await boost::asio::async_read(m_worldSocket, boost::asio::buffer(arr), boost::asio::use_awaitable);
        co_return arr;
    }

    template <typename T>
    boost::asio::awaitable<T> Read()
    {
        T value;
        std::array<uint8_t, sizeof(T)> arr = co_await ReadArray<sizeof(T)>();
        memcpy(&value, arr.data(), sizeof(T));
        co_return value;
    }
};
