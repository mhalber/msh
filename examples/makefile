
# Useful Directories
BIN_DIR = ../bin/
OUT_DIR = out/

# List of source files
ARGPARSE_EXE = msh_argparse_example
ARGPARSE_SRCS = msh_argparse_example.c
ARGPARSE_OBJS = $(ARGPARSE_SRCS:%.c=$(OUT_DIR)%.o)

# Compile and link options
CC = clang
WARNINGS = -Wall -Wextra -Wpedantic
CFLAGS = -O0 -ggdb -std=gnu11 -I../ 

# Libraries and Frameworks
FRAMEWORKS = -framework opengl

# Make targets
all: clean argparse

argparse: $(ARGPARSE_OBJS)
		time $(CC) $(FRAMEWORKS) $(CFLAGS) $(ARGPARSE_OBJS) -o ${BIN_DIR}${ARGPARSE_EXE}

clean:
		rm -f $(OUT_DIR)*.o ${BIN_DIR}${ARGPARSE_EXE}


# Compile command
$(OUT_DIR)%.o: %.c
		time $(CC) $(WARNINGS) $(CFLAGS) -c $< -o $@

# GNU Make: targets that don't build files
.PHONY: all clean
