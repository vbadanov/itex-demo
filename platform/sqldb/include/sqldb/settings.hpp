
#ifndef SQLDB_SETTINGS_HPP_
#define SQLDB_SETTINGS_HPP_

#include <cstddef>

namespace sqldb
{

//===============================================================================
struct ConnectionSettings
{
    size_t connection_timeout_msec;  // timeout for connection creation
    size_t command_timeout_msec;     // timeout for query/command execution
};

//===============================================================================
struct ConnectionPoolSettings
{
    size_t pool_shrink_period_msec;          // time period to check amount of unused connections and remove extra capacity from the pool
    size_t max_connections_limit;            // maximum size of connection pool
    ConnectionSettings connection_settings;  // settings for every connection
};


}

#endif //SQLDB_SETTINGS_HPP_
