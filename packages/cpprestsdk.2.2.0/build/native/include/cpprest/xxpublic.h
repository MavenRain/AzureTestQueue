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
* xxpublic.h
*
* Standard macros and definitions.
* This header has minimal dependency on windows headers and is safe for use in the public API
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#ifndef _MS_WINDOWS
#if defined(_WIN32) || defined(__cplusplus_winrt)
#define _MS_WINDOWS
#endif
#endif // _MS_WINDOWS

#ifdef _NO_ASYNCRTIMP
#define _ASYNCRTIMP
#else
#ifdef _ASYNCRT_EXPORT
#define _ASYNCRTIMP __declspec(dllexport)
#else
#define _ASYNCRTIMP __declspec(dllimport)
#endif
#endif

// forward declarations of windows types

// for UINT_PTR types used in winsock APIs
#ifdef _MS_WINDOWS
#include <BaseTsd.h>
#endif

// for guids, used in comm.h
#ifdef MS_TARGET_APPLE
#include "compat/apple_compat.h"
#else
#ifdef _MS_WINDOWS
#include <Guiddef.h>
#include "compat/windows_compat.h"
#else
#include "boost/uuid/uuid.hpp"
#endif
#endif

#define UNREACHABLE __assume(0)

#ifdef __clang__
#include <cstdio>
#endif

// Define the _NOT_PHONE8_ as non-windows or windows desktop without arm configuration or windows store app or windows phone8.1
#define _NOT_PHONE8_ !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP && !defined(_M_ARM)) || WINAPI_FAMILY == WINAPI_FAMILY_APP || (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP && _WIN32_WINNT == _WIN32_WINNT_WINBLUE)