
#ifndef SQLDB_PARAMETER_HPP_
#define SQLDB_PARAMETER_HPP_

#include <string>
#include <ostream>
#include <libpq-fe.h>

namespace sqldb
{

//===============================================================================
struct null_t
{
};
const null_t null;
std::ostream& operator<< (std::ostream& os, const null_t& value);

//===============================================================================
class bytea : public std::string
{
	public:
		static const Oid BYTEAOID = 17;
		bytea() : std::string() { };
		bytea(size_t n, char c) : std::string(n, c) { };
};


//===============================================================================
template<typename T>
std::string to_string(T value)
{
    return std::to_string(value);
}
template<> std::string to_string(const char* value);
template<> std::string to_string(std::string value);
template<> std::string to_string(null_t value);

//===============================================================================
class Parameter
{
    public:
        Parameter();

        template<typename T>
        Parameter(const T& value);

        Parameter(Parameter&& parameter);
        Parameter(const Parameter&);

        Parameter& operator=(Parameter&& parameter);
        Parameter& operator=(const Parameter&);

        ~Parameter();

        bool is_null() const;
        bool is_binary() const;

        const std::string& data() const;
        const char* c_str() const;

    private:
        bool is_null_;
        bool is_binary_;
        std::string data_;
};

//===============================================================================
template<typename T>
Parameter::Parameter(const T& value)
    : is_null_(false), is_binary_(false), data_(to_string(value))
{
    //pass
}

//===============================================================================
template<> Parameter::Parameter(const null_t& value);
template<> Parameter::Parameter(const bytea& value);


}

#endif //SQLDB_PARAMETER_HPP_
