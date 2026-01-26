#pragma once

#include "srsran/gateways/baseband/baseband_gateway_receiver.h"
#include "srsran/gateways/baseband/buffer/baseband_gateway_buffer_writer.h"
#include "srsran/radio/radio_event_notifier.h"

class LimePluginContext;

namespace srsran {

/// Implements a gateway receiver based on Lime receive stream.
class radio_limesuiteng_rx_stream : public baseband_gateway_receiver
{
private:
  /// Owns the Lime Tx stream.
  std::shared_ptr<LimePluginContext> context;
  unsigned                           portId;

  radio_event_notifier& notifier;
  // srslog::basic_logger& logger;

public:
  /// \brief Constructs a receive Lime stream.
  /// \param[in] usrp Provides the USRP context.
  /// \param[in] description Provides the stream configuration parameters.
  /// \param[in] notifier_ Provides the radio event notification handler.
  radio_limesuiteng_rx_stream(std::shared_ptr<LimePluginContext> context,
                              uint8_t                            portId,
                              radio_event_notifier&              notifier_);

  // See interface for documentation.
  metadata receive(baseband_gateway_buffer_writer& data) override;
};
} // namespace srsran
