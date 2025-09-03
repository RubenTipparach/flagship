CC = gcc
CFLAGS = -Wall -Wno-missing-braces -Wunused-result -std=c99 -O2
INCLUDES = -Iraylib/src -Iinclude

# Detect platform
ifeq ($(OS),Windows_NT)
    # Windows libraries
    LIBS_GL = -Lraylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -lkernel32 -lshell32 -luser32
    LIBS_GLES = -Lraylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -lkernel32 -lshell32 -luser32
else
    # Linux/Unix libraries
    LIBS_GL = -Lraylib/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    LIBS_GLES = -Lraylib/src -lraylib -lGLESv2 -lEGL -lm -lpthread -ldl -lrt -lX11
endif

TARGET = fps_game
SOURCES = src/fps_game.c src/lighting.c src/mesh_generation.c src/mesh_generation_advanced.c src/rendering.c src/maze.c

# Default target
all: $(TARGET)

# Build with raylib dependency check
$(TARGET): $(SOURCES) raylib/src/libraylib.a
	@echo "Trying to build with OpenGL..."
	@$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCES) $(LIBS_GL) 2>/dev/null || \
	(echo "OpenGL failed, trying OpenGL ES..." && \
	 $(CC) $(CFLAGS) $(INCLUDES) -DGRAPHICS_API_OPENGL_ES2 -o $(TARGET) $(SOURCES) $(LIBS_GLES))

# Check if raylib exists, if not prompt to run setup
raylib/src/libraylib.a:
	@echo "Raylib not found. Please run './setup.sh' first to download and build raylib."
	@exit 1

# Force OpenGL ES build (for Raspberry Pi)
gles: $(SOURCES) raylib/src/libraylib.a
	$(CC) $(CFLAGS) $(INCLUDES) -DGRAPHICS_API_OPENGL_ES2 -o $(TARGET) $(SOURCES) $(LIBS_GLES)

# Clean build files
clean:
	rm -f $(TARGET)

# Build and run
run: $(TARGET)
	./$(TARGET)

# Setup raylib
setup:
	./setup.sh

.PHONY: all gles clean run setup