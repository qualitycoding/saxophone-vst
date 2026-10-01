// SPDX-License-Identifier: Apache-2.0
// Geometry of the drawn saxophone keys, in normalised editor coordinates (0..1, origin top-left).
#pragma once
#include "sax/Keys.h"
#include <array>

namespace sax {

enum class KeyShapeKind : std::uint8_t { Circle, Pill, Plate };

struct KeyShape {
    KeyId key = KeyId::Count;
    KeyShapeKind kind = KeyShapeKind::Circle;
    float x = 0, y = 0, w = 0, h = 0;   ///< bounding box, all within [0,1]
    float rotationDeg = 0;
};

/// One entry per KeyId, in KeyId order. Stub sentinel: all entries default-constructed.
const std::array<KeyShape, kKeyCount>& altoKeyLayout() noexcept;

} // namespace sax
