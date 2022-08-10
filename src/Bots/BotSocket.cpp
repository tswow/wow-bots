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
#include <promise.hpp>

using boost::asio::ip::tcp;

BotSocket::BotSocket(boost::asio::io_context& ctx)
    : m_socket(ctx)
    , m_resolver(ctx)
{
}

void BotSocket::Close()
{
    m_socket.close();
}

promise::Promise BotSocket::Connect(std::string const& address, std::string const& port)
{
    return promise::newPromise([this, address, port](promise::Defer& defer) {
        m_resolver.async_resolve(address, port, [this, defer](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {
            if (ec.failed())
            {
                defer.reject();
            }
            else
            {
                boost::asio::async_connect(m_socket, results, [defer](const boost::system::error_code& ec, auto _) {
                    if (ec.failed())
                    {
                        return defer.reject();
                    }
                    else
                    {
                        return defer.resolve();
                    }
                    });
            }
        });
    });
}

promise::Promise BotSocket::ReadVector(uint32_t size)
{
    std::vector<uint8_t>* vec = new std::vector<uint8_t>(size);
    return promise::newPromise([=](promise::Defer& defer) {
        boost::asio::async_read(m_socket, boost::asio::buffer(vec->data(), vec->size()), [vec,size,defer](const boost::system::error_code& ec, auto _) {
            std::vector<uint8_t> vecValue(size);
            memcpy(vecValue.data(), vec->data(), vec->size());
            delete vec;
            if (ec.failed())
            {
                return defer.reject();
            }
            else
            {
                return defer.resolve(vecValue);
            }
        });
    });
}


promise::Promise BotSocket::WriteVector(std::vector<uint8_t> const& value)
{
    uint8_t* arr = new uint8_t[value.size()];
    memcpy(arr, value.data(), value.size());
    size_t size = value.size();
    return promise::newPromise([=](promise::Defer& defer) {
        boost::asio::async_write(m_socket, boost::asio::buffer(arr,size), [=](const boost::system::error_code& ec, auto _) {
            delete[] arr;
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

promise::Promise BotSocket::ReadCString()
{
    std::string* str = new std::string("");
    return promise::doWhile([=](promise::DeferLoop& loop) {
        ReadPOD<uint8_t>().then([=](uint8_t value) {
            if (value == '\0')
            {
                std::string s = *str;
                delete str;
                return loop.doBreak(s);
            }
            else
            {
                str->push_back(value);
                return loop.doContinue();
            }
        })
        .fail([=](){
            delete str;
            loop.reject();
        });
    });
}

promise::Promise BotSocket::ReadString(uint32_t size)
{
    char* arr = new char[size];
    return promise::newPromise([=](promise::Defer& defer) {
        boost::asio::async_read(m_socket, boost::asio::buffer(arr, size), [=](const boost::system::error_code& ec, auto _) {
            std::string str(arr, size);
            delete[] arr;
            if (ec.failed())
            {
                defer.reject(ec);
            }
            else
            {
                defer.resolve(str);
            }
            });
        });
}
