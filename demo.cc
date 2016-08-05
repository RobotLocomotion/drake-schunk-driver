#include <vector>

#include "wsg_command_message.h"
#include "wsg_command_sender.h"
#include "wsg_return_message.h"
#include "wsg_return_receiver.h"

using schunk_driver::WsgCommandMessage;
using schunk_driver::WsgCommandSender;
using schunk_driver::WsgReturnMessage;
using schunk_driver::WsgReturnReceiver;

int main(int argc, char** argv) {
  WsgCommandSender tx;
  WsgReturnReceiver rx;
  std::vector<unsigned char> arg_bytes = {0x00};
  WsgCommandMessage command(schunk_driver::kHome, arg_bytes);
  tx.Send(command);
  std::unique_ptr<WsgReturnMessage> response(nullptr);
  while (!response) {
    response = rx.Receive();
  }
  return 0;
}
