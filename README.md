# PathTracer

A GPU path tracer written in C++17 using OpenGL compute shaders

> [!NOTE]
> This project is still in active development.

<img width="1600" height="900" alt="RandomTest361" src="https://github.com/user-attachments/assets/fc8ac859-8f64-41e2-828d-aff3e94e1280" />
<img width="1600" height="900" alt="RandomTest2416" src="https://github.com/user-attachments/assets/7cba823e-7098-4c83-84a9-3e5044a97319" />
<img width="1920" height="1080" alt="RandomTest11210" src="https://github.com/user-attachments/assets/deed2b58-c81d-4aea-badd-7c79f7552f12" />

## Features

- OBJ model loading
- BVH-accelerated ray traversal
- Reflective, metallic, emissive, and transparent materials
- HDR skyboxes
- Depth of field
- Progressive sample accumulation
- ACES tone mapping and post-processing controls
- Rasterized preview for editing scenes

## Requirements

- Windows
- CMake 3.16 or newer
- A graphics card and driver supporting OpenGL 4.6

## Building

1. Clone or download this repository.
2. Open the repository folder in Visual Studio.
3. Allow Visual Studio to configure the CMake project.
4. Select `PathTracer.exe` as the startup target.
5. Build and run the project.

## Camera controls

- `W`, `A`, `S`, `D` — move
- `Shift` and `Space` — move vertically
- Middle mouse button — rotate the camera
- `V` — switch camera mode
- `C` — enable or disable camera movement
