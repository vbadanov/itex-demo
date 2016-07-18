
#ifndef ORDERBOOK_QUEUE_WRITER_HPP_
#define ORDERBOOK_QUEUE_WRITER_HPP_

#include <queue>
#include <vector>
#include <atomic>

#include <boost/asio/spawn.hpp>

#include <aux/spinlock.hpp>
#include <sqldb/database.hpp>
#include <sqldb/transaction.hpp>

namespace itex
{

namespace demo_manager
{

class OrderbookQueueWriter
{
	public:
		struct QueueEntry
		{
			unsigned long long security_id;
			double timestamp;
			std::vector<double> prices;
			std::vector<double> volumes;
		};

		OrderbookQueueWriter(boost::asio::io_service& is, sqldb::Database& db);
		void start();
		void stop();

		void push(const QueueEntry& entry);

	private:
		boost::asio::io_service& is_;
		sqldb::Database& db_;
		std::queue<QueueEntry> queue_;
		aux::spinlock queue_lock_;
		volatile bool running_;
};




}
}


#endif /* ORDERBOOK_QUEUE_WRITER_HPP_ */
