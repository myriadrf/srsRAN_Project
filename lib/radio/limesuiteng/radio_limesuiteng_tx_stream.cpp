#include "radio_limesuiteng_tx_stream.h"
#include "limesuiteng/LimePlugin.h"
#include "limesuiteng/StreamMeta.h"
#include "limesuiteng/complex.h"
#include "srsran/radio/radio_constants.h"

using namespace srsran;

radio_limesuiteng_tx_stream::radio_limesuiteng_tx_stream(std::shared_ptr<LimePluginContext> ctx,
                                                         uint8_t                            id,
                                                         radio_event_notifier&              notifier_) :
  context(ctx), portId(id), notifier(notifier_), do_work(false)
{
}

// See interface for documentation.
void radio_limesuiteng_tx_stream::transmit(const baseband_gateway_buffer_reader&        data,
                                           const baseband_gateway_transmitter_metadata& inmeta)
{
  if (inmeta.is_empty)
    return;

  if (!do_work)
    return;

  int      start_padding  = inmeta.tx_start.has_value() ? inmeta.tx_start.value() : 0;
  int      tx_end_padding = inmeta.tx_end.has_value() ? inmeta.tx_start.value() : 0;
  unsigned nsamples       = data.get_nof_samples() - tx_end_padding - start_padding;

  // Flatten buffers.
  unsigned                                     nof_channels = data.get_nof_channels();
  static_vector<const ci16_t*, RADIO_MAX_NOF_CHANNELS> buffs_flat_ptr(nof_channels);
  for (unsigned ch = 0; ch < nof_channels; ++ch)
    buffs_flat_ptr[ch] = data.get_channel_buffer(ch).data() + start_padding;

  const lime::complex16_t* const* src = buffs_flat_ptr.data();

  lime::StreamTxMeta meta;
  meta.timestamp    = lime::Timespec(inmeta.ts + start_padding);
  meta.hasTimestamp = true;
  meta.flags        = tx_end_padding ? lime::StreamTxMeta::EndOfBurst : 0;

  int samplesSent = LimePlugin_Write_complex16(context.get(), src, nsamples, portId, meta);
  if (samplesSent <= 0) {
    // error
  }
}

void radio_limesuiteng_tx_stream::start()
{
  do_work = true;
}

void radio_limesuiteng_tx_stream::stop()
{
  do_work = false;
}
