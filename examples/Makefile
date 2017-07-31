OS_NAME := $(shell uname -s)
# Useful Directories
BIN_DIR = ../bin/
OUT_DIR = out/
EXTERN_DIR = ../extern/

# List of source files
EXTERN_SRCS = $(EXTERN_DIR)/glad/glad.c
EXTERN_OBJS = $(OUT_DIR)/glad.o

ARGPARSE_EXE = msh_argparse_example
ARGPARSE_SRCS = msh_argparse_example.c
ARGPARSE_OBJS = $(ARGPARSE_SRCS:%.c=$(OUT_DIR)%.o)

GFX_EXE = msh_gfx_example
GFX_SRCS = msh_gfx_example.c
GFX_OBJS = $(GFX_SRCS:%.c=$(OUT_DIR)%.o)

CAM_EXE = msh_cam_example
CAM_SRCS = msh_cam_example.c
CAM_OBJS = $(CAM_SRCS:%.c=$(OUT_DIR)%.o)

MSH_EXE = msh_example
MSH_SRCS = msh_example.c
MSH_OBJS = $(MSH_SRCS:%.c=$(OUT_DIR)%.o)

DRAW_EXE = msh_draw_example
DRAW_SRCS = msh_draw_example.c
DRAW_OBJS = $(DRAW_SRCS:%.c=$(OUT_DIR)%.o)

# Compile and link options
CC = clang
ifeq ($(OS_NAME), Linux)
CC = gcc
endif
WARNINGS = -Wall -Wextra 
CFLAGS = -O0 -ggdb -std=gnu11 -I../ -I$(EXTERN_DIR) -I/usr/local/include/ 

# Libraries and Frameworks
ifeq ($(OS_NAME), Darwin)
FRAMEWORKS = -framework opengl
GFX_LIBS = -lglfw3
CAM_LIBS = -lglfw3
DRAW_LIBS = -lglfw3
endif

ifeq ($(OS_NAME), Linux)
GL_LIBS = -lglfw3 -lGL -lGLU -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl -lm
GFX_LIBS = $(GL_LIBS)  -L/usr/local/lib/ -lglad
CAM_LIBS = $(GL_LIBS) -L/usr/local/lib/ -lglad
DRAW_LIBS = $(GL_LIBS)
endif

# Make targets
all: clean draw

argparse: $(ARGPARSE_OBJS)
		$(CC) $(FRAMEWORKS) $(CFLAGS) $(ARGPARSE_OBJS) -o ${BIN_DIR}${ARGPARSE_EXE}

gfx: $(GFX_OBJS)
		$(CC) $(FRAMEWORKS) $(CFLAGS) $(GFX_OBJS) $(GFX_LIBS) -o ${BIN_DIR}${GFX_EXE}

cam: $(CAM_OBJS)
		$(CC) $(FRAMEWORKS) $(CFLAGS) $(CAM_OBJS) $(CAM_LIBS) -o ${BIN_DIR}${CAM_EXE}
		
msh: $(MSH_OBJS)
		$(CC) $(FRAMEWORKS) $(CFLAGS) $(MSH_OBJS) $(MSH_LIBS) -o ${BIN_DIR}${MSH_EXE}

draw: $(DRAW_OBJS) $(EXTERN_OBJS)
		$(CC) $(FRAMEWORKS) $(CFLAGS) $(DRAW_OBJS) $(EXTERN_OBJS) $(DRAW_LIBS) -o ${BIN_DIR}${DRAW_EXE}



clean:
		rm -f $(OUT_DIR)*.o ${BIN_DIR}${ARGPARSE_EXE} ${BIN_DIR}${GFX_EXE} ${BIN_DIR}${CAM_EXE} ${BIN_DIR}${MSH_EXE}


# Compile command
$(OUT_DIR)%.o: %.c 
		$(CC) $(WARNINGS) $(CFLAGS) -c $< -o $@

$(OUT_DIR)%.o: ../extern/glad/%.c
		$(CC) $(WARNINGS) $(CFLAGS) -c $< -o $@

# GNU Make: targets that don't build files
.PHONY: all clean
