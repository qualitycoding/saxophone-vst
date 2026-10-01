// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "sax/Keys.h"
namespace sax {
std::string_view keyName(KeyId) noexcept { return {}; }
std::optional<KeyId> keyFromName(std::string_view) noexcept { return std::nullopt; }
}
