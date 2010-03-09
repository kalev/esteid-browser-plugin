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
