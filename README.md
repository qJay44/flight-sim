(WIP)

https://github.com/user-attachments/assets/632c67c2-2253-4ce2-be3e-ca5d21ea6790

---

### 🚀 Prerequisites
* CMake 3.21
* C++23 compiler (GCC / Clang)
* OpenGL drivers (4.6, Core)

---

### 🛠️ Dependencies & Paths

#### System libs (configured via CMake)

* **GLFW3** -- Window creation and input handling
* **Freetype** -- Font rendering engine
* ufbx -- FBX file loader

#### Manual libs (configured via `.env.cmake`)

* 📦 **[GLAD](https://github.com/dav1dde/glad)** — OpenGL Loader Generator
* 📦 **[GLM](https://github.com/g-truc/glm)** — Optimized 3D Math Library for C++
* 📦 **[stb_image.h](https://github.com/nothings/stb)** — image loading
* 📦 **[ImGui](https://github.com/ocornut/imgui)** — GUI for adjusting variable values
* 📦 **[tiny_obj_loader](https://github.com/tinyobjloader/tinyobjloader)** — .obj file loader
* 📦 **[LegitProfiler](https://github.com/Raikiri/LegitProfiler)** — Profiler data on ImGui
* 📦 **[utils](https://github.com/qJay44/utils)** — My own utils (printing warings, errors)

---

## Sources
* https://www.youtube.com/watch?v=7vAHo2B1zLc
* https://www.jakobmaier.at/posts/flight-simulation/
* https://www.shadertoy.com/view/wXcfWn

---

## TODO
- [x] Chunk async load
- [ ] Fix shadows
- [ ] Rewrite entire Erosion generation to be based on world postion coordinates

