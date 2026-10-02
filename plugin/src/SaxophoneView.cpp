// SPDX-License-Identifier: Apache-2.0
// Alto saxophone drawing (D-016). Key positions come from sax::altoKeyLayout(); the body, neck, bow and
// bell are drawn around them in the same normalised coordinates.
#include "SaxophoneView.h"
#include "sax/KeyLayout.h"

namespace {

const juce::Colour kBrassDark{0xff7a5a0c}, kBrassMid{0xffd9a921}, kBrassLight{0xfff3dc8a};
const juce::Colour kPearl{0xfff1ead8}, kPearlEdge{0xff8d8470};
const juce::Colour kKeyMetal{0xffc9a74a};
const juce::Colour kPressed{0xffff7a00}, kPressedGlow{0xffffb347};

juce::Rectangle<float> drawingArea(juce::Rectangle<float> bounds) {
    const float w = std::min(bounds.getWidth(), bounds.getHeight() * SaxophoneView::kAspect);
    return bounds.withSizeKeepingCentre(w, w / SaxophoneView::kAspect);
}

} // namespace

SaxophoneView::SaxophoneView() { setOpaque(true); }

void SaxophoneView::setHighlightedKeys(const sax::KeySet& keys) {
    if (keys == keys_) return;
    keys_ = keys;
    repaint();
}

sax::KeySet SaxophoneView::highlightedKeys() const noexcept { return keys_; }

void SaxophoneView::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff15171c));
    const auto r = drawingArea(getLocalBounds().toFloat());
    auto P = [&r](float x, float y) { return juce::Point<float>(r.getX() + x * r.getWidth(), r.getY() + y * r.getHeight()); };
    const juce::ColourGradient brass(kBrassDark, P(0.38f, 0.0f), kBrassDark, P(0.56f, 0.0f), false);
    juce::ColourGradient tube(kBrassDark, P(0.39f, 0.0f), kBrassDark, P(0.53f, 0.0f), false);
    tube.addColour(0.45, kBrassLight);
    tube.addColour(0.7, kBrassMid);

    // bow + bell (behind the body)
    juce::Path bow;
    bow.startNewSubPath(P(0.455f, 0.80f));
    bow.cubicTo(P(0.455f, 0.99f), P(0.64f, 1.00f), P(0.69f, 0.84f));
    bow.lineTo(P(0.71f, 0.70f));
    g.setGradientFill(tube);
    g.strokePath(bow, juce::PathStrokeType(0.085f * r.getWidth(), juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path bell;
    bell.startNewSubPath(P(0.675f, 0.72f));
    bell.lineTo(P(0.745f, 0.72f));
    bell.lineTo(P(0.82f, 0.585f));
    bell.lineTo(P(0.60f, 0.585f));
    bell.closeSubPath();
    g.setGradientFill(tube);
    g.fillPath(bell);
    const auto rim = juce::Rectangle<float>(P(0.60f, 0.555f), P(0.82f, 0.615f));
    g.setColour(juce::Colour(0xff090a0c));
    g.fillEllipse(rim);
    g.setColour(kBrassLight);
    g.drawEllipse(rim, 2.0f);

    // main tube
    juce::Path body;
    body.startNewSubPath(P(0.425f, 0.17f));
    body.lineTo(P(0.485f, 0.17f));
    body.lineTo(P(0.520f, 0.82f));
    body.lineTo(P(0.390f, 0.82f));
    body.closeSubPath();
    g.setGradientFill(tube);
    g.fillPath(body);
    g.setColour(kBrassDark.darker(0.4f));
    g.strokePath(body, juce::PathStrokeType(1.5f));

    // neck and mouthpiece
    juce::Path neck;
    neck.startNewSubPath(P(0.455f, 0.19f));
    neck.quadraticTo(P(0.455f, 0.045f), P(0.60f, 0.045f));
    g.setColour(kBrassMid);
    g.strokePath(neck, juce::PathStrokeType(0.032f * r.getWidth(), juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path mouthpiece;
    mouthpiece.startNewSubPath(P(0.60f, 0.030f));
    mouthpiece.lineTo(P(0.76f, 0.050f));
    mouthpiece.lineTo(P(0.76f, 0.068f));
    mouthpiece.lineTo(P(0.60f, 0.062f));
    mouthpiece.closeSubPath();
    g.setColour(juce::Colour(0xff1b1b1f));
    g.fillPath(mouthpiece);
    g.setColour(juce::Colour(0xff444450));
    g.strokePath(mouthpiece, juce::PathStrokeType(1.2f));

    // keys
    for (const auto& k : sax::altoKeyLayout()) {
        const bool down = keys_.test(sax::index(k.key));
        const juce::Rectangle<float> box(P(k.x, k.y), P(k.x + k.w, k.y + k.h));
        juce::Path shape;
        if (k.kind == sax::KeyShapeKind::Circle) shape.addEllipse(box);
        else shape.addRoundedRectangle(box, k.kind == sax::KeyShapeKind::Pill ? box.getHeight() * 0.5f : box.getHeight() * 0.25f);
        if (std::abs(k.rotationDeg) > 1e-6f) shape.applyTransform(juce::AffineTransform::rotation(juce::degreesToRadians(k.rotationDeg), box.getCentreX(), box.getCentreY()));

        if (down) { // glow, then a saturated fill: unmistakable at a glance
            g.setColour(kPressedGlow.withAlpha(0.35f));
            g.fillPath(shape, juce::AffineTransform::scale(1.0f, 1.0f));
            g.strokePath(shape, juce::PathStrokeType(box.getHeight() * 0.55f));
            g.setColour(kPressed);
            g.fillPath(shape);
            g.setColour(juce::Colours::white);
            g.strokePath(shape, juce::PathStrokeType(2.0f));
        } else {
            g.setColour(k.kind == sax::KeyShapeKind::Circle ? kPearl : kKeyMetal);
            g.fillPath(shape);
            g.setColour(k.kind == sax::KeyShapeKind::Circle ? kPearlEdge : kBrassDark);
            g.strokePath(shape, juce::PathStrokeType(1.2f));
        }
    }

    // hand labels
    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.setFont(juce::FontOptions(std::max(10.0f, r.getHeight() * 0.028f)));
    g.drawText("LEFT HAND", juce::Rectangle<float>(P(0.04f, 0.31f), P(0.25f, 0.35f)), juce::Justification::centredLeft);
    g.drawText("RIGHT HAND", juce::Rectangle<float>(P(0.04f, 0.66f), P(0.25f, 0.70f)), juce::Justification::centredLeft);
}
