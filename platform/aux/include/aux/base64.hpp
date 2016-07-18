
#ifndef AUX_BASE64_HPP_
#define AUX_BASE64_HPP_


#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

namespace aux
{

namespace base64
{

using namespace boost::archive::iterators;

//===============================================================================
std::string encode(const std::string& val)
{
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

//===============================================================================
std::string decode(const std::string& val)
{
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c)
    {
        return c == '\0';
    });
}


}
}

#endif /* AUX_BASE64_HPP_ */
