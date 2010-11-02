#include "Base64.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <openssl/sha.h>

using namespace boost::archive::iterators;
using namespace boost::algorithm;

template <class T>
struct base64 {
    /* retrieve 6 bit integers from a sequence of 8 bit bytes,
     * convert binary values to base64 characters */
    typedef base64_from_binary<
                transform_width<typename T::const_iterator, 6, 8> >
            iterator;
};

template <class T>
struct base64_text {
    /* retrieve 6 bit integers from a sequence of 8 bit bytes,
     * convert binary values to base64 characters,
     * insert line breaks every 72 characters */
    typedef insert_linebreaks<base64_from_binary<
                transform_width<typename T::const_iterator, 6, 8> >, 72>
            iterator;
};

/* Calculate left-over bits and append markers to the end of the buffer */
static std::string _add_leftovers(std::string ret)
{
    size_t leftover_bits = (ret.size() * 6) % 8;
    if (leftover_bits == 4)
        ret += "==";
    else if (leftover_bits == 2)
        ret += "=";

    return ret;
}

std::string base64_encode(const std::vector<unsigned char>& in)
{
    std::string ret(base64<std::vector<unsigned char> >::iterator(in.begin()),
                    base64<std::vector<unsigned char> >::iterator(in.end()));

    return _add_leftovers(ret);
}

std::string base64_encode(const std::string& in)
{
    std::string ret(base64<std::string>::iterator(in.begin()),
                    base64<std::string>::iterator(in.end()));

    return _add_leftovers(ret);
}

std::string base64_decode(std::string in)
{
    typedef transform_width< binary_from_base64<std::string::const_iterator>, 8, 6 > binary_data;

    // remove linebreaks
    in.erase(std::remove(in.begin(), in.end(), '\n'), in.end());

    size_t pos = in.find_last_not_of('=');
    if (pos == in.size() - 1)
        pos = in.size();

    return std::string(binary_data(in.begin()), binary_data(in.begin() + pos));
}
