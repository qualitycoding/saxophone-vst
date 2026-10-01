// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-018 (unit) — SC-2: every key has a drawable shape on the saxophone picture.
#include "sax/KeyLayout.h"
#include <catch2/catch_test_macros.hpp>
using namespace sax;

TEST_CASE("T-018 layout has one valid shape per key", "[T-018][unit]") {
    const auto& L = altoKeyLayout();
    for (std::size_t i = 0; i < kKeyCount; ++i) {
        INFO("key index " << i);
        const auto& s = L[i];
        CHECK(s.key == static_cast<KeyId>(i));
        CHECK(s.w > 0.005f); CHECK(s.h > 0.005f);
        CHECK(s.x >= 0.0f); CHECK(s.y >= 0.0f);
        CHECK(s.x + s.w <= 1.0f); CHECK(s.y + s.h <= 1.0f);
    }
    // No two keys share the same centre (they must be visually distinguishable).
    for (std::size_t i = 0; i < kKeyCount; ++i)
        for (std::size_t j = i + 1; j < kKeyCount; ++j) {
            const float dx = (L[i].x + L[i].w / 2) - (L[j].x + L[j].w / 2);
            const float dy = (L[i].y + L[i].h / 2) - (L[j].y + L[j].h / 2);
            CHECK(dx * dx + dy * dy > 1e-4f);
        }
}
