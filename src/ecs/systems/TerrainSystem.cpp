#include "TerrainSystem.hpp"

#include "../components/TerrainComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TextureComponent.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../gfx/terrain/GenerationManager.hpp"
#include "../../core/ActiveCamera.hpp"
#include "ProfilerManager.hpp"

namespace ecs::TerrainSystem {

using namespace ecs::component;
using namespace gfx::terrain;
using namespace terrain;

void init(entt::registry& registry, float planetRadius) {
  entt::entity entity = registry.create();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  auto gm = GenerationManager(assetManager, 254, TERRAIN_MAX_NODES);

  assetManager.addShader("TerrainDraw", gfx::Shader("terrain/terrain.vert", "terrain/terrain.frag"));
  assetManager.createMeshPlane_Triangles(128, true);

  TerrainComponent terrainComponent{};
  terrainComponent.planetRadius = planetRadius;
  terrainComponent.ubo.nodesData = gfx::BufferObject::createUniformBuffer(true);
  terrainComponent.ubo.nodesData.storage(nullptr, TERRAIN_MAX_NODES * sizeof(NodeData), GL_DYNAMIC_STORAGE_BIT);

  MeshComponent meshComponent{
    .mesh = assetManager.getMesh("MeshPlane_Triangles128_Instancied"),
    .shader = assetManager.getShader("TerrainDraw")
  };

  TextureComponent textureComponent{};
  textureComponent.textures.push_back(assetManager.getTexture("TerrainNodes"));

  registry.emplace<MeshComponent>(entity, meshComponent);
  registry.emplace<TerrainComponent>(entity, std::move(terrainComponent));
  registry.emplace<TextureComponent>(entity, textureComponent);
  registry.ctx().emplace<GenerationManager>(std::move(gm));
}

void update(entt::registry& registry) {
  auto& gm = registry.ctx().get<GenerationManager>();
  auto& profiler =  registry.ctx().get<ProfilerManager>();
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();
  gm.update();

  for (auto entity : registry.view<TerrainComponent>()) {
    auto& terrain = registry.get<TerrainComponent>(entity);
    std::stack<Quadnode*> activeNodes;
    terrain.heightScale = terrain.planetRadius * terrain.planetRadiusPercent;

    static ProfilerManager::Query queryQt("QuatreeComputePass");
    auto taskQt = profiler.startScopedTaskCpu("QuadtreePass");

    for (Quadnode& quadtree : terrain.quadtrees) {
      quadtree.newFrame(terrain.qtMaxDepth, terrain.qtSplitThreshold, terrain.planetRadius, activeCam.cam->position);
      quadtree.insert();
      quadtree.gatherLeafs(activeNodes);

      while (!quadtree.freedTexLayerIdxs.empty()) {
        gm.freeSlot(quadtree.freedTexLayerIdxs.top());
        quadtree.freedTexLayerIdxs.pop();
      }
    }

    taskQt.end();
    profiler.startScopedTaskGpu(queryQt);

    terrain.activeLeafs = 0;
    while (!activeNodes.empty()) {
      auto* node = activeNodes.top(); activeNodes.pop();

      auto& currLeaf = terrain.leafs[terrain.activeLeafs++];
      currLeaf = NodeData{
        .center = node->center,
        .extents = node->extents,
        .faceIdx = node->face,
        .texLayerIdx = node->texLayerIdx
      };

      if (node->texLayerIdx == -1) {
        currLeaf.texLayerIdx = node->texLayerIdx = gm.acquireSlot();
        gm.generateTerrain(currLeaf, terrain.planetRadius, terrain.heightScale);
      }
    }
  }
}

void render(entt::registry& registry, gfx::Renderer& renderer) {
  const auto& activeCam = registry.ctx().get<core::ActiveCamera>();
  auto terrainView = registry.view<TerrainComponent, MeshComponent, TextureComponent>();

  for (auto entity : terrainView) {
    const auto& terrainComponent = registry.get<TerrainComponent>(entity);
    const auto& meshComponent = registry.get<MeshComponent>(entity);
    const auto& texComponent = registry.get<TextureComponent>(entity);

    if (meshComponent.disabled)
      continue;

    gfx::Renderer::RenderCommand renderCmd{
      .shader = meshComponent.shader,
      .mesh = meshComponent.mesh,
      .textures = texComponent.textures
    };

    terrainComponent.ubo.nodesData.updateSubData(terrainComponent.leafs.data(), terrainComponent.activeLeafs * sizeof(NodeData));
    terrainComponent.ubo.nodesData.bindBase(0);
    meshComponent.mesh->setInstanceCount(terrainComponent.activeLeafs);

    vec3 planetCameraOffset = vec3(0.f) - activeCam.cam->position; // Planet always at the center (0,0,0)
    mat4 localView = activeCam.cam->getLocalView(vec3(0.f));
    mat4 localTranslation = glm::translate(mat4(1.f), planetCameraOffset);

    meshComponent.shader->setUniform1f("u_camFar", activeCam.cam->farPlane);

    meshComponent.shader->setUniformMatrix4f("u_proj", activeCam.cam->cachedProj);
    meshComponent.shader->setUniformMatrix4f("u_localView", localView);
    meshComponent.shader->setUniformMatrix4f("u_localViewInv", glm::inverse(localView));
    meshComponent.shader->setUniformMatrix4f("u_localTranslation", localTranslation);
    meshComponent.shader->setUniform3f("u_planetCameraOffset", planetCameraOffset);
    meshComponent.shader->setUniform1f("u_planetRadius", terrainComponent.planetRadius);
    meshComponent.shader->setUniform1f("u_heightScale", terrainComponent.heightScale);
    meshComponent.shader->setUniform1f("u_heightScaleMesh", terrainComponent.heightScaleMesh);
    meshComponent.shader->setUniform1f("u_seaThreshold", terrainComponent.seaThreshold);
    meshComponent.shader->setUniform1f("u_sandThreshold", terrainComponent.sandThreshold);
    meshComponent.shader->setUniform1f("u_mountainThreshold", terrainComponent.mountainThreshold);

    renderer.submit(std::move(renderCmd));
  }

  renderer.renderFrame();
}

void reload(entt::registry& registry) {
  auto& gm = registry.ctx().get<GenerationManager>();
  auto terrainView = registry.view<TerrainComponent, MeshComponent>();

  gm.freeSlotAll();

  for (auto entity : terrainView) {
    auto& terrainComponent = registry.get<TerrainComponent>(entity);

    terrainComponent.quadtrees[0] = {Quadnode::Right};
    terrainComponent.quadtrees[1] = {Quadnode::Left};
    terrainComponent.quadtrees[2] = {Quadnode::Top};
    terrainComponent.quadtrees[3] = {Quadnode::Bottom};
    terrainComponent.quadtrees[4] = {Quadnode::Front};
    terrainComponent.quadtrees[5] = {Quadnode::Back};
  }
}

} // namespace TerrainSystem

