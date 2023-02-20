/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/phy/lower/lower_phy_controller.h"
#include "srsran/phy/lower/lower_phy_request_handler.h"
#include "srsran/phy/lower/lower_phy_rg_handler.h"

namespace srsran {

/// \brief Lower PHY main interface.
///
/// Provides access to all the lower PHY components.
class lower_phy
{
public:
  /// Default destructor.
  virtual ~lower_phy() = default;

  /// \brief Returns a reference to the lower PHY request handler.
  virtual lower_phy_request_handler& get_request_handler() = 0;

  /// \brief Returns a reference to the lower PHY resource grid handler.
  virtual lower_phy_rg_handler& get_rg_handler() = 0;

  /// \brief Returns a reference to the lower PHY controller.
  virtual lower_phy_controller& get_controller() = 0;
};

} // namespace srsran