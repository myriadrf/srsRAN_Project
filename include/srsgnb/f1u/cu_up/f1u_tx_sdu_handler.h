/*
 *
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/adt/byte_buffer.h"

namespace srsgnb {
namespace srs_cu_up {

/// \brief This interface represents the data entry point of the transmitting side of a F1-U bearer.
/// The upper-layer (e.g. PDCP) will push SDUs (e.g. PDCP PDUs) to the lower layer using this interface.
/// The upper-layer (e.g. PDCP) will also inform the lower layer of SDUs (e.g. PDCP PDUs) to be discarded.
class f1u_tx_sdu_handler
{
public:
  virtual ~f1u_tx_sdu_handler() = default;

  virtual void handle_sdu(byte_buffer sdu, uint32_t count) = 0;
  virtual void discard_sdu(uint32_t count)                 = 0;
};

} // namespace srs_cu_up
} // namespace srsgnb