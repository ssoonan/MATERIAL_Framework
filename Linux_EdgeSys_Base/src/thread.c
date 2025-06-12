/*
 * thread.c - Linux version with basic POSIX thread support
 */

#include "thread.h"
#include "time_funcs.h"
#include "labels.h"

static struct thread_data* thread_list = NULL;
static pthread_mutex_t thread_list_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Add a thread description to the system
 */
struct thread_data* thread_addThreadDescription(
        char* _name,
        int _period_ms,
        int _deadline_ms,
        int _offset_us,
        int _priority,
        unsigned _core_affinity,
        int _partition_id) {
    
    struct thread_data* new_thread = malloc(sizeof(struct thread_data));
    if (!new_thread) return NULL;
    
    // Initialize thread data
    memset(new_thread, 0, sizeof(struct thread_data));
    new_thread->name = strdup(_name);
    new_thread->priority = _priority;
    new_thread->core_affinity = _core_affinity;
    new_thread->partition_id = _partition_id;
    
    // Convert periods to timespec
    new_thread->period.tv_sec = _period_ms / 1000;
    new_thread->period.tv_nsec = (_period_ms % 1000) * 1000000;
    
    new_thread->deadline.tv_sec = _deadline_ms / 1000;
    new_thread->deadline.tv_nsec = (_deadline_ms % 1000) * 1000000;
    
    new_thread->offset.tv_sec = _offset_us / 1000000;
    new_thread->offset.tv_nsec = (_offset_us % 1000000) * 1000;
    
    // Set up CPU affinity mask
    CPU_ZERO(&new_thread->cpu_mask);
    for (int i = 0; i < 32; i++) {
        if (_core_affinity & (1 << i)) {
            CPU_SET(i, &new_thread->cpu_mask);
        }
    }
    
    new_thread->thread_code = thread_baseTaskCode;
    atomic_store(&new_thread->terminate, false);
    
    // Add to thread list
    pthread_mutex_lock(&thread_list_mutex);
    new_thread->next = thread_list;
    thread_list = new_thread;
    pthread_mutex_unlock(&thread_list_mutex);
    
    printf("Added thread: %s (period=%dms, priority=%d)\n", _name, _period_ms, _priority);
    
    return new_thread;
}

/**
 * Register a runnable with a thread
 */
void thread_registerRunnable(char* _name, runnable_spec_t* _runnable_spec) {
    struct thread_data* thread = thread_getThread(_name);
    if (thread) {
        thread->runnable = _runnable_spec;
        printf("Registered runnable for thread: %s\n", _name);
    }
}

/**
 * Get thread by name
 */
struct thread_data* thread_getThread(char* name) {
    pthread_mutex_lock(&thread_list_mutex);
    struct thread_data* current = thread_list;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            pthread_mutex_unlock(&thread_list_mutex);
            return current;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&thread_list_mutex);
    return NULL;
}

/**
 * Base task code for all threads
 */
void* thread_baseTaskCode(void* arg) {
    struct thread_data* thread = (struct thread_data*)arg;
    struct timespec next_activation;
    
    printf("Thread %s started\n", thread->name);
    
    // Set thread priority and scheduling
    struct sched_param param;
    param.sched_priority = thread->priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        printf("Warning: Failed to set priority for thread %s\n", thread->name);
    }
    
    // Set CPU affinity
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &thread->cpu_mask) != 0) {
        printf("Warning: Failed to set CPU affinity for thread %s\n", thread->name);
    }
    
    // Initialize next activation time
    clock_gettime(CLOCK_MONOTONIC, &next_activation);
    next_activation.tv_sec += thread->offset.tv_sec;
    next_activation.tv_nsec += thread->offset.tv_nsec;
    if (next_activation.tv_nsec >= 1000000000) {
        next_activation.tv_sec++;
        next_activation.tv_nsec -= 1000000000;
    }
    
    thread->current_job_id = 0;
    
    while (!atomic_load(&thread->terminate)) {
        // Wait for next activation
        sleep_until(&next_activation);
        
        thread->current_job_id++;
        uint64_t start_time = get_time_ns();
        
        // Execute runnables
        if (thread->runnable) {
            thread_runnable_read(thread);
            thread_runnable_execute(thread);
            thread_runnable_write(thread);
        }
        
        uint64_t end_time = get_time_ns();
        uint64_t exec_time = end_time - start_time;
        
        // Update statistics (simplified)
        thread->execTime_ns.sum += exec_time;
        thread->execTime_ns.count++;
        
        // Calculate next activation
        next_activation.tv_sec += thread->period.tv_sec;
        next_activation.tv_nsec += thread->period.tv_nsec;
        if (next_activation.tv_nsec >= 1000000000) {
            next_activation.tv_sec++;
            next_activation.tv_nsec -= 1000000000;
        }
    }
    
    printf("Thread %s terminated\n", thread->name);
    return NULL;
}

/**
 * Execute runnable read phase
 */
void thread_runnable_read(struct thread_data* ps) {
    if (ps->runnable && ps->runnable->inputLabelCount > 0) {
        for (int i = 0; i < ps->runnable->inputLabelCount; i++) {
            labels_read(ps->runnable->inputLabelIds[i], ps->runnable->inputData[i]);
        }
    }
}

/**
 * Execute runnable
 */
void thread_runnable_execute(struct thread_data* ps) {
    if (ps->runnable && ps->runnable->func) {
        ps->runnable->func(ps->runnable);
    }
}

/**
 * Execute runnable write phase
 */
void thread_runnable_write(struct thread_data* ps) {
    if (ps->runnable && ps->runnable->outputLabelCount > 0) {
        for (int i = 0; i < ps->runnable->outputLabelCount; i++) {
            labels_write(ps->runnable->outputLabelIds[i], ps->runnable->outputData[i]);
        }
    }
}

/**
 * Create all threads
 */
void thread_createThreads() {
    pthread_mutex_lock(&thread_list_mutex);
    struct thread_data* current = thread_list;
    while (current) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        
        if (pthread_create(&current->pthread_id, &attr, current->thread_code, current) != 0) {
            printf("ERROR: Failed to create thread %s\n", current->name);
        } else {
            printf("Created thread %s\n", current->name);
        }
        
        pthread_attr_destroy(&attr);
        current = current->next;
    }
    pthread_mutex_unlock(&thread_list_mutex);
}

/**
 * Kill all threads
 */
void thread_killThreads() {
    pthread_mutex_lock(&thread_list_mutex);
    struct thread_data* current = thread_list;
    while (current) {
        atomic_store(&current->terminate, true);
        current = current->next;
    }
    pthread_mutex_unlock(&thread_list_mutex);
}

/**
 * Join all threads
 */
void thread_joinThreads() {
    pthread_mutex_lock(&thread_list_mutex);
    struct thread_data* current = thread_list;
    while (current) {
        if (current->pthread_id != 0) {
            pthread_join(current->pthread_id, NULL);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&thread_list_mutex);
}

/**
 * Initialize runnables
 */
void thread_initRunnables() {
    pthread_mutex_lock(&thread_list_mutex);
    struct thread_data* current = thread_list;
    while (current) {
        if (current->runnable && current->runnable->init) {
            current->runnable->init(current->runnable);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&thread_list_mutex);
}

/**
 * Deinitialize runnables
 */
void thread_deinitRunnables() {
    pthread_mutex_lock(&thread_list_mutex);
    struct thread_data* current = thread_list;
    while (current) {
        if (current->runnable && current->runnable->deinit) {
            current->runnable->deinit(current->runnable);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&thread_list_mutex);
}

// Stub implementations for other functions
struct thread_data* thread_getThreadPointer() { return thread_list; }
void thread_releaseCondition() {}
void thread_configureStrictPriorities() {}
int thread_setThreadParameters(int policy, int priority, pthread_t tid) { return 0; }
int thread_setAffinity(int numCores, unsigned coreBitmap) { return 0; }
void thread_printInfo() {}
void thread_traceThreadParameters(struct thread_data* thread) {}
void thread_setCancelation() {}
void thread_signalHandler(int signum) {}
void thread_printStatistics() {}
void thread_jobEnd(struct thread_data* thread) {}
struct thread_data* thread_getThreadFromTid(pid_t tid) { return NULL; }
void thread_saveMeasurementFiles(struct thread_data* thread) {}
void thread_saveAllMeasurements() {}
void thread_saveRecord(struct thread_data* thread, uint64_t execTime, uint64_t rt, uint64_t deadline) {}