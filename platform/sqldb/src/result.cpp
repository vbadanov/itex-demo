
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <sqldb/result.hpp>

namespace sqldb
{

//===============================================================================
template<> ignore_t from_string(std::string str)
{
    return ignore_t();
}

//===============================================================================
template<> int from_string(std::string str)
{
    return std::stoi(str, nullptr, 10);
}

//===============================================================================
template<> long from_string(std::string str)
{
    return std::stol(str, nullptr, 10);
}

//===============================================================================
template<> unsigned long from_string(std::string str)
{
    return std::stoul(str, nullptr, 10);
}

//===============================================================================
template<> long long from_string(std::string str)
{
    return std::stoll(str, nullptr, 10);
}

//===============================================================================
template<> unsigned long long from_string(std::string str)
{
    return std::stoull(str, nullptr, 10);
}

//===============================================================================
template<> float from_string(std::string str)
{
    return std::stof(str, nullptr);
}

//===============================================================================
template<> double from_string(std::string str)
{
    return std::stod(str, nullptr);
}

//===============================================================================
template<> long double from_string(std::string str)
{
    return std::stold(str, nullptr);
}

//===============================================================================
template<> std::string from_string(std::string str)
{
    return str;
}

//===============================================================================
inline int decfromhex(int const x)
{
	// ASCII code for HEX char to DEC
	return ((x < 58) ? (x - 48) : ((x < 71) ? (x - 55) : (x - 87)));
}

//===============================================================================
template<> bytea from_string(std::string str)
{
	size_t str_size = str.size();
	size_t data_size = (str_size - 2) / 2;
	bytea data(data_size, 0);
	for(size_t i = 0; i < data_size; ++i)
	{
		data[i] = (decfromhex((int)str[2 + 2*i]) << 4) + decfromhex((int)str[2 + 2*i + 1]);
	}

    return data;
}

//===============================================================================
QueryResult::Row::Row(QueryResult* query_result, size_t row_index)
    : query_result_(query_result), row_index_(row_index)
{
    //pass
}

//===============================================================================
QueryResult::Row::Row(QueryResult::Row&& row)
    : query_result_(row.query_result_), row_index_(row.row_index_)
{
    row.query_result_ = nullptr;
    row.row_index_ = -1;
}

//===============================================================================
QueryResult::Row::~Row()
{
    //pass
}

//===============================================================================
QueryResult::Row& QueryResult::Row::operator=(QueryResult::Row&& row)
{
    query_result_ = row.query_result_;
    row_index_ = row.row_index_;

    row.query_result_ = nullptr;
    row.row_index_ = -1;

    return *this;
}

//===============================================================================
bool QueryResult::Row::is_null(size_t column_index)
{
    return query_result_->is_null(row_index_, column_index);
}

//===============================================================================
bool QueryResult::Row::is_null(std::string column_name)
{
    return query_result_->is_null(row_index_, column_name);
}

//===============================================================================
size_t QueryResult::Row::size()
{
    return query_result_->num_columns();
}
//===============================================================================
QueryResult::iterator::iterator(QueryResult* query_result, size_t current_row_index)
    : query_result_(query_result), current_row_index_(current_row_index)
{
    //pass
}

//===============================================================================
QueryResult::iterator::iterator(const QueryResult::iterator& it)
    : query_result_(it.query_result_), current_row_index_(it.current_row_index_)
{
    //pass
}

//===============================================================================
QueryResult::iterator::~iterator()
{
    //pass
}

//===============================================================================
QueryResult::iterator& QueryResult::iterator::operator++()
{
    ++current_row_index_;
    return *this;
}

//===============================================================================
bool QueryResult::iterator::operator==(const QueryResult::iterator& rhs)
{
    return ((query_result_ == rhs.query_result_) && (current_row_index_ == rhs.current_row_index_));
}

//===============================================================================
bool QueryResult::iterator::operator!=(const QueryResult::iterator& rhs)
{
    return !operator==(rhs);
}

//===============================================================================
QueryResult::Row QueryResult::iterator::operator*()
{
    return std::move(Row(query_result_, current_row_index_));
}

//===============================================================================
QueryResult::QueryResult(QueryResult&& query_result)
    : pg_result_(query_result.pg_result_), result_size_(query_result.result_size_)
{
    query_result.pg_result_ = nullptr;
}

//===============================================================================
QueryResult::QueryResult(PGresult* pg_result)
    : pg_result_(pg_result)
{
    load_result_size_value();
}

//===============================================================================
QueryResult::~QueryResult()
{
    if(pg_result_ != nullptr)
    {
        PQclear(pg_result_);
        pg_result_ = nullptr;
    }
}

//===============================================================================
QueryResult& QueryResult::operator=(QueryResult&& query_result)
{
    pg_result_ = query_result.pg_result_;
    query_result.pg_result_ = nullptr;
    return *this;
}

//===============================================================================
bool QueryResult::is_null(size_t row_index, size_t column_index)
{
    return 1 == PQgetisnull(pg_result_, row_index, column_index);
}

//===============================================================================
bool QueryResult::is_null(size_t row_index, std::string column_name)
{
    return is_null(row_index, get_column_index(column_name));
}

//===============================================================================
size_t QueryResult::get_column_index(std::string column_name)
{
    int index = PQfnumber(pg_result_, column_name.c_str());
    if(index == -1)
    {
        throw ResultException(std::string("Column name ") + column_name + std::string(" does not exist"), get_pg_result_error());
    }
    return index;
}

//===============================================================================
char* QueryResult::get_raw_value(size_t row_index, size_t column_index)
{
    if(row_index > result_size_.first)
    {
        throw ResultException("Row index is out of bound");
    }

    if(column_index > result_size_.second)
    {
        throw ResultException("Column index is out of bound");
    }

    return PQgetvalue(pg_result_, row_index, column_index);
}

//===============================================================================
char* QueryResult::get_raw_value(size_t row_index, std::string column_name)
{
	return get_raw_value(row_index, get_column_index(column_name));
}

//===============================================================================
QueryResult::Status QueryResult::status()
{
    QueryResult::Status status = QueryResult::Status::BAD;
    switch(PQresultStatus(pg_result_))
    {
        case PGRES_EMPTY_QUERY: // why did you sent an empty query, dude? :-)
        case PGRES_BAD_RESPONSE:
        case PGRES_FATAL_ERROR:
            status = QueryResult::Status::BAD;
            break;

        default:
        case PGRES_COMMAND_OK:
        case PGRES_TUPLES_OK:
        case PGRES_SINGLE_TUPLE:
            status = QueryResult::Status::OK;
            break;
    }
    return status;
}

//===============================================================================
std::string QueryResult::get_pg_result_error()
{
    return std::string(PQresultErrorMessage(pg_result_));
}

//===============================================================================
size_t QueryResult::num_rows()
{
    return result_size_.first;
}

//===============================================================================
size_t QueryResult::num_columns()
{
    return result_size_.second;
}

//===============================================================================
std::pair<size_t, size_t>& QueryResult::size()
{
    return result_size_;
}

//===============================================================================
QueryResult::iterator QueryResult::begin()
{
    return QueryResult::iterator(this, 0);
}

//===============================================================================
QueryResult::iterator QueryResult::end()
{
    return QueryResult::iterator(this, PQntuples(pg_result_));
}

//===============================================================================
void QueryResult::load_result_size_value()
{
    result_size_ = std::pair<size_t, size_t>(PQntuples(pg_result_), PQnfields(pg_result_));
}

//===============================================================================
Results::Results()
{
    //pass
}

//===============================================================================
Results::Results(size_t n)
{
    reserve(n);
}

//===============================================================================
Results::Results(Results&& results)
{
    results_list_ = std::move(results.results_list_);
}

//===============================================================================
Results& Results::operator=(Results&& results)
{
    results_list_ = std::move(results.results_list_);
    return *this;
}

//===============================================================================
QueryResult& Results::statement(size_t index)
{
    return results_list_.at(index);
}

//===============================================================================
QueryResult& Results::operator[](size_t index)
{
    return results_list_.at(index);
}

//===============================================================================
void Results::add(QueryResult&& query_result)
{
    results_list_.push_back(std::move(query_result));
}

//===============================================================================
Results::results_list_t::iterator Results::begin()
{
    return results_list_.begin();
}

Results::results_list_t::iterator Results::end()
{
    return results_list_.end();
}

//===============================================================================
void Results::reserve(size_t n)
{
    results_list_.reserve(n);
}

//===============================================================================
size_t Results::size()
{
    return results_list_.size();
}



}


