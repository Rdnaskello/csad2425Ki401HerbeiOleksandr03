# csad2425Ki401HerbeiOleksandr03

## Tic-Tac-Toe Game with Arduino and SFML using UART Communication

### Repository Overview
This repository implements a Tic-Tac-Toe game with a client-server architecture:

- **Client**: Graphical User Interface (GUI) built using SFML.
- **Server**: Arduino-based microcontroller, communicating via UART protocol.

The project includes workflows for building, testing, and managing artifacts as part of Task 2.

---

## Task Details

### Task 1: Repository Initialization
1. **Create GitHub Repository**:
   - Name: `csad2425Ki401HerbeiOleksandr03`
   - Proper README and file structure.
2. **Add Student and Project Details in README.**
3. **Create Branch**: `feature/develop/task1` for development.

### Task 2: Client-Server Communication
#### SW <-> HW Communication Schema:
- The **client** sends a message to the Arduino **server** via UART.
- The Arduino **server** modifies the message and sends it back to the **client**.

#### Build and Test Workflow:
1. **Create a CI pipeline** using GitHub Actions:
   - **Build binaries** for the client application.
   - **Run tests** using mocked UART communication.
   - **Upload artifacts**, including binaries and test reports.

---

## How to Run

### Prerequisites
#### Hardware:
- Arduino Uno (or compatible board).
- Two LEDs for player/game status.

#### Software:
- SFML library installed.
- Arduino IDE for microcontroller code.

---

### Steps:

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/<your-repository-url>.git
2. **Build the Client**:
   ```bash
   mkdir build
   g++ -o build/client lib/client/main.cpp -std=c++17
3. **Run the Client**:
   ```bash
   ./build/client  
 4. **Run the Server**:
 - Upload the Arduino code (located in `arduino/server`) to the Arduino board using Arduino IDE.

## GitHub Actions Workflow

### CI/CD Pipeline
This project uses GitHub Actions to automate the build and test process:

- **Clones Repository**: Uses the `actions/checkout` action.
- **Sets Up MSYS2**: Installs required tools for building the C++ project.
- **Mocks UART Communication**: Generates mock serial input for testing.
- **Builds Project**: Compiles the client-side C++ application.
- **Runs Tests**: Simulates communication between client and server.
- **Uploads Artifacts**: Stores build outputs and test reports for verification.

---

### Artifacts:
- **Binaries**: Compiled `client.exe`.
- **Test Reports**: Results from CI tests.

---

## Technology and Hardware Details

### Technologies:
- **Programming Language**: C++ (Client-Side).
- **Framework**: SFML (Simple and Fast Multimedia Library).
- **Communication**: UART (Serial Communication Protocol).
- **CI/CD Tools**: GitHub Actions.

### Hardware:
- **Microcontroller**: Arduino Uno.
- **Input/Output Components**: Two LEDs for game status indication.
- **Serial Communication Interface**: UART protocol for client-server interaction.

---

## Student Details

- **Student Name**: Herbei Oleksandr
- **Student Number**: 03
- **Group**: KI-401

