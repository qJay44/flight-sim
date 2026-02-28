#pragma once

// NOTE: Provide more tif functions for non-standard formats when needed
#define IMAGE2D_LOAD_NO    0
#define IMAGE2D_LOAD_STB   1 // Fallback to standard formats
#define IMAGE2D_LOAD_R16I  GL_R16I
#define IMAGE2D_LOAD_R16UI GL_R16UI

struct image2D {
  static void write(const std::string& path, uvec2 size, u8 channels, byte* buf);

  image2D() = default;

  image2D(const image2D&) = delete;
  image2D& operator=(const image2D&) = delete;

  image2D(image2D&& other);
  image2D& operator=(image2D&& other);

  image2D(int width, int height);
  image2D(fspath path, GLenum loadType = IMAGE2D_LOAD_STB, bool flipVertically = false);
  ~image2D();

  int width, height, channels;
  void* pixels = nullptr;

  fspath path{};
  bool flipVertically = false;

  void load(fspath path, GLenum loadType = IMAGE2D_LOAD_STB, bool flipVertically = false);

  void clear();

private:
  GLenum loadType = IMAGE2D_LOAD_NO;

  void load_STB();
  void loadTif_R16I();
  void loadTif_R16UI();
};

