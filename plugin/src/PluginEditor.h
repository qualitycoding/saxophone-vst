// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "PluginProcessor.h"
#include "SaxophoneView.h"
#include <juce_audio_processors/juce_audio_processors.h>

class SaxophoneAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit SaxophoneAudioProcessorEditor(SaxophoneAudioProcessor&);
    ~SaxophoneAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    /// Pulls currentKeys() from the processor into the view (also what the timer does).
    void refreshFromProcessor();
    SaxophoneView& saxophoneView() noexcept { return view_; }
private:
    void timerCallback() override { refreshFromProcessor(); }
    SaxophoneAudioProcessor& proc_;
    SaxophoneView view_;
};
