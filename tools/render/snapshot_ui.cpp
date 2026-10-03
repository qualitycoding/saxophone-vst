// SPDX-License-Identifier: Apache-2.0
// sax_ui_snapshots: renders the real editor headlessly for the G-004 review bundle (D-016, GATES.md):
//   written-<NN>-<name>.png          the 33 standard fingerings (written 58..90, concert = written - 9)
//   harmonic-concert-<NN>.png        the 16 harmonic-mode notes (concert 61..76)
// Usage (Linux): xvfb-run -a build-plugin/plugin/sax_ui_snapshots [--out gates/G-004]
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Parameters.h"
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

juce::String fileNoteName(int midi) {
    static const char* names[12] = {"C", "Cs", "D", "Eb", "E", "F", "Fs", "G", "Ab", "A", "Bb", "B"};
    return juce::String(names[midi % 12]) + juce::String(midi / 12 - 1);
}

struct Session {
    SaxophoneAudioProcessor proc;
    std::unique_ptr<SaxophoneAudioProcessorEditor> editor;
    int held = -1;

    Session() {
        proc.prepareToPlay(48000.0, 256);
        editor.reset(static_cast<SaxophoneAudioProcessorEditor*>(proc.createEditor()));
        editor->setSize(900, 600);
    }
    void setHarmonic(bool on) {
        if (auto* p = proc.parameters().getParameter(saxplug::param::harmonic)) p->setValueNotifyingHost(on ? 1.0f : 0.0f);
    }
    juce::Image show(int concertNote) {
        juce::MidiBuffer midi;
        if (held >= 0) midi.addEvent(juce::MidiMessage::noteOff(1, held), 0);
        midi.addEvent(juce::MidiMessage::noteOn(1, concertNote, (juce::uint8) 90), 1);
        held = concertNote;
        juce::AudioBuffer<float> buf(2, 256);
        proc.processBlock(buf, midi);
        editor->refreshFromProcessor();
        return editor->createComponentSnapshot(editor->getLocalBounds());
    }
};

} // namespace

int main(int argc, char** argv) {
    juce::String outDir = "gates/G-004";
    for (int i = 1; i + 1 < argc; ++i)
        if (std::string(argv[i]) == "--out") outDir = argv[i + 1];
    const juce::File dir = juce::File::getCurrentWorkingDirectory().getChildFile(outDir);
    dir.createDirectory();

    juce::ScopedJuceInitialiser_GUI juceInit;
    int failures = 0, written = 0;
    {
        Session s;
        s.setHarmonic(false);
        for (int w = sax::kLowestWritten; w <= sax::kHighestWritten; ++w) {
            const auto f = dir.getChildFile(juce::String::formatted("written-%02d-", w) + fileNoteName(w) + ".png");
            if (savePng(s.show(w - sax::kAltoTransposition), f)) ++written; else { std::fprintf(stderr, "cannot write %s\n", f.getFullPathName().toRawUTF8()); ++failures; }
        }
    }
    int harmonic = 0;
    {
        Session s;
        s.setHarmonic(true);
        for (int n = 61; n <= 76; ++n) {
            const auto f = dir.getChildFile(juce::String::formatted("harmonic-concert-%02d.png", n));
            if (savePng(s.show(n), f)) ++harmonic; else ++failures;
        }
    }
    std::printf("wrote %d fingering + %d harmonic screenshots to %s (%d failures)\n", written, harmonic, dir.getFullPathName().toRawUTF8(), failures);
    return failures == 0 ? 0 : 1;
}
