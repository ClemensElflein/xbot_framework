//
// Created by clemens on 4/23/24.
//

#ifndef TransactionalService_HPP
#define TransactionalService_HPP

#include "TransactionalServiceBase.hpp"

class TransactionalService : public TransactionalServiceBase {
 public:
  explicit TransactionalService(uint16_t service_id)
      : TransactionalServiceBase(service_id, 1000000) {}

 private:
  void tick() override;
};

#endif  // TransactionalService_HPP
