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

#include "ngc_types.h"
#include "srsgnb/adt/optional.h"
#include "srsgnb/asn1/ngap.h"
#include "srsgnb/support/async/async_task.h"

namespace srsgnb {

namespace srs_cu_cp {

struct ngc_message {
  asn1::ngap::ngap_pdu_c pdu;
};

/// This interface is used to push NGC messages to the NGC interface.
class ngc_message_handler
{
public:
  virtual ~ngc_message_handler() = default;

  /// Handle the incoming NGC message.
  virtual void handle_message(const ngc_message& msg) = 0;
};

/// Interface used by NG to inform about events.
class ngc_event_handler
{
public:
  virtual ~ngc_event_handler()          = default;
  virtual void handle_connection_loss() = 0;
};

/// This interface notifies the reception of new NGC messages over the NGC interface.
class ngc_message_notifier
{
public:
  virtual ~ngc_message_notifier() = default;

  /// This callback is invoked on each received NGC message.
  virtual void on_new_message(const ngc_message& msg) = 0;
};

struct ng_setup_request_message {
  asn1::ngap::ng_setup_request_s msg;
  unsigned                       max_setup_retries = 5;
};

struct ng_setup_response_message {
  asn1::ngap::ng_setup_resp_s msg;
  bool                        success;
};

/// Handle NGC interface management procedures as defined in TS 38.413 section 8.7
class ngc_connection_manager
{
public:
  virtual ~ngc_connection_manager() = default;

  /// \brief Initiates the NG Setup procedure.
  /// \param[in] request The NGSetupRequest message to transmit.
  /// \return Returns a ng_setup_response_message struct with the success member set to 'true' in case of a
  /// successful outcome, 'false' otherwise.
  /// \remark The CU transmits the NGSetupRequest as per TS 38.413 section 8.7.1
  /// and awaits the response. If a NGSetupFailure is received the NGC will handle the failure.
  virtual async_task<ng_setup_response_message> handle_ng_setup_request(const ng_setup_request_message& request) = 0;
};

struct ngc_ue_context {
  ue_amf_id_t amf_ue_id = ue_amf_id_t::invalid;
  ue_index_t  ue_index  = INVALID_UE_INDEX;
  du_index_t  du_index  = INVALID_DU_INDEX;
};

struct ngap_initial_ue_message {
  ue_ngap_id_t                            ue_ngap_id;
  byte_buffer                             nas_pdu;
  asn1::ngap::rrcestablishment_cause_opts establishment_cause;
  asn1::ngap::nr_cgi_s                    nr_cgi;
};

struct ngap_ul_nas_transport_message {
  ue_ngap_id_t         ue_ngap_id;
  byte_buffer          nas_pdu;
  asn1::ngap::nr_cgi_s nr_cgi;
};

/// Handle NGC NAS Message procedures as defined in TS 38.413 section 8.6.
class ngc_nas_message_handler
{
public:
  virtual ~ngc_nas_message_handler() = default;

  /// \brief Initiates Initial UE message procedure as per TS 38.413 section 8.6.1.
  /// \param[in] msg The initial UE message to transmit.
  virtual void handle_initial_ue_message(const ngap_initial_ue_message& msg) = 0;

  /// \brief Initiates Uplink NAS transport procedure as per TS 38.413 section 8.6.3.
  /// \param[in] msg The ul nas transfer message to transmit.
  virtual void handle_ul_nas_transport_message(const ngap_ul_nas_transport_message& msg) = 0;
};

/// Combined entry point for the NGC object.
class ngc_interface : public ngc_message_handler,
                      public ngc_event_handler,
                      public ngc_connection_manager,
                      public ngc_nas_message_handler
{
public:
  virtual ~ngc_interface() = default;
};

} // namespace srs_cu_cp

} // namespace srsgnb