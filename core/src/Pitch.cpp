// SPDX-License-Identifier: Apache-2.0
#include "sax/Pitch.h"
#include <cmath>
#include <stdexcept>

namespace sax {

bool isInRange(int concertMidi) noexcept {
    return concertMidi >= kLowestConcert && concertMidi <= kHighestConcert;
}

int writtenFromConcert(int concertMidi) {
    if (!isInRange(concertMidi)) throw std::out_of_range("concert note outside Db3..A5");
    return concertMidi + kAltoTransposition;
}

int concertFromWritten(int writtenMidi) {
    if (writtenMidi < kLowestWritten || writtenMidi > kHighestWritten)
        throw std::out_of_range("written note outside Bb3..F#6");
    return writtenMidi - kAltoTransposition;
}

double equalTemperedHz(double midi, double a4Hz) noexcept {
    return a4Hz * std::exp2((midi - 69.0) / 12.0);
}

} // namespace sax
