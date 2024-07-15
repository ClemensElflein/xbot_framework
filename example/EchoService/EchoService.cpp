//
// Created by clemens on 4/23/24.
//

#include "EchoService.hpp"

#include <iostream>

void EchoService::tick() {
  std::cout << "tick" << std::endl;
  SendMessageCount(echo_count++);
}

bool EchoService::OnInputTextChanged(const char *new_value, uint32_t length) {
  std::cout << "input changed" << std::endl;
  SendEcho(new_value, length);
  SendMessageCount(echo_count++);
  return true;
}

bool EchoService::OnSomeIntegerChanged(const uint32_t &new_value) {
  std::cout << "int changed" << std::endl;
  return true;
}
bool EchoService::OnAPetChanged(const Pet &new_value) {
  return SendAPet(new_value);
}
