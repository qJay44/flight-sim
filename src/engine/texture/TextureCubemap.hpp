#pragma once

#include "Texture.hpp"
#include "image2D.hpp"

class TextureCubemap : public Texture {
public:
  TextureCubemap() = default;
  TextureCubemap(const fspath& folder, const TextureDescriptor& desc, bool async = true);

  TextureCubemap(TextureCubemap&& other);

  TextureCubemap& operator=(TextureCubemap&& other);

  void update();

private:
  struct ImageData {
    image2D images[6];
  };

  std::future<ImageData> texFuture;
  bool loaded = false;

private:
  static ImageData load(fspath folder, GLenum internalFormat);

  void upload(const ImageData& data);
};

