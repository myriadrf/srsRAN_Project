#include "radio_limesuiteng_impl.h"
#include "limesuiteng/LimePlugin.h"
#include <thread>

using namespace lime;
using namespace srsran;

static lime::LogLevel logVerbosity = lime::LogLevel::Debug;
static void           LogCallback(lime::LogLevel lvl, const std::string& msg)
{
  if (lvl > logVerbosity)
    return;
  printf("%s\n", msg.c_str());
}

bool radio_session_limesuiteng_impl::set_tx_gain(unsigned port_idx, double gain_dB)
{
  LimePlugin_SetTxGain(context.get(), gain_dB, port_idx);
  return true;
}

bool radio_session_limesuiteng_impl::set_rx_gain(unsigned port_idx, double gain_dB)
{
  LimePlugin_SetRxGain(context.get(), gain_dB, port_idx);
  return true;
}

bool radio_session_limesuiteng_impl::set_tx_freq(unsigned stream_id, double center_freq_Hz)
{
  auto& port = context->ports.at(stream_id);
  for (DevNode* node : port.nodes) {
    for (int c = 0; c < 2; ++c) {
      if (node->config.channel[c].tx.enabled)
        node->device->SetFrequency(node->chipIndex, lime::TRXDir::Tx, c, center_freq_Hz);
      else
        break;
    }
  }
  return true;
}

bool radio_session_limesuiteng_impl::set_rx_freq(unsigned stream_id, double center_freq_Hz)
{
  auto& port = context->ports.at(stream_id);
  for (DevNode* node : port.nodes) {
    for (int c = 0; c < 2; ++c) {
      if (node->config.channel[c].rx.enabled)
        node->device->SetFrequency(node->chipIndex, lime::TRXDir::Rx, c, center_freq_Hz);
      else
        break;
    }
  }
  return true;
}

class srsRAN_ParamProvider : public LimeSettingsProvider
{
private:
  static std::string trim(const std::string& s)
  {
    std::string out = s;
    while (!out.empty() && std::isspace(out[0]))
      out = out.substr(1);
    while (!out.empty() && std::isspace(out[out.size() - 1]))
      out = out.substr(0, out.size() - 1);
    return out;
  }

  void argsToMap(const std::string& args)
  {
    bool        inKey = true;
    std::string key, val;
    for (size_t i = 0; i < args.size(); i++) {
      const char ch = args[i];
      if (inKey) {
        if (ch == ':')
          inKey = false;
        else if (ch == ',')
          inKey = true;
        else
          key += ch;
      } else {
        if (ch == ',')
          inKey = true;
        else
          val += ch;
      }
      if ((inKey && !val.empty()) || ((i + 1) == args.size())) {
        key = trim(key);
        val = trim(val);
        printf("Key:Value{ %s:%s }\n", key.c_str(), val.c_str());
        if (!key.empty()) {
          if (val[0] == '"')
            strings[key] = val.substr(1, val.size() - 2);
          else
            numbers[key] = stod(val);
        }
        key = "";
        val = "";
      }
    }
  }

public:
  srsRAN_ParamProvider(const char* args) : mArgs(args) { argsToMap(mArgs); }

  bool GetString(std::string& dest, const char* varname) override
  {
    auto iter = strings.find(std::string(varname));
    if (iter == strings.end())
      return false;

    printf("provided: %s\n", varname);

    dest = iter->second;
    return true;
  }

  bool GetDouble(double& dest, const char* varname) override
  {
    auto iter = numbers.find(varname);
    if (iter == numbers.end())
      return false;

    dest = iter->second;
    return true;
  }

private:
  std::string                                  mArgs;
  std::unordered_map<std::string, double>      numbers;
  std::unordered_map<std::string, std::string> strings;
};

static void
TransferStreamsToLimeChannels(const static_vector<radio_configuration::stream, RADIO_MAX_NOF_STREAMS>& streams,
                              LimeRuntimeParameters::ChannelParams&                                    limeChannels,
                              double                                                                   samplingRate)
{
  for (auto& stream : streams) {
    for (auto& channel : stream.channels) {
      limeChannels.freq.push_back(channel.freq.center_frequency_Hz);
      limeChannels.gain.push_back(channel.gain_dB);
      limeChannels.bandwidth.push_back(samplingRate);
    }
  }
}

radio_session_limesuiteng_impl::radio_session_limesuiteng_impl(const radio_configuration::radio& radio_config,
                                                               task_executor&                    async_executor_,
                                                               radio_notification_handler&       notifier_) :
  async_executor(async_executor_), notifier(notifier_)
{
  context                = std::make_shared<LimePluginContext>();
  context->samplesFormat = lime::DataFormat::F32;

  srsRAN_ParamProvider configProvider(radio_config.args.c_str());

  // gathers arguments and initializes devices
  if (LimePlugin_Init(context.get(), LogCallback, &configProvider) != 0)
    return;

  LimeRuntimeParameters state;

  TransferStreamsToLimeChannels(radio_config.rx_streams, state.rx, radio_config.sampling_rate_Hz);
  TransferStreamsToLimeChannels(radio_config.tx_streams, state.tx, radio_config.sampling_rate_Hz);

  size_t portCount = std::max(radio_config.rx_streams.size(), radio_config.tx_streams.size());
  for (size_t p = 0; p < portCount; ++p) {
    int rx_count = 0;
    int tx_count = 0;
    if (p < radio_config.rx_streams.size())
      rx_count = radio_config.rx_streams[p].channels.size();
    if (p < radio_config.tx_streams.size())
      tx_count = radio_config.tx_streams[p].channels.size();

    state.rf_ports.push_back({radio_config.sampling_rate_Hz, rx_count, tx_count});

    bb_gateways.emplace_back(
        std::make_unique<radio_limesuiteng_baseband_gateway>(context, async_executor, notifier, p, rx_count, tx_count));
  }

  int status = LimePlugin_Setup(context.get(), &state);
  if (status != 0)
    return;
}

// See interface for documentation.
baseband_gateway& radio_session_limesuiteng_impl::get_baseband_gateway(unsigned stream_id)
{
  srsran_assert(stream_id < bb_gateways.size(),
                "Stream identifier (i.e., {}) exceeds the number of baseband gateways (i.e., {})",
                stream_id,
                bb_gateways.size());
  return *bb_gateways[stream_id];
}

void radio_session_limesuiteng_impl::start(baseband_gateway_timestamp init_time)
{
  LimePlugin_Start(context.get());
}

void radio_session_limesuiteng_impl::stop()
{
  LimePlugin_Stop(context.get());
}

baseband_gateway_timestamp radio_session_limesuiteng_impl::read_current_time()
{
  // TODO!
  return 1;
}

std::unique_ptr<radio_session> radio_factory_limesuiteng_impl::create(const radio_configuration::radio& config,
                                                                      task_executor&              async_task_executor,
                                                                      radio_notification_handler& notifier)
{
  return std::make_unique<radio_session_limesuiteng_impl>(config, async_task_executor, notifier);
}

radio_config_limesuiteng_config_validator radio_factory_limesuiteng_impl::config_validator;
