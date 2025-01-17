/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved. 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* ws_msg.h
*
* Websocket incoming and outgoing message definitions.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#pragma once

#ifndef _CASA_WS_MSG_H
#define _CASA_WS_MSG_H

#if defined(__cplusplus_winrt) || !defined(_M_ARM)

#include <memory>
#include <limits>

#include "cpprest/xxpublic.h"
#include "cpprest/containerstream.h"

#if _NOT_PHONE8_
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#include <ppltasks.h>
namespace pplx = Concurrency;
#else 
#include "pplx/pplxtasks.h"
#endif

#include "cpprest/uri.h"
#include "cpprest/basic_types.h"
#include "cpprest/asyncrt_utils.h"

namespace web 
{ 
namespace experimental
{
namespace web_sockets
{
namespace client
{

namespace details
{
    class winrt_client;
    class ws_desktop_client;
}


/// <summary>
/// The different types of websocket message.
/// Text type contains UTF-8 encoded data.
/// Interpretation of Binary type is left to the application.
/// Note: The fragment types and control frames like close, ping, pong are not supported on WinRT.
/// </summary>
enum class websocket_message_type
{
    text_message,
    binary_message,
    close,
    ping,
    pong
};

namespace details
{
class _websocket_message
{
public:
    _ASYNCRTIMP void set_body(concurrency::streams::istream instream);

    /// <summary>
    /// Get the streambuf for the message
    /// </summary>
    concurrency::streams::streambuf<uint8_t>& streambuf() { return m_buf; }

    /// <summary>
    /// Set the streambuf for the message
    /// </summary>
    void set_streambuf(concurrency::streams::streambuf<uint8_t> buf) { m_buf = buf; }

    void set_msg_type(websocket_message_type msg_type) { m_msg_type = msg_type; }

    void set_length(size_t len) { m_length = len; }

    size_t length() { return m_length; }

    websocket_message_type message_type() { return m_msg_type; }

    pplx::task_completion_event<void> _get_data_available()  { return m_data_available; }

    void _set_data_available() { m_data_available.set(); }

    _ASYNCRTIMP std::string _extract_string();

    /// <summary>
    /// Prepare the message with an output stream to receive network data
    /// </summary>
    _ASYNCRTIMP void _prepare_to_receive_data();

protected:

    concurrency::streams::streambuf<uint8_t> m_buf;

    websocket_message_type m_msg_type;

    size_t m_length;

    /// <summary> The TCE is used to signal the availability of the message body. </summary>
    pplx::task_completion_event<void> m_data_available;
};
}

/// <summary>
/// Represents an outgoing websocket message
/// </summary>
class websocket_outgoing_message 
{
public:
    websocket_outgoing_message()
        : _m_impl(std::make_shared<details::_websocket_message>()) {}

    /// <summary>
    /// Sets a UTF-8 message as the message body.
    /// </summary>
    /// <params>UTF-8 String containing body of the message.</returns>
    void set_utf8_message(std::string data)
    {
        this->_set_message(std::move(data), websocket_message_type::text_message);
    }

    /// <summary>
    /// Sets a UTF-8 message as the message body.
    /// </summary>
    /// <params>casablanca input stream representing the body of the message.</returns>
    void set_utf8_message(concurrency::streams::istream istream, size_t len)
    {
        this->_set_message(istream, len, websocket_message_type::text_message);
    }

    /// <summary>
    /// Sets binary data as the message body.
    /// </summary>
    /// <params>casablanca input stream representing the body of the message.</returns>
    void set_binary_message(concurrency::streams::istream istream, size_t len)
    {
        this->_set_message(istream, len, websocket_message_type::binary_message);
    }

private:

    friend class details::winrt_client;
    friend class details::ws_desktop_client;

    std::shared_ptr<details::_websocket_message> _m_impl;

    void _set_message(std::string data, websocket_message_type msg_type)
    {
        _m_impl->set_msg_type(msg_type);
        _m_impl->set_length(data.length());
        auto istream = concurrency::streams::bytestream::open_istream<std::string>(std::move(data));
        _m_impl->set_body(istream);
    }

    void _set_message(concurrency::streams::istream istream, size_t len, websocket_message_type msg_type)
    {
        _m_impl->set_msg_type(msg_type);
        _m_impl->set_body(istream);
        _m_impl->set_length(len);
    }

    pplx::task_completion_event<void> m_send_tce;
};

/// <summary>
/// Represents an incoming websocket message
/// </summary>
class websocket_incoming_message
{
public:
    websocket_incoming_message(): _m_impl(std::make_shared<details::_websocket_message>()) { }

    /// <summary>
    /// Extracts the body of the incoming message as a string value, only if the message type is UTF-8.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <returns>String containing body of the message.</returns>
    _ASYNCRTIMP pplx::task<std::string> extract_string() const;

    /// <summary>
    /// Produces a stream which the caller may use to retrieve body from an incoming message.
    /// Can be used for both UTF-8 (text) and binary message types.
    /// </summary>
    /// <returns>A readable, open asynchronous stream.</returns>
    /// <remarks>
    /// This cannot be used in conjunction with any other means of getting the body of the message.
    /// </remarks>
    concurrency::streams::istream body() const
    {
        return _m_impl->streambuf().create_istream();
    }

    /// <summary>
    /// Returns the length of the received message.
    /// </summary>
    size_t length() const
    {
        return _m_impl->length();
    }

    /// <summary>
    /// Returns the type of the received message.
    /// </summary>
    websocket_message_type messge_type() const
    {
        return _m_impl->message_type();
    }

private:
    friend class details::winrt_client;
    friend class details::ws_desktop_client;
    std::shared_ptr<details::_websocket_message> _m_impl;
};

}}}}

#endif  // _NOT_PHONE8_
#endif
#endif  /* _CASA_WS_MSG_H */
