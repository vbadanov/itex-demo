
#ifndef AUX_TIMEOUT_GUARD_HPP_
#define AUX_TIMEOUT_GUARD_HPP_

#include <chrono>
#include <functional>
#include <boost/asio/steady_timer.hpp>

namespace aux
{

typedef std::function<void(const boost::system::error_code& ec)> handler_t;

class timeout_guard
{
    public:
		//===============================================================================
		timeout_guard(boost::asio::steady_timer& timer, const boost::asio::steady_timer::duration& timeout, handler_t handler)
			: timer_(timer)
		{
			timer_.expires_from_now(timeout);
			timer_.async_wait(handler);
		}

		//===============================================================================
		~timeout_guard()
		{
			timer_.cancel();
		}

    private:
        boost::asio::steady_timer& timer_;
};

}
#endif //AUX_TIMEOUT_GUARD_HPP_
