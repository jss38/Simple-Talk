# Simple-Talk (s-talk)

A lightweight, terminal-based chat application that allows two users to communicate over a network using UDP sockets.

## Overview

Simple-Talk (s-talk) is a command-line chat application that enables bidirectional communication between two machines over a network. It uses UDP sockets for message transmission and implements a multi-threaded architecture to handle keyboard input, screen output, and network communication simultaneously.

## Features

- Real-time bidirectional communication
- Multi-threaded design for responsive performance
- Simple command-line interface
- Graceful termination with "!" command
- Cross-platform compatibility (Unix/Linux systems)

## Requirements

- C compiler (gcc recommended)
- POSIX threads library (pthread)
- Unix/Linux operating system

## Building the Application

To compile the application, simply run:

```bash
make
```

This will generate the executable `s-talk`.

To clean the build files:

```bash
make clean
```

## Usage

To start a chat session, run the s-talk executable with the following parameters:

```bash
./s-talk <local-port> <remote-machine-name> <remote-port>
```

Where:
- `<local-port>`: The port number on your machine to listen for incoming messages
- `<remote-machine-name>`: The hostname or IP address of the remote machine
- `<remote-port>`: The port number on the remote machine to send messages to

### Example

On Machine A:
```bash
./s-talk 4020 machine-b 4021
```

On Machine B:
```bash
./s-talk 4021 machine-a 4020
```

## How to Use

1. Start the application on both machines with the appropriate parameters
2. Type your message and press Enter to send
3. Messages from the other user will appear on your screen
4. To terminate the chat session, type "!" and press Enter

## Architecture

The application uses four threads:
1. Keyboard input thread - captures user input
2. Screen output thread - displays received messages
3. Network output thread - sends messages to the remote machine
4. Network input thread - receives messages from the remote machine

Data is passed between threads using a thread-safe list implementation.
