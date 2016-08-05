#include <vector>

#include "wsg_command_message.h"
#include "wsg_command_sender.h"

using schunk_driver::WsgCommandMessage;
using schunk_driver::WsgCommandSender;

int main(int argc, char** argv) {
  WsgCommandSender sender;
  std::vector<unsigned char> arg_bytes = {0x00};
  WsgCommandMessage msg(schunk_driver::kHome,
                        arg_bytes.data(), arg_bytes.size());
  sender.send(msg);
  return 0;
}
