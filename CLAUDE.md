# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MATERIAL Framework is a code generation framework for modeling and automated code generation of edge real-time applications. It supports both QNX and Linux target platforms. It uses the APP4MC Tool Platform to parse Amalthea models and generate platform-specific C code for edge computing systems.

## Architecture

The framework consists of three main components:

1. **CodeGeneration** - Java-based code generator that:
   - Parses Amalthea models (HW, OS, SW, mapping files)
   - Generates node-specific C code for target platforms
   - Creates configuration files for tasks, runnables, channels, and labels
   - Manages compilation and deployment

2. **QNX_EdgeSys_Base** - C-based runtime framework for QNX that:
   - Provides QNX-specific thread management and scheduling
   - Implements QNX native message passing for IPC
   - Handles APS (Adaptive Partitioning Scheduler) reservations
   - Manages runtime statistics and tracing

3. **Linux_EdgeSys_Base** - C-based runtime framework for Linux that:
   - Provides POSIX thread management with real-time scheduling
   - Implements POSIX message queues for IPC
   - Uses standard Linux scheduling (no APS)
   - Manages runtime statistics and basic tracing

## Key Commands

### Running Code Generation
```bash
# In CodeGeneration directory
# Edit Main.java to select experiment (lines 24, 21-22 for flags)
# Edit Generator.java line 199 to choose base project:
#   - "../QNX_EdgeSys_Base" for QNX targets
#   - "../Linux_EdgeSys_Base" for Linux targets
# Run the Java application to generate code
```

### Compiling Generated Code

**For QNX targets:**
```bash
cd TargetProject/<project_name>/<node_name>
./compile.sh  # Note: Update QNX paths in compile.sh first
```

**For Linux targets:**
```bash
cd TargetProject/<project_name>/<node_name>
make clean && make
# Requires: gcc, librt, libpthread
```

### Deploying to Targets

**QNX targets:**
```bash
./transfer.sh  # Uses expect to FTP and telnet to QNX target
```

**Linux targets:**
```bash
./transfer.sh  # Uses scp and ssh to Linux target
```

## Important Configuration Points

1. **compile.sh** - Must update QNX paths:
   - `CWD` and `PWD` to your workspace path
   - `QNX_HOST` and `QNX_TARGET` to your QNX installation
   - Update paths in lines 11-23 before compilation

2. **Main.java** - Control code generation:
   - `COMPILE` flag (line 21) - Auto-compile after generation
   - `TRANSFER` flag (line 22) - Auto-deploy to targets
   - `selectedExperiment` (line 24) - Choose which example to generate

3. **IP Addresses** - Set in Amalthea OS model files for each node

## Code Generation Flow

1. Parser reads Amalthea models (HW, OS, SW, mapping)
2. Creates EdgeSystem model with nodes, tasks, runnables, channels
3. Generates per-node configuration:
   - `sysConfig.h` - System configuration (tasks, scheduling)
   - `user_runnables.c/h` - Task implementations
   - `labels_conf.c/h` - Shared memory labels
   - `channel_conf.h` - IPC channel definitions
4. Copies base project and generated files to TargetProject
5. Optionally compiles and transfers binaries

## Model Structure

Amalthea models must include:
- Hardware model (.amxmi) - Processing units, cores
- OS model (.amxmi) - Operating systems, schedulers
- Software model (.amxmi) - Tasks, runnables, labels, activations
- Mapping model (.amxmi) - Task-to-core assignments

## Scheduling Support

- Partitioned scheduling (fixed core assignment)
- Global scheduling (multi-core)
- APS (Adaptive Partitioning Scheduler) reservations
- Priority-based preemptive scheduling

## Testing Individual Components

To test generated code locally:
```bash
# After generation, in node directory
make clean
make all
# Binary will be in build/aarch64le-debug/
```