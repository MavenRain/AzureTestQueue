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
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* http_listener.h
*
* HTTP Library: HTTP listener (server-side) APIs
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#pragma once

#ifndef _CASA_HTTP_LISTENER_H
#define _CASA_HTTP_LISTENER_H

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
#error "Error: http server APIs are not supported in XP"
#endif //_WIN32_WINNT < _WIN32_WINNT_VISTA

#include <limits>
#include <functional>

#include "cpprest/http_msg.h"

namespace web { 

/// <summary>
/// Declaration to avoid making a dependency on IISHost.
/// </summary>
namespace iis_host
{
    class http_iis_receiver;
}

namespace http
{
namespace experimental {
namespace listener
{

/// <summary>
/// Configuration class used to set various options when constructing and http_listener instance.
/// </summary>
class http_listener_config
{
public:

    /// <summary>
    /// Create an http_listener configuration with default options.
    /// </summary>
    http_listener_config()
        : m_timeout(utility::seconds(120))
    {}

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="other">http_listener_config to copy.</param>
    http_listener_config(const http_listener_config &other)
        : m_timeout(other.m_timeout)
    {}

    /// <summary>
    /// Move constructor.
    /// <summary>
    /// <param name="other">http_listener_config to move from.</param>
    http_listener_config(http_listener_config &&other)
        : m_timeout(std::move(other.m_timeout))
    {}

    /// <summary>
    /// Assignment operator.
    /// </summary>
    /// <returns>http_listener_config instance.</returns>
    http_listener_config & operator=(const http_listener_config &rhs)
    {
        if(this != &rhs)
        {
            m_timeout = rhs.m_timeout;
        }
        return *this;
    }

    /// <summary>
    /// Assignment operator.
    /// </summary>
    /// <returns>http_listener_config instance.</returns>
    http_listener_config & operator=(http_listener_config &&rhs)
    {
        if(this != &rhs)
        {
            m_timeout = std::move(rhs.m_timeout);
        }
        return *this;
    }

    /// <summary>
    /// Get the timeout
    /// </summary>
    /// <returns>The timeout (in seconds).</returns>
    utility::seconds timeout() const
    {
        return m_timeout;
    }

    /// <summary>
    /// Set the timeout
    /// </summary>
    /// <param name="timeout">The timeout (in seconds) used for each send and receive operation on the client.</param>
    void set_timeout(utility::seconds timeout)
    {
        m_timeout = std::move(timeout);
    }

private:

    utility::seconds m_timeout;
};

namespace details
{

/// <summary>
/// Internal class for pointer to implementation design pattern.
/// </summary>
class http_listener_impl
{
public:

    http_listener_impl()
        : m_closed(true)
    {
    }

    _ASYNCRTIMP http_listener_impl(http::uri address);
    _ASYNCRTIMP http_listener_impl(http::uri address, http_listener_config config);

    _ASYNCRTIMP pplx::task<void> open();
    _ASYNCRTIMP pplx::task<void> close();

    /// <summary>
    /// Handler for all requests. The HTTP host uses this to dispatch a message to the pipeline.
    /// </summary>
    /// <remarks>Only HTTP server implementations should call this API.</remarks>
    _ASYNCRTIMP void handle_request(http::http_request msg);

    const http::uri & uri() const { return m_uri; }

    const http_listener_config & configuration() const { return m_config; }

    // Handlers
    std::function<void(http::http_request)> m_all_requests;
    std::map<http::method, std::function<void(http::http_request)>> m_supported_methods;

private:

    // Default implementation for TRACE and OPTIONS.
    void handle_trace(http::http_request message);
    void handle_options(http::http_request message);

    // Gets a comma seperated string containing the methods supported by this listener.
    utility::string_t get_supported_methods() const;

    http::uri m_uri;
    http_listener_config m_config;

    // Used to record that the listener is closed.
    bool m_closed;
};

} // namespace details

/// <summary>
/// A class for listening and processing HTTP requests at a specific URI.
/// </summary>
class http_listener
{
public:

    /// <summary>
    /// Create a listener from a URI.
    /// </summary>
    /// <remarks>The listener will not have been opened when returned.</remarks>
    /// <param name="address">URI at which the listener should accept requests.</param>
    http_listener(http::uri address)
        : m_impl(utility::details::make_unique<details::http_listener_impl>(std::move(address)))
    {
    }

    /// <summary>
    /// Create a listener with specified URI and configuration.
    /// </summary>
    /// <param name="address">URI at which the listener should accept requests.</param>
    /// <param name="config">Configuration to create listener with.</param>
    http_listener(http::uri address, http_listener_config config)
        : m_impl(utility::details::make_unique<details::http_listener_impl>(std::move(address), std::move(config)))
    {
    }

    /// <summary>
    /// Default constructor.
    /// </summary>
    /// <remarks>The resulting listener cannot be used for anything, but is useful to initialize a variable
    /// that will later be overwritten with a real listener instance.</remarks>
    http_listener() 
        : m_impl(utility::details::make_unique<details::http_listener_impl>())
    { 
    }

    /// <summary>
    /// Destructor frees any held resources.
    /// </summary>
    /// <remarks>Call close() before allowing a listener to be destroyed.</remarks>
    _ASYNCRTIMP ~http_listener();
    
    /// <summary>
    /// Asychronously open the listener, i.e. start accepting requests.
    /// </summary>
    /// <returns>A task that will be completed once this listener is actually opened, accepting requests.</returns>
    pplx::task<void> open()
    {
        return m_impl->open();
    }

    /// <summary>
    /// Asychrnoously stop accepting requests and close all connections.
    /// </summary>
    /// <returns>A task that will be completed once this listener is actually closed, no longer accepting requests.</returns> 
    /// <remarks>
    /// This function will stop accepting requests and wait for all outstanding handler calls
    /// to finish before completing the task. Waiting on the task returned from close() within 
    /// a handler and blocking waiting for its result will result in a deadlock.
    ///
    /// Call close() before allowing a listener to be destroyed.
    /// </remarks>
    pplx::task<void> close()
    {
        return m_impl->close();
    }

    /// <summary>
    /// Add a general handler to support all requests.
    /// </summary>
    /// <param name="handler">Funtion object to be called for all requests.</param>
    void support(std::function<void(http_request)> handler)  
    {
        m_impl->m_all_requests = handler;
    }

    /// <summary>
    /// Add support for a specific HTTP method.
    /// </summary>
    /// <param name="method">An HTTP method.</param>
    /// <param name="handler">Funtion object to be called for all requests for the given HTTP method.</param>
    void support(const http::method &method, std::function<void(http_request)> handler)
    {
        m_impl->m_supported_methods[method] = handler;
    }

    /// <summary>
    /// Get the URI of the listener.
    /// </summary>
    /// <returns>The URI this listener is for.</returns>
    const http::uri & uri() const { return m_impl->uri(); }

    /// <summary>
    /// Get the configuration of this listener.
    /// </summary>
    /// <returns>Configuration this listener was constructed with.</returns>
    const http_listener_config & configuration() const { return m_impl->configuration(); }

    /// <summary>
    /// Move constructor.
    /// </summary>
    /// <param name="other">http_listener instance to construct this one from.</param>
    http_listener(http_listener &&other)
        : m_impl(std::move(other.m_impl))
    {
    }

    /// <summary>
    /// Move assignment operator.
    /// </summary>
    /// <param name="other">http_listener to replace this one with.</param>
    http_listener &operator=(http_listener &&other)
    {
        if(this != &other)
        {
            m_impl = std::move(other.m_impl);
        }
        return *this;
    }

private:

    // No copying of listeners.
    http_listener(const http_listener &other);
    http_listener &operator=(const http_listener &other);

    std::unique_ptr<details::http_listener_impl> m_impl;
};

} // namespace listener
} // namespace experimental
} // namespace http
} // namespace web

#endif  /* _CASA_HTTP_LISTENER_H */