// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "sax/Keys.h"
#include "sax/SaxVoice.h"
#include <juce_audio_processors/juce_audio_processors.h>

class SaxophoneAudioProcessor final : public juce::AudioProcessor {
public:
    SaxophoneAudioProcessor();
    ~SaxophoneAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    using juce::AudioProcessor::processBlock; // keep the double-precision overload visible
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Saxophone"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    /// Lock-free snapshot for the editor (D-010).
    sax::KeySet currentKeys() const noexcept;
    int currentConcertNote() const noexcept; ///< -1 when silent / released
    juce::AudioProcessorValueTreeState& parameters() noexcept { return apvts_; }

private:
    sax::VoiceParameters readParameters() const noexcept;
    void handleMidi(const juce::MidiMessage&) noexcept;

    juce::AudioProcessorValueTreeState apvts_;
    std::shared_ptr<const sax::ResonatorTable> table_;
    sax::SaxVoice voice_;
    double sampleRate_ = 48000.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaxophoneAudioProcessor)
};
