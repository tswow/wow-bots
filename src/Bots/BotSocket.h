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

class BotSocket
{
public:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver m_resolver;
    BotSocket(boost::asio::io_context& ctx);
    void Close();
    promise::Promise Connect(std::string const& ip, std::string const& port);
    promise::Promise WriteVector(std::vector<uint8_t> const& value);
    promise::Promise ReadVector(uint32_t size);
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
