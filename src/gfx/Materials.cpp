#include "Materials.hpp"

#include "texture/image2D.hpp"
#include "utils/types.hpp"
#include "utils/utils.hpp"

namespace gfx {

Materials::Materials(ivec2 size, int count) : nextWriteSlot(count - 1) {
  TextureDescriptor descAlbedo {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_SRGB8, .format = GL_RGB , .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descNormal {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGB8 , .format = GL_RGB , .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descORMH   {.target = GL_TEXTURE_2D_ARRAY, .internalFormat = GL_RGBA8, .format = GL_RGBA, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
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

} // namespace gfx

