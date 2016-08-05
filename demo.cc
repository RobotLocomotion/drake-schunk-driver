#include <vector>

#include "wsg.h"

using schunk_driver::WsgCommandMessage;
using schunk_driver::WsgReturnMessage;

int main(int argc, char** argv) {
  schunk_driver::Wsg wsg;

  std::vector<unsigned char> arg_bytes = {0x00};
  WsgCommandMessage command(schunk_driver::kHome, arg_bytes);

  std::unique_ptr<WsgReturnMessage> response =
      wsg.SendAndAwaitResponse(command, 0.01);

  return 0;
}
