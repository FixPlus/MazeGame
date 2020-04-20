CC = g++
CFLAGS = -std=c++17 -I./external -I./external/glm -I./external/gli -I./base -O2
LDFLAGS = -L./libs/vulkan -lvulkan -L./base -lxcb

DEFS = -D VK_USE_PLATFORM_XCB_KHR

BASE_SOURCES =   base/vulkanexamplebase.cpp base/VulkanTools.cpp base/VulkanDebug.cpp base/VulkanUIOverlay.cpp external/imgui/imgui.cpp external/imgui/imgui_draw.cpp external/imgui/imgui_widgets.cpp external/imgui/imgui_demo.cpp


MAZE_DIRECTORY = Maze

MAZE_SOURCES = $(BASE_SOURCES) $(MAZE_DIRECTORY)/drawer.cpp $(MAZE_DIRECTORY)/VulkanExample.cpp $(MAZE_DIRECTORY)/Maze.cpp 

MAZE_OBJECTS = $(MAZE_SOURCES:.cpp=.o)

SHADER_DIR = ./shaders

SHADERS = $(SHADER_DIR)/triangle.vert $(SHADER_DIR)/triangle.frag $(SHADER_DIR)/uioverlay.vert $(SHADER_DIR)/uioverlay.frag
TEMP = $(SHADERS:.vert=.vert.spv)
COMP_SHADERS = $(TEMP:.frag=.frag.spv )

OBJECTS_TO_CLEAN = $(MAZE_OBJECTS) 

TEST_DIRECTORY = ./tests4

MAZE_EXEC = MazeGame

$(MAZE_EXEC): $(MAZE_OBJECTS)
	$(CC) $(CFLAGS) $(MAZE_OBJECTS)  -o $@ $(LDFLAGS) $(CFLAGS) $(DEFS)

include .depend

all: $(MAZE_EXEC) compile_shaders

test_gen: test_generator/test_generator.o
	$(CC) $(CFLAGS) test_generator/test_generator.o -o $@ $(LDFLAGS) $(DEFINES) $(DEFS)

test_analyzer: test_generator/test_analyzer.o
	$(CC) $(CFLAGS) test_generator/test_analyzer.o -o $@ $(LDFLAGS) $(DEFINES) $(DEFS)

.cpp.o:
	$(CC)  $(CFLAGS) -c -o $@ $< $(LDFLAGS) $(DEFINES) $(DEFS)

compile_shaders:
	./shaders/glslc ./shaders/triangle.vert -o ./shaders/triangle.vert.spv
	./shaders/glslc ./shaders/triangle.frag -o ./shaders/triangle.frag.spv
	./shaders/glslc ./shaders/uioverlay.frag -o ./shaders/uioverlay.frag.spv
	./shaders/glslc ./shaders/uioverlay.vert -o ./shaders/uioverlay.vert.spv


clean:
	rm -f $(OBJECTS_TO_CLEAN) *.o $(COMP_SHADERS) $(MAZE_EXEC)