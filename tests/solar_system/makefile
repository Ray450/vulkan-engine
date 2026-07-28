

# Detect OS
ifeq ($(OS),Windows_NT)

TARGET = main
CC = g++
CFLAGS = -std=c++17 -DDEBUG -D_USE_MATH_DEFINES -I../../external -I../../audio -I../../graphics -I"C:/glfw-3.4.bin.WIN64/include" -I"C:/VulkanSDK/1.3.280.0/Include"
LIBS = -L"C:/glfw-3.4.bin.WIN64/lib-mingw-w64" -L"C:/VulkanSDK/1.3.280.0/Lib" -lglfw3 -lglu32 -lgdi32 -lvulkan-1 -lopengl32

# Paths
MATH_FOLDER = ../../graphics/math
GRAPHICS_FOLDER = ../../graphics
VULKAN_FOLDER = ../../graphics/vulkan
PRIMITIVES_FOLDER = ../../graphics/primitives
AUDIO_FOLDER = ../../audio
SHADER_FOLDER = ../../shaders
SHADER_FOLDER2 = ../../shaders/compiled
VULKAN_SDK_BIN = C:/VulkanSDK/1.3.280.0/Bin

# Detect source files
SRC = main.cpp $(wildcard $(MATH_FOLDER)/*.cpp) $(wildcard $(GRAPHICS_FOLDER)/*.cpp) $(wildcard $(VULKAN_FOLDER)/*.cpp) $(wildcard $(PRIMITIVES_FOLDER)/*.cpp) $(wildcard $(AUDIO_FOLDER)/*.cpp)
OBJS = $(SRC:.cpp=.o)

# Detect GLSL shader files
VERT_SHADERS = $(wildcard $(SHADER_FOLDER)/*.vert)
FRAG_SHADERS = $(wildcard $(SHADER_FOLDER)/*.frag)
COMP_SHADERS = $(wildcard $(SHADER_FOLDER)/*.comp)

# Define SPIR-V outputs
VERT_SPV = $(patsubst $(SHADER_FOLDER)/%.vert,$(SHADER_FOLDER2)/%.vert.spv,$(VERT_SHADERS))
FRAG_SPV = $(patsubst $(SHADER_FOLDER)/%.frag,$(SHADER_FOLDER2)/%.frag.spv,$(FRAG_SHADERS))
COMP_SPV = $(patsubst $(SHADER_FOLDER)/%.comp,$(SHADER_FOLDER2)/%.comp.spv,$(COMP_SHADERS))

all: $(TARGET) $(VERT_SPV) $(FRAG_SPV) $(COMP_SPV)

# Compile C++ files
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

# Pattern rule for C++ compilation
%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

# Pattern rules for GLSL compilation
$(SHADER_FOLDER2)/%.vert.spv: $(SHADER_FOLDER)/%.vert
	mkdir "$(SHADER_FOLDER2)" 2>nul || exit 0
	$(VULKAN_SDK_BIN)/glslc.exe $< -o $@

$(SHADER_FOLDER2)/%.frag.spv: $(SHADER_FOLDER)/%.frag
	mkdir "$(SHADER_FOLDER2)" 2>nul || exit 0
	$(VULKAN_SDK_BIN)/glslc.exe $< -o $@

$(SHADER_FOLDER2)/%.comp.spv: $(SHADER_FOLDER)/%.comp
	mkdir "$(SHADER_FOLDER2)" 2>nul || exit 0
	$(VULKAN_SDK_BIN)/glslc.exe $< -o $@
run:
	./main

clean:
	del /Q *.exe *.o $(subst /,\,$(GRAPHICS_FOLDER))\*.o $(subst /,\,$(PRIMITIVES_FOLDER))\*.o $(subst /,\,$(AUDIO_FOLDER))\*.o $(subst /,\,$(VULKAN_FOLDER))\*.o $(subst /,\,$(MATH_FOLDER))\*.o $(subst /,\,$(SHADER_FOLDER2))\*.spv
else

TARGET = main
CC = g++
# Use -lGL and -lX... for general Linux linking, and standard -I for includes.
# The project likely needs standard GLFW includes, which are often installed
# to /usr/include/ or similar, so we remove the specific C: paths.
# We also use forward slashes for all paths, which is standard on Linux.
CFLAGS = -std=c++17 -DDEBUG -I../../external -I../../audio -I../../graphics
# The LIBS are simplified to use the standard library names on Linux.
LIBS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Paths - Use standard Linux forward slashes and assume system-installed glslc
MATH_FOLDER = ../../graphics/math
GRAPHICS_FOLDER = ../../graphics
VULKAN_FOLDER = ../../graphics/vulkan
PRIMITIVES_FOLDER = ../../graphics/primitives
AUDIO_FOLDER = ../../audio
SHADER_FOLDER = ../../shaders
SHADER_FOLDER2 = ../../shaders/compiled
# The glslc compiler is expected to be in the system's PATH
GLSLC_COMPILER = glslc

# Detect source files
SRC = main.cpp $(wildcard $(MATH_FOLDER)/*.cpp) $(wildcard $(GRAPHICS_FOLDER)/*.cpp) $(wildcard $(VULKAN_FOLDER)/*.cpp) $(wildcard $(PRIMITIVES_FOLDER)/*.cpp) $(wildcard $(AUDIO_FOLDER)/*.cpp)
OBJS = $(SRC:.cpp=.o)

# Detect GLSL shader files
VERT_SHADERS = $(wildcard $(SHADER_FOLDER)/*.vert)
FRAG_SHADERS = $(wildcard $(SHADER_FOLDER)/*.frag)
COMP_SHADERS = $(wildcard $(SHADER_FOLDER)/*.comp)

# Define SPIR-V outputs
VERT_SPV = $(patsubst $(SHADER_FOLDER)/%.vert,$(SHADER_FOLDER2)/%.vert.spv,$(VERT_SHADERS))
FRAG_SPV = $(patsubst $(SHADER_FOLDER)/%.frag,$(SHADER_FOLDER2)/%.frag.spv,$(FRAG_SHADERS))
COMP_SPV = $(patsubst $(SHADER_FOLDER)/%.comp,$(SHADER_FOLDER2)/%.comp.spv,$(COMP_SHADERS))

all: $(TARGET) $(VERT_SPV) $(FRAG_SPV) $(COMP_SPV)

# Compile C++ files
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

# Pattern rule for C++ compilation
%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

# Pattern rules for GLSL compilation
# Use the Linux glslc command
$(SHADER_FOLDER2)/%.vert.spv: $(SHADER_FOLDER)/%.vert
	mkdir -p $(SHADER_FOLDER2)
	$(GLSLC_COMPILER) $< -o $@

$(SHADER_FOLDER2)/%.frag.spv: $(SHADER_FOLDER)/%.frag
	mkdir -p $(SHADER_FOLDER2)
	$(GLSLC_COMPILER) $< -o $@

$(SHADER_FOLDER2)/%.comp.spv: $(SHADER_FOLDER)/%.comp
	mkdir -p $(SHADER_FOLDER2)
	$(GLSLC_COMPILER) $< -o $@

run:
	./main

# Use the Linux 'rm' command instead of Windows 'del /Q'
clean:
	rm -f $(OBJS)
	rm -f $(GRAPHICS_FOLDER)/*.o $(AUDIO_FOLDER)/*.o $(VULKAN_FOLDER)/*.o $(MATH_FOLDER)/*.o
	rm -rf $(SHADER_FOLDER2)
	rm -f $(TARGET)
endif