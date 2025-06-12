/*
 * channel.c - Linux version using POSIX message queues
 */
#include "channel.h"

/**
 * Function to initialize channel subsystem (Linux version)
 */
void channel_init() {
    printf("Channel subsystem initialized for Linux\n");
}

/**
 * Function to create a new message queue channel for a server thread
 */
bool channel_create(channel_t* channel) {
    struct mq_attr attr;
    
    channel->ready = false;
    channel->is_server = true;
    
    // Set up message queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    
    // Create message queue name
    channel->mq_name = malloc(strlen(channel->name) + 10);
    sprintf(channel->mq_name, "/%s_mq", channel->name);
    
    // Remove existing message queue if it exists
    mq_unlink(channel->mq_name);
    
    // Create the message queue
    channel->mq_recv = mq_open(channel->mq_name, O_CREAT | O_RDONLY, 0644, &attr);
    if (channel->mq_recv == (mqd_t)-1) {
        printf("ERROR: Failed to create message queue %s: %s\n", channel->mq_name, strerror(errno));
        return false;
    }
    
    printf("[Channel] Created message queue channel %s\n", channel->name);
    
    channel->ready = true;
    return true;
}

/**
 * Function to connect to a message queue channel for a client thread
 */
bool channel_connect(channel_t* channel) {
    channel->ready = false;
    channel->is_server = false;
    
    // Create message queue name
    channel->mq_name = malloc(strlen(channel->name) + 10);
    sprintf(channel->mq_name, "/%s_mq", channel->name);
    
    // Open the message queue
    channel->mq_send = mq_open(channel->mq_name, O_WRONLY);
    if (channel->mq_send == (mqd_t)-1) {
        printf("ERROR: Failed to open message queue %s: %s\n", channel->mq_name, strerror(errno));
        return false;
    }
    
    printf("[Channel] Successfully connected to channel %s\n", channel->name);
    
    channel->ready = true;
    return true;
}

/**
 * Function to send payload over an existing channel (client)
 */
bool channel_send(channel_t* channel, uint8_t* data, int size) {
    if (!channel->ready) {
        return false;
    }
    
    msg_common_t* common = (msg_common_t*) data;
    common->type = CHANNEL_MSG_TYPE;
    
    // Send the message
    if (mq_send(channel->mq_send, (char*)data, size, 0) == -1) {
        printf("ERROR: Failed to send message on channel %s: %s\n", channel->name, strerror(errno));
        channel->ready = false;
        return false;
    }
    
    return true;
}

/**
 * Function to receive data on an existing channel (server)
 */
bool channel_receive(channel_t* channel, uint8_t* data, int size) {
    if (!channel->ready) {
        return false;
    }
    
    // Receive the message
    ssize_t bytes_received = mq_receive(channel->mq_recv, (char*)data, size, NULL);
    if (bytes_received == -1) {
        if (errno == EAGAIN) {
            // No message available (non-blocking mode)
            return false;
        }
        printf("ERROR: Failed to receive message on channel %s: %s\n", channel->name, strerror(errno));
        channel->ready = false;
        return false;
    }
    
    msg_common_t* common = (msg_common_t*) data;
    
    // Check message type
    if (common->type == CHANNEL_MSG_TYPE) {
        return true;
    }
    
    return false;
}

/**
 * Call the function to close the channel.
 */
void channel_cleanup(channel_t* channel) {
    if (channel->mq_send != (mqd_t)-1) {
        mq_close(channel->mq_send);
    }
    if (channel->mq_recv != (mqd_t)-1) {
        mq_close(channel->mq_recv);
    }
    
    // If server, unlink the message queue
    if (channel->is_server && channel->mq_name) {
        mq_unlink(channel->mq_name);
    }
    
    // Free allocated memory
    if (channel->mq_name) {
        free(channel->mq_name);
    }
}