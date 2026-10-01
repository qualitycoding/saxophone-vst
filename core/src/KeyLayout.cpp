// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "sax/KeyLayout.h"
namespace sax {
const std::array<KeyShape, kKeyCount>& altoKeyLayout() noexcept {
    static const std::array<KeyShape, kKeyCount> empty{};
    return empty;
}
}
