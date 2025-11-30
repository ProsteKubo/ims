# Build & Tooling

## Overview
This project implements a hybrid simulation of opioid interaction (Tolerance vs Metabolic Saturation) using **SIMLIB/C++**.
It includes a **Text User Interface (TUI)** for real-time visualization of the Petri net states and physiological variables.

## Requirements

### System Dependencies
*   **OS**: macOS (or Linux)
*   **Compiler**: C++11 compliant (GCC/Clang)
*   **Build Tool**: Make

### Libraries
1.  **SIMLIB/C++**: Core simulation library.
    *   [Download](http://www.fit.vutbr.cz/~peringer/SIMLIB/)
2.  **ncurses**: Required for the TUI visualization.
    *   macOS: Pre-installed or `brew install ncurses`
    *   Linux: `sudo apt-get install libncurses5-dev`

## Setup Instructions

### 1. Install SIMLIB
```bash
wget http://www.fit.vutbr.cz/~peringer/SIMLIB/simlib-3.02.tar.gz
tar -zxf simlib-3.02.tar.gz
cd simlib
make
sudo make install
```

### 2. Build the Project
The project uses a standard Makefile. To compile the simulation and the TUI:

```bash
make
```

This will generate the `simulation` executable.

## Running the Simulation

```bash
./simulation
```

### TUI Controls
The simulation launches in a TUI mode by default, visualizing:
*   **Petri Net State**: Current behavioral state (Pain, Relief, Toxic, etc.)
*   **Physiological Metrics**: Real-time values for Concentration (C), Tolerance (Tol), and Effect (E).
*   **Alerts**: Visual indicators for Overdose/Toxic thresholds.

To run in headless mode (data export only):
```bash
./simulation --headless
```

## Build Configuration (Reference)
The build system links against `simlib` and `ncurses`.

```makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
LDFLAGS = -lsimlib -lm -lncurses

TARGET = simulation
SRCS = main.cpp model.cpp tui.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
```
