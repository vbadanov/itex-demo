
#ifndef ORDERBOOK_ENTRY_HPP_
#define ORDERBOOK_ENTRY_HPP_

#include <vector>
#include <list>
#include <atomic>

#include <boost/circular_buffer.hpp>
#include <boost/asio/spawn.hpp>

#include <aux/spinlock.hpp>
#include <sqldb/database.hpp>
#include <sqldb/transaction.hpp>

namespace itex
{

namespace quant
{

//===============================================================================
class OrderbookEntry
{
public:
	OrderbookEntry();
	OrderbookEntry(size_t orderbook_depth);
	OrderbookEntry(const OrderbookEntry& e) noexcept;
	OrderbookEntry(OrderbookEntry&& e) noexcept;
	~OrderbookEntry();
	OrderbookEntry& operator=(const OrderbookEntry& e) noexcept;
	OrderbookEntry& operator=(OrderbookEntry&& e) noexcept;
	size_t orderbook_depth();
	double& security_id();
	double& timestamp();
	double* bids();
	double* asks();
	double* data_ptr();
	size_t size();
	void clear();
	void resize(size_t orderbook_depth);

private:
	size_t orderbook_depth_;
	size_t size_;
	double* data_;
};

}
}

#endif /* ORDERBOOK_ENTRY_HPP_ */
