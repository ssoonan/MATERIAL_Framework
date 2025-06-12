/*
 * channel.h - Linux version using POSIX message queues
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <inttypes.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>

/**
 * Type used to identify all messages sent by a user communication channel.
 */
#define CHANNEL_MSG_TYPE 10

/**
 * Status used to send back to the client to indicate that the message was received ok.
 */
#define REPLY_OK 20

/**
 * Maximum message size for POSIX message queues
 */
#define MAX_MSG_SIZE 8192

/**
 * Message header structure
 */
typedef struct {
    uint32_t type;
    uint32_t size;
} msg_header_t;

/**
 * Struct type to encapsulate the information needed for one communication channel.
 */
typedef struct {
    char* name;             // Global name of the channel
    
    mqd_t mq_send;         // Message queue for sending
    mqd_t mq_recv;         // Message queue for receiving
    
    char* mq_name;         // Message queue name
    
    void* msg;             // Pointer to the memory area used to prepare/store a message
    
    bool ready;            // A flag to indicate that the channel is configured and can be used
    bool is_server;        // Flag to indicate if this is server or client side
} channel_t;

/**
 * Struct we use to send at the beginning of each message
 */
typedef struct {
    msg_header_t hdr;      // Message header
    uint8_t type;          // Type of this message
} msg_common_t;

void channel_init();
bool channel_create(channel_t* channel);
bool channel_connect(channel_t* channel);
void channel_cleanup(channel_t* channel);
bool channel_send(channel_t* channel, uint8_t* data, int size);
bool channel_receive(channel_t* channel, uint8_t* data, int size);

#endif /* CHANNEL_H_ */