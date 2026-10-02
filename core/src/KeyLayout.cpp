// SPDX-License-Identifier: Apache-2.0
// Key geometry for the drawn alto saxophone, normalised to the view (origin top-left, 0..1).
// The body tube is drawn by SaxophoneView around x ~ 0.45; these positions are consistent with it.
#include "sax/KeyLayout.h"

namespace sax {
namespace {

constexpr KeyShape C(KeyId k, float x, float y, float d) { return {k, KeyShapeKind::Circle, x, y, d, d, 0.0f}; }
constexpr KeyShape P(KeyId k, float x, float y, float w, float h, float rot = 0.0f) { return {k, KeyShapeKind::Pill, x, y, w, h, rot}; }
constexpr KeyShape T(KeyId k, float x, float y, float w, float h) { return {k, KeyShapeKind::Plate, x, y, w, h, 0.0f}; }

const std::array<KeyShape, kKeyCount> kLayout = {{
    P(KeyId::Octave,     0.50f, 0.12f, 0.060f, 0.030f),         // left-thumb octave key (rear, near neck)
    C(KeyId::FrontF,     0.38f, 0.20f, 0.040f),                 // front F
    C(KeyId::PalmD,      0.28f, 0.15f, 0.045f),                 // palm keys, upper left
    C(KeyId::PalmEb,     0.28f, 0.215f, 0.045f),
    C(KeyId::PalmF,      0.28f, 0.28f, 0.045f),
    C(KeyId::LH1,        0.42f, 0.26f, 0.070f),                 // left-hand stack
    P(KeyId::Bis,        0.51f, 0.335f, 0.035f, 0.025f),        // bis Bb, between LH1 and LH2
    C(KeyId::LH2,        0.42f, 0.36f, 0.070f),
    C(KeyId::LH3,        0.42f, 0.46f, 0.070f),
    T(KeyId::GSharp,     0.32f, 0.50f, 0.055f, 0.035f),         // left pinky table
    T(KeyId::LowCSharp,  0.32f, 0.56f, 0.055f, 0.035f),
    T(KeyId::LowB,       0.32f, 0.62f, 0.055f, 0.035f),
    T(KeyId::LowBb,      0.32f, 0.68f, 0.055f, 0.035f),
    P(KeyId::SideE,      0.56f, 0.30f, 0.040f, 0.030f),         // right-hand side keys (played by RH knuckle)
    P(KeyId::SideC,      0.56f, 0.36f, 0.040f, 0.030f),
    P(KeyId::SideBb,     0.56f, 0.42f, 0.040f, 0.030f),
    P(KeyId::HighFSharp, 0.61f, 0.24f, 0.040f, 0.030f),
    C(KeyId::RH1,        0.42f, 0.58f, 0.070f),                 // right-hand stack
    C(KeyId::RH2,        0.42f, 0.68f, 0.070f),
    C(KeyId::RH3,        0.42f, 0.78f, 0.070f),
    P(KeyId::AltFSharp,  0.52f, 0.70f, 0.040f, 0.030f),
    T(KeyId::LowEb,      0.54f, 0.86f, 0.055f, 0.035f),         // right pinky table
    T(KeyId::LowC,       0.54f, 0.92f, 0.055f, 0.035f),
}};

} // namespace

const std::array<KeyShape, kKeyCount>& altoKeyLayout() noexcept { return kLayout; }

} // namespace sax
