// SPDX-License-Identifier: Apache-2.0
// Plugin state persistence (D-013): UTF-8 JSON object, schema "saxophone-vst/state@1".
// Deserialization is a trust boundary: it never throws, bounds input size, ignores unknown keys,
// falls back to defaults for missing / wrongly typed keys and clamps every value.
#include "sax/State.h"
#include <algorithm>
#include <cmath>
#include <nlohmann/json.hpp>

namespace sax {
namespace {

constexpr std::size_t kMaxStateBytes = 64 * 1024;
constexpr const char* kSchema = "saxophone-vst/state@1";

using nlohmann::json;

void readFloat(const json& obj, const char* key, float& out) noexcept {
    const auto it = obj.find(key);
    if (it == obj.end() || !it->is_number()) return;
    const double d = it->get<double>();
    if (!std::isfinite(d)) return;
    out = static_cast<float>(std::min(std::max(d, -1.0e12), 1.0e12)); // keep the float conversion in range
}

} // namespace

std::string serializeState(const VoiceParameters& p) {
    json j;
    j["schema"] = kSchema;
    j["overblow"] = p.overblow;
    j["harmonicMode"] = p.harmonicMode;
    j["reedHardness"] = p.reedHardness;
    j["brightness"] = p.brightness;
    j["breathNoise"] = p.breathNoise;
    j["vibratoRateHz"] = p.vibratoRateHz;
    j["vibratoDepth"] = p.vibratoDepth;
    j["portamentoMs"] = p.portamentoMs;
    j["tuningA4Hz"] = p.tuningA4Hz;
    j["outputGainDb"] = p.outputGainDb;
    return j.dump();
}

std::optional<VoiceParameters> deserializeState(std::string_view text) noexcept {
    try {
        if (text.size() > kMaxStateBytes) return std::nullopt;
        const json j = json::parse(text.begin(), text.end(), nullptr, /*allow_exceptions=*/false);
        if (j.is_discarded() || !j.is_object()) return std::nullopt;
        const auto sc = j.find("schema");
        if (sc == j.end() || !sc->is_string() || sc->get<std::string>() != kSchema) return std::nullopt;

        VoiceParameters p;
        readFloat(j, "overblow", p.overblow);
        const auto hm = j.find("harmonicMode");
        if (hm != j.end() && hm->is_boolean()) p.harmonicMode = hm->get<bool>();
        readFloat(j, "reedHardness", p.reedHardness);
        readFloat(j, "brightness", p.brightness);
        readFloat(j, "breathNoise", p.breathNoise);
        readFloat(j, "vibratoRateHz", p.vibratoRateHz);
        readFloat(j, "vibratoDepth", p.vibratoDepth);
        readFloat(j, "portamentoMs", p.portamentoMs);
        readFloat(j, "tuningA4Hz", p.tuningA4Hz);
        readFloat(j, "outputGainDb", p.outputGainDb);
        return clamped(p);
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace sax
