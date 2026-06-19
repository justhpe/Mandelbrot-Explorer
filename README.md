# Mandelbrot-Explorer

## About

A real-time, high-performance fractal explorer powered by modern OpenGL and C++. This application offloads all heavy mathematical computations directly to the GPU using custom fragment shaders, allowing for smooth navigation and exploration of complex mathematical spaces.

![Mandelbrot Explorer](img/demo.png)

## Features

- **GPU-Accelerated Rendering:** Computes complex numbers per-pixel inside GLSL fragment shaders for maximum frame rates
- **Dual-Precision Modes:** Switch instantly between FP32 and FP64
- **Advanced Zooming & Navigation:**
  - **Scroll Zoom:** Zoom in and out precisely where your mouse cursor is pointing
  - **Box Selection:** Click and drag a gold box to instantly crop and zoom into a specific region
- **OS-Aware Scaling:** Automatic GUI and font scaling
- **Smooth Zoom Interpolation:** Implements linear interpolation (LERP) so transitions between zoom states look fluid rather than abrupt

## Roadmap

- [x] Gui
- [x] Dynamic scaling
- [x] Drawing mandelbrot set
- [x] Zoom
- [x] GPU rendering
- [x] Smooth transitions (at high res)
- [x] Burning ship
- [x] Color sets

## Requirements

### System Requirements
* **Operating System:** Windows 10 or Windows 11 (64-bit)
* **IDE (for building):** Visual Studio 2026 with C++ Desktop Development workload

### Hardware Requirements
- **Graphics Card (GPU):** Dedicated or integrated GPU with **OpenGL 4.6** support
- **FP64 Support:** A GPU that natively supports double-precision floating-point operations (required for the "Use FP64" deep-zoom mode)
  > *Note: Most modern NVIDIA and AMD cards support this perfectly. On some integrated graphics, the FP64 mode might experience lower frame rates*

## Usage

### Quick Start

If you just want to explore the fractals without building the project from scratch:
1. Go to the **Releases** section on the right side of this GitHub page.
2. Download the `windows_x64.zip`.
3. Extract the ZIP archive to a folder of your choice.
4. Run the `.exe` file. *(Note: If Windows Defender or your firewall prompts you, allow the application to run).*

### Compilation

The repository includes a pre-configured Visual Studio solution. To build it locally:
1. **Clone the repository:**
   ```bash
   git clone [https://github.com/justhpe/Mandelbrot-Explorer.git](https://github.com/justhpe/Mandelbrot-Explorer.git)
   cd Mandelbrot-Explorer