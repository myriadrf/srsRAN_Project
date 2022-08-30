/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pdcp_tx_test.h"
#include "../../lib/pdcp/pdcp_entity_impl.h"
#include "pdcp_test_vectors.h"
#include "srsgnb/pdcp/pdcp_config.h"
#include "srsgnb/support/timers.h"
#include <gtest/gtest.h>
#include <queue>

using namespace srsgnb;

TEST_P(pdcp_tx_test, create_new_entity)
{
  init(GetParam());

  ASSERT_NE(pdcp_tx, nullptr);
}

TEST_P(pdcp_tx_test, sn_pack)
{
  init(GetParam());

  // 12 bit PDUs
  byte_buffer buf_count0_snlen12{pdu1_count0_snlen12};       // [HFN | SN] 0000 0000 0000 0000 0000 | 0000 0000 0000
  byte_buffer buf_count2048_snlen12{pdu1_count2048_snlen12}; // [HFN | SN] 0000 0000 0000 0000 0000 | 0001 0000 0000
  byte_buffer buf_count4096_snlen12{pdu1_count4096_snlen12}; // [HFN | SN] 0000 0000 0000 0000 0001 | 0000 0000 0000
  byte_buffer buf_count4294967295_snlen12{pdu1_count4294967295_snlen12}; // All 1's

  // 18 bit PDUs
  byte_buffer buf_count0_snlen18{pdu1_count0_snlen18};           // [HFN | SN] 0000 0000 0000 00|00 0000 0000 0000 0000
  byte_buffer buf_count131072_snlen18{pdu1_count131072_snlen18}; // [HFN | SN] 0000 0000 0000 00|10 0000 0000 0000 0000
  byte_buffer buf_count262144_snlen18{pdu1_count262144_snlen18}; // [HFN | SN] 0000 0000 0000 01|00 0000 0000 0000 0000
  byte_buffer buf_count4294967295_snlen18{pdu1_count4294967295_snlen18}; // All 1's

  auto test_writer = [this](uint32_t sn, byte_buffer_view expected_hdr) {
    byte_buffer buf = {};
    pdcp_tx->write_data_pdu_header(buf, sn);
    ASSERT_EQ(buf.length(), expected_hdr.length());
    ASSERT_EQ(buf, expected_hdr);
  };

  if (config.sn_size == pdcp_sn_size::size12bits) {
    test_writer(0, byte_buffer_view(buf_count0_snlen12, 0, 2));
    test_writer(2048, byte_buffer_view(buf_count2048_snlen12, 0, 2));
    test_writer(4096, byte_buffer_view(buf_count4096_snlen12, 0, 2));
    test_writer(4294967295, byte_buffer_view(buf_count4294967295_snlen12, 0, 2));
  } else if (config.sn_size == pdcp_sn_size::size18bits) {
    test_writer(0, byte_buffer_view(buf_count0_snlen18, 0, 3));
    test_writer(131072, byte_buffer_view(buf_count131072_snlen18, 0, 3));
    test_writer(262144, byte_buffer_view(buf_count262144_snlen18, 0, 3));
    test_writer(4294967295, byte_buffer_view(buf_count4294967295_snlen18, 0, 3));
  } else {
    FAIL();
  }
}

///////////////////////////////////////////////////////////////////
// Finally, instantiate all testcases for each supported SN size //
///////////////////////////////////////////////////////////////////
std::string test_param_info_to_string(const ::testing::TestParamInfo<pdcp_sn_size>& info)
{
  fmt::memory_buffer buffer;
  fmt::format_to(buffer, "{}bit", to_number(info.param));
  return fmt::to_string(buffer);
}

INSTANTIATE_TEST_SUITE_P(pdcp_tx_test_all_sn_sizes,
                         pdcp_tx_test,
                         ::testing::Values(pdcp_sn_size::size12bits, pdcp_sn_size::size18bits),
                         test_param_info_to_string);

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}