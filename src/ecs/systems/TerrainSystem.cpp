#include "TerrainSystem.hpp"

#include "../components/CameraComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/TerrainComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TextureComponent.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../gfx/terrain/GenerationManager.hpp"
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

  MeshComponent meshComponent{
    .mesh = assetManager.getMesh("MeshPlane_Triangles128_Instancied"),
    .shader = assetManager.getShader("TerrainDraw")
  };

  TerrainComponent terrainComponent{};
  terrainComponent.planetRadius = planetRadius;
  terrainComponent.ubo.nodesData = gfx::BufferObject::createUniformBuffer(true);
  terrainComponent.ubo.nodesData.storage(nullptr, TERRAIN_MAX_NODES * sizeof(NodeData), GL_DYNAMIC_STORAGE_BIT);

  TextureComponent textureComponent{};
  textureComponent.textures.push_back(assetManager.getTexture("TerrainNodes"));

  registry.emplace<MeshComponent>(entity, meshComponent);
  registry.emplace<TerrainComponent>(entity, std::move(terrainComponent));
  registry.emplace<TransformComponent>(entity, TransformComponent{});
  registry.emplace<TextureComponent>(entity, textureComponent);
  registry.ctx().emplace<GenerationManager>(std::move(gm));
}

void update(entt::registry& registry) {
  auto& gm = registry.ctx().get<GenerationManager>();
  auto& profiler =  registry.ctx().get<ProfilerManager>();
  gm.update();

  [[maybe_unused]] core::Camera* activeCam = nullptr;
  vec3 activeCamPos{};
  for (auto entity : registry.view<CameraComponent, TransformComponent>()) {
    const auto& camComponent = registry.get<CameraComponent>(entity);
    if (camComponent.isActive) {
      const auto& transComponent = registry.get<TransformComponent>(entity);

      activeCam = camComponent.cam;
      activeCamPos = transComponent.pos;
      break;
    }
  }

  for (auto entity : registry.view<TerrainComponent>()) {
    auto& terrain = registry.get<TerrainComponent>(entity);
    std::stack<Quadnode*> activeNodes;
    terrain.heightScale = terrain.planetRadius * terrain.planetRadiusPercent;

    static ProfilerManager::Query queryQt("QuatreeComputePass");
    auto taskQt = profiler.startScopedTaskCpu("QuadtreePass");

    for (Quadnode& quadtree : terrain.quadtrees) {
      quadtree.newFrame(terrain.qtMaxDepth, terrain.qtSplitThreshold, terrain.planetRadius, activeCamPos);
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

void prerender(entt::registry& registry, core::Camera* activeCam, vec3 activeCamPos) {
  auto terrainView = registry.view<TerrainComponent, MeshComponent, TransformComponent>();
  for (auto entity : terrainView) {
    const auto& terrainComponent = registry.get<TerrainComponent>(entity);
    const auto& meshComponent = registry.get<MeshComponent>(entity);
    const auto& transComponent = registry.get<TransformComponent>(entity);

    terrainComponent.ubo.nodesData.updateSubData(terrainComponent.leafs.data(), terrainComponent.activeLeafs * sizeof(NodeData));
    terrainComponent.ubo.nodesData.bindBase(0);
    meshComponent.mesh->setInstanceCount(terrainComponent.activeLeafs);

    vec3 planetCameraOffset = transComponent.pos - activeCamPos;
    mat4 localView = activeCam->getLocalView(vec3(0.f));
    mat4 localTranslation = glm::translate(mat4(1.f), planetCameraOffset);

    meshComponent.shader->setUniform3f("u_planetCameraOffset", planetCameraOffset);
    meshComponent.shader->setUniform1f("u_planetRadius", terrainComponent.planetRadius);
    meshComponent.shader->setUniform1f("u_heightScale", terrainComponent.heightScale);
    meshComponent.shader->setUniform1f("u_heightScaleMesh", terrainComponent.heightScaleMesh);
    meshComponent.shader->setUniform1f("u_seaThreshold", terrainComponent.seaThreshold);
    meshComponent.shader->setUniform1f("u_sandThreshold", terrainComponent.sandThreshold);
    meshComponent.shader->setUniform1f("u_mountainThreshold", terrainComponent.mountainThreshold);
    meshComponent.shader->setUniformMatrix4f("u_localView", localView);
    meshComponent.shader->setUniformMatrix4f("u_localViewInv", glm::inverse(localView));
    meshComponent.shader->setUniformMatrix4f("u_localTranslation", localTranslation);
  }
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

