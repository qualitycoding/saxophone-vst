// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): full drawing in S-014.
#include "SaxophoneView.h"
SaxophoneView::SaxophoneView() = default;
void SaxophoneView::paint(juce::Graphics& g) { g.fillAll(juce::Colours::black); }
void SaxophoneView::setHighlightedKeys(const sax::KeySet&) {}
sax::KeySet SaxophoneView::highlightedKeys() const noexcept { return keys_; }
