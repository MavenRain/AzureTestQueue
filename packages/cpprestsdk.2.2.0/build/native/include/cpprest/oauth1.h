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
* oauth1.h
*
* HTTP Library: Oauth 1.0 
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#pragma once

#ifndef _CASA_OAUTH1_H
#define _CASA_OAUTH1_H

#include "cpprest/http_msg.h"

namespace web
{
namespace http
{
namespace oauth1
{
namespace details
{

class oauth1_handler;

// State currently used by oauth1_config to authenticate request.
// The state varies for every request (due to timestamp and nonce).
// The state also contains extra transmitted protocol parameters during
// authorization flow (i.e. 'oauth_callback' or 'oauth_verifier').
class oauth1_state
{
public:
    oauth1_state(utility::string_t timestamp, utility::string_t nonce,
            utility::string_t extra_key=utility::string_t(),
            utility::string_t extra_value=utility::string_t()) :
        m_timestamp(std::move(timestamp)),
        m_nonce(std::move(nonce)),
        m_extra_key(std::move(extra_key)),
        m_extra_value(std::move(extra_value))
    {}

    const utility::string_t& timestamp() const { return m_timestamp; }
    void set_timestamp(utility::string_t timestamp) { m_timestamp = std::move(timestamp); }

    const utility::string_t& nonce() const { return m_nonce; }
    void set_nonce(utility::string_t nonce) { m_nonce = std::move(nonce); }

    const utility::string_t& extra_key() const { return m_extra_key; }
    void set_extra_key(utility::string_t key) { m_extra_key = std::move(key); }

    const utility::string_t& extra_value() const { return m_extra_value; }
    void set_extra_value(utility::string_t value) { m_extra_value = std::move(value); }

private:
    utility::string_t m_timestamp;
    utility::string_t m_nonce;
    utility::string_t m_extra_key;
    utility::string_t m_extra_value;
};

// Constant strings for OAuth 1.0.
typedef utility::string_t oauth1_string;
class oauth1_strings
{
public:
#define _OAUTH1_STRINGS
#define DAT(a_, b_) _ASYNCRTIMP static const oauth1_string a_;
#include "cpprest/http_constants.dat"
#undef _OAUTH1_STRINGS
#undef DAT
};

} // namespace web::http::oauth1::details

namespace experimental
{

/// <summary>
/// Constant strings for OAuth 1.0 signature methods.
/// </summary>
typedef utility::string_t oauth1_method;
class oauth1_methods
{
public:
#define _OAUTH1_METHODS
#define DAT(a,b) _ASYNCRTIMP static const oauth1_method a;
#include "cpprest/http_constants.dat"
#undef _OAUTH1_METHODS
#undef DAT
};

/// <summary>
/// Exception type for OAuth 1.0 errors.
/// </summary>
class oauth1_exception : public std::exception
{
public:
    oauth1_exception(utility::string_t msg) : m_msg(utility::conversions::to_utf8string(std::move(msg))) {}
    ~oauth1_exception() _noexcept {}
    const char* what() const _noexcept { return m_msg.c_str(); }

private:
    std::string m_msg;
};

/// <summary>
/// OAuth 1.0 token and associated information.
/// </summary>
class oauth1_token
{
public:
    oauth1_token(utility::string_t access_token, utility::string_t secret) :
        m_token(std::move(access_token)),
        m_secret(std::move(secret))
    {}

    /// <summary>
    /// Get access token validity state.
    /// If true, token is a valid access token.
    /// </summary>
    /// <returns>Access token validity state of the token.</returns>
    bool is_valid_access_token() const { return !(access_token().empty() || secret().empty()); }

    /// <summary>
    /// Get access token.
    /// </summary>
    /// <returns>The access token string.</returns>
    const utility::string_t& access_token() const { return m_token; }
    /// <summary>
    /// Set access token.
    /// </summary>
    /// <param name="token">Access token string to set.</param>
    void set_access_token(utility::string_t access_token) { m_token = std::move(access_token); }

    /// <summary>
    /// Get token secret.
    /// </summary>
    /// <returns>Token secret string.</returns>
    const utility::string_t& secret() const { return m_secret; }
    /// <summary>
    /// Set token secret.
    /// </summary>
    /// <param name="secret">Token secret string to set.</param>
    void set_secret(utility::string_t secret) { m_secret = std::move(secret); }

private:
    friend class oauth1_config;

    oauth1_token() {}

    utility::string_t m_token;
    utility::string_t m_secret;
};

/// <summary>
/// OAuth 1.0 configuration class.
/// </summary>
class oauth1_config
{
public:
    oauth1_config(utility::string_t consumer_key, utility::string_t consumer_secret,
            utility::string_t temp_endpoint, utility::string_t auth_endpoint,
            utility::string_t token_endpoint, utility::string_t callback_uri,
            oauth1_method method, utility::string_t realm=utility::string_t()) :
        m_consumer_key(std::move(consumer_key)),
        m_consumer_secret(std::move(consumer_secret)),
        m_temp_endpoint(std::move(temp_endpoint)),
        m_auth_endpoint(std::move(auth_endpoint)),
        m_token_endpoint(std::move(token_endpoint)),
        m_callback_uri(std::move(callback_uri)),
        m_realm(std::move(realm)),
        m_method(std::move(method)),
        m_is_authorization_completed(false)
    {}

    /// <summary>
    /// Builds an authorization URI to be loaded in a web browser/view.
    /// The URI is built with auth_endpoint() as basis.
    /// The method creates a task for HTTP request to first obtain a
    /// temporary token. The authorization URI build based on this token.
    /// </summary>
    /// <returns>Authorization URI to be loaded in a web browser/view.</returns>
    _ASYNCRTIMP pplx::task<utility::string_t> build_authorization_uri();

    /// <summary>
    /// Fetch an access token based on redirected URI.
    /// The URI is expected to contain 'oauth_verifier'
    /// parameter, which is then used to fetch an access token using the
    /// token_from_verifier() method.
    /// See: http://tools.ietf.org/html/rfc5849#section-2.2
    /// The received 'oauth_token' is parsed and verified to match the current token().
    /// When access token is successfully obtained, set_token() is called, and config is
    /// ready for use by oauth1_handler.
    /// </summary>
    /// <param name="redirected_uri">The URI where web browser/view was redirected after resource owner's authorization.</param>
    /// <returns>Task that fetches the access token based on redirected URI.</returns>
    _ASYNCRTIMP pplx::task<void> token_from_redirected_uri(const web::http::uri& redirected_uri);

    /// <summary>
    /// Creates a task with HTTP request to fetch an access token from the token endpoint.
    /// The request exchanges a verifier code to an access token.
    /// If successful, the resulting token is set as active via set_token().
    /// See: http://tools.ietf.org/html/rfc5849#section-2.3
    /// </summary>
    /// <param name="verifier">Verifier received via redirect upon successful authorization.</param>
    /// <returns>Task that fetches the access token based on the verifier.</returns>
    pplx::task<void> token_from_verifier(utility::string_t verifier)
    {
        return _request_token(_generate_auth_state(details::oauth1_strings::verifier, uri::encode_data_string(std::move(verifier))), false);
    }

    /// <summary>
    /// Get consumer key used in authorization and authentication.
    /// </summary>
    /// <returns>Consumer key string.</returns>
    const utility::string_t& consumer_key() const { return m_consumer_key; }
    /// <summary>
    /// Set consumer key used in authorization and authentication.
    /// </summary>
    /// <param name="key">Consumer key string to set.</param>
    void set_consumer_key(utility::string_t key) { m_consumer_key = std::move(key); }

    /// <summary>
    /// Get consumer secret used in authorization and authentication.
    /// </summary>
    /// <returns>Consumer secret string.</returns>
    const utility::string_t& consumer_secret() const { return m_consumer_secret; }
    /// <summary>
    /// Set consumer secret used in authorization and authentication.
    /// </summary>
    /// <param name="secret">Consumer secret string to set.</param>
    void set_consumer_secret(utility::string_t secret) { m_consumer_secret = std::move(secret); }

    /// <summary>
    /// Get temporary token endpoint URI string.
    /// </summary>
    /// <returns>Temporary token endpoint URI string.</returns>
    const utility::string_t& temp_endpoint() const { return m_temp_endpoint; }
    /// <summary>
    /// Set temporary token endpoint URI string.
    /// </summary>
    /// <param name="temp_endpoint">Temporary token endpoint URI string to set.</param>
    void set_temp_endpoint(utility::string_t temp_endpoint) { m_temp_endpoint = std::move(temp_endpoint); }

    /// <summary>
    /// Get authorization endpoint URI string.
    /// </summary>
    /// <returns>Authorization endpoint URI string.</returns>
    const utility::string_t& auth_endpoint() const { return m_auth_endpoint; }
    /// <summary>
    /// Set authorization endpoint URI string.
    /// </summary>
    /// <param name="auth_endpoint">Authorization endpoint URI string to set.</param>
    void set_auth_endpoint(utility::string_t auth_endpoint) { m_auth_endpoint = std::move(auth_endpoint); }

    /// <summary>
    /// Get token endpoint URI string.
    /// </summary>
    /// <returns>Token endpoint URI string.</returns>
    const utility::string_t& token_endpoint() const { return m_token_endpoint; }
    /// <summary>
    /// Set token endpoint URI string.
    /// </summary>
    /// <param name="token_endpoint">Token endpoint URI string to set.</param>
    void set_token_endpoint(utility::string_t token_endpoint) { m_token_endpoint = std::move(token_endpoint); }

    /// <summary>
    /// Get callback URI string.
    /// </summary>
    /// <returns>Callback URI string.</returns>
    const utility::string_t& callback_uri() const { return m_callback_uri; }
    /// <summary>
    /// Set callback URI string.
    /// </summary>
    /// <param name="callback_uri">Callback URI string to set.</param>
    void set_callback_uri(utility::string_t callback_uri) { m_callback_uri = std::move(callback_uri); }

    /// <summary>
    /// Get token.
    /// </summary>
    /// <returns>Token.</returns>
    const oauth1_token& token() const
    {
        if (m_is_authorization_completed)
        {
            // Return the token object only if authorization has been completed.
            // Otherwise the token object holds a temporary token which should not be
            // returned to the user.
            return m_token;
        }
        else
        {
            static const oauth1_token empty_token;
            return empty_token;
        }
    }
    /// <summary>
    /// Set token.
    /// </summary>
    /// <param name="token">Token to set.</param>
    void set_token(oauth1_token token)
    {
        m_token = std::move(token);
        m_is_authorization_completed = true;
    }

    /// <summary>
    /// Get signature method.
    /// </summary>
    /// <returns>Signature method.</returns>
    const oauth1_method& method() const { return m_method; }
    /// <summary>
    /// Set signature method.
    /// </summary>
    /// <param name="method">Signature method.</param>
    void set_method(oauth1_method method) { m_method = std::move(method); }

    /// <summary>
    /// Get authentication realm.
    /// </summary>
    /// <returns>Authentication realm string.</returns>
    const utility::string_t& realm() const { return m_realm; }
    /// <summary>
    /// Set authentication realm.
    /// </summary>
    /// <param name="realm">Authentication realm string to set.</param>
    void set_realm(utility::string_t realm) { m_realm = std::move(realm); }

    /// <summary>
    /// Returns enabled state of the configuration.
    /// The oauth1_handler will perform OAuth 1.0 authentication only if
    /// this method returns true.
    /// Return value is true if access token is valid (=fetched or manually set)
    /// and both consumer_key() and consumer_secret() are set (=non-empty).
    /// </summary>
    /// <returns>The configuration enabled state.</returns>
    bool is_enabled() const { return token().is_valid_access_token() && !(consumer_key().empty() || consumer_secret().empty()); }

    // Builds signature base string according to:
    // http://tools.ietf.org/html/rfc5849#section-3.4.1.1
    _ASYNCRTIMP utility::string_t _build_signature_base_string(http_request request, details::oauth1_state state) const;

    // Builds HMAC-SHA1 signature according to:
    // http://tools.ietf.org/html/rfc5849#section-3.4.2
    utility::string_t _build_hmac_sha1_signature(http_request request, details::oauth1_state state) const
    {
        auto text(_build_signature_base_string(std::move(request), std::move(state)));
        auto digest(_hmac_sha1(_build_key(), std::move(text)));
        auto signature(utility::conversions::to_base64(std::move(digest)));
        return signature;
    }

    // Builds PLAINTEXT signature according to:
    // http://tools.ietf.org/html/rfc5849#section-3.4.4
    utility::string_t _build_plaintext_signature() const
    {
        return _build_key();
    }

    details::oauth1_state _generate_auth_state(utility::string_t extra_key, utility::string_t extra_value)
    {
        return details::oauth1_state(_generate_timestamp(), _generate_nonce(), std::move(extra_key), std::move(extra_value));
    }

    details::oauth1_state _generate_auth_state()
    {
        return details::oauth1_state(_generate_timestamp(), _generate_nonce());
    }

private:
    friend class web::http::client::http_client_config;
    friend class web::http::oauth1::details::oauth1_handler;

    oauth1_config() :
        m_is_authorization_completed(false)
    {}

    utility::string_t _generate_nonce()
    {
        return m_nonce_generator.generate();
    }

    static utility::string_t _generate_timestamp()
    {
        utility::ostringstream_t os;
        os << utility::datetime::utc_timestamp();
        return os.str();
    }

    _ASYNCRTIMP static std::vector<unsigned char> _hmac_sha1(const utility::string_t& key, const utility::string_t& data);

    static utility::string_t _build_base_string_uri(const uri& u);

    utility::string_t _build_normalized_parameters(web::http::uri u, const details::oauth1_state& state) const;

    utility::string_t _build_signature(http_request request, details::oauth1_state state) const;

    utility::string_t _build_key() const
    {
        return uri::encode_data_string(consumer_secret()) + _XPLATSTR("&") + uri::encode_data_string(m_token.secret());
    }

    void _authenticate_request(http_request &req)
    {
        _authenticate_request(req, _generate_auth_state());
    }
    
    _ASYNCRTIMP void _authenticate_request(http_request &req, details::oauth1_state state);

    _ASYNCRTIMP pplx::task<void> _request_token(details::oauth1_state state, bool is_temp_token_request);

    utility::string_t m_consumer_key;
    utility::string_t m_consumer_secret;
    oauth1_token m_token;
    bool m_is_authorization_completed;
    oauth1_method m_method;
    utility::string_t m_realm;

    utility::string_t m_temp_endpoint;
    utility::string_t m_auth_endpoint;
    utility::string_t m_token_endpoint;
    utility::string_t m_callback_uri;

    utility::nonce_generator m_nonce_generator;
};

} // namespace web::http::oauth1::experimental

namespace details
{

class oauth1_handler : public http_pipeline_stage
{
public:
    oauth1_handler(std::shared_ptr<experimental::oauth1_config> cfg) :
        m_config(std::move(cfg))
    {}

    virtual pplx::task<http_response> propagate(http_request request) override
    {
        if (m_config)
        {
            m_config->_authenticate_request(request);
        }
        return next_stage()->propagate(request);
    }

private:
    std::shared_ptr<experimental::oauth1_config> m_config;
};

}}}} // namespace web::http::oauth1::details

#endif  /* _CASA_OAUTH1_H */
