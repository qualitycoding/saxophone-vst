// SPDX-License-Identifier: Apache-2.0
#include "sax/ResonatorTable.h"
#include "sax/Errors.h"
#include <cmath>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace sax {
namespace {

using nlohmann::json;

constexpr std::size_t kMaxBytes = 1u << 20; // 1 MiB (D-009)

[[noreturn]] void fail(const std::string& what) { throw ParseError("resonator table: " + what); }

double number(const json& obj, const char* key) {
    if (!obj.is_object()) fail("expected an object");
    const auto it = obj.find(key);
    if (it == obj.end() || !it->is_number()) fail(std::string("missing or non-numeric field '") + key + "'");
    const double v = it->get<double>();
    if (!std::isfinite(v)) fail(std::string("non-finite value in '") + key + "'");
    return v;
}

RegisterHole parseHole(const json& e) {
    const auto it = e.find("hole");
    if (it == e.end() || !it->is_string()) fail("missing 'hole'");
    const std::string h = it->get<std::string>();
    if (h == "none") return RegisterHole::None;
    if (h == "body") return RegisterHole::Body;
    if (h == "neck") return RegisterHole::Neck;
    fail("unknown hole '" + h + "'");
}

} // namespace

ResonatorTable ResonatorTable::fromJson(std::string_view text) {
    if (text.size() > kMaxBytes) fail("file larger than 1 MiB");
    json doc;
    try {
        doc = json::parse(text.begin(), text.end());
    } catch (const std::exception& e) {
        fail(std::string("invalid JSON (") + e.what() + ")");
    }
    try {
        if (!doc.is_object()) fail("top level is not an object");
        const auto sc = doc.find("schema");
        if (sc == doc.end() || !sc->is_string() || sc->get<std::string>() != "saxophone-vst/resonators@1")
            fail("wrong or missing schema tag");
        const auto en = doc.find("entries");
        if (en == doc.end() || !en->is_array()) fail("missing 'entries' array");

        ResonatorTable t;
        for (const json& e : *en) {
            if (!e.is_object()) fail("entry is not an object");
            const double wd = number(e, "written");
            if (wd != std::floor(wd) || wd < 0.0 || wd > 127.0) fail("bad 'written'");
            const int written = static_cast<int>(wd);
            const RegisterHole hole = parseHole(e);
            for (const Entry& x : t.entries_)
                if (x.written == written && x.hole == hole) fail("duplicate (written, hole) entry");

            ResonatorParams p;
            if (e.contains("tuning_scale")) {
                p.tuningScale = number(e, "tuning_scale");
                if (p.tuningScale < 0.8 || p.tuningScale > 1.25) fail("tuning_scale outside [0.8, 1.25]");
            }
            if (e.contains("harmonic_stretch")) {
                p.harmonicStretch = number(e, "harmonic_stretch");
                if (p.harmonicStretch < 0.8 || p.harmonicStretch > 1.25) fail("harmonic_stretch outside [0.8, 1.25]");
            }
            const auto md = e.find("modes");
            if (md == e.end() || !md->is_array()) fail("missing 'modes'");
            if (md->size() < 4 || md->size() > 16) fail("mode count outside [4, 16]");
            double lastIm = -1.0;
            for (const json& m : *md) {
                Mode mode;
                mode.pole = {number(m, "re_s"), number(m, "im_s")};
                mode.residue = {number(m, "re_c"), number(m, "im_c")};
                if (!(mode.pole.real() < 0.0)) fail("unstable pole (Re >= 0)");
                if (!(mode.pole.imag() > lastIm)) fail("modes not sorted by increasing Im(pole)");
                lastIm = mode.pole.imag();
                p.modes.push_back(mode);
            }
            t.entries_.push_back({written, hole, std::move(p)});
        }
        return t;
    } catch (const ParseError&) {
        throw;
    } catch (const std::exception& e) {
        fail(std::string("malformed (") + e.what() + ")");
    }
}

const ResonatorParams& ResonatorTable::lookup(int writtenMidi, RegisterHole hole) const {
    for (const Entry& e : entries_)
        if (e.written == writtenMidi && e.hole == hole) return e.params;
    throw std::out_of_range("ResonatorTable::lookup: no entry for this (written, hole)");
}

std::size_t ResonatorTable::size() const noexcept { return entries_.size(); }

} // namespace sax
