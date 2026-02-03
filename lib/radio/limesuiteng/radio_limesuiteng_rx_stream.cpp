#include "radio_limesuiteng_rx_stream.h"
#include "limesuiteng/LimePlugin.h"
#include "limesuiteng/StreamConfig.h"
#include "limesuiteng/StreamMeta.h"
#include "limesuiteng/complex.h"
#include "srsran/radio/radio_constants.h"

using namespace srsran;

radio_limesuiteng_rx_stream::radio_limesuiteng_rx_stream(std::shared_ptr<LimePluginContext> ctx,
                                                         uint8_t                            id,
                                                         radio_event_notifier&              notifier_) :
  context(ctx), portId(id), notifier(notifier_), do_work(false)
{
}

baseband_gateway_receiver::metadata radio_limesuiteng_rx_stream::receive(baseband_gateway_buffer_writer& buffers)
{
  if (!do_work)
    return {};

  // Flatten buffers.
  uint32_t nof_channels = buffers.get_nof_channels();
  assert(nof_channels > 0);

  uint32_t nsamples = buffers[0].size();

  static_vector<ci16_t*, RADIO_MAX_NOF_CHANNELS> buffs_flat_ptr(nof_channels);
  for (unsigned ch = 0; ch < nof_channels; ++ch)
    buffs_flat_ptr[ch] = buffers.get_channel_buffer(ch).data();

  lime::complex16_t** dest = buffs_flat_ptr.data();

  lime::StreamRxMeta meta;

  int samplesGot = LimePlugin_Read_complex16(context.get(), dest, nsamples, portId, meta);
  if (samplesGot <= 0) {
    printf("ERROR\n");
  }

  baseband_gateway_receiver::metadata ret = {};
  ret.ts                                  = meta.timestamp.GetTicks();

  return ret;
}

void radio_limesuiteng_rx_stream::start()
{
  do_work = true;
}

void radio_limesuiteng_rx_stream::stop()
{
  do_work = false;
}
