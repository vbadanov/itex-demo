
#include <sqldb/parameter.hpp>
#include <iostream>

namespace sqldb
{

//===============================================================================
std::ostream& operator<< (std::ostream& os, const null_t& value)
{
    return (os << "NULL");
}

//===============================================================================
template<> std::string to_string(const char* value)
{
	return std::string(value);
}

//===============================================================================
template<> std::string to_string(std::string value)
{
	return value;
}

//===============================================================================
template<> std::string to_string(null_t value)
{
    return std::string("NULL");
}

//===============================================================================
Parameter::Parameter()
    : is_null_(true), is_binary_(false), data_(std::string())
{
    //pass
}

//===============================================================================
template<> Parameter::Parameter(const null_t& value)
    : Parameter()
{
    //pass
}

//===============================================================================
template<> Parameter::Parameter(const bytea& value)
	: is_null_(false), is_binary_(true), data_(value)
{
    //pass
}

//===============================================================================
Parameter::Parameter(Parameter&& parameter)
{
    *this = parameter;
}

//===============================================================================
Parameter::Parameter(const Parameter& parameter)
{
    is_null_ = parameter.is_null_;
    is_binary_ = parameter.is_binary_;
    data_ = parameter.data_;
}

//===============================================================================
Parameter::~Parameter()
{
    //pass
}

//===============================================================================
Parameter& Parameter::operator=(Parameter&& parameter)
{
    is_null_ = parameter.is_null_;
    is_binary_ = parameter.is_binary_;
    data_ = std::move(parameter.data_);
    return *this;
}

//===============================================================================
Parameter& Parameter::operator=(const Parameter& parameter)
{
    is_null_ = parameter.is_null_;
    is_binary_ = parameter.is_binary_;
    data_ = parameter.data_;
    return *this;
}

//===============================================================================
bool Parameter::is_null() const
{
    return is_null_;
}

//===============================================================================
bool Parameter::is_binary() const
{
	return is_binary_;
}

//===============================================================================
const std::string& Parameter::data() const
{
	return data_;
}

//===============================================================================
const char* Parameter::c_str() const
{
    return is_null_ ? nullptr : data_.c_str();
}





}

