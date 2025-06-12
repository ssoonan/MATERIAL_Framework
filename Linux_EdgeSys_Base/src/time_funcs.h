/*
 * time_funcs.h - Linux version
 */

#ifndef TIME_FUNCS_H_
#define TIME_FUNCS_H_

#include <time.h>
#include <stdint.h>
#include <errno.h>

uint64_t get_time_ns();
void burn_cycles(uint64_t exec_time_us, clockid_t cid);
void sleep_until(struct timespec* target_time);

#endif /* TIME_FUNCS_H_ */