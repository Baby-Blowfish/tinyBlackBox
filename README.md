# TinyBlackBox

A real-time video processing application that implements a multi-threaded system for capturing, displaying, and recording video frames.

## System Architecture

### System Overview
```
+------------------+     +------------------+     +------------------+
|   Input Video    |     |  Frame Buffer    |     |  Output Video    |
|      File        | --> |      Pool        | --> |      File        |
+------------------+     +------------------+     +------------------+
        ^                        ^                        ^
        |                        |                        |
        v                        v                        v
+------------------+     +------------------+     +------------------+
|   Capture        |     |   Display        |     |   Record         |
|   Thread         |     |   Thread         |     |   Thread         |
+------------------+     +------------------+     +------------------+
        |                        |                        |
        +------------------------+------------------------+
                                 |
                                 v
                        +------------------+
                        |   UI Thread      |
                        |   (Control)      |
                        +------------------+
```

### Thread Model
```
+------------------+     +------------------+     +------------------+
|   Capture        |     |   Display        |     |   Record         |
|   Thread         |     |   Thread         |     |   Thread         |
+------------------+     +------------------+     +------------------+
        |                        |                        |
        |  Reads frames          |  Renders frames        |  Writes frames
        |  at 30 FPS             |  to framebuffer       |  to file
        |                        |                        |
        |  Manages timing        |  Double buffering      |  Handles I/O
        |  (33ms intervals)      |  for display          |  operations
        |                        |                        |
        |  Implements loop       |  UI overlay            |  Frame buffering
        |  playback              |                        |
        |                        |                        |
        v                        v                        v
+------------------+     +------------------+     +------------------+
|   Frame Pool     |     |   Frame Pool     |     |   Frame Pool     |
|   Management     |     |   Management     |     |   Management     |
+------------------+     +------------------+     +------------------+
```

### Data Flow
```
[Source File] → Capture Thread → Frame Pool → Display Thread → [Framebuffer]
                                    ↓
                            Record Thread → [Output File]

Detailed Flow:
+-------------+     +-------------+     +-------------+     +-------------+
| Source File | --> |   Capture   | --> | Frame Pool  | --> |   Display   |
| (30 FPS)    |     |   Thread    |     | (Shared)    |     |   Thread    |
+-------------+     +-------------+     +-------------+     +-------------+
                                                              |
                                                              v
                                                          +-------------+
                                                          | Framebuffer |
                                                          +-------------+
                                    |
                                    v
                            +-------------+     +-------------+
                            |   Record    | --> | Output File |
                            |   Thread    |     | (30 FPS)    |
                            +-------------+     +-------------+
```

### Component Interaction
```
+------------------+     +------------------+     +------------------+
|   Frame Pool     |     |   Memory Pool    |     |   Queue System   |
|   Management     |     |   Management     |     |   Management     |
+------------------+     +------------------+     +------------------+
        |                        |                        |
        |  Thread-safe frame     |  Memory allocation     |  Thread-safe
        |  allocation            |  optimization          |  queues
        |                        |                        |
        |  Frame recycling       |  Memory pooling        |  Synchronization
        |                        |                        |
        |  No data copying       |  Fragmentation         |  Blocking/
        |                        |  reduction             |  non-blocking ops
        |                        |                        |
        v                        v                        v
+------------------+     +------------------+     +------------------+
|   Frame Buffer   |     |   Memory         |     |   Thread         |
|   Operations     |     |   Operations     |     |   Communication  |
+------------------+     +------------------+     +------------------+
```

## Project Structure

### Source Files (`/src`)
1. **Core Video Processing**
   - `capture.c` (5.2KB): Frame capture from source file
   - `display.c` (3.0KB): Frame rendering to framebuffer
   - `record.c` (3.9KB): Frame recording to output file
   - `main.c` (2.2KB): Application entry point and thread management

2. **Frame Management**
   - `frame.c` (4.4KB): Frame data structure and operations
   - `frame_pool.c` (4.4KB): Frame buffer pool implementation
   - `fbDraw.c` (24KB): Framebuffer drawing operations
   - `memory_pool.c` (3.3KB): Memory allocation and management

3. **System Components**
   - `queue.c` (1.5KB): Thread-safe queue implementation
   - `task.c` (1.6KB): Task scheduling and management
   - `ui.c` (3.0KB): User interface handling
   - `log.c` (983B): Logging system
   - `util.c` (129B): Utility functions

### Header Files (`/include`)
1. **Core Headers**
   - `capture.h` (1.5KB): Frame capture interface
   - `display.h` (923B): Display operations interface
   - `record.h` (1.2KB): Recording operations interface
   - `thread_arg.h` (853B): Thread argument structures

2. **Frame Management Headers**
   - `frame.h` (4.9KB): Frame data structures
   - `frame_pool.h` (4.4KB): Frame pool interface
   - `fbDraw.h` (9.7KB): Framebuffer drawing interface
   - `memory_pool.h` (3.9KB): Memory pool interface

3. **System Headers**
   - `queue.h` (2.2KB): Queue data structure interface
   - `task.h` (3.2KB): Task management interface
   - `ui.h` (1.6KB): UI interface
   - `log.h` (1.5KB): Logging interface
   - `util.h` (159B): Utility functions interface
   - `console_color.h` (265B): Console color definitions

## Technical Details

### Video Format
1. **File Structure**
   - Header: Two integers (width, height)
   - Frame data: width * height bytes per frame
   - Grayscale format (1 byte per pixel)
   - 30 FPS frame rate

2. **Frame Processing**
   - Frame size: width * height bytes
   - Frame timing: 33ms per frame
   - Continuous playback with loop
   - Pointer-based frame transfer

### Build System
1. **Compiler Settings**
   - GCC compiler
   - Wall and Wextra warnings enabled
   - Debug support (-g -O0)
   - pthread library linked

2. **Directory Structure**
   - `/src`: Source files
   - `/include`: Header files
   - `/bin`: Executable output
   - `/test`: Test files

### Key Features
1. **Performance Optimization**
   - Pointer-based frame transfer
   - Frame pool implementation
   - Memory pooling
   - Thread-safe operations

2. **Control Operations**
   - Start/Stop/Restart functionality
   - Real-time frame processing
   - Continuous playback
   - Thread synchronization

3. **Error Handling**
   - Comprehensive logging system
   - Error recovery mechanisms
   - Resource cleanup
   - Thread safety checks

## Development Tools
1. **Build Tools**
   - Make build system
   - GCC compiler
   - Debug support
   - Test framework

2. **Libraries**
   - POSIX Threads (pthread)
   - Standard C Library
   - Check testing framework

## What I Learned
1. **Thread Programming**
   - POSIX thread management
   - Thread synchronization
   - Inter-thread communication
   - Resource management

2. **Memory Management**
   - Memory pooling
   - Frame buffer management
   - Resource optimization
   - Memory safety

3. **Video Processing**
   - Frame buffer operations
   - Real-time processing
   - File I/O optimization
   - Display synchronization

4. **System Design**
   - Component architecture
   - Thread safety
   - Resource management
   - Error handling

## Building and Running

### Prerequisites
- GCC compiler
- Make build system
- POSIX Threads library
- Check testing framework (for running tests)

### Building the Project
1. **Standard Build**
   ```bash
   make
   ```
   This will create the executable in the `bin` directory.

2. **Debug Build**
   ```bash
   make debug
   ```
   Creates a debug version with additional debugging information.

3. **Clean Build**
   ```bash
   make clean
   ```
   Removes all built files from the `bin` directory.

### Running Tests
```bash
make test
```
This will compile and run the frame module tests.

## Command Guide

### Program Controls
1. **Start Processing**
   - Starts video capture, display, and recording
   - Initializes all threads and resources
   - Begins frame processing at 30 FPS

2. **Stop Processing**
   - Pauses all video processing
   - Maintains current state
   - Preserves resources for restart

3. **Restart Processing**
   - Resumes from the beginning
   - Reinitializes frame capture
   - Maintains display and recording settings

### File Operations
1. **Input File Format**
   - Binary file containing video data
   - Header: Two integers (width, height)
   - Frame data: width * height bytes per frame
   - Grayscale format (1 byte per pixel)

2. **Output File Format**
   - Same format as input
   - Created in the specified output directory
   - Maintains original frame rate

### Performance Options
1. **Frame Pool Size**
   - Configurable number of frame buffers
   - Affects memory usage and performance
   - Default: Optimized for 30 FPS processing

2. **Memory Management**
   - Pre-allocated memory pools
   - Configurable pool sizes
   - Optimized for continuous operation

### Logging
1. **Log Levels**
   - ERROR: Critical system errors
   - WARNING: Non-critical issues
   - INFO: General operation information
   - DEBUG: Detailed debugging information

2. **Log Output**
   - Console output with color coding
   - Timestamp for each log entry
   - Thread identification
   - Operation context

