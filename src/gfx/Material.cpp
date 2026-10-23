#include "Material.hpp"

#include "texture/Texture2D.hpp"
#include "utils/types.hpp"
#include "utils/utils.hpp"

namespace gfx {

Material::Material(std::string name, bool flipVertically) {
  load(name, flipVertically);
}

void Material::load(std::string name, bool flipVertically) {
  namespace fs = std::filesystem;
  static const fspath rootPath = "res/tex/pbr";
  fspath folderPath = rootPath / name;

  if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
    TextureDescriptor descSRGB {.internalFormat = GL_SRGB8, .format = GL_RGB , .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
    TextureDescriptor descRGBA {.internalFormat = GL_RGBA8, .format = GL_RGBA, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};

    auto it = fs::directory_iterator(folderPath);
    if (it == fs::directory_iterator{})
      error("[Material::loadFrom] Directory ({}) is empty", folderPath.string());

    std::unordered_map<std::string, int> m = {
      {"albedo", 0},
      {"normal", 1},
      {"ORMH"  , 2},
    };

    for (const auto& entry : it) {
      fspath path = entry.path();

      auto it = m.find(path.stem());
      if (it != m.end())
        switch (it->second) {
          case 0:
            texAlbedo = Texture2D(image2D(path, {.flipVertically = flipVertically, .desiredChannels = 3}), descSRGB);
            break;
          case 1:
            texNormal = Texture2D(image2D(path, {.flipVertically = flipVertically, .desiredChannels = 3}), {});
            break;
          case 2:
            texORMH = Texture2D(image2D(path, {.flipVertically = flipVertically, .desiredChannels = 4}), descRGBA);
            break;
        }
    }

    currFolder = folderPath;
  } else {
    error("[Material::loadFrom] Provided folder ({}) is invalid", folderPath.string());
  }
}

} // namespace gfx

