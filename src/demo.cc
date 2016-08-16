#include <cassert>
#include <vector>

#include "wsg.h"

int main(int argc, char** argv) {
  schunk_driver::Wsg wsg;

  assert(wsg.Home(schunk_driver::Wsg::kPositive));
  assert(wsg.Home(schunk_driver::Wsg::kNegative));
  assert(wsg.Home(schunk_driver::Wsg::kPositive));

  assert(wsg.Preposition(schunk_driver::Wsg::kPrepositionClampOnBlock,
                         schunk_driver::Wsg::kPrepositionAbsolute,
                         80, 30));
  assert(wsg.Grasp(62, 200));

  return 0;
}
