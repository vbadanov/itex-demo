
#include <sqldb/parameters-list.hpp>

namespace sqldb
{

//===============================================================================
ParametersList::ParametersList()
{
    //pass
}

//===============================================================================
ParametersList::ParametersList(ParametersList&& param_list)
{
    parameters_ = std::move(param_list.parameters_);
}

//===============================================================================
ParametersList::~ParametersList()
{
    //pass
}

//===============================================================================
ParametersList& ParametersList::operator=(ParametersList&& param_list)
{
    parameters_ = std::move(param_list.parameters_);
    return *this;
}

//===============================================================================
Parameter& ParametersList::operator[](const size_t& index)
{
    return parameters_[index];
}

const Parameter& ParametersList::operator[](const size_t& index) const
{
    return parameters_[index];
}

//===============================================================================
size_t ParametersList::size() const
{
    return parameters_.size();
}

//===============================================================================
void ParametersList::reserve (const size_t& n)
{
    parameters_.reserve(n);
}

//===============================================================================
ParametersList::parameters_t::iterator ParametersList::begin()
{
    return parameters_.begin();
}


//===============================================================================
ParametersList::parameters_t::iterator ParametersList::end()
{
    return parameters_.end();
}

//===============================================================================
PostgresqlParametersList::PostgresqlParametersList(const ParametersList& param_list)
    : n_params_((int)param_list.size()),
      param_types_(new Oid [(int)param_list.size()]),
      param_values_(new const char* [(int)param_list.size()]),
      param_lengths_(new int [(int)param_list.size()]),
      param_formats_(new int [(int)param_list.size()])
{
    for(int i = 0; i < n_params_; ++i)
    {
        const Parameter& param = param_list[i];
        param_types_[i] = 0;
        param_values_[i] = param.is_null() ? nullptr : (param.is_binary() ? param.data().data() : param.c_str());
        param_lengths_[i] = param.is_binary() ? param.data().size() : 0;
        param_formats_[i] = param.is_binary() ? 1 : 0;
    }
}

//===============================================================================
PostgresqlParametersList::~PostgresqlParametersList()
{
    if(param_values_ != nullptr)
    {
        delete[] param_values_;
    }
}

//===============================================================================
int PostgresqlParametersList::get_n_params() const
{
    return n_params_;
}

//===============================================================================
const Oid* PostgresqlParametersList::get_param_types() const
{
    return param_types_;
}

//===============================================================================
const char* const * PostgresqlParametersList::get_param_values() const
{
    return param_values_;
}

//===============================================================================
const int* PostgresqlParametersList::get_param_lengths() const
{
    return param_lengths_;
}

//===============================================================================
const int* PostgresqlParametersList::get_param_formats() const
{
    return param_formats_;
}



}
