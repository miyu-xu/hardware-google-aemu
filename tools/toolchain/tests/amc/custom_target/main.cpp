#include "message.h"
#include "msg_1.h"
#include "my_pkgconfig_header.h"

#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << generated::msg0 << std::endl;
  std::cout << generated::msg1 << std::endl;
  std::cout << MY_PKGCONFIG_GREETING << std::endl;
  return 0;
}
