#include "wsg_return_receiver.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

namespace schunk_driver {

WsgReturnReceiver::WsgReturnReceiver(
    const char* local_addr, in_port_t local_port,
    const char* gripper_addr, in_port_t gripper_port)
    : fd_(socket(AF_INET, SOCK_DGRAM, 0)),
      local_sockaddr_({.sin_family = AF_INET,
              .sin_port = htons(local_port),
              .sin_addr = { local_addr
                            ? inet_addr(local_addr)
                            : INADDR_ANY }}),
      gripper_sockaddr_({.sin_family = AF_INET,
              .sin_port = htons(gripper_port),
              .sin_addr = { inet_addr(gripper_addr) }}) {
  assert(fd_ > 0);
  int bind_result = bind(fd_, (struct sockaddr *) &local_sockaddr_,
                         sizeof(struct sockaddr_in));
  if (bind_result != 0) {
    std::cerr << "bind failed: " << errno << std::endl;
    assert(bind_result == 0);
  }
}

WsgReturnReceiver::~WsgReturnReceiver() { close(fd_); }

std::unique_ptr<WsgReturnMessage> WsgReturnReceiver::Receive() {
  unsigned char buffer[1024];
  struct sockaddr_storage src_sockaddr;
  socklen_t src_sockaddr_len = sizeof(src_sockaddr);
  ssize_t read_size = recvfrom(
      fd_, buffer, sizeof(buffer), MSG_DONTWAIT,
      (struct sockaddr*) &src_sockaddr, &src_sockaddr_len);
  // TODO(ggould-tri) check that src_sockaddr matchines gripper_sockaddr_

  if (read_size < 0) {
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
      return std::unique_ptr<WsgReturnMessage>(nullptr);
    } else {
      std::cerr << "Error reading from UDP socket" << errno
                << " " << strerror(errno) << std::endl;
      assert(read_size >= 0);
      ::abort();
    }
  } else if (read_size == sizeof(buffer)) {
    std::cerr << "received unreasonably large datagram" << std::endl;
    assert(read_size < static_cast<int>(sizeof(buffer)));
    ::abort();
  } else {
    std::vector<unsigned char> message_buffer(read_size);
    memcpy(message_buffer.data(), buffer, read_size);
    return WsgReturnMessage::Parse(message_buffer);
  }
}

}  // namespace schunk_driver
