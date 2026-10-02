// SPDX-License-Identifier: Apache-2.0
// Draws an alto saxophone from sax::altoKeyLayout() and highlights pressed keys (D-016).
#pragma once
#include "sax/Keys.h"
#include <juce_gui_basics/juce_gui_basics.h>

class SaxophoneView final : public juce::Component {
public:
    SaxophoneView();
    void paint(juce::Graphics&) override;
    /// Called by the editor's 60 Hz timer; repaints only if the set changed. Stub: ignores input.
    void setHighlightedKeys(const sax::KeySet& keys);
    sax::KeySet highlightedKeys() const noexcept;
private:
    sax::KeySet keys_;
};
