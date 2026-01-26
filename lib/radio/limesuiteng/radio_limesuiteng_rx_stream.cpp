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
  context(ctx), portId(id), notifier(notifier_)
{
}

baseband_gateway_receiver::metadata radio_limesuiteng_rx_stream::receive(baseband_gateway_buffer_writer& data)
{
  unsigned nsamples = data.get_nof_samples();

  // Flatten buffers.
  unsigned                                     nof_channels = data.get_nof_channels();
  static_vector<void*, RADIO_MAX_NOF_CHANNELS> buffs_flat_ptr(nof_channels);
  for (unsigned ch = 0; ch < nof_channels; ++ch)
    buffs_flat_ptr[ch] = &data[ch][0];

  lime::complex32f_t** dest = (lime::complex32f_t**)buffs_flat_ptr.data();

  lime::StreamRxMeta meta;
  int              samplesGot = LimePlugin_Read_complex32f(context.get(), dest, nsamples, portId, meta);
  if (samplesGot <= 0) {
    // error
  }

  baseband_gateway_receiver::metadata ret = {};
  ret.ts                                  = meta.timestamp.GetTicks();

  return ret;
}
