# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2 -std=c99 # Use C99 standard
CPPFLAGS = -Iinclude # Preprocessor flags (include paths)
LDFLAGS = -Llib     # Linker flags (library paths)
# Added -lglu32 needed for gluPerspective/gluLookAt/gluOrtho2D
LDLIBS = -lfreeglut -lglew32 -lopengl32 -lm -lglu32
WINDOWS_LINK_FLAGS = -mwindows # Suppress console window on Windows

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Files
TARGET = game.exe # Renamed executable slightly
# Use wildcard to find all .c files in src directory
SOURCES = $(wildcard $(SRC_DIR)/*.c)
# Automatically generate object file names from source file names
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Define the executable path
EXECUTABLE = $(BIN_DIR)/$(TARGET)

# Phony targets (targets that don't represent files)
.PHONY: all clean run directories help

# Default target: Build everything
all: directories $(EXECUTABLE)
	@echo "Build successful!"
	@echo "Executable: $(EXECUTABLE)"
	@echo "Remember to copy freeglut.dll and glew32.dll to the $(BIN_DIR) directory."

# Rule to create the executable by linking object files
$(EXECUTABLE): $(OBJECTS)
	@echo "Linking..."
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS) $(WINDOWS_LINK_FLAGS)

# Pattern rule to compile .c files into .o files in the OBJ_DIR
# $<: name of the first prerequisite (the .c file)
# $@: name of the target (the .o file)
# | $(OBJ_DIR): Order-only prerequisite - ensures OBJ_DIR exists before compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Rule to create the necessary output directories if they don't exist
# Using a phony target and order-only prerequisites for directories
directories: $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	@echo "Creating directory: $@"
	@mkdir $(OBJ_DIR) 2>nul || @echo Directory $(OBJ_DIR) already exists.

$(BIN_DIR):
	@echo "Creating directory: $@"
	@mkdir $(BIN_DIR) 2>nul || @echo Directory $(BIN_DIR) already exists.


# Target to clean up generated files (Windows cmd compatible)
clean:
	@echo "Cleaning up..."
	@if exist $(subst /,\,$(OBJ_DIR)) rmdir /s /q $(subst /,\,$(OBJ_DIR)) 2>nul || echo "$(OBJ_DIR) does not exist."
	@if exist $(subst /,\,$(BIN_DIR)) rmdir /s /q $(subst /,\,$(BIN_DIR)) 2>nul || echo "$(BIN_DIR) does not exist."
	@echo "Cleanup complete."


# Target to build and run the application
run: all
	@echo "Running $(EXECUTABLE)..."
	# Use start for Windows cmd/mingw to run in a separate window
	start "" "$(subst /,\,$(EXECUTABLE))"


# Help target (optional)
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  run      - Build and run the project"
	@echo "  clean    - Remove compiled object files and the executable"
	@echo "  help     - Show this help message"