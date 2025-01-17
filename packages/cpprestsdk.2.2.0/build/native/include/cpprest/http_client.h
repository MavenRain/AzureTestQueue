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
* http_client.h
*
* HTTP Library: Client-side APIs.
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#pragma once

#ifndef _CASA_HTTP_CLIENT_H
#define _CASA_HTTP_CLIENT_H

#if defined (__cplusplus_winrt)
#define __WRL_NO_DEFAULT_LIB__
#include <wrl.h>
#include <msxml6.h>
namespace web { namespace http{namespace client{
typedef IXMLHTTPRequest2* native_handle;}}}
#else
namespace web { namespace http{namespace client{
typedef void* native_handle;}}}
#endif // __cplusplus_winrt

#include <memory>
#include <limits>

#include "cpprest/xxpublic.h"
#include "cpprest/http_msg.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#include <ppltasks.h>
namespace pplx = Concurrency;
#else 
#include "pplx/pplxtasks.h"
#endif

#include "cpprest/json.h"
#include "cpprest/uri.h"
#include "cpprest/web_utilities.h"
#include "cpprest/basic_types.h"
#include "cpprest/asyncrt_utils.h"

#if !defined(CPPREST_TARGET_XP) && !defined(_PHONE8_)
#include "cpprest/oauth1.h"
#endif

#include "cpprest/oauth2.h"

namespace web 
{
namespace http
{
namespace client
{

// credentials and web_proxy class has been moved from web::http::client namespace to web namespace.
// The below using declarations ensure we dont break existing code.
// Please use the web::credentials and web::web_proxy class going forward.
using web::credentials;
using web::web_proxy;

#ifdef _MS_WINDOWS
namespace details {
#ifdef __cplusplus_winrt
        class winrt_client ;
#else
        class winhttp_client;
#endif // __cplusplus_winrt
}  
#endif // _MS_WINDOWS

/// <summary>
/// HTTP client configuration class, used to set the possible configuration options
/// used to create an http_client instance.
/// </summary>
class http_client_config
{
public:
    http_client_config() : 
        m_guarantee_order(false),
        m_timeout(utility::seconds(30)),
        m_chunksize(0)
#if !defined(__cplusplus_winrt)
        , m_validate_certificates(true)
#endif
        , m_set_user_nativehandle_options([](native_handle)->void{})
#ifdef _MS_WINDOWS
#if !defined(__cplusplus_winrt)
        , m_buffer_request(false)
#endif
#endif
    {
    }

#if !defined(CPPREST_TARGET_XP) && !defined(_PHONE8_)
    /// <summary>
    /// Get OAuth 1.0 configuration.
    /// </summary>
    /// <returns>Shared pointer to OAuth 1.0 configuration.</returns>
    const std::shared_ptr<oauth1::experimental::oauth1_config> oauth1() const
    {
        return m_oauth1;
    }

    /// <summary>
    /// Set OAuth 1.0 configuration.
    /// </summary>
    /// <param name="config">OAuth 1.0 configuration to set.</param>
    void set_oauth1(oauth1::experimental::oauth1_config config)
    {
        m_oauth1 = std::make_shared<oauth1::experimental::oauth1_config>(std::move(config));
    }
#endif

    /// <summary>
    /// Get OAuth 2.0 configuration.
    /// </summary>
    /// <returns>Shared pointer to OAuth 2.0 configuration.</returns>
    const std::shared_ptr<oauth2::experimental::oauth2_config> oauth2() const
    {
        return m_oauth2;
    }

    /// <summary>
    /// Set OAuth 2.0 configuration.
    /// </summary>
    /// <param name="config">OAuth 2.0 configuration to set.</param>
    void set_oauth2(oauth2::experimental::oauth2_config config)
    {
        m_oauth2 = std::make_shared<oauth2::experimental::oauth2_config>(std::move(config));
    }

    /// <summary>
    /// Get the web proxy object
    /// </summary>
    /// <returns>A reference to the web proxy object.</returns>
    const web_proxy& proxy() const
    {
        return m_proxy;
    }

    /// <summary>
    /// Set the web proxy object
    /// </summary>
    /// <param name="proxy">A reference to the web proxy object.</param>
    void set_proxy(const web_proxy& proxy)
    {
        m_proxy = proxy;
    }

    /// <summary>
    /// Get the client credentials
    /// </summary>
    /// <returns>A reference to the client credentials.</returns>
    const http::client::credentials& credentials() const
    {
        return m_credentials;
    }

    /// <summary>
    /// Set the client credentials
    /// </summary>
    /// <param name="cred">A reference to the client credentials.</param>
    void set_credentials(const http::client::credentials& cred)
    {
        m_credentials = cred;
    }

    /// <summary>
    /// Get the 'guarantee order' property
    /// </summary>
    /// <returns>The value of the property.</returns>
    bool guarantee_order() const
    {
        return m_guarantee_order;
    }

    /// <summary>
    /// Set the 'guarantee order' property
    /// </summary>
    /// <param name="guarantee_order">The value of the property.</param>
    void set_guarantee_order(bool guarantee_order)
    {
        m_guarantee_order = guarantee_order;
    }

    /// <summary>
    /// Get the timeout
    /// </summary>
    /// <returns>The timeout (in seconds) used for each send and receive operation on the client.</returns>
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
        m_timeout = timeout;
    }

    /// <summary>
    /// Get the client chunk size.
    /// </summary>
    /// <returns>The internal buffer size used by the http client when sending and receiving data from the network.</returns>
    size_t chunksize() const
    {
        return m_chunksize == 0 ? 64 * 1024 : m_chunksize;
    }

    /// <summary>
    /// Sets the client chunk size.
    /// </summary>
    /// <param name="size">The internal buffer size used by the http client when sending and receiving data from the network.</param>
    /// <remarks>This is a hint -- an implementation may disregard the setting and use some other chunk size.</remarks>
    void set_chunksize(size_t size)
    {
        m_chunksize = size;
    }

    /// <summary>
    /// Returns true if the default chunk size is in use.
    /// <remarks>If true, implementations are allowed to choose whatever size is best.</remarks>
    /// </summary>
    /// <returns>True if default, false if set by user.</returns>
    bool is_default_chunksize() const
    {
        return m_chunksize == 0;
    }

#if !defined(__cplusplus_winrt)
    /// <summary>
    /// Gets the server certificate validation property.
    /// </summary>
    /// <returns>True if certificates are to be verified, false otherwise.</returns>
    bool validate_certificates() const
    {
        return m_validate_certificates;
    }

    /// <summary>
    /// Sets the server certificate validation property.
    /// </summary>
    /// <param name="validate_cert">False to turn ignore all server certificate validation errors, true otherwise.</param>
    /// <remarks>Note ignoring certificate errors can be dangerous and should be done with caution.</remarks>
    void set_validate_certificates(bool validate_certs)
    {
        m_validate_certificates = validate_certs;
    }
#endif

#ifdef _MS_WINDOWS
#if !defined(__cplusplus_winrt)
    /// <summary>
    /// Checks if request data buffering is turned on, the default is off.
    /// </summary>
    /// <returns>True if buffering is enabled, false otherwise</returns>
    bool buffer_request() const
    {
        return m_buffer_request;
    }

    /// <summary>
    /// Sets the request buffering property. 
    /// If true, in cases where the request body/stream doesn't support seeking the request data will be buffered.
    /// This can help in situations where an authentication challenge might be expected.
    /// </summary>
    /// <param name="buffer_request">True to turn on buffer, false otherwise.</param>
    /// <remarks>Please note there is a performance cost due to copying the request data.</remarks>
    void set_buffer_request(bool buffer_request)
    {
        m_buffer_request = buffer_request;
    }
#endif
#endif
    
    /// <summary>
    /// Sets a callback to enable custom setting of winhttp options
    /// </summary>
    /// <param name="callback">A user callback allowing for customization of the request</param>
    void set_nativehandle_options(std::function<void(native_handle)> callback)
    {
         m_set_user_nativehandle_options = callback;
    }

private:
#if !defined(CPPREST_TARGET_XP) && !defined(_PHONE8_)
    std::shared_ptr<oauth1::experimental::oauth1_config> m_oauth1;
#endif // !defined(CPPREST_TARGET_XP) && !defined(_PHONE8_)

    std::shared_ptr<oauth2::experimental::oauth2_config> m_oauth2;
    web_proxy m_proxy;
    http::client::credentials m_credentials;
    // Whether or not to guarantee ordering, i.e. only using one underlying TCP connection.
    bool m_guarantee_order;

    // IXmlHttpRequest2 doesn't allow configuration of certificate verification.
#if !defined(__cplusplus_winrt)
    bool m_validate_certificates;
#endif

#ifdef _MS_WINDOWS
#if !defined(__cplusplus_winrt)
    bool m_buffer_request;
#endif
#endif

    std::function<void(native_handle)> m_set_user_nativehandle_options;

    utility::seconds m_timeout;
    size_t m_chunksize;

#ifdef _MS_WINDOWS
#ifdef __cplusplus_winrt
    friend class details::winrt_client;
#else
    friend class details::winhttp_client;
#endif // __cplusplus_winrt  
#endif // _MS_WINDOWS

    /// <summary>
    /// Invokes a user callback to allow for customization of the requst
    /// </summary>
    /// <param name="handle">The internal http_request handle</param>
    /// <returns>True if users set WinHttp/IXAMLHttpRequest2 options correctly, false otherwise.</returns>
    void call_user_nativehandle_options(native_handle handle) const
    {
         m_set_user_nativehandle_options(handle);
    }
};

/// <summary>
/// HTTP client class, used to maintain a connection to an HTTP service for an extended session.
/// </summary>
class http_client
{
public:
    /// <summary>
    /// Creates a new http_client connected to specified uri.
    /// </summary>
    /// <param name="base_uri">A string representation of the base uri to be used for all requests. Must start with either "http://" or "https://"</param>
    _ASYNCRTIMP http_client(uri base_uri);

    /// <summary>
    /// Creates a new http_client connected to specified uri.
    /// </summary>
    /// <param name="base_uri">A string representation of the base uri to be used for all requests. Must start with either "http://" or "https://"</param>
    /// <param name="client_config">The http client configuration object containing the possible configuration options to intitialize the <c>http_client</c>. </param>
    _ASYNCRTIMP http_client(uri base_uri, http_client_config client_config);

    /// <summary>
    /// Note the destructor doesn't necessarily close the connection and release resources.
    /// The connection is reference counted with the http_responses.
    /// </summary>
    ~http_client() {}

    /// <summary>
    /// Gets the base uri
    /// </summary>
    /// <returns>
    /// A base uri initialized in constructor
    /// </return>
    const uri& base_uri() const
    {
        return _base_uri;
    }

    /// <summary>
    /// Get client configuration object
    /// </summary>
    /// <returns>A reference to the client configuration object.</returns>
    _ASYNCRTIMP const http_client_config& client_config() const;

    /// <summary>
    /// Adds an HTTP pipeline stage to the client.
    /// </summary>
    /// <param name="handler">A function object representing the pipeline stage.</param>
    void add_handler(std::function<pplx::task<http_response>(http_request, std::shared_ptr<http::http_pipeline_stage>)> handler)
    {
        m_pipeline->append(std::make_shared< ::web::http::details::function_pipeline_wrapper>(handler));
    }

    /// <summary>
    /// Adds an HTTP pipeline stage to the client.
    /// </summary>
    /// <param name="stage">A shared pointer to a pipeline stage.</param>
    void add_handler(std::shared_ptr<http::http_pipeline_stage> stage)
    {
        m_pipeline->append(stage);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="request">Request to send.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    _ASYNCRTIMP pplx::task<http_response> request(http_request request, pplx::cancellation_token token = pplx::cancellation_token::none());

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(method mtd, pplx::cancellation_token token = pplx::cancellation_token::none())
    {
        http_request msg(std::move(mtd));
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment,
        pplx::cancellation_token token = pplx::cancellation_token::none())
    {
        http_request msg(std::move(mtd));
        msg.set_request_uri(path_query_fragment);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">The data to be used as the message body, represented using the json object library.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment, 
        const json::value &body_data,
        pplx::cancellation_token token = pplx::cancellation_token::none())
    {
        http_request msg(std::move(mtd));
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body_data);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment,
        const utility::string_t &body_data,
        utility::string_t content_type = _XPLATSTR("text/plain"),
        pplx::cancellation_token token = pplx::cancellation_token::none())
    {
        http_request msg(std::move(mtd));
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body_data, std::move(content_type));
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment,
        const utility::string_t &body_data,
        pplx::cancellation_token token)
    {
        return request(mtd, path_query_fragment, body_data, _XPLATSTR("text/plain"), token);
    }

#if !defined (__cplusplus_winrt)
    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment,
        concurrency::streams::istream body,
        utility::string_t content_type = _XPLATSTR("application/octet-stream"),
        pplx::cancellation_token token = pplx::cancellation_token::none())
    {
        http_request msg(std::move(mtd));
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body, std::move(content_type));
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment,
        concurrency::streams::istream body,
        pplx::cancellation_token token)
    {
        return request(mtd, path_query_fragment, body, _XPLATSTR("application/octet-stream"), token);
    }
#endif // __cplusplus_winrt

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_length">Size of the message body.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    /// <remarks>Winrt requires to provide content_length.</remarks>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment,
        concurrency::streams::istream body,
        size_t content_length,
        utility::string_t content_type= _XPLATSTR("application/octet-stream"),
        pplx::cancellation_token token = pplx::cancellation_token::none())
    {
        http_request msg(std::move(mtd));
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body, content_length, std::move(content_type));
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_length">Size of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    /// <remarks>Winrt requires to provide content_length.</remarks>
    pplx::task<http_response> request(
        method mtd, 
        const utility::string_t &path_query_fragment,
        concurrency::streams::istream body,
        size_t content_length,
        pplx::cancellation_token token)
    {
        return request(mtd, path_query_fragment, body, content_length, _XPLATSTR("application/octet-stream"), token);
    }

private:

    void build_pipeline(uri base_uri, http_client_config client_config);
    
    std::shared_ptr<::web::http::http_pipeline> m_pipeline;

    uri _base_uri;
};

}}} // namespaces

#endif  /* _CASA_HTTP_CLIENT_H */
