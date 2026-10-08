#include "TerrainSystem.hpp"

#include "../components/TerrainComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TextureComponent.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/WaterComponent.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../gfx/mesh/frustum.hpp"
#include "../../gfx/terrain/GenerationManager.hpp"
#include "../../core/ActiveCamera.hpp"
#include "../../core/EngineContext.hpp"
#include "../../core/math/frustum/Frustum.hpp"
#include "ProfilerManager.hpp"

namespace ecs::TerrainSystem {

using namespace ecs::component;
using namespace gfx::terrain;
using namespace terrain;

namespace {

void createLandEntity(entt::registry& registry) {
  entt::entity entity = registry.create();

  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();

  // ----- Add to the asset manager ------------------------------------------------------------------------------------------------ //

  // NOTE: The mesh resolution is the same as [MESH_RESOLUTION]
  std::string meshName = assetManager.createMeshPlane_Triangles(128, "TerrainLand", true, true);

  // ----- Components -------------------------------------------------------------------------------------------------------------- //

  TerrainComponent terrainComponent{};
  terrainComponent.ubo.nodesData = gfx::BufferObject::createUniformBuffer(true);
  terrainComponent.ubo.nodesData.storage(nullptr, TERRAIN_MAX_NODES * sizeof(NodeData), GL_DYNAMIC_STORAGE_BIT);

  MeshComponent meshComponent{
    .mesh = assetManager.getMesh(meshName),
    .shader = assetManager.getShader("TerrainDrawLand")
  };

  TextureComponent textureComponent{};
  textureComponent.textures.push_back(assetManager.getTexture("TerrainNodes"));

  CameraComponent cameraComponent{
    .cam = assetManager.getCamera("Terrain"),
    .isDetached = true,
  };

  // NOTE: Just for camera movement //

  TransformComponent transComponent{
    .pos = activeCam.cam->position
  };

  VelocityComponent velComponent{
    .scale = 1e4f
  };

  InputComponent inputComponent{
    .shiftMultiplier = 10.f
  };

  ////////////////////////////////////

  // --------------------------------------------------------------------------------------------------------------------------------- //

  registry.emplace<MeshComponent>(entity, meshComponent);
  registry.emplace<TerrainComponent>(entity, std::move(terrainComponent));
  registry.emplace<TextureComponent>(entity, textureComponent);
  registry.emplace<CameraComponent>(entity, cameraComponent);

  registry.emplace<TransformComponent>(entity, transComponent);
  registry.emplace<VelocityComponent>(entity, velComponent);
  registry.emplace<InputComponent>(entity, inputComponent);
}

void createWaterEntity(entt::registry& registry) {
  entt::entity entity = registry.create();

  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();
  auto& gm = registry.ctx().get<GenerationManager>();
  auto& water = gm.getWater();

  // ----- Add to the asset manager ------------------------------------------------------------------------------------------------ //

  std::string meshName = assetManager.createMeshPlane_Triangles(256, "TerrainWater", true, true);

  // ----- Components -------------------------------------------------------------------------------------------------------------- //

  WaterComponent waterComponent{
    .foamSharpness = 1.f,
    .heightScaleScale = 0.03f,
    .quadsPerAxis = 4,
  };

  MeshComponent meshComponent{
    .mesh = assetManager.getMesh(meshName),
    .shader = assetManager.getShader("TerrainDrawWater")
  };

  TextureComponent textureComponent{
    .textures = {
      &water.getTexDisplacement(),
      &water.getTexDerivatives(),
      &water.getTexTurbulence()
    }
  };

  CameraComponent cameraComponent{
    .cam = assetManager.getCamera("Terrain"),
    .isDetached = true,
  };

  // NOTE: Just for camera movement //

  TransformComponent transComponent{
    .pos = activeCam.cam->position
  };

  VelocityComponent velComponent{
    .scale = 1e4f
  };

  InputComponent inputComponent{
    .shiftMultiplier = 10.f
  };

  ////////////////////////////////////

  // --------------------------------------------------------------------------------------------------------------------------------- //

  registry.emplace<MeshComponent>(entity, meshComponent);
  registry.emplace<WaterComponent>(entity, waterComponent);
  registry.emplace<TextureComponent>(entity, textureComponent);
  registry.emplace<CameraComponent>(entity, cameraComponent);

  registry.emplace<TransformComponent>(entity, transComponent);
  registry.emplace<VelocityComponent>(entity, velComponent);
  registry.emplace<InputComponent>(entity, inputComponent);
}

} // namespace

void init(entt::registry& registry) {
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  auto& profiler  = registry.ctx().get<ProfilerManager>();
  auto& renderer  = registry.ctx().get<gfx::Renderer>();
  auto gm = GenerationManager(&renderer, &profiler, assetManager);
  auto& terrainConfig = gm.getConfig();

  assetManager.addCamera("Terrain", {
    .farPlane = 1e6f,
    .position = {0.f, 0.f, terrainConfig.planetRadius + 25.f},
  });

  core::ActiveCamera activeCam{
    .cam = assetManager.getCamera("Terrain"),
  };

  assetManager.addShader("TerrainDrawLand", gfx::Shader("terrain/terrain.vert", "terrain/terrain.frag"));
  assetManager.addShader("TerrainDrawWater", gfx::Shader("terrain/water.vert", "terrain/water.frag"));

  assetManager.addMesh("TerrainFrustum", gfx::frustum::create(*assetManager.getCamera("Terrain")));

  assetManager.getShader("TerrainComputeHeight" )->setOnReloadCallback([&registry]() { reload(registry); });
  assetManager.getShader("TerrainComputeNormals")->setOnReloadCallback([&registry]() { reload(registry); });

  registry.ctx().emplace<GenerationManager>(std::move(gm));
  registry.ctx().insert_or_assign<core::ActiveCamera>(std::move(activeCam));

  createLandEntity(registry);
  createWaterEntity(registry);
}

void update(entt::registry& registry) {
  auto& gm = registry.ctx().get<GenerationManager>();
  auto& profiler  = registry.ctx().get<ProfilerManager>();
  auto& ctx = registry.ctx().get<core::EngineContext>();
  const auto& terrainConfig = gm.getConfig();
  gm.update(ctx.time, ctx.dt);

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
        .heightScale = terrainConfig.planetRadius * terrainConfig.planetRadiusPercent,
        .camPos = camComponent.cam->position,
        .frustum = &frustum,
      });

      quadtree.insert();
      quadtree.gatherLeafs(activeNodes);
      assert(activeNodes.size() < TERRAIN_MAX_NODES);

      while (!quadtree.removedNodeDatas.empty()) {
        const auto& data = quadtree.removedNodeDatas.top();
        gm.freeSlot(data.key, data.texLayerIdx);
        quadtree.removedNodeDatas.pop();
      }
    }

    taskQt.end();
    auto taskLeafsPush = profiler.startScopedTaskCpu("LeafsPush");

    terrain.activeLeafs = 0;
    std::stack<Quadnode*> nodesToGenerate;

    // Prepare nodes for UBO
    while (!activeNodes.empty()) {
      auto* node = activeNodes.top(); activeNodes.pop();
      if (!node->onFrustum)
        continue;

      // Defer nodes that need a new texture
      if (node->texLayerIdx == -1) {
        bool isCached = gm.isSlotCached(node->key);
        node->texLayerIdx = gm.acquireSlot(node->key);

        if (!isCached) {
          nodesToGenerate.push(node);
          continue;
        }
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
    gm.generateTexures(terrain.activeLeafs - nodesToGenerateIdxOffset, nodesToGenerateIdxOffset, terrain.ubo.nodesData);
  }
}

void render(entt::registry& registry, gfx::Renderer& renderer) {
  static ProfilerManager::Query queryRenderLand("TerrainRenderLand");
  static ProfilerManager::Query queryRenderWater("TerrainRenderWater");

  auto& profiler  = registry.ctx().get<ProfilerManager>();
  auto& gm = registry.ctx().get<GenerationManager>();
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  const auto& terrainConfig = gm.getConfig();
  auto terrainLandView = registry.view<TerrainComponent, MeshComponent, TextureComponent, CameraComponent>();
  auto terrainWaterView = registry.view<WaterComponent, MeshComponent, TextureComponent, CameraComponent>();

  // ----- Land -------------------------------------------------------------------------------------------------------------------- //

  for (auto entity : terrainLandView) {
    const auto& terrainComponent = registry.get<TerrainComponent>(entity);
    const auto& meshComponent = registry.get<MeshComponent>(entity);
    const auto& texComponent = registry.get<TextureComponent>(entity);
    const auto& camComponent = registry.get<CameraComponent>(entity);

    // if (meshComponent.disabled)
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

    {
      auto _task = profiler.startScopedTaskGpu(queryRenderLand);
      renderer.renderFrame();
    }

    // ----- Camera frustum ---------------------------------------------------------------------------------------------------------- //

    if (!terrainComponent.renderFrustum || camComponent.cam == activeCam.cam)
      continue;

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
    renderer.renderFrame();
  }

  // ----- Water ------------------------------------------------------------------------------------------------------------------- //

  for (auto entity : terrainWaterView) {
    const auto& waterComponent = registry.get<WaterComponent>(entity);
    const auto& meshComponent = registry.get<MeshComponent>(entity);
    const auto& texComponent = registry.get<TextureComponent>(entity);
    // const auto& camComponent = registry.get<CameraComponent>(entity);

    if (meshComponent.disabled)
      continue;

    vec3 planetCameraOffset = vec3(0.f) - activeCam.cam->position; // Planet always at the center (0,0,0)
    mat4 localView = activeCam.cam->getLocalView(vec3(0.f));
    mat4 localTranslation = glm::translate(mat4(1.f), planetCameraOffset);

    meshComponent.mesh->setInstanceCount(6 * waterComponent.quadsPerAxis * waterComponent.quadsPerAxis);

    meshComponent.shader->setUniformMatrix4f("u_proj", activeCam.cam->cachedProj);
    meshComponent.shader->setUniformMatrix4f("u_localView", localView);
    meshComponent.shader->setUniformMatrix4f("u_localTranslation", localTranslation);
    meshComponent.shader->setUniform3f("u_planetCameraOffset", planetCameraOffset);
    meshComponent.shader->setUniform3f("u_camPos", activeCam.cam->position);
    meshComponent.shader->setUniform1f("u_camFar", activeCam.cam->farPlane);
    meshComponent.shader->setUniform1f("u_planetRadius", terrainConfig.planetRadius);
    meshComponent.shader->setUniform1f("u_heightScale", terrainConfig.planetRadius * terrainConfig.planetRadiusPercent);
    meshComponent.shader->setUniform1f("u_heightScaleScale", waterComponent.heightScaleScale);
    meshComponent.shader->setUniform1f("u_foamSharpness", waterComponent.foamSharpness);
    meshComponent.shader->setUniform1i("u_quadsPerAxis", waterComponent.quadsPerAxis);

    gfx::Renderer::RenderCommand renderCmdWater{
      .shader = meshComponent.shader,
      .mesh = meshComponent.mesh,
      .enableCullFace = true,
      .enableDepthTest = true,
      .textures = texComponent.textures,
      .priority = 1,
    };

    renderer.submit(renderCmdWater);
  }

  {
    auto _task = profiler.startScopedTaskGpu(queryRenderWater);
    renderer.renderFrame();
  }

  // --------------------------------------------------------------------------------------------------------------------------------- //
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

