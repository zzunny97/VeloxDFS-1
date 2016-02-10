#pragma once

#include "../messages/message.hh"
#include "../common/context.hh"

#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

namespace eclipse {
namespace network {

using boost::asio::ip::tcp;
using namespace messages;
using namespace boost::asio;
using namespace boost::system;
using namespace network;

class Channel {
  public:
    Channel (Context&, int);
    ~Channel () = default;

    virtual void on_connect () = 0;
    virtual void do_read () = 0;
    virtual void do_write (messages::Message*) = 0; 
    void update_recv(tcp::socket*);

    const int header_size = 16;
    tcp::socket* send_socket, *recv_socket;

  protected:
    Logger* logger = nullptr;
    io_service& iosvc;
    int id;
};
}
}
