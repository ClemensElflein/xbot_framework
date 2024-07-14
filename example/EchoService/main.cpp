
#include <ulog.h>
#include <unistd.h>

#include <cstdio>
#include <mutex>

#include "EchoService.hpp"
#include "RemoteLogging.hpp"
#include "portable/system.hpp"

void console_logger(ulog_level_t severity, char* msg, const void* arg) {
  printf("[%s]: %s\n", ulog_level_name(severity), msg);
}

int main() {
  xbot::comms::system::initSystem();
  ULOG_SUBSCRIBE(console_logger, ULOG_DEBUG_LEVEL);
  startRemoteLogging();
  EchoService service{1};

  service.start();

  while (1) {
    sleep(1);
  }
}
