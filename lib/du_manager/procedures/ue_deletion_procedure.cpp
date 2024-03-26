/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "ue_deletion_procedure.h"
#include "srsran/support/async/execute_on.h"

using namespace srsran;
using namespace srs_du;

ue_deletion_procedure::ue_deletion_procedure(du_ue_index_t             ue_index_,
                                             du_ue_manager_repository& ue_mng_,
                                             const du_manager_params&  du_params_) :
  ue_index(ue_index_),
  ue_mng(ue_mng_),
  du_params(du_params_),
  proc_logger(srslog::fetch_basic_logger("DU-MNG"), name(), ue_index)
{
}

void ue_deletion_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  proc_logger.log_proc_started();

  ue = ue_mng.find_ue(ue_index);
  if (ue == nullptr) {
    proc_logger.log_proc_failure("ueId does not exist.");
    CORO_EARLY_RETURN();
  }

  // > Disconnect DRBs from F1-U and SRBs from F1-C to stop handling traffic in flight and delivery notifications.
  CORO_AWAIT(disconnect_inter_layer_interfaces());

  // > Remove UE from F1AP.
  du_params.f1ap.ue_mng.handle_ue_deletion_request(ue_index);

  // > Remove UE from MAC.
  CORO_AWAIT_VALUE(const mac_ue_delete_response mac_resp, launch_mac_ue_delete());
  if (not mac_resp.result) {
    proc_logger.log_proc_failure("Failed to remove UE from MAC.");
  }

  // > Remove UE object from DU UE manager.
  ue_mng.remove_ue(ue_index);

  proc_logger.log_proc_completed();

  CORO_RETURN();
}

async_task<mac_ue_delete_response> ue_deletion_procedure::launch_mac_ue_delete()
{
  mac_ue_delete_request mac_msg{};
  mac_msg.ue_index   = ue->ue_index;
  mac_msg.rnti       = ue->rnti;
  mac_msg.cell_index = ue->pcell_index;
  return du_params.mac.ue_cfg.handle_ue_delete_request(mac_msg);
}

async_task<void> ue_deletion_procedure::disconnect_inter_layer_interfaces()
{
  // Note: If the DRB was not deleted on demand by the CU-CP via F1AP UE Context Modification Procedure, there is a
  // chance that the CU-UP will keep pushing new F1-U PDUs to the DU. To avoid dangling references during UE removal,
  // we start by first disconnecting the DRBs from the F1-U interface.

  return ue->disconnect_notifiers();
}
