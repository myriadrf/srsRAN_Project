#pragma once

#include "radio_config_limesuiteng_validator.h"
#include "radio_limesuiteng_baseband_gateway.h"
#include "srsran/radio/radio_factory.h"
#include "srsran/radio/radio_management_plane.h"

class LimePluginContext;

namespace srsran {

/// Describes a radio session based on Lime that also implements the management and data plane functions.
class radio_session_limesuiteng_impl : public radio_session, private radio_management_plane
{
private:
  /// Baseband gateways.
  std::vector<std::unique_ptr<radio_limesuiteng_baseband_gateway>> bb_gateways;

  /// Asynchronous executor.
  task_executor& async_executor;
  /// Event notifier.
  radio_event_notifier& notifier;

  std::shared_ptr<LimePluginContext> context;

public:
  baseband_gateway_timestamp read_current_time() override;

public:
  /// Constructs a radio session based on Lime.
  radio_session_limesuiteng_impl(const radio_configuration::radio& radio_config,
                                 task_executor&                    async_executor,
                                 radio_event_notifier&             notifier_);

  // See interface for documentation.
  radio_management_plane& get_management_plane() override { return *this; }

  // See interface for documentation.
  baseband_gateway& get_baseband_gateway(unsigned stream_id) override;

  // See interface for documentation.
  void start(baseband_gateway_timestamp init_time) override;

  // See interface for documentation.
  void stop() override;

  // See interface for documentation.
  bool set_tx_gain(unsigned port_idx, double gain_dB) override;

  // See interface for documentation.
  bool set_rx_gain(unsigned port_idx, double gain_dB) override;

  bool set_tx_freq(unsigned stream_id, double center_freq_Hz) override;
  bool set_rx_freq(unsigned stream_id, double center_freq_Hz) override;
};

class radio_factory_limesuiteng_impl : public radio_factory
{
public:
  // See interface for documentation.
  const radio_configuration::validator& get_configuration_validator() const override { return config_validator; }

  // See interface for documentation.
  std::unique_ptr<radio_session> create(const radio_configuration::radio& config,
                                        task_executor&                    async_task_executor,
                                        radio_event_notifier&             notifier) override;

private:
  static radio_config_limesuiteng_config_validator config_validator;
};

} // namespace srsran
