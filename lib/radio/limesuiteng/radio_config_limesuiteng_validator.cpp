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

#include "radio_config_limesuiteng_validator.h"

namespace srsran {

radio_config_limesuiteng_config_validator::radio_config_limesuiteng_config_validator() {}

radio_config_limesuiteng_config_validator::~radio_config_limesuiteng_config_validator() {}

bool radio_config_limesuiteng_config_validator::is_configuration_valid(const radio_configuration::radio& config) const
{
  return true;
}

} // namespace srsran
