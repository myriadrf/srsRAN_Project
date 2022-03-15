
#ifndef SRSGNB_MAC_UL_MANAGER_H
#define SRSGNB_MAC_UL_MANAGER_H

#include "../../ran/gnb_format.h"
#include "../mac_config.h"
#include "../mac_config_interfaces.h"
#include "mac_ul_ue_manager.h"
#include "pdu_rx_handler.h"
#include "srsgnb/mac/mac.h"
#include "srsgnb/support/async/execute_on.h"

namespace srsgnb {

class mac_ul_manager final : public mac_ul_configurer
{
public:
  mac_ul_manager(mac_common_config_t& cfg_, mac_ul_sdu_notifier& ul_ccch_notifier_, sched_interface& sched_) :
    cfg(cfg_),
    logger(cfg.logger),
    ue_manager(cfg, ul_ccch_notifier_),
    pdu_handler(logger, sched_, ul_ccch_notifier_)
  {
    for (size_t i = 0; i < MAX_NOF_UES; ++i) {
      rnti_resources[i] = &cfg.ul_exec;
    }
  }

  async_task<bool> add_ue(const mac_ue_create_request_message& request) override
  {
    // TODO: define dispatch policy to UL workers
    return dispatch_and_resume_on(cfg.ul_exec, cfg.ctrl_exec, [this, request]() { return ue_manager.add_ue(request); });
  }

  async_task<bool> reconfigure_ue(const mac_ue_reconfiguration_request_message& request) override
  {
    return dispatch_and_resume_on(
        cfg.ul_exec, cfg.ctrl_exec, [this, request]() { return ue_manager.reconfigure_ue(request); });
  }

  async_task<void> remove_ue(const mac_ue_delete_request_message& msg) override
  {
    return dispatch_and_resume_on(
        cfg.ul_exec, cfg.ctrl_exec, [this, ue_index = msg.ue_index]() { ue_manager.remove_ue(ue_index); });
  }

  /// Handles FAPI Rx_Data.Indication.
  /// The PDUs contained in the Rx_Data.Indication are dispatched to different executors, depending on their RNTI.
  void handle_rx_data_indication(mac_rx_data_indication& msg)
  {
    for (mac_rx_pdu& pdu : msg.pdus) {
      // 1. Fork each PDU to different executors based on the PDU RNTI.
      rnti_resources[pdu.rnti]->execute(
          [this, pdu = decoded_pdu_rx{msg.sl_rx, msg.cell_index, std::move(pdu)}]() mutable {
            // 2. Decode Rx PDU and handle respective subPDUs.
            handle_rx_pdu_impl(pdu);
          });
    }
  }

private:
  /// Decodes MAC UL PDU, dispatches CEs to scheduler and SDUs to upper layers.
  void handle_rx_pdu_impl(decoded_pdu_rx& pdu_ctx)
  {
    // 1. Decode MAC UL PDU.
    if (not pdu_handler.decode_rx_pdu(pdu_ctx)) {
      return;
    }

    // 2. Check UE exists.
    mac_ul_ue* ue = ue_manager.find_rnti(pdu_ctx.pdu_rx.rnti);

    if (pdu_ctx.ce_crnti == INVALID_RNTI) {
      // 4. In case C-RNTI CE was not present, handle MAC CEs and MAC UL SDUs.
      pdu_handler.handle_rx_subpdus(ue, pdu_ctx);

    } else {
      // 4. Dispatch continuation to different execution context.
      rnti_resources[pdu_ctx.ce_crnti]->execute([this, pdu_ctx = std::move(pdu_ctx)]() mutable {
        // 5. In case C-RNTI CE is present, find UE with provided C-RNTI.
        mac_ul_ue* ue = ue_manager.find_rnti(pdu_ctx.ce_crnti);
        if (ue == nullptr) {
          logger.warning("Couldn't find UE with RNTI=0x{:x}", pdu_ctx.ce_crnti);
          return;
        }

        // 6. handle MAC CEs and MAC UL SDUs.
        pdu_handler.handle_rx_subpdus(ue, pdu_ctx);
      });
    }
  }

  mac_common_config_t&  cfg;
  srslog::basic_logger& logger;

  mac_ul_ue_manager ue_manager;

  /// Object that handles incoming UL MAC PDUs.
  pdu_rx_handler pdu_handler;

  /// List of executors for each RNTI.
  circular_array<task_executor*, MAX_NOF_UES> rnti_resources{};
};

} // namespace srsgnb

#endif // SRSGNB_MAC_UL_MANAGER_H
