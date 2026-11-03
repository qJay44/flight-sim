#include "Materials.hpp"

#include "texture/image2D.hpp"
#include "utils/types.hpp"
#include "utils/utils.hpp"

namespace gfx {

Materials::Materials(ivec2 size, int count) : nextWriteSlot(count - 1) {
  TextureDescriptor descGeneral{
    .target = GL_TEXTURE_2D_ARRAY,
    .minFilter = GL_LINEAR_MIPMAP_LINEAR,
    .wrapS = GL_REPEAT,
    .wrapT = GL_REPEAT,
  };

  TextureDescriptor descAlbedo = descGeneral;
  descAlbedo.internalFormat = GL_SRGB8;
  descAlbedo.format = GL_RGB;

  TextureDescriptor descNormal = descGeneral;
  descNormal.internalFormat = GL_RGB8;
  descNormal.format = GL_RGB;

  TextureDescriptor descORMH = descGeneral;
  descORMH.internalFormat = GL_RGBA8;
  descORMH.format = GL_RGBA;

  texAlbedo = Texture2DArray(count, size, descAlbedo);
  texNormal = Texture2DArray(count, size, descNormal);
  texORMH = Texture2DArray(count, size, descORMH);
}

Materials::Materials(int size, int count) : Materials(ivec2(size), count) {}

void Materials::add(const std::string& name) {
  namespace fs = std::filesystem;

  if (nextWriteSlot < 0)
    error("[Materials::add] Trying to add [{}] while there are no more slots", name);

  static const fspath rootPath = "res/tex/pbr";
  fspath folderPath = rootPath / name;

  if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
    auto it = fs::directory_iterator(folderPath);
    if (it == fs::directory_iterator{})
      error("[Materials::add] Directory ({}) is empty", folderPath.string());

    static std::unordered_map<std::string, int> m = {
      {"albedo", 0},
      {"normal", 1},
      {"ORMH"  , 2},
    };

    for (const auto& entry : it) {
      fspath path = entry.path();
      auto it = m.find(path.stem());

      if (it != m.end()) {
        switch (it->second) {
          case 0: {
            image2D img(path, {.desiredChannels = 3});
            texAlbedo.upload(nextWriteSlot, {img.width, img.height}, img.pixels, GL_RGB, GL_UNSIGNED_BYTE);
            break;
          }
          case 1: {
            image2D img(path, {.desiredChannels = 3});
            texNormal.upload(nextWriteSlot, {img.width, img.height}, img.pixels, GL_RGB, GL_UNSIGNED_BYTE);
            break;
          }
          case 2: {
            image2D img(path, {.desiredChannels = 4});
            texORMH.upload(nextWriteSlot, {img.width, img.height}, img.pixels, GL_RGBA, GL_UNSIGNED_BYTE);
            break;
          }
        }
      }
    }

    nextWriteSlot--;
  } else {
    error("[Materials::add] Provided folder ({}) is invalid", folderPath.string());
  }
}

void Materials::generateMipmaps() {
  texAlbedo.bind(0);
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  texNormal.bind(0);
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  texORMH.bind(0);
  glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

} // namespace gfx

