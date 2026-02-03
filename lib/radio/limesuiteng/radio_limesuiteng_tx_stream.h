#pragma once

#include "srsran/gateways/baseband/baseband_gateway_transmitter.h"
#include "srsran/gateways/baseband/buffer/baseband_gateway_buffer_reader.h"
#include "srsran/radio/radio_event_notifier.h"

class LimePluginContext;

namespace srsran {

/// Implements a gateway receiver based on Lime receive stream.
class radio_limesuiteng_tx_stream : public baseband_gateway_transmitter
{
private:
  /// Owns the Lime Tx stream.
  std::shared_ptr<LimePluginContext> context;
  unsigned                           portId;

  radio_event_notifier& notifier;
  // srslog::basic_logger& logger;
  bool do_work;

public:
  /// \brief Constructs a receive Lime stream.
  /// \param[in] usrp Provides the USRP context.
  /// \param[in] description Provides the stream configuration parameters.
  /// \param[in] notifier_ Provides the radio event notification handler.
  radio_limesuiteng_tx_stream(std::shared_ptr<LimePluginContext> context,
                              uint8_t                            portId,
                              radio_event_notifier&              notifier_);

  // See interface for documentation.
  void transmit(const baseband_gateway_buffer_reader&        data,
                const baseband_gateway_transmitter_metadata& inmeta) override;

  void start();
  void stop();
};
} // namespace srsran
