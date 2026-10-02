// SPDX-License-Identifier: Apache-2.0
#include "PluginProcessor.h"
#include "Parameters.h"
#include "PluginEditor.h"
#include "sax/State.h"
#include <algorithm>

static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout() {
    using namespace saxplug;
    using F = juce::AudioParameterFloat; using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add(std::make_unique<F>(juce::ParameterID{param::overblow, 1}, "Overblow", R{0.f, 1.f}, 0.f));
    l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{param::harmonic, 1}, "Harmonic", false));
    l.add(std::make_unique<F>(juce::ParameterID{param::reedHardness, 1}, "Reed hardness", R{0.f, 1.f}, .5f));
    l.add(std::make_unique<F>(juce::ParameterID{param::brightness, 1}, "Mouthpiece brightness", R{0.f, 1.f}, .5f));
    l.add(std::make_unique<F>(juce::ParameterID{param::breathNoise, 1}, "Breath noise", R{0.f, 1.f}, .3f));
    l.add(std::make_unique<F>(juce::ParameterID{param::vibratoRate, 1}, "Vibrato rate", R{3.f, 8.f}, 5.5f));
    l.add(std::make_unique<F>(juce::ParameterID{param::vibratoDepth, 1}, "Vibrato depth", R{0.f, 1.f}, 0.f));
    l.add(std::make_unique<F>(juce::ParameterID{param::portamento, 1}, "Portamento (ms)", R{0.f, 500.f}, 30.f));
    l.add(std::make_unique<F>(juce::ParameterID{param::tuning, 1}, "Tuning A4 (Hz)", R{415.f, 466.f}, 440.f));
    l.add(std::make_unique<F>(juce::ParameterID{param::outputGain, 1}, "Output gain (dB)", R{-24.f, 12.f}, 0.f));
    return l;
}

namespace {
std::shared_ptr<const sax::ResonatorTable> loadTable() {
    return std::make_shared<const sax::ResonatorTable>(sax::ResonatorTable::fromJson(sax::embeddedResonatorJson()));
}
} // namespace

SaxophoneAudioProcessor::SaxophoneAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "STATE", makeLayout()),
      table_(loadTable()),
      voice_(table_) {}
SaxophoneAudioProcessor::~SaxophoneAudioProcessor() = default;

void SaxophoneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    sampleRate_ = std::clamp(sampleRate, 22050.0, 192000.0); // hosts outside the supported range are clamped, never rejected
    voice_.prepare(sampleRate_, std::max(1, samplesPerBlock));
    setLatencySamples(voice_.latencySamples());
}
void SaxophoneAudioProcessor::releaseResources() { voice_.reset(); }

bool SaxophoneAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

sax::VoiceParameters SaxophoneAudioProcessor::readParameters() const noexcept {
    using namespace saxplug;
    auto get = [this](const char* id) { return apvts_.getRawParameterValue(id)->load(); };
    sax::VoiceParameters p;
    p.overblow = get(param::overblow);
    p.harmonicMode = get(param::harmonic) >= 0.5f;
    p.reedHardness = get(param::reedHardness);
    p.brightness = get(param::brightness);
    p.breathNoise = get(param::breathNoise);
    p.vibratoRateHz = get(param::vibratoRate);
    p.vibratoDepth = get(param::vibratoDepth);
    p.portamentoMs = get(param::portamento);
    p.tuningA4Hz = get(param::tuning);
    p.outputGainDb = get(param::outputGain);
    return p;
}

void SaxophoneAudioProcessor::handleMidi(const juce::MidiMessage& m) noexcept {
    if (m.isNoteOn(false)) {
        voice_.noteOn(m.getNoteNumber(), static_cast<float>(m.getVelocity()) / 127.0f);
    } else if (m.isNoteOff(true)) {
        voice_.noteOff(m.getNoteNumber());
    } else if (m.isAllNotesOff() || m.isAllSoundOff()) {
        voice_.allNotesOff();
    } else if (m.isController()) {
        const int cc = m.getControllerNumber();
        if (cc == 2 || cc == 11) voice_.setBreath(static_cast<float>(m.getControllerValue()) / 127.0f); // D-011
    } else if (m.isPitchWheel()) {
        voice_.setPitchBend(static_cast<float>(m.getPitchWheelValue() - 8192) / 8192.0f);
    } else if (m.isChannelPressure()) {
        voice_.setAftertouch(static_cast<float>(m.getChannelPressureValue()) / 127.0f);
    }
}

void SaxophoneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() == 0 || n <= 0) return;

    voice_.setParameters(readParameters());
    float* out = buffer.getWritePointer(0);
    int pos = 0;
    for (const auto meta : midi) {
        const int t = std::clamp(meta.samplePosition, pos, n);
        if (t > pos) {
            voice_.process(out + pos, t - pos);
            pos = t;
        }
        handleMidi(meta.getMessage());
    }
    if (pos < n) voice_.process(out + pos, n - pos);
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch) buffer.copyFrom(ch, 0, out, n);
}

juce::AudioProcessorEditor* SaxophoneAudioProcessor::createEditor() { return new SaxophoneAudioProcessorEditor(*this); }

void SaxophoneAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {
    const std::string text = sax::serializeState(readParameters());
    dest.replaceAll(text.data(), text.size());
}

void SaxophoneAudioProcessor::setStateInformation(const void* data, int size) {
    if (data == nullptr || size <= 0) return;
    const auto parsed = sax::deserializeState(std::string_view(static_cast<const char*>(data), static_cast<std::size_t>(size)));
    if (!parsed) return; // malformed state: keep current values
    using namespace saxplug;
    auto set = [this](const char* id, float v) {
        if (auto* p = apvts_.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(v));
    };
    set(param::overblow, parsed->overblow);
    set(param::harmonic, parsed->harmonicMode ? 1.0f : 0.0f);
    set(param::reedHardness, parsed->reedHardness);
    set(param::brightness, parsed->brightness);
    set(param::breathNoise, parsed->breathNoise);
    set(param::vibratoRate, parsed->vibratoRateHz);
    set(param::vibratoDepth, parsed->vibratoDepth);
    set(param::portamento, parsed->portamentoMs);
    set(param::tuning, parsed->tuningA4Hz);
    set(param::outputGain, parsed->outputGainDb);
}

sax::KeySet SaxophoneAudioProcessor::currentKeys() const noexcept { return voice_.currentKeys(); }
int SaxophoneAudioProcessor::currentConcertNote() const noexcept { return voice_.currentConcertNote(); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SaxophoneAudioProcessor(); }
