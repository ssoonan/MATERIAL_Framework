/*
 * time_funcs.c - Linux version
 */

#include "time_funcs.h"
#include <stdio.h>
#include <unistd.h>

/**
 * Get current time in nanoseconds
 */
uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/**
 * Burn CPU cycles for the specified time
 */
void burn_cycles(uint64_t exec_time_us, clockid_t cid) {
    uint64_t start_time = get_time_ns();
    uint64_t target_time = start_time + (exec_time_us * 1000);
    
    // Simple busy wait
    while (get_time_ns() < target_time) {
        // Busy loop
    }
}

/**
 * Sleep until target time
 */
void sleep_until(struct timespec* target_time) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, target_time, NULL);
}