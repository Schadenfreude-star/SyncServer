#include <iostream>
#include <boost/asio.hpp>
#include <set>
using boost::asio::ip::tcp;
const int MAX_LENGTH = 1024;
typedef std::shared_ptr<tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;

void session(socket_ptr sock)
{
	try
	{
		for (;;)
		{
			char data[MAX_LENGTH];
			memset(data, '\0', MAX_LENGTH);
			boost::system::error_code error;
			// size_t length = boost::asio::read(sock, boost::asio::buffer(data, MAX_LENGTH), error);
			size_t length = sock->read_some(boost::asio::buffer(data, MAX_LENGTH), error);
			if (error == boost::asio::error::eof)
			{
				std::cout << "Connection closed by peer.";
				break;
			}
			if (error)
			{
				throw boost::system::system_error(error);
			}

			std::cout << "Received from " << sock->remote_endpoint().address().to_string() << std::endl;
			std::cout << "Received message is " << data << std::endl;
			// 回传给对方
			write(*sock, boost::asio::buffer(data, length));
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

void server(boost::asio::io_context& ioc, unsigned short port_num)
{
	tcp::acceptor a(ioc, tcp::endpoint(tcp::v4(), port_num));
	for (;;)
	{
		socket_ptr socket(new tcp::socket(ioc));
		a.accept(*socket);
		auto t = std::make_shared<std::thread>(session, socket);
		thread_set.insert(t);
	}
}

int main()
{
	try
	{
		boost::asio::io_context ioc;
		server(ioc, 10086);
		for (auto& t : thread_set)
		{
			t->join();
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}
