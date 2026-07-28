# Vulkan Engine

A C/C++ Vulkan rendering engine, started from the [vulkan-tutorial.com](https://vulkan-tutorial.com) walkthrough and grown well past it into a custom renderer with its own graphics, audio, and (early) GPU compute layers.

## Layout

```
graphics/
  vulkan/       Vulkan backend: device, swapchain, pipeline, renderpass, sync,
                buffers, descriptors, images, and a compute path (vulkan_compute.*)
  primitives/   Higher-level drawables: polygon batches, textures, font atlas/batch,
  math/         Matrix math
  memory.h, logger.*
audio/          Audio backend, MIDI, and sound generation (built on miniaudio)
shaders/        GLSL sources (.vert/.frag/.comp); compiled SPIR-V goes in shaders/compiled/
external/       Vendored headers (stb_*, miniaudio)
fonts/          Ubuntu font family (text rendering)
assets/         Textures, OBJ models, planet/skybox images used by the test scenes
tests/
  testbed/      General scene/feature sandbox (2D/3D shapes, textures, models)
  solar_system/ A solar system scene (skybox, planets orbit/zoom camera)
```

There's no top-level build file yet — each folder under `tests/` has its own `makefile` that builds a standalone executable by pulling in the shared `graphics/`, `audio/`, and `math/` sources.

## Building

Each test target is self-contained. From inside a test folder (e.g. `tests/testbed/`):

```
make
make run
```

**Windows:** expects GLFW at `C:/glfw-3.4.bin.WIN64` and the Vulkan SDK at `C:/VulkanSDK/1.3.280.0`, with `glslc.exe` used to compile shaders (paths are hardcoded in the makefile — update them if your SDK/GLFW live elsewhere).

**Linux:** expects `glfw`, `vulkan`, and X11 dev libraries on the system, and `glslc` available on `PATH`.

`make` compiles sources and cross-compiles every `.vert`/`.frag`/`.comp` in `shaders/` to SPIR-V under `shaders/compiled/`. `make clean` removes build artifacts.
