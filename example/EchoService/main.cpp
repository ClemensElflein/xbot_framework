
#include <ulog.h>
#include <unistd.h>

#include <Io.hpp>
#include <cstdio>
#include <mutex>

#include "EchoService.hpp"
#include "RemoteLogging.hpp"
#include "portable/system.hpp"

#ifdef ULOG_ENABLED
void console_logger(ulog_level_t severity, char* msg, const void* arg) {
  printf("[%s]: %s\n", ulog_level_name(severity), msg);
}
#endif

int main() {
  xbot::comms::system::initSystem();
#ifdef ULOG_ENABLED
  ULOG_SUBSCRIBE(console_logger, ULOG_DEBUG_LEVEL);
#endif

  startRemoteLogging();
  EchoService service{1};

  service.start();

  xbot::comms::Io::start();

  while (1) {
    sleep(1);
  }
}
