// SPDX-License-Identifier: Apache-2.0
// sax_snapshot_ui: renders the saxophone view for every concert note (key highlighting) and the full
// editor to PNG files, for the G-004 human review bundle.
//   xvfb-run -a sax_snapshot_ui --out DIR
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "SaxophoneView.h"
#include "sax/Fingering.h"
#include "sax/Pitch.h"
#include <cstdio>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <string>

namespace {

bool savePng(const juce::Image& img, const juce::File& file) {
    file.deleteFile();
    juce::FileOutputStream out(file);
    if (!out.openedOk()) return false;
    juce::PNGImageFormat png;
    return png.writeImageToStream(img, out);
}

} // namespace

int main(int argc, char** argv) {
    juce::String outDir = "ui_snapshots";
    for (int i = 1; i + 1 < argc; ++i)
        if (std::string(argv[i]) == "--out") outDir = argv[i + 1];
    const juce::File dir = juce::File::getCurrentWorkingDirectory().getChildFile(outDir);
    dir.createDirectory();

    juce::ScopedJuceInitialiser_GUI juceInit;
    int failures = 0;

    SaxophoneView view;
    view.setBounds(0, 0, 440, 580);
    for (int n = sax::kLowestConcert; n <= sax::kHighestConcert; ++n) {
        const auto res = sax::resolveNote(n, false);
        view.setHighlightedKeys(res->fingering.keys);
        juce::Image img(juce::Image::ARGB, 440, 580, true);
        juce::Graphics g(img);
        view.paintEntireComponent(g, true);
        const juce::File f = dir.getChildFile(juce::String::formatted("key_concert_%02d_written_%02d.png", n, res->fingering.writtenMidi));
        if (!savePng(img, f)) { std::fprintf(stderr, "cannot write %s\n", f.getFullPathName().toRawUTF8()); ++failures; }
    }

    SaxophoneAudioProcessor proc;
    proc.prepareToPlay(48000.0, 256);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 61, (juce::uint8) 90), 0);
    juce::AudioBuffer<float> buf(2, 256);
    for (int i = 0; i < 20; ++i) { proc.processBlock(buf, midi); midi.clear(); }
    std::unique_ptr<juce::AudioProcessorEditor> ed(proc.createEditor());
    if (auto* se = dynamic_cast<SaxophoneAudioProcessorEditor*>(ed.get())) se->refreshFromProcessor();
    if (!savePng(ed->createComponentSnapshot(ed->getLocalBounds()), dir.getChildFile("editor.png"))) ++failures;

    std::printf("wrote %d key snapshots + editor.png to %s (%d failures)\n", sax::kHighestConcert - sax::kLowestConcert + 1, dir.getFullPathName().toRawUTF8(), failures);
    return failures == 0 ? 0 : 1;
}
