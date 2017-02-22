
# Useful Directories
BIN_DIR = ../bin/
OUT_DIR = out/

# List of source files
ARGPARSE_EXE = msh_argparse_example
ARGPARSE_SRCS = msh_argparse_example.c
ARGPARSE_OBJS = $(ARGPARSE_SRCS:%.c=$(OUT_DIR)%.o)

GFX_EXE = msh_gfx_example
GFX_SRCS = msh_gfx_example.c
GFX_OBJS = $(GFX_SRCS:%.c=$(OUT_DIR)%.o)

CAM_EXE = msh_cam_example
CAM_SRCS = msh_cam_example.c
CAM_OBJS = $(CAM_SRCS:%.c=$(OUT_DIR)%.o)


# Compile and link options
CC = clang++
WARNINGS = -Wall -Wextra 
CFLAGS = -O0 -ggdb -std=c++11 -I../ -I/usr/local/include/ 

# Libraries and Frameworks
FRAMEWORKS = -framework opengl
GFX_LIBS = -lglfw3
CAM_LIBS = -lglfw3

# Make targets
all: clean argparse gfx cam

argparse: $(ARGPARSE_OBJS)
		time $(CC) $(FRAMEWORKS) $(CFLAGS) $(ARGPARSE_OBJS) -o ${BIN_DIR}${ARGPARSE_EXE}

gfx: $(GFX_OBJS)
		time $(CC) $(FRAMEWORKS) $(CFLAGS) $(GFX_OBJS) $(GFX_LIBS) -o ${BIN_DIR}${GFX_EXE}

cam: $(CAM_OBJS)
		time $(CC) $(FRAMEWORKS) $(CFLAGS) $(CAM_OBJS) $(CAM_LIBS) -o ${BIN_DIR}${CAM_EXE}


clean:
		rm -f $(OUT_DIR)*.o ${BIN_DIR}${ARGPARSE_EXE} ${BIN_DIR}${GFX_EXE} ${BIN_DIR}${CAM_EXE}


# Compile command
$(OUT_DIR)%.o: %.c
		time $(CC) $(WARNINGS) $(CFLAGS) -c $< -o $@

# GNU Make: targets that don't build files
.PHONY: all clean
