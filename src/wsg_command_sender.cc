#include "wsg_command_sender.h"

#include <cassert>
#include <iomanip>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

namespace schunk_driver {

WsgCommandSender::WsgCommandSender(
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
}

WsgCommandSender::~WsgCommandSender() { close(fd_); }

void WsgCommandSender::Send(const WsgCommandMessage& msg) {
  std::vector<unsigned char> data_to_send;
  msg.Serialize(data_to_send);
#ifdef DEBUG
  for (const auto& c : data_to_send) {
    std::cout << std::setw(2) << std::setfill('0') << std::hex
              << static_cast<int>(c);
  }
#endif
  size_t send_result = sendto(fd_,
                              data_to_send.data(), data_to_send.size(),
                              0,
                              (struct sockaddr *) &gripper_sockaddr_,
                              sizeof(struct sockaddr_in));
  assert(send_result == data_to_send.size());
  (void)(send_result);  // Avoid "unused" warning when assertions are off.
#ifdef DEBUG
  std::cout << "  sent!" << std::endl;
#endif
}

}  // namespace schunk_driver
