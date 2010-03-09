#include "converter.h"
#include "debug.h"
#include <iconv.h>

#ifdef ICONV_SECOND_ARGUMENT_IS_CONST
    #define ICONV_CONST const
#else
    #define ICONV_CONST
#endif

#if defined (__APPLE__) && defined (__LP64__)
#define LIBICONV_PLUG 1
#endif

std::string Converter::iconvConvert(const std::string & str_in, const char *tocode, const char *fromcode)
{
    iconv_t iso_utf;
    ICONV_CONST char *inptr;
    char *outptr;
    size_t inbytes, outbytes, result;
    char outbuf[128];
    std::string out;

    iso_utf = iconv_open(tocode, fromcode);
    if (iso_utf == (iconv_t)-1) {
        ESTEID_DEBUG("error in iconv_open");
        return str_in;
    }

    inptr = (ICONV_CONST char*)str_in.c_str();
    inbytes = str_in.size();

    outptr = outbuf;
    outbytes = sizeof(outbuf) - 1;

    result = iconv(iso_utf, &inptr, &inbytes, &outptr, &outbytes);
    if (result == (size_t)-1) {
        ESTEID_DEBUG("error converting with iconv");
        return str_in;
    }
    *outptr = '\0';

    iconv_close(iso_utf);

    out += outbuf;
    return out;
}


std::string Converter::CP1252_to_UTF8(const std::string & str_in)
{
    return iconvConvert(str_in, "UTF-8", "CP1252");
}
