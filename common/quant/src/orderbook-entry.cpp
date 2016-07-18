
#include <thread>
#include <mutex>
#include <iostream>
#include <cmath>
#include <map>

#include <aux/utils.hpp>
#include <quant/orderbook-entry.hpp>

namespace itex
{

namespace quant
{

//===============================================================================
OrderbookEntry::OrderbookEntry()
	: orderbook_depth_(0), size_(0), data_(nullptr)
{
	//pass
}

//===============================================================================
OrderbookEntry::OrderbookEntry(size_t orderbook_depth)
	: orderbook_depth_(orderbook_depth),
	  size_((orderbook_depth * 2 * 2) + 2),
	  data_(aux::allocate_buffer<double>(size_))
{
	clear();
}

//===============================================================================
OrderbookEntry::OrderbookEntry(const OrderbookEntry& e) noexcept
	: orderbook_depth_(e.orderbook_depth_),
	  size_(e.size_),
	  data_(aux::allocate_buffer<double>(size_))
{
	std::copy(e.data_, e.data_ + e.size_, data_);
}

//===============================================================================
OrderbookEntry::OrderbookEntry(OrderbookEntry&& e) noexcept
	: orderbook_depth_(e.orderbook_depth_),
	  size_(e.size_),
	  data_(e.data_)
{
	e.orderbook_depth_ = 0;
	e.size_ = 0;
	e.data_ = nullptr;
}

//===============================================================================
OrderbookEntry::~OrderbookEntry()
{
	if(data_ != nullptr)
	{
		free(data_);
	}
}

//===============================================================================
OrderbookEntry& OrderbookEntry::operator=(const OrderbookEntry& e) noexcept
{
	OrderbookEntry new_orderbook_entry(e);
	*this = std::move(new_orderbook_entry);
	return *this;
}

//===============================================================================
OrderbookEntry& OrderbookEntry::operator=(OrderbookEntry&& e) noexcept
{
	orderbook_depth_ = e.orderbook_depth_;
	e.orderbook_depth_ = 0;

	size_ = e.size_;
	e.size_ = 0;

	if(data_ != nullptr)
	{
		free(data_);
	}
	data_ = e.data_;
	e.data_ = nullptr;

	return *this;
}

//===============================================================================
size_t OrderbookEntry::orderbook_depth()
{
	return orderbook_depth_;
}

//===============================================================================
double& OrderbookEntry::security_id()
{
	return data_[0];
}

//===============================================================================
double& OrderbookEntry::timestamp()
{
	return data_[1];
}

//===============================================================================
double* OrderbookEntry::bids()
{
	return data_ + 2;
}

//===============================================================================
double* OrderbookEntry::asks()
{
	return data_ + 2 + orderbook_depth_ * 2;
}

//===============================================================================
double* OrderbookEntry::data_ptr()
{
	return data_;
}

//===============================================================================
size_t OrderbookEntry::size()
{
	return size_;
}

//===============================================================================
void OrderbookEntry::clear()
{
	if(data_ != nullptr && size_ > 0)
	{
		std::fill_n(data_, size_, 0.0);
	}
}

//===============================================================================
void OrderbookEntry::resize(size_t orderbook_depth)
{
	OrderbookEntry e(orderbook_depth);
	*this = std::move(e);
}


}
}
