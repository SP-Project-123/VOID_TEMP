# Compiler and configuration
CC = gcc
CFLAGS = -Wall -std=c99 -Wno-missing-braces

# Raylib paths (fallback to standard installation paths if not set as env variables)
RAYLIB_PATH ?= C:/raylib/raylib/src
RAYLIB_INCLUDE ?= $(RAYLIB_PATH)
RAYLIB_LIB ?= $(RAYLIB_PATH)

# Compile flags
INCLUDES = -Isrc/include -I$(RAYLIB_INCLUDE)
LDFLAGS = -L$(RAYLIB_LIB) -lraylib -lopengl32 -lgdi32 -lwinmm

# Source files
SRC = src/main.c \
      src/core/game.c \
      src/map/tilemap.c \
      src/player/player.c \
      src/zombie/zombie.c

# Target binary
TARGET = game.exe

# Default build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(INCLUDES) $(LDFLAGS)

# Clean built files
clean:
	@if exist $(TARGET) del /Q $(TARGET)

.PHONY: all clean
