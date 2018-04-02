#include "wsg_return_message.h"

#include <cassert>
#include <cstring>

namespace schunk_driver {

std::unique_ptr<WsgReturnMessage>
WsgReturnMessage::Parse(std::vector<unsigned char>& buffer) {
  assert(buffer.size() >= 10);
  assert(buffer[0] == 0xaa);
  assert(buffer[1] == 0xaa);
  assert(buffer[2] == 0xaa);
  int command = buffer[3];
  int payload_size = buffer[4] + (buffer[5] << 8);
  assert(payload_size >= 2);
  assert(static_cast<int>(buffer.size()) == payload_size + 8);
  int status_code = buffer[6] + (buffer[7] << 8);
  int params_size = payload_size - 2;
  (void)(params_size);  // (Unused.)
  std::vector<unsigned char> params(&buffer[8], &buffer[payload_size + 6]);
  // TODO(ggould-tri) Validate checksum.
  return std::unique_ptr<WsgReturnMessage>(
      new WsgReturnMessage(command, status_code, params));
}


}  // namespace schunk_driver
