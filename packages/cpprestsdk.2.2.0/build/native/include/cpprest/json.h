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
* json.h
*
* HTTP Library: JSON parser and writer
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#pragma once

#ifndef _CASA_JSON_H
#define _CASA_JSON_H

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "cpprest/xxpublic.h"
#include "cpprest/basic_types.h"
#include "cpprest/asyncrt_utils.h"

namespace web { namespace json
{

    // Various forward declarations.
    namespace details 
    {
        class _Value; 
        class _Number; 
        class _Null; 
        class _Boolean; 
        class _String; 
        class _Object; 
        class _Array;
        template <typename CharType> class JSON_Parser;
    }

    namespace details
    {
        extern bool g_keep_json_object_unsorted;
    }

    /// <summary>
    /// Preserve the order of the name/value pairs when parsing a JSON object
    /// </summary>
    void _ASYNCRTIMP keep_object_element_order(bool keep_order);

#ifdef _MS_WINDOWS
#ifdef _DEBUG
#define ENABLE_JSON_VALUE_VISUALIZER
#endif
#endif

    class number;
    class array;
    class object;

    /// <summary>
    /// A JSON value represented as a C++ class.
    /// </summary>
    class value
    {
    public:
        /// <summary>
        /// This enumeration represents the various kinds of JSON values.
        /// </summary>
        enum value_type { Number, Boolean, String, Object, Array, Null };

        /// <summary>
        /// Constructor creating a null value
        /// </summary>
        _ASYNCRTIMP value();

        /// <summary>
        /// Constructor creating a JSON number value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        _ASYNCRTIMP value(int32_t value);

        /// <summary>
        /// Constructor creating a JSON number value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        _ASYNCRTIMP value(uint32_t value);

        /// <summary>
        /// Constructor creating a JSON number value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        _ASYNCRTIMP value(int64_t value);

        /// <summary>
        /// Constructor creating a JSON number value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        _ASYNCRTIMP value(uint64_t value);

        /// <summary>
        /// Constructor creating a JSON number value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        _ASYNCRTIMP value(double value);

        /// <summary>
        /// Constructor creating a JSON Boolean value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        _ASYNCRTIMP explicit value(bool value);

        /// <summary>
        /// Constructor creating a JSON string value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from, a C++ STL string of the platform-native character width</param>
        _ASYNCRTIMP explicit value(utility::string_t);

        /// <summary>
        /// Constructor creating a JSON string value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from, a C++ STL string of the platform-native character width</param>
        /// <remarks>This constructor exists in order to avoid string literals matching another constructor,
        /// as is very likely. For example, conversion to bool does not require a user-defined conversion,
        /// and will therefore match first, which means that the JSON value turns up as a boolean.</remarks>
        _ASYNCRTIMP explicit value(const utility::char_t *);

        /// <summary>
        /// Copy constructor
        /// </summary>
        _ASYNCRTIMP value(const value &);

        /// <summary>
        /// Move constructor
        /// </summary>
        _ASYNCRTIMP value(value &&);

        /// <summary>
        /// Assignment operator.
        /// </summary>
        /// <returns>The JSON value object that contains the result of the assignment.</returns>
        _ASYNCRTIMP value &operator=(const value &);

        /// <summary>
        /// Move assignment operator.
        /// </summary>
        /// <returns>The JSON value object that contains the result of the assignment.</returns>
        _ASYNCRTIMP value &operator=(value &&);

        // Static factories

        /// <summary>
        /// Creates a null value
        /// </summary>
        /// <returns>A JSON null value</returns>
        static _ASYNCRTIMP value __cdecl null();

        /// <summary>
        /// Creates a number value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        /// <returns>A JSON number value</returns>
        static _ASYNCRTIMP value __cdecl number(double value);

        /// <summary>
        /// Creates a number value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        /// <returns>A JSON number value</returns>
        static _ASYNCRTIMP value __cdecl number(int32_t value);

        /// <summary>
        /// Creates a Boolean value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        /// <returns>A JSON Boolean value</returns>
        static _ASYNCRTIMP value __cdecl boolean(bool value);

        /// <summary>
        /// Creates a string value
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from</param>
        /// <returns>A JSON string value</returns>
        static _ASYNCRTIMP value __cdecl string(utility::string_t value);

#ifdef _MS_WINDOWS
private:
        // Only used internally by JSON parser.
        static _ASYNCRTIMP value __cdecl string(const std::string &value);
public:
#endif

        /// <summary>
        /// Creates an object value
        /// </summary>
        /// <param name="keep_order">Whether to preserve the original order of the fields</param>
        /// <returns>An empty JSON object value</returns>
        static _ASYNCRTIMP json::value __cdecl object(bool keep_order = false);

        /// <summary>
        /// Creates an object value from a collection of field/values
        /// </summary>
        /// <param name="fields">Field names associated with JSON values</param>
        /// <param name="keep_order">Whether to preserve the original order of the fields</param>
        /// <returns>A non-empty JSON object value</returns>
        static _ASYNCRTIMP json::value __cdecl object(std::vector<std::pair<::utility::string_t, value>> fields, bool keep_order = false);

        /// <summary>
        /// Creates an empty JSON array
        /// </summary>
        /// <returns>An empty JSON array value</returns>
        static _ASYNCRTIMP json::value __cdecl array();

        /// <summary>
        /// Creates a JSON array
        /// </summary>
        /// <param name="size">The initial number of elements of the JSON value</param>
        /// <returns>A JSON array value</returns>
        static _ASYNCRTIMP json::value __cdecl array(size_t size);

        /// <summary>
        /// Creates a JSON array
        /// </summary>
        /// <param name="elements">A vector of JSON values</param>
        /// <returns>A JSON array value</returns>
        static _ASYNCRTIMP json::value __cdecl array(std::vector<value> elements);

        /// <summary>
        /// Accesses the type of JSON value the current value instance is
        /// </summary>
        /// <returns>The value's type</returns>
        _ASYNCRTIMP json::value::value_type __cdecl type() const;

        /// <summary>
        /// Is the current value a null value?
        /// </summary>
        /// <returns><c>true</c> if the value is a null value, <c>false</c> otherwise</returns>
        bool is_null() const { return type() == Null; };

        /// <summary>
        /// Is the current value a number value?
        /// </summary>
        /// <returns><c>true</c> if the value is a number value, <c>false</c> otherwise</returns>
        bool is_number() const { return type() == Number; }

        /// <summary>
        /// Is the current value represented as an integer number value?
        /// </summary>
        /// <remarks>
        /// Note that if a json value is a number but represented as a double it can still
        /// be retrieved as a integer using as_integer(), however the value will be truncated.
        /// </remarks>
        /// <returns><c>true</c> if the value is an integer value, <c>false</c> otherwise.</returns>
        _ASYNCRTIMP bool is_integer() const;

        /// <summary>
        /// Is the current value represented as an double number value?
        /// </summary>
        /// <remarks>
        /// Note that if a json value is a number but represented as a int it can still
        /// be retrieved as a double using as_double().
        /// </remarks>
        /// <returns><c>true</c> if the value is an double value, <c>false</c> otherwise.</returns>
        _ASYNCRTIMP bool is_double() const;

        /// <summary>
        /// Is the current value a Boolean value?
        /// </summary>
        /// <returns><c>true</c> if the value is a Boolean value, <c>false</c> otherwise</returns>
        bool is_boolean() const { return type() == Boolean; }

        /// <summary>
        /// Is the current value a string value?
        /// </summary>
        /// <returns><c>true</c> if the value is a string value, <c>false</c> otherwise</returns>
        bool is_string() const { return type() == String; }

        /// <summary>
        /// Is the current value an array?
        /// </summary>
        /// <returns><c>true</c> if the value is an array, <c>false</c> otherwise</returns>
        bool is_array() const { return type() == Array; }

        /// <summary>
        /// Is the current value an object?
        /// </summary>
        /// <returns><c>true</c> if the value is an object, <c>false</c> otherwise</returns>
        bool is_object() const { return type() == Object; }

        /// <summary>
        /// Gets the number of children of the value.
        /// </summary>
        /// <returns>The number of children. 0 for all non-composites.</returns>
        size_t size() const;

        /// <summary>
        /// Parses a string and construct a JSON value.
        /// </summary>
        /// <param name="value">The C++ value to create a JSON value from, a C++ STL double-byte string</param>
        _ASYNCRTIMP static value parse(const utility::string_t&);

        /// <summary>
        /// Serializes the current JSON value to a C++ string.
        /// </summary>
        /// <returns>A string representation of the value</returns>
        _ASYNCRTIMP utility::string_t serialize() const;

        /// <summary>
        /// Serializes the current JSON value to a C++ string.
        /// </summary>
        /// <returns>A string representation of the value</returns>
        CASABLANCA_DEPRECATED("This API is deprecated and has been renamed to avoid confusion with as_string(), use ::web::json::value::serialize() instead.")
        _ASYNCRTIMP utility::string_t to_string() const;

        /// <summary>
        /// Parses a JSON value from the contents of an input stream using the native platform character width.
        /// </summary>
        /// <param name="input">The stream to read the JSON value from</param>
        /// <returns>The JSON value object created from the input stream.</returns>
        _ASYNCRTIMP static value parse(utility::istream_t &input);

        /// <summary>
        /// Writes the current JSON value to a stream with the native platform character width.
        /// </summary>
        /// <param name="stream">The stream that the JSON string representation should be written to.</param>
        _ASYNCRTIMP void serialize(utility::ostream_t &stream) const;

#ifdef _MS_WINDOWS
        /// <summary>
        /// Parses a JSON value from the contents of a single-byte (UTF8) stream.
        /// </summary>
        /// <param name="stream">The stream to read the JSON value from</param>
        _ASYNCRTIMP static value parse(std::istream& stream);

        /// <summary>
        /// Serializes the content of the value into a single-byte (UTF8) stream.
        /// </summary>
        /// <param name="stream">The stream that the JSON string representation should be written to.</param>
        _ASYNCRTIMP void serialize(std::ostream& stream) const;
#endif

        /// <summary>
        /// Converts the JSON value to a C++ double, if and only if it is a number value.
        /// Throws <see cref="json_exception"/>  if the value is not a number
        /// </summary>
        /// <returns>A double representation of the value</returns>
        _ASYNCRTIMP double as_double() const;

        /// <summary>
        /// Converts the JSON value to a C++ integer, if and only if it is a number value.
        /// Throws <see cref="json_exception"/> if the value is not a number
        /// </summary>
        /// <returns>An integer representation of the value</returns>
        _ASYNCRTIMP int as_integer() const;

        /// <summary>
        /// Converts the JSON value to a number class, if and only if it is a number value.
        /// Throws <see cref="json_exception"/>  if the value is not a number
        /// </summary>
        /// <returns>An instance of number class</returns>
        _ASYNCRTIMP json::number as_number() const;

        /// <summary>
        /// Converts the JSON value to a C++ bool, if and only if it is a Boolean value.
        /// </summary>
        /// <returns>A C++ bool representation of the value</returns>
        _ASYNCRTIMP bool as_bool() const;

        /// <summary>
        /// Converts the JSON value to a json array, if and only if it is an array value.
        /// </summary>
        /// <remarks>The returned <c>json::array</c> should have the same or shorter lifetime as <c>this</c></remarks>
        /// <returns>An array representation of the value</returns>
        _ASYNCRTIMP json::array& as_array();

        /// <summary>
        /// Converts the JSON value to a json array, if and only if it is an array value.
        /// </summary>
        /// <remarks>The returned <c>json::array</c> should have the same or shorter lifetime as <c>this</c></remarks>
        /// <returns>An array representation of the value</returns>
        _ASYNCRTIMP const json::array& as_array() const;

        /// <summary>
        /// Converts the JSON value to a json object, if and only if it is an object value.
        /// </summary>
        /// <returns>An object representation of the value</returns>
        _ASYNCRTIMP json::object& as_object();

        /// <summary>
        /// Converts the JSON value to a json object, if and only if it is an object value.
        /// </summary>
        /// <returns>An object representation of the value</returns>
        _ASYNCRTIMP const json::object& as_object() const;

        /// <summary>
        /// Converts the JSON value to a C++ STL string, if and only if it is a string value.
        /// </summary>
        /// <returns>A C++ STL string representation of the value</returns>
        _ASYNCRTIMP utility::string_t as_string() const;

        /// <summary>
        /// Compares two JSON values for equality.
        /// </summary>
        /// <param name="other">The JSON value to compare with.</param>
        /// <returns>True iff the values are equal.</returns>
        _ASYNCRTIMP bool operator==(const value& other) const;
            
        /// <summary>
        /// Compares two JSON values for inequality.
        /// </summary>
        /// <param name="other">The JSON value to compare with.</param>
        /// <returns>True iff the values are unequal.</returns>
        bool operator!=(const value& other) const
        {
            return !((*this) == other);
        }

        /// <summary>
        /// Tests for the presence of a field.
        /// </summary>
        /// <param name="key">The name of the field</param>
        /// <returns>True if the field exists, false otherwise.</returns>
        bool has_field(const utility::string_t &key) const;

        /// <summary>
        /// Accesses a field of a JSON object.
        /// </summary>
        /// <param name="key">The name of the field</param>
        /// <returns>The value kept in the field; null if the field does not exist</returns>
        CASABLANCA_DEPRECATED("This API is deprecated and will be removed in a future release, use json::value::at() instead.")
        value get(const utility::string_t &key) const;

        /// <summary>
        /// Accesses an element of a JSON array. Throws when index out of bounds.
        /// </summary>
        /// <param name="index">The index of an element in the JSON array.</param>
        /// <returns>A reference to the value.</returns>
        _ASYNCRTIMP json::value& at(size_t index);

        /// <summary>
        /// Accesses an element of a JSON array. Throws when index out of bounds.
        /// </summary>
        /// <param name="index">The index of an element in the JSON array.</param>
        /// <returns>A reference to the value.</returns>
        _ASYNCRTIMP const json::value& at(size_t index) const;

        /// <summary>
        /// Accesses an element of a JSON object. If the key doesn't exist, this method throws.
        /// </summary>
        /// <param name="key">The key of an element in the JSON object.</param>
        /// <returns>If the key exists, a reference to the value.</returns>
        _ASYNCRTIMP json::value& at(const utility::string_t& key);
        
        /// <summary>
        /// Accesses an element of a JSON object. If the key doesn't exist, this method throws.
        /// </summary>
        /// <param name="key">The key of an element in the JSON object.</param>
        /// <returns>If the key exists, a reference to the value.</returns>
        _ASYNCRTIMP const json::value& at(const utility::string_t& key) const;

        /// <summary>
        /// Accesses a field of a JSON object.
        /// </summary>
        /// <param name="key">The name of the field</param>
        /// <returns>A reference to the value kept in the field.</returns>
        _ASYNCRTIMP value & operator [] (const utility::string_t &key);

#ifdef _MS_WINDOWS
private:
        // Only used internally by JSON parser
        _ASYNCRTIMP value & operator [] (const std::string &key)
        {
            // JSON object stores its field map as a unordered_map of string_t, so this conversion is hard to avoid
            return operator[](utility::conversions::to_string_t(key));
        }
public:
#endif
        
        /// <summary>
        /// Accesses an element of a JSON array.
        /// </summary>
        /// <param name="key">The index of an element in the JSON array</param>
        /// <returns>The value kept at the array index; null if outside the boundaries of the array</returns>
        CASABLANCA_DEPRECATED("This API is deprecated and will be removed in a future release, use json::value::at() instead.")
        value get(size_t index) const;

        /// <summary>
        /// Accesses an element of a JSON array.
        /// </summary>
        /// <param name="index">The index of an element in the JSON array.</param>
        /// <returns>A reference to the value kept in the field.</returns>
        _ASYNCRTIMP value & operator [] (size_t index);

    private:
        friend class web::json::details::_Object;
        friend class web::json::details::_Array;
        template<typename CharType> friend class web::json::details::JSON_Parser;

#ifdef _MS_WINDOWS
        /// <summary>
        /// Writes the current JSON value as a double-byte string to a string instance.
        /// </summary>
        /// <param name="string">The string that the JSON representation should be written to.</param>
        _ASYNCRTIMP void format(std::basic_string<utf16char> &string) const;
#endif
        /// <summary>
        /// Serializes the content of the value into a string instance in UTF8 format
        /// </summary>
        /// <param name="string">The string that the JSON representation should be written to</param>
        _ASYNCRTIMP void format(std::basic_string<char>& string) const;

#ifdef ENABLE_JSON_VALUE_VISUALIZER
        explicit value(std::unique_ptr<details::_Value> v, value_type kind) : m_value(std::move(v)), m_kind(kind)
#else
        explicit value(std::unique_ptr<details::_Value> v) : m_value(std::move(v))
#endif
        {}

        std::unique_ptr<details::_Value> m_value;
#ifdef ENABLE_JSON_VALUE_VISUALIZER
        value_type m_kind;
#endif
    };

    /// <summary>
    /// A single exception type to represent errors in parsing, converting, and accessing
    /// elements of JSON values.
    /// </summary>
    class json_exception : public std::exception
    { 
    private:
        std::string _message;
    public:
        json_exception() {}
        json_exception(const utility::char_t * const &message) : _message(utility::conversions::to_utf8string(message)) { }

        // Must be narrow string because it derives from std::exception
        const char* what() const _noexcept
        {
            return _message.c_str();
        }
        ~json_exception() _noexcept {}
    };

    /// <summary>
    /// A JSON array represented as a C++ class.
    /// </summary>
    class array
    {
        typedef std::vector<json::value> storage_type;

    public:
        typedef storage_type::iterator iterator;
        typedef storage_type::const_iterator const_iterator;
        typedef storage_type::reverse_iterator reverse_iterator;
        typedef storage_type::const_reverse_iterator const_reverse_iterator;
        typedef storage_type::size_type size_type;
    
    private:
        array() : m_elements() { }
        array(size_type size) : m_elements(size) { }
        array(storage_type elements) : m_elements(std::move(elements)) { }

    public:
        /// <summary>
        /// Gets the beginning iterator element of the array
        /// </summary>
        /// <returns>An <c>iterator</c> to the beginning of the JSON array.</returns>
        iterator begin()
        {
            return m_elements.begin();
        }

        /// <summary>
        /// Gets the beginning const iterator element of the array.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the beginning of the JSON array.</returns>
        const_iterator begin() const
        {
            return m_elements.cbegin();
        }

        /// <summary>
        /// Gets the end iterator element of the array
        /// </summary>
        /// <returns>An <c>iterator</c> to the end of the JSON array.</returns>
        iterator end()
        {
            return m_elements.end();
        }

        /// <summary>
        /// Gets the end const iterator element of the array.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the end of the JSON array.</returns>
        const_iterator end() const
        {
            return m_elements.cend();
        }

        /// <summary>
        /// Gets the beginning reverse iterator element of the array
        /// </summary>
        /// <returns>An <c>reverse_iterator</c> to the beginning of the JSON array.</returns>
        reverse_iterator rbegin()
        {
            return m_elements.rbegin();
        }

        /// <summary>
        /// Gets the beginning const reverse iterator element of the array
        /// </summary>
        /// <returns>An <c>const_reverse_iterator</c> to the beginning of the JSON array.</returns>
        const_reverse_iterator rbegin() const
        {
            return m_elements.rbegin();
        }

        /// <summary>
        /// Gets the end reverse iterator element of the array
        /// </summary>
        /// <returns>An <c>reverse_iterator</c> to the end of the JSON array.</returns>
        reverse_iterator rend()
        {
            return m_elements.rend();
        }

        /// <summary>
        /// Gets the end const reverse iterator element of the array
        /// </summary>
        /// <returns>An <c>const_reverse_iterator</c> to the end of the JSON array.</returns>
        const_reverse_iterator rend() const
        {
            return m_elements.crend();
        }

        /// <summary>
        /// Gets the beginning const iterator element of the array.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the beginning of the JSON array.</returns>
        const_iterator cbegin() const
        {
            return m_elements.cbegin();
        }

        /// <summary>
        /// Gets the end const iterator element of the array.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the end of the JSON array.</returns>
        const_iterator cend() const
        {
            return m_elements.cend();
        }

        /// <summary>
        /// Gets the beginning const reverse iterator element of the array.
        /// </summary>
        /// <returns>A <c>const_reverse_iterator</c> to the beginning of the JSON array.</returns>
        const_reverse_iterator crbegin() const
        {
            return m_elements.crbegin();
        }

        /// <summary>
        /// Gets the end const reverse iterator element of the array.
        /// </summary>
        /// <returns>A <c>const_reverse_iterator</c> to the end of the JSON array.</returns>
        const_reverse_iterator crend() const
        {
            return m_elements.crend();
        }

        /// <summary>
        /// Accesses an element of a JSON array. Throws when index out of bounds.
        /// </summary>
        /// <param name="index">The index of an element in the JSON array.</param>
        /// <returns>A reference to the value kept in the field.</returns>
        json::value& at(size_type index)
        {
            if (index >= m_elements.size())
                throw json_exception(_XPLATSTR("index out of bounds"));

            return m_elements[index];
        }

        /// <summary>
        /// Accesses an element of a JSON array. Throws when index out of bounds.
        /// </summary>
        /// <param name="index">The index of an element in the JSON array.</param>
        /// <returns>A reference to the value kept in the field.</returns>
        const json::value& at(size_type index) const
        {
            if (index >= m_elements.size())
                throw json_exception(_XPLATSTR("index out of bounds"));

            return m_elements[index];
        }

        /// <summary>
        /// Accesses an element of a JSON array.
        /// </summary>
        /// <param name="index">The index of an element in the JSON array.</param>
        /// <returns>A reference to the value kept in the field.</returns>
        json::value& operator[](size_type index)
        {
            SafeInt<size_type> nMinSize(index);
            nMinSize += 1;
            SafeInt<size_type> nlastSize(m_elements.size());
            if (nlastSize < nMinSize)
                m_elements.resize(nMinSize);

            return m_elements[index];
        }

        /// <summary>
        /// Gets the number of elements of the array.
        /// </summary>
        /// <returns>The number of elements.</returns>
        size_type size() const
        {
            return m_elements.size();
        }

    private:
        storage_type m_elements;

        friend class details::_Array;
        template<typename CharType> friend class json::details::JSON_Parser;
    };

    /// <summary>
    /// A JSON object represented as a C++ class.
    /// </summary>
    class object
    {
        typedef std::vector<std::pair<utility::string_t, json::value>> storage_type;

    public:
        typedef storage_type::iterator iterator;
        typedef storage_type::const_iterator const_iterator;
        typedef storage_type::reverse_iterator reverse_iterator;
        typedef storage_type::const_reverse_iterator const_reverse_iterator;
        typedef storage_type::size_type size_type;

    private:
        object(bool keep_order = false) : m_elements(), m_keep_order(keep_order) { }
        object(storage_type elements, bool keep_order = false) : m_elements(std::move(elements)), m_keep_order(keep_order)
        {
            if (!keep_order) {
                sort(m_elements.begin(), m_elements.end(), compare_pairs);
            }
        }
        object(const object& obj); // non copyable
        object& operator=(const object& obj); // non copyable

    public:
        /// <summary>
        /// Gets the beginning iterator element of the object
        /// </summary>
        /// <returns>An <c>iterator</c> to the beginning of the JSON object.</returns>
        iterator begin()
        {
            return m_elements.begin();
        }

        /// <summary>
        /// Gets the beginning const iterator element of the object.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the beginning of the JSON object.</returns>
        const_iterator begin() const
        {
            return m_elements.cbegin();
        }

        /// <summary>
        /// Gets the end iterator element of the object
        /// </summary>
        /// <returns>An <c>iterator</c> to the end of the JSON object.</returns>
        iterator end()
        {
            return m_elements.end();
        }

        /// <summary>
        /// Gets the end const iterator element of the object.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the end of the JSON object.</returns>
        const_iterator end() const
        {
            return m_elements.cend();
        }

        /// <summary>
        /// Gets the beginning reverse iterator element of the object
        /// </summary>
        /// <returns>An <c>reverse_iterator</c> to the beginning of the JSON object.</returns>
        reverse_iterator rbegin()
        {
            return m_elements.rbegin();
        }

        /// <summary>
        /// Gets the beginning const reverse iterator element of the object
        /// </summary>
        /// <returns>An <c>const_reverse_iterator</c> to the beginning of the JSON object.</returns>
        const_reverse_iterator rbegin() const
        {
            return m_elements.rbegin();
        }

        /// <summary>
        /// Gets the end reverse iterator element of the object
        /// </summary>
        /// <returns>An <c>reverse_iterator</c> to the end of the JSON object.</returns>
        reverse_iterator rend()
        {
            return m_elements.rend();
        }

        /// <summary>
        /// Gets the end const reverse iterator element of the object
        /// </summary>
        /// <returns>An <c>const_reverse_iterator</c> to the end of the JSON object.</returns>
        const_reverse_iterator rend() const
        {
            return m_elements.crend();
        }

        /// <summary>
        /// Gets the beginning const iterator element of the object.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the beginning of the JSON object.</returns>
        const_iterator cbegin() const
        {
            return m_elements.cbegin();
        }

        /// <summary>
        /// Gets the end const iterator element of the object.
        /// </summary>
        /// <returns>A <c>const_iterator</c> to the end of the JSON object.</returns>
        const_iterator cend() const
        {
            return m_elements.cend();
        }

        /// <summary>
        /// Gets the beginning const reverse iterator element of the object.
        /// </summary>
        /// <returns>A <c>const_reverse_iterator</c> to the beginning of the JSON object.</returns>
        const_reverse_iterator crbegin() const
        {
            return m_elements.crbegin();
        }

        /// <summary>
        /// Gets the end const reverse iterator element of the object.
        /// </summary>
        /// <returns>A <c>const_reverse_iterator</c> to the end of the JSON object.</returns>
        const_reverse_iterator crend() const
        {
            return m_elements.crend();
        }

        /// <summary>
        /// Accesses an element of a JSON object. If the key doesn't exist, this method throws.
        /// </summary>
        /// <param name="key">The key of an element in the JSON object.</param>
        /// <returns>If the key exists, a reference to the value kept in the field.</returns>
        json::value& at(const utility::string_t& key)
        {
            auto iter = find_by_key(key);

            if (iter == m_elements.end() || key != (iter->first))
                throw web::json::json_exception(_XPLATSTR("Key not found"));

            return iter->second;
        }

        /// <summary>
        /// Accesses an element of a JSON object. If the key doesn't exist, this method throws.
        /// </summary>
        /// <param name="key">The key of an element in the JSON object.</param>
        /// <returns>If the key exists, a reference to the value kept in the field.</returns>
        const json::value& at(const utility::string_t& key) const
        {
            auto iter = find_by_key(key);

            if (iter == m_elements.end() || key != (iter->first))
                throw web::json::json_exception(_XPLATSTR("Key not found"));

            return iter->second;
        }

        /// <summary>
        /// Accesses an element of a JSON object.
        /// </summary>
        /// <param name="key">The key of an element in the JSON object.</param>
        /// <returns>If the key exists, a reference to the value kept in the field, otherwise a newly created null value that will be stored for the given key.</returns>
        json::value& operator[](const utility::string_t& key)
        {
            auto iter = find_by_key(key);

            if (iter == m_elements.end() || key != (iter->first))
                return m_elements.insert(iter, std::pair<utility::string_t, value>(key, value()))->second;

            return iter->second;
        }

        /// <summary>
        /// Gets an iterator to an element of a JSON object.
        /// </summary>
        /// <param name="key">The key of an element in the JSON object.</param>
        /// <returns>A const iterator to the value kept in the field.</returns>
        const_iterator find(const utility::string_t& key) const
        {
            return find_internal(key);
        }

        /// <summary>
        /// Gets the number of elements of the object.
        /// </summary>
        /// <returns>The number of elements.</returns>
        size_type size() const
        {
            return m_elements.size();
        }

        /// <summary>
        /// Checks if there are any elements in the JSON object.
        /// </summary>
        /// <returns>True iff empty.</returns>
        bool empty() const
        {
            return m_elements.empty();
        }
    private:

        static bool compare_pairs(const std::pair<utility::string_t, value>& p1, const std::pair<utility::string_t, value>& p2)
        {
            return p1.first < p2.first;
        }
        static bool compare_with_key(const std::pair<utility::string_t, value>& p1, const utility::string_t& key)
        {
            return p1.first < key;
        }

        storage_type::const_iterator find_by_key(const utility::string_t& key) const
        {
            if (m_keep_order)
            {
                return std::find_if(m_elements.begin(), m_elements.end(),
                    [&key](const std::pair<utility::string_t, value>& p) {
                    return p.first == key;
                });
            }
            else
            {
                return std::lower_bound(m_elements.begin(), m_elements.end(), key, compare_with_key);
            }
        }

        storage_type::iterator find_by_key(const utility::string_t& key)
        {
            if (m_keep_order)
            {
                return std::find_if(m_elements.begin(), m_elements.end(),
                    [&key](const std::pair<utility::string_t, value>& p) {
                    return p.first == key;
                });
            }
            else
            {
                return std::lower_bound(m_elements.begin(), m_elements.end(), key, compare_with_key);
            }
        }

        const json::value& at_internal(const utility::string_t& key) const
        {
            auto iter = find_by_key(key);

            if (iter == m_elements.end() || key != (iter->first))
                throw web::json::json_exception(_XPLATSTR("Key not found"));

            return iter->second;
        }

        const_iterator find_internal(const utility::string_t& key) const
        {
            auto iter = find_by_key(key);

            if (iter != m_elements.end() && key != (iter->first))
                return m_elements.end();

            return iter;
        }

        iterator find_internal(const utility::string_t& key)
        {
            auto iter = find_by_key(key);

            if (iter != m_elements.end() && key != (iter->first))
                return m_elements.end();

            return iter;
        }

        const bool m_keep_order;
        storage_type m_elements;
        friend class details::_Object;

        template<typename CharType> friend class json::details::JSON_Parser;
   };

    /// <summary>
    /// A JSON number represented as a C++ class.
    /// </summary>
    class number
    {
        // Note that these constructors make sure that only negative integers are stored as signed int64 (while others convert to unsigned int64).
        // This helps handling number objects e.g. comparing two numbers.

        number(double value)  : m_value(value), m_type(double_type) { }
        number(int32_t value) : m_intval(value), m_type(value < 0 ? signed_type : unsigned_type) { }
        number(uint32_t value) : m_intval(value), m_type(unsigned_type) { }
        number(int64_t value) : m_intval(value), m_type(value < 0 ? signed_type : unsigned_type) { }
        number(uint64_t value) : m_uintval(value), m_type(unsigned_type) { }

    public:

#pragma region "is" checkers
        /// <summary>
        /// Does the number fit into int32?
        /// </summary>
        /// <returns><c>true</c> if the number fits into int32, <c>false</c> otherwise</returns>
        _ASYNCRTIMP bool is_int32() const;
        
        /// <summary>
        /// Does the number fit into unsigned int32?
        /// </summary>
        /// <returns><c>true</c> if the number fits into unsigned int32, <c>false</c> otherwise</returns>
        _ASYNCRTIMP bool is_uint32() const;

        /// <summary>
        /// Does the number fit into int64?
        /// </summary>
        /// <returns><c>true</c> if the number fits into int64, <c>false</c> otherwise</returns>
        _ASYNCRTIMP bool is_int64() const;

        /// <summary>
        /// Does the number fit into unsigned int64?
        /// </summary>
        /// <returns><c>true</c> if the number fits into unsigned int64, <c>false</c> otherwise</returns>
        bool is_uint64() const
        {
            switch (m_type)
            {
            case signed_type : return m_intval >= 0;
            case unsigned_type : return true;
            case double_type :
            default :
                return false;
            }
        }
#pragma endregion

#pragma region "to" converters
        /// <summary>
        /// Converts the JSON number to a C++ double.
        /// </summary>
        /// <returns>A double representation of the number</returns>
        double to_double() const
        {
            switch (m_type)
            {
            case double_type : return m_value;
            case signed_type : return static_cast<double>(m_intval);
            case unsigned_type : return static_cast<double>(m_uintval);
            default : return false;
            }
        }

        /// <summary>
        /// Converts the JSON number to int32.
        /// </summary>
        /// <returns>An int32 representation of the number</returns>
        int32_t to_int32() const
        {
            if (m_type == double_type)
                return static_cast<int32_t>(m_value);
            else
                return static_cast<int32_t>(m_intval);
        }

        /// <summary>
        /// Converts the JSON number to unsigned int32.
        /// </summary>
        /// <returns>An usigned int32 representation of the number</returns>
        uint32_t to_uint32() const
        {
            if (m_type == double_type)
                return static_cast<uint32_t>(m_value);
            else
                return static_cast<uint32_t>(m_intval);
        }

        /// <summary>
        /// Converts the JSON number to int64.
        /// </summary>
        /// <returns>An int64 representation of the number</returns>
        int64_t to_int64() const
        {
            if (m_type == double_type)
                return static_cast<int64_t>(m_value);
            else
                return static_cast<int64_t>(m_intval);
        }

        /// <summary>
        /// Converts the JSON number to unsigned int64.
        /// </summary>
        /// <returns>An unsigned int64 representation of the number</returns>
        uint64_t to_uint64() const
        {
            if (m_type == double_type)
                return static_cast<uint64_t>(m_value);
            else
                return static_cast<uint64_t>(m_intval);
        }
#pragma endregion

        /// <summary>
        /// Is the number represented internally as an integral type?
        /// </summary>
        /// <returns><c>true</c> if the number is represented as an integral type, <c>false</c> otherwise</returns>
        bool is_integral() const
        {
            return m_type != double_type;
        }

        /// <summary>
        /// Compares two JSON numbers for equality.
        /// </summary>
        /// <param name="other">The JSON number to compare with.</param>
        /// <returns>True iff the numbers are equal.</returns>
        bool operator==(const number &other) const
        {
            if (m_type != other.m_type)
                return false;

            switch (m_type)
            {
            case json::number::type::signed_type :
                return m_intval == other.m_intval;
            case json::number::type::unsigned_type :
                return m_uintval == other.m_uintval;
            case json::number::type::double_type :
                return m_value == other.m_value;
            }
            UNREACHABLE;
        }

    private:
        union
        {
            int64_t m_intval;
            uint64_t m_uintval;
            double  m_value;
        };

        enum type
        {
            signed_type=0, unsigned_type, double_type
        } m_type;

        friend class details::_Number;
    };

    namespace details
    {
        class _Value 
        {
        public:
            virtual std::unique_ptr<_Value> _copy_value() = 0;

            virtual bool has_field(const utility::string_t &) const { return false; }
            virtual value get_field(const utility::string_t &) const { throw json_exception(_XPLATSTR("not an object")); }
            virtual value get_element(array::size_type) const { throw json_exception(_XPLATSTR("not an array")); }

            virtual value &index(const utility::string_t &) { throw json_exception(_XPLATSTR("not an object")); }
            virtual value &index(array::size_type) { throw json_exception(_XPLATSTR("not an array")); }

            virtual const value &cnst_index(const utility::string_t &) const { throw json_exception(_XPLATSTR("not an object")); }
            virtual const value &cnst_index(array::size_type) const { throw json_exception(_XPLATSTR("not an array")); }

            // Common function used for serialization to strings and streams.
            virtual void serialize_impl(std::string& str) const
            {
                format(str); 
            }
#ifdef _MS_WINDOWS
            virtual void serialize_impl(std::wstring& str) const
            {
                format(str);
            }
#endif

            virtual utility::string_t to_string() const
            {
                utility::string_t str;
                serialize_impl(str);
                return str;
            }

            virtual json::value::value_type type() const { return json::value::Null; }

            virtual bool is_integer() const { throw json_exception(_XPLATSTR("not a number")); }
            virtual bool is_double() const { throw json_exception(_XPLATSTR("not a number")); }

            virtual json::number as_number() { throw json_exception(_XPLATSTR("not a number")); }
            virtual double as_double() const { throw json_exception(_XPLATSTR("not a number")); }
            virtual int as_integer() const { throw json_exception(_XPLATSTR("not a number")); }
            virtual bool as_bool() const { throw json_exception(_XPLATSTR("not a boolean")); }
            virtual json::array& as_array() { throw json_exception(_XPLATSTR("not an array")); }
            virtual const json::array& as_array() const { throw json_exception(_XPLATSTR("not an array")); }
            virtual json::object& as_object() { throw json_exception(_XPLATSTR("not an object")); }
            virtual const json::object& as_object() const { throw json_exception(_XPLATSTR("not an object")); }
            virtual utility::string_t as_string() const { throw json_exception(_XPLATSTR("not a string")); }

            virtual size_t size() const { return 0; }

            virtual ~_Value() {}

        protected:
            _Value() {}

            virtual void format(std::basic_string<char>& stream) const
            {
                stream.append("null");
            }
#ifdef _MS_WINDOWS
            virtual void format(std::basic_string<wchar_t>& stream) const
            {
                stream.append(L"null");
            }
#endif
        private:

            friend class web::json::value;
        };

        class _Null : public _Value
        {
        public:

            virtual std::unique_ptr<_Value> _copy_value()
            {
                return utility::details::make_unique<_Null>();
            }

            virtual json::value::value_type type() const { return json::value::Null; }

            _Null() { }

        private:
            template<typename CharType> friend class json::details::JSON_Parser;
        };

        class _Number : public _Value
        {
        public:
            _Number(double value)  : m_number(value) { }
            _Number(int32_t value) : m_number(value) { }
            _Number(uint32_t value) : m_number(value) { }
            _Number(int64_t value) : m_number(value) { }
            _Number(uint64_t value) : m_number(value) { }

            virtual std::unique_ptr<_Value> _copy_value()
            {
                return utility::details::make_unique<_Number>(*this);
            }

            virtual json::value::value_type type() const { return json::value::Number; }

            virtual bool is_integer() const { return m_number.is_integral(); }
            virtual bool is_double() const { return !m_number.is_integral(); }

            virtual double as_double() const
            {
                return m_number.to_double();
            }

            virtual int as_integer() const
            {
                return m_number.to_int32();
            }

            virtual number as_number() { return m_number; }

        protected:
            virtual void format(std::basic_string<char>& stream) const ;
#ifdef _MS_WINDOWS
            virtual void format(std::basic_string<wchar_t>& stream) const;
#endif
        private:
            template<typename CharType> friend class json::details::JSON_Parser;

            json::number m_number;
        };

        class _Boolean : public _Value
        {
        public:

            virtual std::unique_ptr<_Value> _copy_value()
            {
                return utility::details::make_unique<_Boolean>(*this);
            }

            virtual json::value::value_type type() const { return json::value::Boolean; }

            virtual bool as_bool() const { return m_value; }

        protected:
            virtual void format(std::basic_string<char>& stream) const
            {
                stream.append(m_value ? "true" : "false");
            }

#ifdef _MS_WINDOWS
            virtual void format(std::basic_string<wchar_t>& stream) const
            {
                stream.append(m_value ? L"true" : L"false");
            }
#endif
        private:
            template<typename CharType> friend class json::details::JSON_Parser;
        public:
            _Boolean(bool value) : m_value(value) { }
        private:
            bool m_value;
        };

        class _String : public _Value
        {
        public:

            _String(utility::string_t value) : m_string(std::move(value))
            {
                m_has_escape_char = has_escape_chars(*this);
            }
            _String(utility::string_t value, bool escaped_chars)
                : m_string(std::move(value)),
                  m_has_escape_char(escaped_chars)
            { }

#ifdef _MS_WINDOWS
            _String(std::string &&value) : m_string(utility::conversions::to_utf16string(std::move(value)))
            {
                m_has_escape_char = has_escape_chars(*this);
            }
            _String(std::string &&value, bool escape_chars) 
                : m_string(utility::conversions::to_utf16string(std::move(value))),
                  m_has_escape_char(escape_chars)
            { }
#endif

            _String(const _String& other) : web::json::details::_Value(other)
            {
                copy_from(other);
            }

            _String& operator=(const _String& other)
            {
                if (this != &other) {
                    copy_from(other);
                }
                return *this;
            }

            virtual std::unique_ptr<_Value> _copy_value()
            {
                return utility::details::make_unique<_String>(*this);
            }

            virtual json::value::value_type type() const { return json::value::String; }

            virtual utility::string_t as_string() const;

            virtual void serialize_impl(std::string& str) const
            {
                 serialize_impl_char_type(str);
            }
#ifdef _MS_WINDOWS
            virtual void serialize_impl(std::wstring& str) const
            {
                serialize_impl_char_type(str);
            }
            
#endif

        protected:
            virtual void format(std::basic_string<char>& str) const;
#ifdef _MS_WINDOWS
            virtual void format(std::basic_string<wchar_t>& str) const;
#endif

        private:
            friend class _Object;
            friend class _Array;

            size_t get_reserve_size() const
            {        
                return m_string.size() + 2;
            }

            template <typename CharType>
            void serialize_impl_char_type(std::basic_string<CharType>& str) const
            {
                // To avoid repeated allocations reserve some space all up front.
                // size of string + 2 for quotes
                str.reserve(get_reserve_size());
                format(str);
            }

            void copy_from(const _String& other)
            {
                m_string = other.m_string;
                m_has_escape_char = other.m_has_escape_char;
            }

            std::string as_utf8_string() const;
            utf16string as_utf16_string() const;

            utility::string_t m_string;

            // There are significant performance gains that can be made by knowning whether
            // or not a character that requires escaping is present.
            bool m_has_escape_char;
            static bool has_escape_chars(const _String &str);
        };

        template<typename CharType>
        _ASYNCRTIMP void append_escape_string(std::basic_string<CharType>& str, const std::basic_string<CharType>& escaped);

        void format_string(const utility::string_t& key, utility::string_t& str);

#ifdef _MS_WINDOWS
        void format_string(const utility::string_t& key, std::string& str);
#endif

        class _Object : public _Value
        {
        public:

            _Object(bool keep_order) : m_object(keep_order) { }

            _Object(object::storage_type fields, bool keep_order) : m_object(std::move(fields), keep_order) { }

            _ASYNCRTIMP _Object(const _Object& other);

            virtual ~_Object() {}

            virtual std::unique_ptr<_Value> _copy_value()
            {
                return utility::details::make_unique<_Object>(*this);
            }

            virtual json::object& as_object() { return m_object;    }

            virtual const json::object& as_object() const { return m_object;    }

            virtual json::value::value_type type() const { return json::value::Object; }

            virtual bool has_field(const utility::string_t &) const;

            _ASYNCRTIMP virtual json::value &index(const utility::string_t &key);

            bool is_equal(const _Object* other) const
            {
                if ( m_object.size() != other->m_object.size())
                    return false;

                return std::equal(std::begin(m_object), std::end(m_object), std::begin(other->m_object));
            }

            virtual void serialize_impl(std::string& str) const
            {
                // To avoid repeated allocations reserve some space all up front.
                str.reserve(get_reserve_size());
                format(str); 
            }
#ifdef _MS_WINDOWS
            virtual void serialize_impl(std::wstring& str) const
            {
                // To avoid repeated allocations reserve some space all up front.
                str.reserve(get_reserve_size());
                format(str);
            }
#endif
            size_t size() const { return m_object.size(); }

        protected:
            virtual void format(std::basic_string<char>& str) const
            {
                format_impl(str);
            }
#ifdef _MS_WINDOWS
            virtual void format(std::basic_string<wchar_t>& str) const
            {
                format_impl(str);
            }
#endif

        private:
            json::object m_object;

            template<typename CharType> friend class json::details::JSON_Parser;

            _ASYNCRTIMP void map_fields();

            template<typename CharType>
            void format_impl(std::basic_string<CharType>& str) const
            {
                str.push_back('{');
                if(!m_object.empty())
                {
                    auto lastElement = m_object.end() - 1;
                    for (auto iter = m_object.begin(); iter != lastElement; ++iter)
                    {
                        format_string(iter->first, str);
                        str.push_back(':');
                        iter->second.format(str);
                        str.push_back(',');
                    }
                    format_string(lastElement->first, str);
                    str.push_back(':');
                    lastElement->second.format(str);
                }
                str.push_back('}');
            }

            size_t get_reserve_size() const
            {
                // This is a heuristic we can tune more in the future:
                // Basically size of string plus 
                // sum size of value if an object, array, or string.
                size_t reserveSize = 2; // For brackets {}
                for(auto iter = m_object.begin(); iter != m_object.end(); ++iter)
                {
                    reserveSize += iter->first.length() + 2;    // 2 for quotes
                    size_t valueSize = iter->second.size() * 20; // Multipler by each object/array element
                    if(valueSize == 0)
                    {
                        if(iter->second.type() == json::value::String)
                        {
                            valueSize = static_cast<_String *>(iter->second.m_value.get())->get_reserve_size();
                        }
                        else
                        {
                            valueSize = 5; // true, false, or null
                        }
                    }
                    reserveSize += valueSize;
                }
                return reserveSize;
            }
        };

        class _Array : public _Value
        {
        public:
            _Array() {}
            _Array(array::size_type size) : m_array(size) {}
            _Array(array::storage_type elements) : m_array(std::move(elements)) { }

            virtual std::unique_ptr<_Value> _copy_value()
            {
                return utility::details::make_unique<_Array>(*this);
            }

            virtual json::value::value_type type() const { return json::value::Array; }

            virtual json::array& as_array() { return m_array; }
            virtual const json::array& as_array() const { return m_array; }

            virtual json::value &index(json::array::size_type index)
            {
                return m_array[index];
            }

            bool is_equal(const _Array* other) const
            {
                if ( m_array.size() != other->m_array.size())
                    return false;

                auto iterT  = m_array.cbegin();
                auto iterO  = other->m_array.cbegin();
                auto iterTe = m_array.cend();
                auto iterOe = other->m_array.cend();

                for (; iterT != iterTe && iterO != iterOe; ++iterT, ++iterO)
                {
                    if ( *iterT != *iterO )
                        return false;
                }

                return true;
            }

            virtual void serialize_impl(std::string& str) const
            {
                // To avoid repeated allocations reserve some space all up front.
                str.reserve(get_reserve_size());
                format(str);  
            }
#ifdef _MS_WINDOWS
            virtual void serialize_impl(std::wstring& str) const
            {
                // To avoid repeated allocations reserve some space all up front.
                str.reserve(get_reserve_size());
                format(str); 
            }
#endif
            size_t size() const { return m_array.size(); }

        protected:
            virtual void format(std::basic_string<char>& str) const
            {
                format_impl(str);
            }
#ifdef _MS_WINDOWS
            virtual void format(std::basic_string<wchar_t>& str) const
            {
                format_impl(str);
            }
#endif
        private:
            json::array m_array;

            template<typename CharType> friend class json::details::JSON_Parser;

            template<typename CharType>
            void format_impl(std::basic_string<CharType>& str) const
            {
                str.push_back('[');
                if(!m_array.m_elements.empty())
                {
                    auto lastElement = m_array.m_elements.end() - 1;
                    for (auto iter = m_array.m_elements.begin(); iter != lastElement; ++iter)
                    {
                        iter->format(str);
                        str.push_back(',');
                    }
                    lastElement->format(str);
                }
                str.push_back(']');
            }

            size_t get_reserve_size() const
            {
                // This is a heuristic we can tune more in the future:
                // Basically sum size of each value if an object, array, or string by a multiplier.
                size_t reserveSize = 2; // For brackets []
                for(auto iter = m_array.cbegin(); iter != m_array.cend(); ++iter)
                {
                    size_t valueSize = iter->size() * 20; // Per each nested array/object

                    if(valueSize == 0)
                        valueSize = 5; // true, false, or null

                    reserveSize += valueSize;
                }
                return reserveSize;
            }
        };
    } // namespace details

    /// <summary>
    /// Gets the number of children of the value.
    /// </summary>
    /// <returns>The number of children. 0 for all non-composites.</returns>
    inline size_t json::value::size() const
    {
        return m_value->size();
    }

    /// <summary>
    /// Test for the presence of a field.
    /// </summary>
    /// <param name="key">The name of the field</param>
    /// <returns>True if the field exists, false otherwise.</returns>
    inline bool json::value::has_field(const utility::string_t& key) const
    {
        return m_value->has_field(key);
    }

    /// <summary>
    /// Access a field of a JSON object.
    /// </summary>
    /// <param name="key">The name of the field</param>
    /// <returns>The value kept in the field; null if the field does not exist</returns>
    inline json::value json::value::get(const utility::string_t& key) const
    {
        return m_value->get_field(key);
    }

    /// <summary>
    /// Access an element of a JSON array.
    /// </summary>
    /// <param name="key">The index of an element in the JSON array</param>
    /// <returns>The value kept at the array index; null if outside the boundaries of the array</returns>
    inline json::value json::value::get(size_t index) const
    {
        return m_value->get_element(index);
    }

    /// <summary>
    /// A standard <c>std::ostream</c> operator to facilitate writing JSON values to streams.
    /// </summary>
    /// <param name="os">The output stream to write the JSON value to.</param>
    /// <param name="val">The JSON value to be written to the stream.</param>
    /// <returns>The output stream object</returns>
    _ASYNCRTIMP utility::ostream_t& operator << (utility::ostream_t &os, const json::value &val);

    /// <summary>
    /// A standard <c>std::istream</c> operator to facilitate reading JSON values from streams.
    /// </summary>
    /// <param name="is">The input stream to read the JSON value from.</param>
    /// <param name="val">The JSON value object read from the stream.</param>
    /// <returns>The input stream object.</returns>
    _ASYNCRTIMP utility::istream_t& operator >> (utility::istream_t &is, json::value &val);

}} // namespace web::json

#endif  /* _CASA_JSON_H */
