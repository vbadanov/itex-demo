
#ifndef WORKSET_HPP_
#define WORKSET_HPP_

#include <vector>
#include <list>
#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>
#include <cmath>
#include <map>

#include <boost/circular_buffer.hpp>
#include <boost/asio/spawn.hpp>


#include <aux/spinlock.hpp>
#include <aux/rw_lock.hpp>

namespace itex
{

namespace quant
{

//===============================================================================
template<class Entry>
class Workset
{
	public:
		//===============================================================================
		class Publisher
		{
			friend class Workset;

			public:
				//===============================================================================
				void push(const Entry& entry)
				{
					Entry e = entry;
					push(std::move(e));
				}

				//===============================================================================
				void push(Entry&& entry)
				{
					ws_.workset_lock_.writer_lock();
					if(ws_.full())
					{
						for(auto subscriber : ws_.subscribers_)
						{
							subscriber->current_index_ = ((subscriber->current_index_ > 0) ? (subscriber->current_index_ - 1) : 0);
						}
					}
					ws_.push_back(entry);
					ws_.workset_lock_.writer_unlock();
				}

				//===============================================================================
				void release()
				{
					ws_.remove_publisher(this);
				}

			protected:
				//===============================================================================
				Publisher() = delete;

				//===============================================================================
				Publisher(Workset& ws)
					: ws_(ws)
				{
					//pass
				}

				//===============================================================================
				~Publisher()
				{
					//pass
				};

			private:
				Workset& ws_;

		};

		//===============================================================================
		//===============================================================================
		class Subscriber
		{
			friend class Workset;

			public:
				//===============================================================================
				bool try_pop(Entry& entry)
				{
					bool res = false;
					if(!ws_.workset_lock_.reader_try_lock())
					{
						return false;
					}

					if(!ws_.empty() && current_index_ < ws_.size())
					{
						entry = ws_[current_index_];
						++current_index_;
						res = true;
					}

					ws_.workset_lock_.reader_unlock();
					return res;
				}

				//===============================================================================
				void release()
				{
					ws_.remove_subscriber(this);
				}

				//===============================================================================
				size_t num_available()
				{
					size_t available = 0;
					ws_.workset_lock_.reader_lock();
					available = ws_.size() - current_index_;
					ws_.workset_lock_.reader_unlock();
					return available;
				}

				//===============================================================================
				Subscriber* clone()
				{
					Subscriber* new_subscriber = ws_.get_subscriber();
					new_subscriber->current_index_ = current_index_;
					return new_subscriber;
				}

			protected:

				//===============================================================================
				Subscriber() = delete;

				//===============================================================================
				Subscriber(Workset& ws)
					: ws_(ws), current_index_(0)
				{
					//pass
				}

				//===============================================================================
				~Subscriber()
				{
					//pass
				};

			private:
				Workset& ws_;
				size_t current_index_;

		};


		//===============================================================================
		friend class Publisher;
		friend class Subscriber;

		//===============================================================================
		Workset() = delete;

		//===============================================================================
		Workset(size_t capacity)
			: capacity_(capacity)
		{
			workset_.set_capacity(capacity_);
		}

		//===============================================================================
		~Workset()
		{
			for(auto& e : publishers_)
			{
				delete e;
			}
			publishers_.clear();

			for(auto& e : subscribers_)
			{
				delete e;
			}
			subscribers_.clear();
		};

		//===============================================================================
		Publisher* get_publisher()
		{
			workset_lock_.writer_lock();
			typename Workset<Entry>::Publisher* publisher = new typename Workset<Entry>::Publisher(*this);
			publishers_.push_back(publisher);
			workset_lock_.writer_unlock();
			return publisher;
		}

		//===============================================================================
		Subscriber* get_subscriber()
		{
			workset_lock_.writer_lock();
			Workset<Entry>::Subscriber* subscriber = new Workset<Entry>::Subscriber(*this);
			subscribers_.push_back(subscriber);
			workset_lock_.writer_unlock();
			return subscriber;
		}

		//===============================================================================
		size_t capacity()
		{
			return capacity_;
		}

		//===============================================================================
		size_t size()
		{
			return workset_.size();
		}

		//===============================================================================
		bool empty()
		{
			return workset_.empty();
		}

		//===============================================================================
		bool full()
		{
			return workset_.full();
		}


	protected:
		//===============================================================================
		void push_back(const Entry& entry)
		{
			workset_.push_back(entry);
		}

		//===============================================================================
		void push_back(Entry&& entry)
		{
			workset_.push_back(entry);
		}

		//===============================================================================
		Entry& operator[](size_t index)
		{
			return workset_[index];
		}

		//===============================================================================
		void remove_publisher(Publisher* publisher)
		{
			workset_lock_.writer_lock();
			publishers_.remove(publisher);
			delete publisher;
			workset_lock_.writer_unlock();
		}

		//===============================================================================
		void remove_subscriber(Subscriber* subscriber)
		{
			workset_lock_.writer_lock();
			subscribers_.remove(subscriber);
			delete subscriber;
			workset_lock_.writer_unlock();
		}


	private:
		size_t capacity_;
		boost::circular_buffer<Entry> workset_;
		std::list<Publisher*> publishers_;
		std::list<Subscriber*> subscribers_;
		aux::rw_lock workset_lock_;
};

}
}

#endif /* WORKSET_HPP_ */
