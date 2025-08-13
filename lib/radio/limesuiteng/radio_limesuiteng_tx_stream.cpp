#include "radio_limesuiteng_tx_stream.h"
#include "limesuiteng/LimePlugin.h"
#include "limesuiteng/StreamConfig.h"
#include "limesuiteng/complex.h"
#include "srsran/radio/radio_constants.h"

using namespace srsran;

radio_limesuiteng_tx_stream::radio_limesuiteng_tx_stream(std::shared_ptr<LimePluginContext> ctx,
                                                         uint8_t                            id,
                                                         radio_notification_handler&        notifier_) :
  context(ctx), portId(id), notifier(notifier_)
{
}

// See interface for documentation.
void radio_limesuiteng_tx_stream::transmit(const baseband_gateway_buffer_reader&        data,
                                           const baseband_gateway_transmitter_metadata& inmeta)
{
  if (inmeta.is_empty)
    return;

  unsigned nsamples = data.get_nof_samples();

  // Flatten buffers.
  unsigned                                     nof_channels = data.get_nof_channels();
  static_vector<void*, RADIO_MAX_NOF_CHANNELS> buffs_flat_ptr(nof_channels);
  for (unsigned ch = 0; ch < nof_channels; ++ch)
    buffs_flat_ptr[ch] = (void*)data[ch].data();

  lime::complex32f_t** src = (lime::complex32f_t**)buffs_flat_ptr.data();

  lime::StreamMeta meta;
  meta.timestamp        = inmeta.ts;
  meta.waitForTimestamp = true;

  int samplesSent = LimePlugin_Write_complex32f(context.get(), src, nsamples, portId, meta);
  if (samplesSent <= 0) {
    // error
  }
}
