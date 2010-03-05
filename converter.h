#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>

namespace Converter
{
    std::string iconvConvert(const std::string&, const char*, const char*);
    std::string CP1252_to_UTF8(const std::string&);
};

#endif //CONVERTER_H
