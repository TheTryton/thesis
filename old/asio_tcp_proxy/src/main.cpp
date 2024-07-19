#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <span>
#include <print>
#include <format>
#include <iostream>
#include <utility>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::redirect_error;
using boost::asio::use_awaitable;

class proxy_session
    : public std::enable_shared_from_this<proxy_session>
{
private:
    boost::asio::io_context& _context;
    tcp::socket _clientSocket;
    tcp::socket _serverSocket;
public:
    proxy_session(boost::asio::io_context& context, tcp::socket clientSocket, tcp::socket serverSocket)
        : _context(context)
        , _clientSocket(std::move(clientSocket))
        , _serverSocket(std::move(serverSocket))
    {
    }
public:
    void start()
    {
        co_spawn(
            _context,
            [self = shared_from_this()]{ return self->forwardServerToClient(); },
            detached
        );

        co_spawn(
            _context,
            [self = shared_from_this()]{ return self->forwardClientToServer(); },
            detached
        );
    }
private:
    awaitable<void> forward(tcp::socket& from, tcp::socket& to)
    {
        constexpr static size_t bufferSize = 1024*1024;
        std::vector<char> data(bufferSize);

        while(true)
        {
            auto bytesRead = co_await from.async_read_some(boost::asio::buffer(data), use_awaitable);
            if(bytesRead != 0)
                std::print(std::cout, "Read bytes: {}\n", bytesRead);
            co_await async_write(to, boost::asio::buffer(data, bytesRead), use_awaitable);
        }
    }

    awaitable<void> forwardServerToClient()
    {
        co_await forward(_serverSocket, _clientSocket);
    }

    awaitable<void> forwardClientToServer()
    {
        co_await forward(_clientSocket, _serverSocket);
    }
};

//----------------------------------------------------------------------

awaitable<void> start_tcp_proxy(
    boost::asio::io_context& context,
    std::string_view proxiedAddress, std::string_view proxiedPort,
    uint16_t proxyPort
)
{
    tcp::resolver resolver(context);
    tcp::acceptor acceptor(context, {tcp::v4(), proxyPort});

    for (;;)
    {
        std::print(std::cout, "Waiting for next client\n");
        try
        {
            tcp::socket clientSocket = co_await acceptor.async_accept(use_awaitable);
            const auto endpoint = clientSocket.remote_endpoint();
            const auto address = endpoint.address();
            const auto port = endpoint.port();
            std::print(std::cout, "Received connection from: {}:{}\n", address.to_string(), port);
            try
            {
                const auto proxiedResolvedAddress = co_await resolver.async_resolve(proxiedAddress, proxiedPort, use_awaitable);

                tcp::socket serverSocket(context);
                std::print(std::cout, "Trying to establish connection to {}:{}\n", proxiedAddress, proxiedPort);
                try
                {
                    co_await boost::asio::async_connect(serverSocket, proxiedResolvedAddress, use_awaitable);
                    std::print(std::cout, "Established connection to {}:{}\n", proxiedAddress, proxiedPort);

                    try
                    {
                        std::make_shared<proxy_session>(context, std::move(clientSocket), std::move(serverSocket))->start();
                    }
                    catch (const std::exception& exception)
                    {
                        std::print("Failed to start proxy session: {}\n",exception.what());
                    }
                }
                catch (const std::exception& exception)
                {
                    std::print("Failed to establish connection to address: {}:{}, {}\n", proxiedAddress, proxiedPort, exception.what());
                }
            }
            catch (const std::exception& exception)
            {
                std::print("Failed to resolve address: {}:{}, {}\n", proxiedAddress, proxiedPort, exception.what());
            }
        }
        catch (const std::exception& exception)
        {
            std::print("Failed to accept connection: {}\n", exception.what());
        }
    }
}

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    constexpr std::string_view serverAddress = "localhost";
    constexpr std::string_view serverPort = "25565";
    constexpr uint16_t connectionPort = 25566;

    try
    {
        boost::asio::io_context io_context(1);

        co_spawn(
            io_context,
            start_tcp_proxy(io_context, serverAddress, serverPort, connectionPort),
            detached
        );

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ io_context.stop(); });

        io_context.run();
    }
    catch (const std::exception& exception)
    {
        std::print("Finished spoofing: {}", exception.what());
    }

    return 0;
}