# Makefile

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -Iinclude

# Linker flags
LDFLAGS = -LC:/raylib/w64devkit/x86_64-w64-mingw32/lib -lraylib -lgdi32 -lwinmm

# Source files
SRCS = $(wildcard *.c)

# Object files
OBJS = $(SRCS:.c=.o)

# Executable names (one for each source file)
TARGETS = $(SRCS:.c=.exe)

# Default target
all: $(TARGETS)

# Link object files to create the executables
%.exe: %.o
	$(CC) $< -o $@ $(LDFLAGS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGETS)

# Run the program
run: $(TARGET)
	IF EXIST $(TARGET).exe $(TARGET).exe
