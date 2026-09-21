# 🍺 Beer Bottle Simulation (2D)

A real-time 2D beer bottle simulation written in C++ and OpenGL inspired by the new GTA6 trailer beer bottles. Renders a textured beer bottle with a translucent, animated liquid (including a wavy surface) and rising bubbles, all running in a simple GLFW window.

## Features

- **Textured bottle** loaded from a PNG using `stb_image`
- **Animated liquid** with a time-based sinusoidal wave on the top surface
- **Rising bubbles** simulated as soft circular points with per-bubble speed, size, and phase
- **Alpha blending** for the semi-transparent liquid and soft-edged bubbles
- **Custom GLSL shaders** for liquid colour, bottle texture, and bubble points
- **ESC to quit**

## Requirements

- **C++ compiler** (g++ or clang++)
- **OpenGL 3.3** capable GPU / driver
- **GLEW** — OpenGL extension loader
- **GLFW** — windowing and input
- **GLM** — math (matrix transforms, if used)
- **stb_image** — single-header image loader (system package or vendored)

### Installing dependencies

**Debian / Ubuntu:**
bash
sudo apt install build-essential libglew-dev libglfw3-dev libglm-dev libstb-dev

**Build**

g++ beer_bottle_simulation.cpp -o beer_bottle_simulation -lGL -lglfw -lGLEW

**Run**

./beer_bottle_simulation
