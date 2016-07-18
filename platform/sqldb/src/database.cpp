
#include <boost/asio/use_future.hpp>
#include <sqldb/database.hpp>

namespace sqldb
{

//===============================================================================
Database::Database(boost::asio::io_service& is, const std::string& connection_string, const ConnectionPoolSettings& conn_pool_settings)
    : is_(is),
      connection_string_(connection_string),
      pool_settings_(conn_pool_settings),
      pool_shrink_timer_(is_, std::chrono::milliseconds(pool_settings_.pool_shrink_period_msec)),
      pool_stats_per_period_ {0,0}
{
    pool_shrink_timer_.async_wait(std::bind(&Database::pool_shrink_timer_handler, this, std::placeholders::_1));
}

//===============================================================================
Database::~Database()
{
    std::lock_guard<aux::spinlock> lg(pool_lock_);

    for(auto elt : pool_queue_)
    {
        if(elt != nullptr)
        {
            delete elt;
        }
    }
    pool_queue_.clear();
}

//===============================================================================
boost::asio::io_service& Database::get_io_service()
{
    return is_;
}

//===============================================================================
Connection* Database::async_get_connection(boost::asio::yield_context& yield)
{
    Connection* connection = nullptr;
    { // lock pool only for
        std::lock_guard<aux::spinlock> lg(pool_lock_);

        while(pool_queue_.size() > 0)
        {
            connection = pool_queue_.front();
            pool_queue_.pop_front();
            pool_stats_per_period_.update(pool_queue_.size());

            if(connection == nullptr) { continue; }

            if(connection->status() == Connection::Status::BAD)
            {
                delete connection;
                connection = nullptr;
            }
            else if(connection->status() == Connection::Status::CONNECTED)
            {
                break;
            }
        };
    }

    // pool is not locked here! - this is by desing - just return connection ptr without adding it to pool (it will be returned to pool later)

    if(connection == nullptr)
    {
        //create new connection
        connection = new Connection(*this, connection_string_, pool_settings_.connection_settings);
        while(connection != nullptr &&  connection->status() != Connection::Status::CONNECTED)
        {
            try
            {
                connection->async_connect(yield);
            }
            catch (ConnectionException& e)
            {
                //TODO: add logging here
            }
        }
    }

    return connection;
}

//===============================================================================
void Database::return_connection(Connection* connection)
{
    std::lock_guard<aux::spinlock> lg(pool_lock_);
    pool_queue_.push_back(connection);
    pool_stats_per_period_.update(pool_queue_.size());
}

//===============================================================================
void Database::pool_shrink_timer_handler(const boost::system::error_code& ec)
{
    std::lock_guard<aux::spinlock> lg(pool_lock_);

    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    // removing extra capacity - closing excess connections to DB server = saving nature :-)
    size_t current_size = pool_queue_.size();
    size_t average_size = (pool_stats_per_period_.counter == 0) ? 0 : (pool_stats_per_period_.cumulative_size / pool_stats_per_period_.counter);
    size_t num_to_shrink = (current_size > average_size) ? (current_size - average_size)/2 : 0;
    for(size_t i = 0; i < num_to_shrink; ++i)
    {
        delete pool_queue_.front();
        pool_queue_.pop_front();
    }

    // start new iteration
    pool_stats_per_period_ = {0, 0};
    pool_shrink_timer_.expires_from_now(std::chrono::milliseconds(pool_settings_.pool_shrink_period_msec));
    pool_shrink_timer_.async_wait(std::bind(&Database::pool_shrink_timer_handler, this, std::placeholders::_1));
}

//===============================================================================
void PoolStats::update(size_t value)
{
    counter++;
    cumulative_size += value;
}


}
