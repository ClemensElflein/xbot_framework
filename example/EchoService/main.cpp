
#include <Pet_encode.h>
#include <ulog.h>
#include <unistd.h>

#include <cstdio>
#include <mutex>
#include <xbot-service/Io.hpp>

#include "EchoService.hpp"
#include "xbot-service/RemoteLogging.hpp"
#include "xbot-service/portable/system.hpp"

#ifdef ULOG_ENABLED
void console_logger(ulog_level_t severity, char* msg, const void* arg) {
  printf("[%s]: %s\n", ulog_level_name(severity), msg);
}
#endif

int main() {
  Pet encoded_pet{};
  int err;
  uint8_t pet3[20];
  const uint8_t first_name[] = "Gary";
  const uint8_t last_name[] = "Giraffe";
  const uint8_t timestamp3[] = {0x01, 0x02, 0x03, 0x04, 0x0a, 0x0b, 0x0c, 0x0d};

  encoded_pet.names[0].value = first_name;
  encoded_pet.names[0].len = sizeof(first_name) - 1;
  encoded_pet.names[1].value = last_name;
  encoded_pet.names[1].len = sizeof(last_name) - 1;
  encoded_pet.names_count = 2;
  encoded_pet.birthday.value = timestamp3;
  encoded_pet.birthday.len = sizeof(timestamp3);
  encoded_pet.species_choice = Pet::Pet_species_other_c;

  err = cbor_encode_Pet(pet3, sizeof(pet3), &encoded_pet, NULL);

  if (err != ZCBOR_SUCCESS) {
    printf("Encoding failed for pet3: %d\r\n", err);
  }

  xbot::service::system::initSystem();
#ifdef ULOG_ENABLED
  ULOG_SUBSCRIBE(console_logger, ULOG_DEBUG_LEVEL);
#endif

  startRemoteLogging();
  EchoService service{1};

  service.start();

  xbot::service::Io::start();

  while (1) {
    sleep(1);
  }
}
