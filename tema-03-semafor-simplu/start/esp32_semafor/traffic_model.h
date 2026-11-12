// Darius Borca - model pur, verificabil si fara placa.
#pragma once
#include <stdint.h>
enum Phase { CAR_GREEN, CAR_YELLOW, ALL_RED, PED_GREEN };
static const char* const PHASE_NAMES[] = {"CAR_GREEN", "CAR_YELLOW", "ALL_RED", "PED_GREEN"};
struct TrafficState { Phase phase; uint32_t since; bool requested; };
inline void advance(TrafficState& s, uint32_t now) {
    uint32_t elapsed = now - s.since;
    Phase next = s.phase;
    switch (s.phase) {
        case CAR_GREEN: if (s.requested && elapsed >= CAR_GREEN_MIN_MS) next = CAR_YELLOW; break;
        case CAR_YELLOW: if (elapsed >= CAR_YELLOW_MS) next = ALL_RED; break;
        case ALL_RED: if (elapsed >= ALL_RED_MS) { next = PED_GREEN; s.requested = false; } break;
        case PED_GREEN: if (elapsed >= PED_GREEN_MS) next = CAR_GREEN; break;
    }
    if (next != s.phase) { s.phase = next; s.since = now; }
}
inline unsigned seconds_left(const TrafficState& s, uint32_t now) {
    uint32_t elapsed = now - s.since;
    return s.phase == PED_GREEN && elapsed < PED_GREEN_MS ? (PED_GREEN_MS - elapsed + 999) / 1000 : 0;
}
