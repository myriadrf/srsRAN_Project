/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#pragma once

#include "radio_limesuiteng_rx_stream.h"
#include "radio_limesuiteng_tx_stream.h"
#include "srsran/gateways/baseband/baseband_gateway.h"
#include <memory>

class LimePluginContext;

namespace srsran {

/// \brief Implement baseband gateway interface for Lime.
///
/// It contains a transmit stream and a receive stream.
class radio_limesuiteng_baseband_gateway : public baseband_gateway
{
public:
  radio_limesuiteng_baseband_gateway(std::shared_ptr<LimePluginContext> ctx,
                                     task_executor&                     async_executor,
                                     radio_notification_handler&        notifier,
                                     uint32_t                           portId,
                                     uint8_t                            rxCount,
                                     uint8_t                            txCount) :
    context(ctx)
  {
    if (txCount)
      tx_stream = std::make_unique<radio_limesuiteng_tx_stream>(ctx, portId, notifier);
    if (rxCount)
      rx_stream = std::make_unique<radio_limesuiteng_rx_stream>(ctx, portId, notifier);
  }

  // See interface for documentation.
  baseband_gateway_transmitter& get_transmitter() override { return *tx_stream; }

  // See interface for documentation.
  baseband_gateway_receiver& get_receiver() override { return *rx_stream; }

  // See interface for documentation.
  unsigned get_transmitter_optimal_buffer_size() const override { return 1; } // tx_stream->get_buffer_size(); }

  // See interface for documentation.
  unsigned get_receiver_optimal_buffer_size() const override { return 1; } // rx_stream->get_buffer_size(); }

private:
  std::shared_ptr<LimePluginContext> context;
  /// Transmit stream.
  std::unique_ptr<radio_limesuiteng_tx_stream> tx_stream;
  /// Receive stream.
  std::unique_ptr<radio_limesuiteng_rx_stream> rx_stream;
};

} // namespace srsran
