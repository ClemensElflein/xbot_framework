//
// Created by clemens on 7/14/24.
//

#ifndef IO_H
#define IO_H
#include <ServiceIo.h>
namespace xbot::comms {
class Io {
 public:
  // Register a new service for IO.
  // This service will then be able to receive
  static bool registerServiceIo(ServiceIo* service);

  static bool start();
};
}  // namespace xbot::comms

#endif  // IO_H
