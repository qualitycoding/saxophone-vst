// SPDX-License-Identifier: Apache-2.0
#include "PluginEditor.h"
#include "Parameters.h"

namespace {
juce::String noteName(int midi) {
    static const char* names[12] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
    return juce::String(names[midi % 12]) + juce::String(midi / 12 - 1);
}
} // namespace

SaxophoneAudioProcessorEditor::SaxophoneAudioProcessorEditor(SaxophoneAudioProcessor& p) : AudioProcessorEditor(p), proc_(p) {
    using namespace saxplug;
    addAndMakeVisible(view_);

    title_.setText("ALTO SAXOPHONE", juce::dontSendNotification);
    title_.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    title_.setColour(juce::Label::textColourId, juce::Colour(0xffe9c46a));
    addAndMakeVisible(title_);
    noteLabel_.setFont(juce::FontOptions(16.0f));
    noteLabel_.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    addAndMakeVisible(noteLabel_);

    harmonic_.setButtonText("Harmonic mode (play the note as an overblown octave of the lower fingering)");
    harmonic_.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(harmonic_);
    harmonicAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc_.parameters(), param::harmonic, harmonic_);

    addRow(rows_[0], "Overblow", param::overblow, "", 2);
    addRow(rows_[1], "Reed hardness", param::reedHardness, "", 2);
    addRow(rows_[2], "Mouthpiece brightness", param::brightness, "", 2);
    addRow(rows_[3], "Breath noise", param::breathNoise, "", 2);
    addRow(rows_[4], "Vibrato rate", param::vibratoRate, " Hz", 1);
    addRow(rows_[5], "Vibrato depth", param::vibratoDepth, "", 2);
    addRow(rows_[6], "Portamento", param::portamento, " ms", 0);
    addRow(rows_[7], "Tuning A4", param::tuning, " Hz", 1);
    addRow(rows_[8], "Output gain", param::outputGain, " dB", 1);

    setResizable(true, true);
    setResizeLimits(675, 450, 1800, 1200); // 0.75x .. 2x of 900x600 (D-016)
    getConstrainer()->setFixedAspectRatio(1.5);
    setSize(900, 600);
    refreshFromProcessor();
    startTimerHz(60);
}

SaxophoneAudioProcessorEditor::~SaxophoneAudioProcessorEditor() { stopTimer(); }

void SaxophoneAudioProcessorEditor::addRow(Row& row, const char* title, const char* paramId, const char* suffix, int decimals) {
    row.label.setText(title, juce::dontSendNotification);
    row.label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    row.slider.setSliderStyle(juce::Slider::LinearHorizontal);
    row.slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 22);
    row.slider.setTextValueSuffix(suffix);
    row.slider.setNumDecimalPlacesToDisplay(decimals);
    row.slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe9c46a));
    row.slider.setColour(juce::Slider::trackColourId, juce::Colour(0xffb5832a));
    row.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    addAndMakeVisible(row.label);
    addAndMakeVisible(row.slider);
    row.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc_.parameters(), paramId, row.slider);
    // The attachment installs the parameter's own (7-decimal) text conversion; override it for display.
    row.slider.textFromValueFunction = [decimals](double v) { return juce::String(v, decimals); }; // the slider appends the suffix
    row.slider.valueFromTextFunction = [](const juce::String& t) { return t.getDoubleValue(); };
    row.slider.updateText();
}

void SaxophoneAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff1d2027)); }

void SaxophoneAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(getWidth() / 90);
    const float s = static_cast<float>(getHeight()) / 600.0f; // everything scales with the window
    view_.setBounds(area.removeFromLeft(static_cast<int>(area.getWidth() * 0.6f)));
    area.removeFromLeft(static_cast<int>(14 * s));
    title_.setFont(juce::FontOptions(24.0f * s, juce::Font::bold));
    noteLabel_.setFont(juce::FontOptions(16.0f * s));
    title_.setBounds(area.removeFromTop(static_cast<int>(36 * s)));
    noteLabel_.setBounds(area.removeFromTop(static_cast<int>(26 * s)));
    area.removeFromTop(static_cast<int>(10 * s));
    harmonic_.setBounds(area.removeFromTop(static_cast<int>(44 * s)));
    area.removeFromTop(static_cast<int>(6 * s));
    const int rowH = area.getHeight() / static_cast<int>(rows_.size());
    for (auto& r : rows_) {
        auto row = area.removeFromTop(rowH);
        r.label.setBounds(row.removeFromTop(static_cast<int>(20 * s)));
        r.slider.setBounds(row);
    }
}

void SaxophoneAudioProcessorEditor::refreshFromProcessor() {
    view_.setHighlightedKeys(proc_.currentKeys());
    const int note = proc_.currentConcertNote();
    if (note != shownNote_) {
        shownNote_ = note;
        noteLabel_.setText(note < 0 ? juce::String("Play a MIDI note (concert Db3 - A5)")
                                    : "Sounding: " + noteName(note) + "  (concert)  -  written " + noteName(note + 9),
                           juce::dontSendNotification);
    }
}
