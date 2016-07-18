
#include <chrono>
#include <orderbook-queue-writer.hpp>

namespace itex
{

namespace demo_manager
{

//===============================================================================
OrderbookQueueWriter::OrderbookQueueWriter(boost::asio::io_service& is, sqldb::Database& db)
	: is_(is), db_(db)
{
	//pass
}


//===============================================================================
void OrderbookQueueWriter::start()
{
	running_ = true;
    boost::asio::spawn(is_, [&](boost::asio::yield_context yield)
    {
    	while(running_)
    	{
    		double start_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    		QueueEntry entry;
    		bool is_empty = false;

    		{
    		    std::lock_guard<aux::spinlock> lg(queue_lock_);
    		    is_empty = queue_.empty();
    		    if(!is_empty)
    		    {
    		    	entry = queue_.front();
    		    	queue_.pop();
    		    }
    		}

    		if(is_empty)
    		{
    			//async delay
    	        boost::asio::steady_timer timer(is_);
    	        timer.expires_from_now(std::chrono::milliseconds(1));
    	        timer.async_wait(yield);

    	        continue;
    		}

    		sqldb::Transaction tr(db_);
    		sqldb::Results results = tr.async_exec("INSERT INTO orderbook_log (security_id, timestamp) VALUES ($1, $2) RETURNING id", sqldb::make_params(entry.security_id, entry.timestamp), yield);

    		unsigned long long orderbook_log_id = results[0].get_value<unsigned long long>(0, "id");

    		std::string insert_statement = "INSERT INTO orderbook_log_details (orderbook_log_id, price, volume) VALUES ";
    		sqldb::ParametersList pl;

    		unsigned int orderbook_size = entry.prices.size();
    		for(unsigned int i = 0; i < orderbook_size; ++i)
    		{
    			insert_statement += (std::string("($") + std::to_string(i*3+1) + ", $" + std::to_string(i*3+2) + ", $" + std::to_string(i*3+3) + ")");
    			if(i < (orderbook_size-1))
    			{
    				insert_statement += ", ";
    			}
    			pl << orderbook_log_id << entry.prices[i] << entry.volumes[i];
    		}
    		tr.async_exec(insert_statement, pl, yield);

    		tr.async_commit(yield);

    		double end_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    	}
    });

}

//===============================================================================
void OrderbookQueueWriter::stop()
{
	running_ = false;
}

//===============================================================================
void OrderbookQueueWriter::push(const QueueEntry& entry)
{
    std::lock_guard<aux::spinlock> lg(queue_lock_);
    queue_.push(entry);
}




}
}



