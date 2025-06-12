#!/bin/bash

################################################################################
# Linux transfer script - uses scp instead of QNX-specific FTP/telnet
################################################################################

TARGET_HOST="192.168.1.211"
TARGET_USER="pi"
TARGET_PATH="/home/pi/material_framework/"
BINARY_NAME="Linux_EdgeSys_Base"

echo "Transferring $BINARY_NAME to Linux target $TARGET_HOST..."

# Create directory on target if it doesn't exist
ssh ${TARGET_USER}@${TARGET_HOST} "mkdir -p ${TARGET_PATH}"

# Copy the binary to the target
scp build/${BINARY_NAME} ${TARGET_USER}@${TARGET_HOST}:${TARGET_PATH}

# Make it executable
ssh ${TARGET_USER}@${TARGET_HOST} "chmod +x ${TARGET_PATH}${BINARY_NAME}"

echo "Transfer complete. To run:"
echo "ssh ${TARGET_USER}@${TARGET_HOST}"
echo "cd ${TARGET_PATH}"
echo "./${BINARY_NAME}"