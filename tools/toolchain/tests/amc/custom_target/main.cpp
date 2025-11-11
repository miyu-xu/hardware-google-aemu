#include "message.h"
#include "msg_1.h"
#include "my_pkgconfig_header.h"
#include "msg_2.h"
#include "msg_3.h"

#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << generated::msg0 << std::endl;
  std::cout << generated::msg1 << std::endl;
  std::cout << MY_PKGCONFIG_GREETING << std::endl;
  std::cout << generated::msg2 << std::endl;
  std::cout << generated::msg3 << std::endl;
  return 0;
}
