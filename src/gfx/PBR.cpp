#include "PBR.hpp"

#include "texture/Texture2D.hpp"
#include "utils/types.hpp"
#include "utils/utils.hpp"

namespace gfx {

namespace fs = std::filesystem;

void Material::load(fspath folderPath, bool flipVertically) {
  // using enum TextureIdx;

  if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
    TextureDescriptor descSRGBA{.internalFormat = GL_SRGB_ALPHA, .format = GL_RGBA, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
    TextureDescriptor descRGB  {                                                    .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
    TextureDescriptor descSRGB {.internalFormat = GL_SRGB8     ,                    .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
    TextureDescriptor descRed  {.internalFormat = GL_R8        , .format = GL_RED , .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};

    auto it = fs::directory_iterator(folderPath);
    if (it == fs::directory_iterator{})
      error("[Material::loadFrom] Directory ({}) is empty", folderPath.string());

    ivec2 res{-1};

    for (const auto& entry : it) {
      fspath path = entry.path();

      ivec2 info;
      int c;
      image2D::info(path, info.x, info.y, c);

      if (res.x != -1) {
        if (glm::any(glm::notEqual(res, info)))
          error("[Material::load] File [{}] has different resolution [{}] (expected {})", path.string(), info.x, info.y, res.x, res.y);
      } else {
        res = info;
      }
    }

    currFolder = folderPath;
  } else {
    error("[Material::loadFrom] Provided folder ({}) is invalid", folderPath.string());
  }
}

} // namespace gfx

