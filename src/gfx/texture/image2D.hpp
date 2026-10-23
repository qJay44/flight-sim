#pragma once

namespace gfx {

struct image2D {
  enum LoadType {
    LOAD_NO,
    LOAD_STB,
    LOAD_STBF,
  };

  struct LoadDescriptor {
    bool flipVertically = false;
    LoadType loadType = LOAD_STB;
    int desiredChannels = 0;
  };

  static void info(fspath path, int& width, int& height, int& channels);
  static void write(fspath path, uvec2 size, u8 channels, byte* buf);

  image2D() = default;

  image2D(const image2D&) = delete;
  image2D& operator=(const image2D&) = delete;

  image2D(image2D&& other);
  image2D& operator=(image2D&& other);

  image2D(int width, int height);
  image2D(fspath path, const LoadDescriptor& desc = {false, LOAD_STB, 0});
  ~image2D();

  int width, height, channels;
  void* pixels = nullptr;

  fspath path{};

  void load(fspath path, const LoadDescriptor& desc = {false, LOAD_STB, 0});

  void clear();

private:
  GLenum loadType = LOAD_NO;

  void load_STB(const LoadDescriptor& desc);
  void load_STBF(const LoadDescriptor& desc);
};

} // namespace gfx

