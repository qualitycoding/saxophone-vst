// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): controls and layout in S-014.
#include "PluginEditor.h"
SaxophoneAudioProcessorEditor::SaxophoneAudioProcessorEditor(SaxophoneAudioProcessor& p)
    : AudioProcessorEditor(p), proc_(p) { addAndMakeVisible(view_); setSize(900, 600); }
SaxophoneAudioProcessorEditor::~SaxophoneAudioProcessorEditor() { stopTimer(); }
void SaxophoneAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colours::black); }
void SaxophoneAudioProcessorEditor::resized() { view_.setBounds(getLocalBounds()); }
void SaxophoneAudioProcessorEditor::refreshFromProcessor() {}
