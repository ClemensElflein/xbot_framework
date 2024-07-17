//
// Created by clemens on 7/14/24.
//

#ifndef IO_H
#define IO_H
#include <xbot-service/ServiceIo.h>
namespace xbot::service {
class Io {
 public:
  // Register a new service for IO.
  // This service will then be able to receive
  static bool registerServiceIo(ServiceIo* service);

  static bool start();
};
}  // namespace xbot::service

#endif  // IO_H
