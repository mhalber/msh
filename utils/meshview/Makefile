OS_NAME := $(shell uname -s)

# Useful Directories
BIN_DIR = ../../bin/
OUT_DIR = out/
MSH_DIR = ../../
EXTERN_DIR = ../../extern/

# List of source files
EXTERN_SRCS = $(EXTERN_DIR)/glad/glad.c
EXTERN_OBJS = $(OUT_DIR)/glad.o

MESHVIEW_EXE = meshview
MESHVIEW_SRCS = meshview.c
MESHVIEW_OBJS = $(MESHVIEW_SRCS:%.c=$(OUT_DIR)%.o)

# Compile and link options
CC = gcc
ifeq ($(OS_NAME), CYGWIN_NT-10.0)
CC = /bin/x86_64-w64-mingw32-gcc.exe
endif
WARNINGS = -Wall
CFLAGS = -O3 -g0 -std=gnu11 -DNDEBUG -DMSH_NDEBUG -I/usr/local/include -I../../ -I$(EXTERN_DIR)  

# Libraries and Frameworks
ifeq ($(OS_NAME), Darwin)
FRAMEWORKS = -framework opengl
MESHVIEW_LIBS = -L/usr/local/lib/ -lglfw
endif

ifeq ($(OS_NAME), CYGWIN_NT-10.0)
GL_LIBS = -L/usr/local/lib -lglfw3 -lGL -lGLU -lX11 -lXi -lXrandr -lXinerama -lXcursor -lrt -lm -pthread -ldl -lm
MESHVIEW_LIBS = $(GL_LIBS)  -L/usr/local/lib/ -lglad
endif

# Make targets
all: clean gfx

gfx: $(MESHVIEW_OBJS) $(EXTERN_OBJS)
		$(CC) $(FRAMEWORKS) $(CFLAGS) $(MESHVIEW_OBJS) $(EXTERN_OBJS) $(MESHVIEW_LIBS) -o ${BIN_DIR}${MESHVIEW_EXE}

clean:
		rm -f $(OUT_DIR)*.o ${BIN_DIR}${MESHVIEW_EXE}

# Compile command
$(OUT_DIR)%.o: %.c 
		$(CC) $(WARNINGS) $(CFLAGS) -c $< -o $@

$(OUT_DIR)%.o: $(EXTERN_DIR)/glad/%.c
		$(CC) $(WARNINGS) $(CFLAGS) -c $< -o $@

# GNU Make: targets that don't build files
.PHONY: all clean
