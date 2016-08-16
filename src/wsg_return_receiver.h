#pragma once

#include <netinet/in.h>

#include "wsg_return_message.h"

namespace schunk_driver {

class WsgReturnReceiver {
 public:
  WsgReturnReceiver(const char* local_addr, in_port_t local_port,
                    const char* gripper_addr, in_port_t gripper_port);

  ~WsgReturnReceiver();

  std::unique_ptr<WsgReturnMessage> Receive();

 private:
  const int fd_;
  const struct sockaddr_in local_sockaddr_;
  const struct sockaddr_in gripper_sockaddr_;
};

}  // namespace schunk_driver
