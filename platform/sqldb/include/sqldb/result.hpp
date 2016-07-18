
#ifndef SQLDB_RESULT_HPP_
#define SQLDB_RESULT_HPP_

#include <vector>
#include <iterator>
#include <utility>
#include <libpq-fe.h>
#include <boost/lexical_cast.hpp>
#include <sqldb/exception.hpp>
#include <sqldb/parameter.hpp>

namespace sqldb
{

//===============================================================================
struct ignore_t
{
};
const ignore_t ignore;

//===============================================================================
template<typename T>
T from_string(std::string str)
{
    return boost::lexical_cast<T>(str);
}

template<> ignore_t from_string(std::string str);
template<> int from_string(std::string str);
template<> long from_string(std::string str);
template<> unsigned long from_string(std::string str);
template<> long long from_string(std::string str);
template<> unsigned long long from_string(std::string str);
template<> float from_string(std::string str);
template<> double from_string(std::string str);
template<> long double from_string(std::string str);
template<> std::string from_string(std::string str);
template<> bytea from_string(std::string str);

//===============================================================================
class QueryResult
{
    public:
        enum class Status
        {
            OK,
            BAD
        };

        //===============================================================================
        class Row
        {
            public:
                Row() = delete;
                Row(QueryResult* query_result, size_t row_index);
                Row(Row&) = delete;
                Row(Row&& row);
                ~Row();

                Row& operator=(const Row&) = delete;
                Row& operator=(Row&& row);

                bool is_null(size_t column_index);
                bool is_null(std::string column_name);

                template<typename T>
                T get(size_t column_index);

                template<typename T>
                T get(std::string column_name);

                template<typename T>
                T operator[](size_t column_index);

                template<typename T>
                T operator[](std::string column_name);

                template<typename ...Args>
                void copy_to(Args&... args);

                size_t size();

            protected:
                template<typename T, typename ...Args>
                void copy_column_value(size_t column_index, T& var, Args&... args);

                template<typename T>
                void copy_column_value(size_t column_index, T& var);

            private:
                QueryResult* query_result_;
                size_t row_index_;
        };

        //===============================================================================
        class iterator : public std::iterator<std::input_iterator_tag, Row>
        {
            public:
                iterator(QueryResult* query_result, size_t current_row_index);
                iterator(const iterator& it);
                ~iterator();
                iterator& operator++();
                bool operator==(const iterator& rhs);
                bool operator!=(const iterator& rhs);
                Row operator*();

            private:
                QueryResult* query_result_;
                size_t current_row_index_;
        };

        //===============================================================================
        QueryResult() = delete;
        QueryResult(const QueryResult& query_result) = delete;
        QueryResult(QueryResult&& query_result);
        QueryResult(PGresult* pg_result);
        ~QueryResult();

        QueryResult& operator=(const QueryResult&) = delete;
        QueryResult& operator=(QueryResult&& query_result);

        bool is_null(size_t row_index, size_t column_index);
        bool is_null(size_t row_index, std::string column_name);

        size_t get_column_index(std::string column_name);

        char* get_raw_value(size_t row_index, size_t column_index);
        char* get_raw_value(size_t row_index, std::string column_name);

        template<typename T>
        T get_value(size_t row_index, size_t column_index);

        template<typename T>
        T get_value(size_t row_index, std::string column_name);

        Status status();
        std::string get_pg_result_error();

        size_t num_rows();
        size_t num_columns();
        std::pair<size_t, size_t>& size();

        iterator begin();
        iterator end();

    protected:
        void load_result_size_value();

    private:
        PGresult* pg_result_;
        std::pair<size_t, size_t> result_size_;

};

//===============================================================================
class Results
{
    public:
        typedef std::vector<QueryResult> results_list_t;

        Results();
        Results(size_t n);

        Results(const Results&) = delete;
        Results(Results&& results);

        Results& operator=(const Results&) = delete;
        Results& operator=(Results&& results);


        QueryResult& statement(size_t index);
        QueryResult& operator[](size_t index);

        void add(QueryResult&& query_result);

        results_list_t::iterator begin();
        results_list_t::iterator end();

        void reserve(size_t n);
        size_t size();

    private:
        results_list_t results_list_;
};


//===============================================================================
template<typename T>
T QueryResult::Row::get(size_t column_index)
{
    return query_result_->get_value<T>(row_index_, column_index);
}

//===============================================================================
template<typename T>
T QueryResult::Row::get(std::string column_name)
{
    return query_result_->get_value<T>(row_index_, column_name);
}

//===============================================================================
template<typename T>
T QueryResult::Row::operator[](size_t column_index)
{
    return get<T>(column_index);
}

//===============================================================================
template<typename T>
T QueryResult::Row::operator[](std::string column_name)
{
    return get<T>(column_name);
}

//===============================================================================
template<typename ...Args>
void QueryResult::Row::copy_to(Args&... args)
{
    if(sizeof...(Args) > size())
    {
        throw ResultException("QueryResult::Row::copy_to() - number of parameters is greater then row size");
    }
    copy_column_value(0, args...);
}

template<typename T, typename ...Args>
void QueryResult::Row::copy_column_value(size_t column_index, T& var, Args&... args)
{
    var = get<T>(column_index);
    copy_column_value(++column_index, args...);
}

template<typename T>
void QueryResult::Row::copy_column_value(size_t column_index, T& var)
{
    var = get<T>(column_index);
}

//===============================================================================
template<typename T>
T QueryResult::get_value(size_t row_index, size_t column_index)
{
    return from_string<T>(get_raw_value(row_index, column_index));
}

//===============================================================================
template<typename T>
T QueryResult::get_value(size_t row_index, std::string column_name)
{
    return from_string<T>(get_raw_value(row_index, column_name));
}


}

#endif //SQLDB_RESULT_HPP_
