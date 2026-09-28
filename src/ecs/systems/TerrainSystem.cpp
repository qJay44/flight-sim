#include "TerrainSystem.hpp"

#include "../components/TerrainComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TextureComponent.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../gfx/mesh/frustum.hpp"
#include "../../gfx/terrain/GenerationManager.hpp"
#include "../../core/ActiveCamera.hpp"
#include "../../core/math/frustum/Frustum.hpp"
#include "ProfilerManager.hpp"

namespace ecs::TerrainSystem {

using namespace ecs::component;
using namespace gfx::terrain;
using namespace terrain;

void init(entt::registry& registry) {
  entt::entity entity = registry.create();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  auto gm = GenerationManager(assetManager, 160, TERRAIN_MAX_NODES);
  auto& terrainConfig = gm.getConfig();

  // ----- Add to the asset manager ------------------------------------------------------------------------------------------------ //

  assetManager.addCamera("Terrain", {
    .farPlane = 1e6f,
    .position = {0.f, 0.f, terrainConfig.planetRadius + 25.f},
  });

  assetManager.addShader("TerrainDraw", gfx::Shader("terrain/terrain.vert", "terrain/terrain.frag"));

  std::string meshName = assetManager.createMeshPlane_Triangles(128, true, true);
  assetManager.addMesh("TerrainFrustum", gfx::frustum::create(*assetManager.getCamera("Terrain")));

  assetManager.getShader("TerrainHeightCompute")->setOnReloadCallback([&registry]() { reload(registry); });

  // ----- Components -------------------------------------------------------------------------------------------------------------- //

  TerrainComponent terrainComponent{};
  terrainComponent.ubo.nodesData = gfx::BufferObject::createUniformBuffer(true);
  terrainComponent.ubo.nodesData.storage(nullptr, TERRAIN_MAX_NODES * sizeof(NodeData), GL_DYNAMIC_STORAGE_BIT);

  MeshComponent meshComponent{
    .mesh = assetManager.getMesh(meshName),
    .shader = assetManager.getShader("TerrainDraw")
  };
  meshComponent.shader->setUniform1i("u_baseVertexCount", 128 * 128);

  TextureComponent textureComponent{};
  textureComponent.textures.push_back(assetManager.getTexture("TerrainNodes"));

  CameraComponent cameraComponent{
    .cam = assetManager.getCamera("Terrain"),
    .isDetached = true,
  };

  // NOTE: Just for camera movement //

  TransformComponent transComponent{
    .pos = {0.f, 0.f, terrainConfig.planetRadius + 25.f}
  };

  VelocityComponent velComponent{
    .scale = 1e4f
  };

  InputComponent inputComponent{
    .shiftMultiplier = 10.f
  };

  ////////////////////////////////////

  // --------------------------------------------------------------------------------------------------------------------------------- //

  core::ActiveCamera activeCam{
    .cam = assetManager.getCamera("Terrain"),
  };

  registry.emplace<MeshComponent>(entity, meshComponent);
  registry.emplace<TerrainComponent>(entity, std::move(terrainComponent));
  registry.emplace<TextureComponent>(entity, textureComponent);
  registry.emplace<CameraComponent>(entity, cameraComponent);

  registry.emplace<TransformComponent>(entity, transComponent);
  registry.emplace<VelocityComponent>(entity, velComponent);
  registry.emplace<InputComponent>(entity, inputComponent);

  registry.ctx().emplace<GenerationManager>(std::move(gm));
  registry.ctx().insert_or_assign<core::ActiveCamera>(std::move(activeCam));
}

void update(entt::registry& registry) {
  auto& gm = registry.ctx().get<GenerationManager>();
  auto& profiler  = registry.ctx().get<ProfilerManager>();
  auto& renderer  = registry.ctx().get<gfx::Renderer>();
  const auto& terrainConfig = gm.getConfig();
  gm.update();

  for (auto entity : registry.view<TerrainComponent, CameraComponent>()) {
    auto& terrain = registry.get<TerrainComponent>(entity);
    auto& camComponent = registry.get<CameraComponent>(entity);
    auto frustum = core::math::frustum::Frustum(*camComponent.cam);
    std::stack<Quadnode*> activeNodes;

    auto taskQt = profiler.startScopedTaskCpu("QuadtreePass");

    for (Quadnode& quadtree : terrain.quadtrees) {
      quadtree.newFrame({
        .maxDepth = terrain.qtMaxDepth,
        .splitThreshold = terrain.qtSplitThreshold,
        .planetRadius = terrainConfig.planetRadius,
        .camPos = camComponent.cam->position,
        .frustum = &frustum,
      });

      quadtree.insert();
      quadtree.gatherLeafs(activeNodes);
      assert(activeNodes.size() < TERRAIN_MAX_NODES);

      while (!quadtree.freedTexLayerIdxs.empty()) {
        gm.freeSlot(quadtree.freedTexLayerIdxs.top());
        quadtree.freedTexLayerIdxs.pop();
      }
    }

    taskQt.end();

    terrain.activeLeafs = 0;
    std::stack<Quadnode*> nodesToGenerate;

    // Prepare nodes for UBO
    while (!activeNodes.empty()) {
      auto* node = activeNodes.top(); activeNodes.pop();

      // Defer nodes that need new textures
      if (node->texLayerIdx == -1) {
        node->texLayerIdx = gm.acquireSlot();
        nodesToGenerate.push(node);
        continue;
      }

      auto& currLeaf = terrain.leafs[terrain.activeLeafs++];
      currLeaf = NodeData{
        .center = node->center,
        .extents = node->extents,
        .faceIdx = node->face,
        .texLayerIdx = node->texLayerIdx,
      };
    }

    // Place deferred nodes after cached (with textures) nodes
    const size_t nodesToGenerateIdxOffset = terrain.activeLeafs;
    while (!nodesToGenerate.empty()) {
      auto* node = nodesToGenerate.top(); nodesToGenerate.pop();

      auto& currLeaf = terrain.leafs[terrain.activeLeafs++];
      currLeaf = NodeData{
        .center = node->center,
        .extents = node->extents,
        .faceIdx = node->face,
        .texLayerIdx = node->texLayerIdx
      };
    }

    terrain.ubo.nodesData.updateSubData(terrain.leafs.data(), terrain.activeLeafs * sizeof(NodeData));

    static ProfilerManager::Query queryQt("QuatreeComputePass");
    auto _taskQtCS = profiler.startScopedTaskGpu(queryQt);

    // Generate textures for deferred nodes
    gm.generateTexures(terrain.activeLeafs - nodesToGenerateIdxOffset, nodesToGenerateIdxOffset, renderer, terrain.ubo.nodesData);
  }
}

void render(entt::registry& registry, gfx::Renderer& renderer) {
  auto& gm = registry.ctx().get<GenerationManager>();
  const auto& terrainConfig = gm.getConfig();
  auto terrainView = registry.view<TerrainComponent, MeshComponent, TextureComponent, CameraComponent>();
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();

  for (auto entity : terrainView) {
    const auto& terrainComponent = registry.get<TerrainComponent>(entity);
    const auto& meshComponent = registry.get<MeshComponent>(entity);
    const auto& texComponent = registry.get<TextureComponent>(entity);
    const auto& camComponent = registry.get<CameraComponent>(entity);

    if (meshComponent.disabled)
      continue;

    gfx::Renderer::RenderCommand renderCmd{
      .shader = meshComponent.shader,
      .mesh = meshComponent.mesh,
      .enableCullFace = true,
      .enableDepthTest = true,
      .textures = texComponent.textures,
      .priority = 1,
    };

    terrainComponent.ubo.nodesData.bindBase(0);
    meshComponent.mesh->setInstanceCount(terrainComponent.activeLeafs);

    vec3 planetCameraOffset = vec3(0.f) - activeCam.cam->position; // Planet always at the center (0,0,0)
    mat4 localView = activeCam.cam->getLocalView(vec3(0.f));
    mat4 localTranslation = glm::translate(mat4(1.f), planetCameraOffset);

    meshComponent.shader->setUniformMatrix4f("u_proj", activeCam.cam->cachedProj);
    meshComponent.shader->setUniformMatrix4f("u_localView", localView);
    meshComponent.shader->setUniformMatrix4f("u_localTranslation", localTranslation);
    meshComponent.shader->setUniform3f("u_planetCameraOffset", planetCameraOffset);
    meshComponent.shader->setUniform1f("u_camFar", activeCam.cam->farPlane);
    meshComponent.shader->setUniform1f("u_planetRadius", terrainConfig.planetRadius);
    meshComponent.shader->setUniform1f("u_heightScale", terrainConfig.planetRadius * terrainConfig.planetRadiusPercent);
    meshComponent.shader->setUniform1f("u_seaThreshold", terrainComponent.seaThreshold);
    meshComponent.shader->setUniform1f("u_sandThreshold", terrainComponent.sandThreshold);
    meshComponent.shader->setUniform1f("u_mountainThreshold", terrainComponent.mountainThreshold);

    renderer.submit(renderCmd);

    if (!terrainComponent.renderFrustum || camComponent.cam == activeCam.cam)
      continue;

    auto& assetManager = registry.ctx().get<gfx::AssetManager>();
    auto* frustumShader = assetManager.getShader("FrustumDraw");
    auto* frustumMesh = assetManager.getMesh("TerrainFrustum");

    gfx::frustum::update(*frustumMesh, *camComponent.cam);

    localTranslation = glm::translate(mat4(1.f), camComponent.cam->position - activeCam.cam->position);

    frustumShader->setUniformMatrix4f("u_proj", activeCam.cam->cachedProj);
    frustumShader->setUniformMatrix4f("u_localView", localView);
    frustumShader->setUniformMatrix4f("u_localTranslation", localTranslation);
    frustumShader->setUniform3f("u_color", vec3(1.f));
    frustumShader->setUniform1f("u_camFar", activeCam.cam->farPlane);

    gfx::Renderer::RenderCommand frustumRenderCmd{
      .shader = frustumShader,
      .mesh = frustumMesh,
      .enableCullFace = false,
      .enableDepthTest = true,
      .priority = 2,
    };

    renderer.submit(frustumRenderCmd);
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

