# Define the compiler
CXX = clang++

# Define the compiler flags
CXXFLAGS = -Wall -Wextra -fsanitize=address -std=c++23

# Define the libraries to link
LIBS = -lwsock32 -lws2_32

# Define the source files
SRCS = upload.cpp

# Define the output executable
TARGET = upload

# Default rule to build the target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# Rule to clean the build directory
clean:
	del /f $(TARGET).exe

# Rule to rebuild the project
rebuild: clean all

.PHONY: all clean rebuild

#mingw32-make -f Makefile