// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "PluginProcessor.h"
#include "SaxophoneView.h"
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

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
    struct Row {
        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    void timerCallback() override { refreshFromProcessor(); }
    void addRow(Row& row, const char* title, const char* paramId, const char* suffix, int decimals);

    SaxophoneAudioProcessor& proc_;
    SaxophoneView view_;
    juce::Label title_, noteLabel_;
    juce::ToggleButton harmonic_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> harmonicAttachment_;
    std::array<Row, 9> rows_;
    int shownNote_ = -2;
};
