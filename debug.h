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

#include <iostream>

#ifdef DEBUG
#include <cstdio>
#include <cstring>
#include "esteid.h"

#if WIN32 
#define snprintf sprintf_s 
#endif

#define ESTEID_DEBUG(...) do { \
    char msg[1024]; \
    snprintf(msg, 1024, __VA_ARGS__); \
    eidlog << "DEBUG [" << __FILE__ << ":" << __LINE__ << "] - " << \
        msg << std::endl; \
    } while(0)
#else

#define ESTEID_DEBUG(...)

#endif

#endif //ESTEIDDEBUG_H
