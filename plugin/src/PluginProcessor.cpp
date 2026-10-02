// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): outputs silence; full implementation in S-013 (plan/PLAN.md).
#include "PluginProcessor.h"
#include "Parameters.h"
#include "PluginEditor.h"

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

SaxophoneAudioProcessor::SaxophoneAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "STATE", makeLayout()) {}
SaxophoneAudioProcessor::~SaxophoneAudioProcessor() = default;
void SaxophoneAudioProcessor::prepareToPlay(double, int) {}
void SaxophoneAudioProcessor::releaseResources() {}
bool SaxophoneAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}
void SaxophoneAudioProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer&) { b.clear(); }
juce::AudioProcessorEditor* SaxophoneAudioProcessor::createEditor() { return new SaxophoneAudioProcessorEditor(*this); }
void SaxophoneAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void SaxophoneAudioProcessor::setStateInformation(const void*, int) {}
sax::KeySet SaxophoneAudioProcessor::currentKeys() const noexcept { return {}; }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SaxophoneAudioProcessor(); }
