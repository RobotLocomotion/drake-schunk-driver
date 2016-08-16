#pragma once

#include <netinet/in.h>

namespace schunk_driver {

// Device preconfigured defaults:
const static in_port_t kLocalPort = 1501;
const static in_port_t kGripperPort = 1500;
const static char* kGripperAddrStr = "192.168.1.20";

}  // namespace schunk_driver
