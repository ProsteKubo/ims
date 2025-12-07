# Makefile for The Deadly Spiral IMS Project
# Pharmacokinetic/Pharmacodynamic Simulation using SIMLIB

CC = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -g
INCLUDES = -I/usr/local/include
LDFLAGS = -L/usr/local/lib
LIBS = -lsimlib -lm

# Source and target
SRC_DIR   = src
BUILD_DIR = build
TARGET    = sim
SOURCES   = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)
OBJECTS   = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Default target
all: $(TARGET)

# Build the simulation executable
$(TARGET): $(OBJECTS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	@echo ""
	@echo "==================================================================="
	@echo "  Build successful!"
	@echo "  Run with: ./$(TARGET)"
	@echo "==================================================================="

# Compile source files into build directory (mirrors src/ tree)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run the simulation
run: $(TARGET)
	@echo ""
	@echo "==================================================================="
	@echo "  Running simulation..."
	@echo "==================================================================="
	@echo ""
	LD_LIBRARY_PATH=/usr/local/lib:$$LD_LIBRARY_PATH ./$(TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build artifacts"

# Clean and rebuild
rebuild: clean all

# Display help
help:
	@echo "Available targets:"
	@echo "  make          - Build the simulation"
	@echo "  make run      - Build and run the simulation"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make rebuild  - Clean and rebuild"
	@echo "  make help     - Show this help message"

.PHONY: all run clean rebuild help
