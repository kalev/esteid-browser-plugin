/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ESTEIDDEBUG_H
#define ESTEIDDEBUG_H

#ifndef NDEBUG

#include <cstdio>
#include "logging.h"

#ifdef _WIN32
#define snprintf sprintf_s 
#endif

#define ESTEID_DEBUG(...) do \
{ \
    char msg[1024]; \
    snprintf(msg, 1024, __VA_ARGS__); \
    FBLOG_INFO("esteid-browser-plugin", msg); \
} while(0)

#define ESTEID_DEBUG_SCOPE() do \
{ \
    FBLOG_INFO("esteid-browser-plugin", ""); \
} while(0)

#else

#define ESTEID_DEBUG(...)
#define ESTEID_DEBUG_SCOPE()

#endif //NDEBUG

#endif //ESTEIDDEBUG_H
