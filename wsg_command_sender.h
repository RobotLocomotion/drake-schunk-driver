#pragma once

#include <cassert>
#include <iomanip>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

#include "defaults.h"
#include "wsg_command_message.h"

namespace schunk_driver {

class WsgCommandSender {
 public:
  WsgCommandSender(const char* local_addr, in_port_t local_port,
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
#if 0
    int bind_result = bind(fd_, (struct sockaddr *) &local_sockaddr_,
                           sizeof(struct sockaddr_in));
    if (bind_result != 0) {
      std::cerr << "bind failed: " << errno << std::endl;
      assert(bind_result == 0);
    }
#endif
  }

  WsgCommandSender()
      : WsgCommandSender(nullptr, kLocalPort, kGripperAddrStr, kGripperPort)
  {}

  ~WsgCommandSender() { close(fd_); }

  void Send(const WsgCommandMessage& msg) {
    std::vector<unsigned char> data_to_send;
    msg.Serialize(data_to_send);
    for (const auto& c : data_to_send) {
      std::cout << std::setw(2) << std::setfill('0') << std::hex
                << static_cast<int>(c);
    }
    size_t send_result = sendto(fd_,
                                data_to_send.data(), data_to_send.size(),
                                0,
                                (struct sockaddr *) &gripper_sockaddr_,
                                sizeof(struct sockaddr_in));
    assert(send_result == data_to_send.size());
    std::cout << "  sent!" << std::endl;
  }

 private:
  const int fd_;
  const struct sockaddr_in local_sockaddr_;
  const struct sockaddr_in gripper_sockaddr_;
};

}  // namespace schunk_driver
