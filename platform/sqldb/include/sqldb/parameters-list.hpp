
#ifndef SQLDB_PARAMETERS_LIST_HPP_
#define SQLDB_PARAMETERS_LIST_HPP_

#include <iostream>
#include <vector>
#include <libpq-fe.h>
#include <sqldb/parameter.hpp>

namespace sqldb
{

//===============================================================================
class ParametersList
{
    friend class Connection;

    public:
        typedef std::vector<Parameter> parameters_t;

        ParametersList();
        ParametersList(ParametersList&& param_list);
        ParametersList(const ParametersList& param_list) = delete;
        ~ParametersList();

        ParametersList& operator=(ParametersList&& param_list);
        ParametersList& operator=(ParametersList& param_list) = delete;

        template<typename T>
        ParametersList& operator<<(const T& value);

        Parameter& operator[](const size_t& index);
        const Parameter& operator[](const size_t& index) const;
        size_t size() const;
        void reserve (const size_t& n);

        parameters_t::iterator begin();
        parameters_t::iterator end();

    protected:


    private:
        parameters_t parameters_;
};

//===============================================================================
template<typename T>
ParametersList& ParametersList::operator<<(const T& value)
{
    parameters_.emplace_back(value);
    return *this;
}

//===============================================================================
template<typename ...Args>
ParametersList make_params(Args... args)
{
    ParametersList param_list;
    param_list.reserve(sizeof...(Args));
    fill_params(param_list, args...);
    return param_list;
}

template<typename T, typename ...Args>
void fill_params(ParametersList& param_list, const T& value, Args... args)
{
    param_list << value;
    fill_params(param_list, args...);
}

template<typename T>
void fill_params(ParametersList& param_list, const T& value)
{
    param_list << value;
}

//===============================================================================
class PostgresqlParametersList
{
    public:
        PostgresqlParametersList() = delete;
        PostgresqlParametersList(PostgresqlParametersList&) = delete;
        PostgresqlParametersList(PostgresqlParametersList&&) = delete;
        PostgresqlParametersList(const ParametersList& param_list);
        ~PostgresqlParametersList();
        PostgresqlParametersList& operator=(PostgresqlParametersList&) = delete;
        PostgresqlParametersList& operator=(PostgresqlParametersList&&) = delete;
        int get_n_params() const;
        const Oid* get_param_types() const;
        const char* const * get_param_values() const;
        const int* get_param_lengths() const;
        const int* get_param_formats() const;

    private:
        int n_params_;
        Oid* param_types_;
        const char** param_values_;
        int* param_lengths_;
        int* param_formats_;
};

}

#endif //SQLDB_PARAMETERS_LIST_HPP_
