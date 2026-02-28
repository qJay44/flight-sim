#include "TextureCubemap.hpp"

#include "utils/utils.hpp"

TextureCubemap::TextureCubemap(const fspath& folder, const TextureDescriptor& desc, bool async)
  : Texture(desc)
{
  if (desc.target != GL_TEXTURE_CUBE_MAP)
    error("[TextureCubemap::TextureCubemap] Wrong tartget for [{}]", desc.uniformName);

  if (async) {
    texFuture = std::async(std::launch::async, load, folder, desc.internalFormat);
  } else {
    const auto& data = load(folder, desc.internalFormat);
    upload(data);
    loaded = true;
  }
}

TextureCubemap::TextureCubemap(TextureCubemap&& other)
  : texFuture(std::move(other.texFuture)),
    loaded(other.loaded)
{
  Texture::operator=(std::move(other));
}

TextureCubemap& TextureCubemap::operator=(TextureCubemap&& other) {
  if (this != &other) {
    Texture::operator=(std::move(other));
    texFuture = std::move(other.texFuture);
    loaded = other.loaded;
  }

  return *this;
}

void TextureCubemap::update() {
  if (!loaded && texFuture.valid()) {
    auto status = texFuture.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
      upload(texFuture.get());
      loaded = true;
    }
  }
}

TextureCubemap::ImageData TextureCubemap::load(fspath folder, GLenum internalFormat) {
  namespace fs = std::filesystem;

  // NOTE: Order matters
  constexpr const char* texNames[6] = {
    "right",
    "left",
    "top",
    "bottom",
    "front",
    "back",
  };

  std::string extension;
  if (fs::exists(folder) && fs::is_directory(folder)) {
    auto it = fs::directory_iterator(folder);
    if (it != std::filesystem::end(it))
      extension = it->path().extension().string();
    else
      error("[TextureCubemap::load] Folder is empty [{}]", folder.string());
  } else {
    error("[TextureCubemap::load] Incorrect path [{}]", folder.string());
  }

  ImageData data;

  for (int i = 0; i < 6; i++) {
    fspath filePath = std::format("{}/{}{}", folder.string().c_str(), texNames[i], extension.c_str());
    data.images[i] = image2D(filePath, internalFormat);
  }

  return data;
}

void TextureCubemap::upload(const ImageData& data) {
  glGenTextures(1, &id);
  bind();
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_R, desc.wrapR);

  for (int i = 0; i < 6; i++) {
    const image2D& img = data.images[i];
    if (!img.pixels)
      error("[TextureCubemap::upload] Got no pixels to upload [{}]", desc.uniformName);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, desc.internalFormat, img.width, img.height, 0, desc.format, desc.type, img.pixels);
  }

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);

  unbind();
}

