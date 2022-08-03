/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "fapi_to_mac_data_msg_translator.h"
#include "fapi_to_mac_time_msg_translator.h"
#include "mac_to_fapi_translator.h"
#include "srsgnb/fapi_adaptor/mac/mac_fapi_adaptor.h"

namespace srsgnb {
namespace fapi_adaptor {

/// \brief MAC-FAPI adaptor implementation.
///
/// This class owns all the components of the MAC-FAPI adaptor. It gives access to all the required interfaces so that
/// the adaptor can be connected with the rest of layers.
class mac_fapi_adaptor_impl : public mac_fapi_adaptor
{
public:
  mac_fapi_adaptor_impl(fapi::slot_message_gateway& msg_gw, unsigned sector_id_, subcarrier_spacing scs);

  // See interface for documentation.
  fapi::slot_data_message_notifier& get_slot_data_notifier() override;

  // See interface for documentation.
  fapi::slot_time_message_notifier& get_slot_time_notifier() override;

  // See interface for documentation.
  mac_cell_result_notifier& get_cell_result_notifier() override;

  // See interface for documentation.
  void set_cell_slot_handler(mac_cell_slot_handler& mac_slot_handler) override;

private:
  const unsigned                  sector_id;
  mac_to_fapi_translator          mac_translator;
  fapi_to_mac_data_msg_translator fapi_data_translator;
  fapi_to_mac_time_msg_translator fapi_time_translator;
};

} // namespace fapi_adaptor
} // namespace srsgnb
