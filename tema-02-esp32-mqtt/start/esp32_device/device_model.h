// Darius Borca - interpretare comenzi, aceeasi functie in firmware si teste.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
inline void execute_command(const char* text, uint32_t& periodMs, float& threshold, char* reply, size_t replySize) {
    char action[24]{}, value[32]{}, extra[2]{};
    int count = sscanf(text, "%23s %31s %1s", action, value, extra);
    snprintf(reply, replySize, "ERROR comanda sau parametru invalid");
        if (count == 2 && strcmp(action, "SET_INTERVAL") == 0) {
        char* end; errno = 0; long n = strtol(value, &end, 10);
        if (!errno && !*end && n >= 50 && n <= 3600000) {
            periodMs = n; snprintf(reply, replySize, "OK SET_INTERVAL %ld", n);
        }
    } else if (count == 2 && strcmp(action, "SET_THRESHOLD") == 0) {
        char* end; errno = 0; float n = strtof(value, &end);
        if (!errno && !*end && isfinite(n)) {
            threshold = n; snprintf(reply, replySize, "OK SET_THRESHOLD %.2f", n);
        }
    } else if (count == 1 && strcmp(action, "RESET") == 0) {
        periodMs = PERIOD_MS_INITIAL; threshold = THRESHOLD_INITIAL;
        snprintf(reply, replySize, "OK RESET");
    } else if (count == 1 && strcmp(action, "STATUS") == 0) {
        snprintf(reply, replySize, "STATUS interval=%lu threshold=%.2f", (unsigned long)periodMs, threshold);
    }
}
