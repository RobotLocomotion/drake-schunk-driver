#include "wsg_command_message.h"

#include <cstring>
#include <vector>

#include "crc.h"

namespace schunk_driver {

void WsgCommandMessage::Serialize(std::vector<unsigned char>& buffer) const {
  buffer.resize(payload_.size() + 8);
  buffer[0] = 0xaa;
  buffer[1] = 0xaa;
  buffer[2] = 0xaa;
  buffer[3] = command_ & 0xff;
  buffer[4] = payload_.size() & 0xFF;
  buffer[5] = (payload_.size() << 8) & 0xFF;
  memcpy(buffer.data() + 6, payload_.data(), payload_.size());
  uint16_t crc = checksum_update_crc16(buffer.data(), payload_.size() + 6);
  buffer[payload_.size() + 6] = crc & 0xFF;
  buffer[payload_.size() + 7] = (crc >> 8) & 0xFF;
}

}  // namespace schunk_driver
