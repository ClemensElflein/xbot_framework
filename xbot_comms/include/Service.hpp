//
// Created by clemens on 3/18/24.
//

#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <ServiceIo.h>

#include <xbot/config.hpp>

#include "portable/queue.hpp"
#include "portable/thread.hpp"
#include "xbot/datatypes/XbotHeader.hpp"

namespace xbot::comms {
class Service : public ServiceIo {
 public:
  explicit Service(uint16_t service_id, uint32_t tick_rate_micros,
                   void *processing_thread_stack,
                   size_t processing_thread_stack_size);

  virtual ~Service();

  /*
   * @brief Start the service.
   *
   * This method starts the service.
   *
   * @return True if the service started successfully, false otherwise.
   */
  bool start();

  /**
   * Since the portable thread implementation does not know what a class is, we
   * use this helper to start the service.
   * @param service Pointer to the service to start
   * @return null
   */
  static void *startProcessingHelper(void *service) {
    static_cast<Service *>(service)->runProcessing();
    return nullptr;
  }

 protected:
  // Buffer to prepare service advertisements, static to allow reuse between
  // services
  uint8_t sd_buffer[config::max_packet_size - sizeof(datatypes::XbotHeader)];

  // Scratch space for the header. This will only ever be accessed in the
  // process_thread, so we don't need a mutex Don't make it static, so that
  // multiple services can build packets in parallel (it will happen often)
  datatypes::XbotHeader header_{};

  bool SendData(uint16_t target_id, const void *data, size_t size);

 private:
  /**
   * The main thread for the service.
   * Here the implementation can do its processing.
   */
  void *processing_thread_stack_;
  size_t processing_thread_stack_size_;
  XBOT_THREAD_TYPEDEF process_thread_{};

  uint32_t tick_rate_micros_;
  uint32_t last_tick_micros_ = 0;
  uint32_t last_service_discovery_micros_ = 0;
  uint32_t last_heartbeat_micros_ = 0;
  uint32_t heartbeat_micros_ = 0;
  uint32_t target_ip = 0;
  uint32_t target_port = 0;

  void heartbeat();

  void runProcessing();

  void fillHeader();

  bool SendDataClaimAck();

  virtual void tick() = 0;

  virtual bool advertiseService() = 0;

  virtual bool handlePacket(const datatypes::XbotHeader *header,
                            const void *payload) = 0;
};
}  // namespace xbot::comms

#endif  // SERVICE_HPP
