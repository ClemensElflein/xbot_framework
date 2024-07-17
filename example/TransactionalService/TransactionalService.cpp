//
// Created by clemens on 4/23/24.
//

#include "TransactionalService.hpp"

int count1 = 1;
int count2 = 2;
float float3 = 0.1f;

void TransactionalService::tick() {
  count1++;
  count2++;
  float3 += 0.1f;
  StartTransaction();
  SendSyncedCount1(count1);
  SendSyncedCount2(count2);
  SendSyncedFloat(float3);
  CommitTransaction();
  SendNotSyncedCount1(count1);
  SendNotSyncedCount2(count2);
  SendNotSyncedFloat(float3);
}
