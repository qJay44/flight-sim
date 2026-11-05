#include "JetSystem.hpp"

#include "PhysicsSystem.hpp"
#include "../components/JetCoreComponent.hpp"
#include "../components/JetPartComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/CameraComponent.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../../gfx/fbx/model.hpp"
#include "../../gfx/AssetManager.hpp"
#include "../../gfx/Shader.hpp"
#include "../../gfx/Renderer.hpp"
#include "../../core/math/fjet/weights.hpp"
#include "../../core/ActiveCamera.hpp"
#include "../../core/EngineContext.hpp"
#include "ProfilerManager.hpp"
#include "global.hpp"

namespace ecs::system::JetSystem {

using namespace component;
using namespace core::math::fjet;

namespace {

entt::entity createCoreEntity(entt::registry& registry) {
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();
  auto entity = registry.create();

  JetCoreComponent jetCoreComponent{
  };

  PhysicsComponent phyComponent{
    .mass = weights::total,
    .inverseMass = weights::total == 0.f ? 0.f : 1.f / weights::total,
    .drag = 0.02f,
    .localInertia = { 471906.f, 684784.f, 212878.f }, // Calculate at runtime?
  };

  TransformComponent transformComponent{
    .position = {0.f, 0.f, 1e5f + 250.f}
  };

  CameraComponent camComponent{
    .cam = assetManager.getCamera("Jet"),
    .isDetached = false,
  };

  InputComponent inputComponent{
    .disabled = true,
    .shiftMultiplier = 10.f
  };

  registry.emplace<JetCoreComponent>(entity, std::move(jetCoreComponent));
  registry.emplace<PhysicsComponent>(entity, std::move(phyComponent));
  registry.emplace<TransformComponent>(entity, std::move(transformComponent));
  registry.emplace<CameraComponent>(entity, std::move(camComponent));
  registry.emplace<InputComponent>(entity, std::move(inputComponent));

  return entity;
}

entt::entity createPartEntity(entt::registry& registry, gfx::fbx::UfbxMesh& ufbxMesh, int part) {
  auto entity = registry.create();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();

  JetPartComponent jetPartComponent{
    .massWeight = weights::weights[part],
    .offset = ufbxMesh.offset,
    .localBoxMin = ufbxMesh.minPos,
    .localBoxMax = ufbxMesh.maxPos,
  };

  TransformComponent transformComponent{
    .position = {0.f, 0.f, 1e5f + 250.f}
  };

  PhysicsComponent phyComponent{
    .mass = weights::total * jetPartComponent.massWeight,
  };

  MeshComponent meshComponent{
    .mesh = assetManager.getMesh("FBX_" + ufbxMesh.name),
    .shader = assetManager.getShader("Jet"),
  };

  registry.emplace<JetPartComponent>(entity, std::move(jetPartComponent));
  registry.emplace<TransformComponent>(entity, std::move(transformComponent));
  registry.emplace<PhysicsComponent>(entity, std::move(phyComponent));
  registry.emplace<MeshComponent>(entity, std::move(meshComponent));

  return entity;
}

vec3 projectOnPlane(vec3 vec, vec3 normal) {
  vec3 n = glm::normalize(normal);
  float d = glm::dot(vec, n);

  return vec - (d * n);
}

float getLiftCoeff(float angleRad) {
  float a = glm::degrees(std::abs(angleRad));
  float cl = 0.f;

  if (a < 30.f)
    cl = sin(glm::radians(a * 3.f));
  else if (a < 90.f)
    cl = cos(glm::radians((a - 30.f) * 1.5f));

  return angleRad >= 0.f ? cl : -cl;
}

float getLiftCoeffYaw(float angleRad) {
  float a = glm::degrees(std::abs(angleRad));
  if (a > 90.f)
    return 0.f;

  float cl = 1.2f * sin(glm::radians(a * 2.5f));

  return angleRad >= 0.f ? cl : -cl;
}

void calcState(JetCoreComponent& core, PhysicsComponent& phy) {
  auto invRotation = glm::conjugate(phy.orientation);
  core.velocity = phy.velocity;
  core.localVelocity = invRotation * core.velocity;
  core.localAngularVelocity = invRotation * phy.angularVelocity;
}

void calcAngleOfAttack(JetCoreComponent& core) {
  if (glm::length2(core.localVelocity) < 0.1f) {
    core.angleOfAttack = 0.f;
    core.angleOfAttackYaw = 0.f;
    return;
  }

  core.angleOfAttack = atan2(-core.localVelocity.y, core.localVelocity.z);
  core.angleOfAttackYaw = atan2(core.localVelocity.x, core.localVelocity.z);
}

vec3 calcGForceLimit(JetCoreComponent& core, vec3 controlInput) {
  vec3 input = controlInput * core.gLimit;

  if (input.x < 0.f)
    input.x = controlInput.x * core.gLimitPitch;

  return input * 9.81f;
}

vec3 calcGForce(vec3 angularVelocity, vec3 vel) {
  return glm::cross(angularVelocity, vel);
}

float calcGLimitter(JetCoreComponent& core, vec3 maxAngularVelocity) {
  vec3 maxInput = global::normalizeSafe(core.controlInput);
  vec3 limit = calcGForceLimit(core, maxInput);
  vec3 maxGForce = calcGForce(maxInput * maxAngularVelocity, core.localVelocity);

  float maxGForceLen = glm::length(maxGForce);
  float limitLen = glm::length(limit);
  core.lastG = maxGForceLen / 9.81f;

  if (maxGForceLen > limitLen)
    return limitLen / maxGForceLen;

  return 1.f;
}

vec3 calcLift(JetCoreComponent& core, vec3 right, float liftPower, float liftCoeff) {
  vec3 liftVelocity = projectOnPlane(core.localVelocity, right);
  vec3 liftVelocityNorm = global::normalizeSafe(liftVelocity);
  float v2 = glm::length2(liftVelocity);

  float liftForce = v2 * liftCoeff * liftPower;
  vec3 liftDir = cross(liftVelocityNorm, right);
  vec3 lift = liftDir * liftForce;

  float dragForce = liftCoeff * liftCoeff * core.inducedDrag;
  vec3 dragDir = -liftVelocityNorm;
  vec3 finalInduceDrag = dragDir * v2 * dragForce;

  return lift + finalInduceDrag;
}

float calcSteering(float dt, float angularVelocity, float targetVelocity, float acc) {
  float err = targetVelocity - angularVelocity;
  float a = acc * dt;

  return glm::clamp(err, -a, a);
}

void updateThrust(JetCoreComponent& core, PhysicsComponent& phy) {
  PhysicsSystem::addRelativeForce(phy, core.throttle * core.maxThrust * core.localOrientation);
  core.throttle *= 0.9f;
}

void updateDrag(JetCoreComponent& core, PhysicsComponent& phy) {
  if (glm::length2(core.localVelocity) < 0.1f)
    return;

  float ad = core.airbrakeDeployed * core.airbrakeDrag;
  float fd = core.flapsDeployed * core.flapsDrag;
  float totalForwardCd = core.Cd_forward + ad + fd;

  vec3 lvAbs = abs(core.localVelocity);
  vec3 dragForceLocal{
    -core.localVelocity.x * lvAbs.x * core.Cd_side,
    -core.localVelocity.y * lvAbs.y * core.Cd_vertical,
    -core.localVelocity.z * lvAbs.z * totalForwardCd
  };

  PhysicsSystem::addRelativeForce(phy, dragForceLocal);
}

void updateLift(JetCoreComponent& core, PhysicsComponent& phy) {
  if (glm::length2(core.localVelocity) < 1.f)
    return;

  float flapsLiftPower = core.flapsDeployed * core.flapsLiftPower;
  float flapsAOABias = core.flapsDeployed * core.flapsAOABias;

  float flapsCoeff = getLiftCoeff(core.angleOfAttack + glm::radians(flapsAOABias));
  vec3 liftForce = calcLift(core, {1.f, 0.f, 0.f}, core.liftPower + flapsLiftPower, flapsCoeff);

  float rudderCoeff = getLiftCoeffYaw(core.angleOfAttackYaw);
  vec3 liftForceYaw = calcLift(core, {0.f, 1.f, 0.f}, core.rudderPower, rudderCoeff);

  PhysicsSystem::addRelativeForce(phy, liftForce);
  PhysicsSystem::addRelativeForce(phy, liftForceYaw);
}

void updateSteering(JetCoreComponent& core, PhysicsComponent& phy, float dt) {
  float speed = glm::max(0.f, core.localVelocity.z);
  float steeringPower = glm::clamp(speed / 20.f, 0.1f, 1.f);
  float gForceScaleing = calcGLimitter(core, glm::radians(core.turnSpeed) * steeringPower);

  vec3 targetAV = core.controlInput * core.turnSpeed * steeringPower * gForceScaleing;
  vec3 av = glm::degrees(core.localAngularVelocity);

  vec3 correction {
    calcSteering(dt, av.x, targetAV.x, core.turnAcceleration.x * steeringPower),
    calcSteering(dt, av.y, targetAV.y, core.turnAcceleration.y * steeringPower),
    calcSteering(dt, av.z, targetAV.z, core.turnAcceleration.z * steeringPower),
  };

  vec3 aeroAngularDrag = -core.localAngularVelocity * speed * 0.5f;

  PhysicsSystem::addRelativeTorqueInstantly(phy, glm::radians(correction));
  PhysicsSystem::addTorque(phy, aeroAngularDrag);
}

void updateTransforms(entt::registry& registry, JetCoreComponent& core, PhysicsComponent& phy, float dt) {
  using namespace weights;

  core.animFlaps.rotate(core.flapsDeployed * 2.f - 1.f, dt);
  core.animAirbrake.rotate(core.airbrakeDeployed * 2.f - 1.f, dt);
  core.animPitch.rotate(core.controlInput.x, dt);
  core.animYaw.rotate(core.controlInput.y, dt);
  core.animRoll.rotate(core.controlInput.z, dt);

  auto& leftFlap      = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_FLAP]);
  auto& airbrake      = registry.get<JetPartComponent>(core.partEntities[IDX_AIRBRAKE]);
  auto& leftElevator  = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_ELEVATOR]);
  auto& leftRudder    = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_FLAP]);
  auto& leftAileron   = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_AILERON]);
  auto& rightFlap     = registry.get<JetPartComponent>(core.partEntities[IDX_RIGHT_FLAP]);
  auto& rightElevator = registry.get<JetPartComponent>(core.partEntities[IDX_RIGHT_ELEVATOR]);
  auto& rightRudder   = registry.get<JetPartComponent>(core.partEntities[IDX_RIGHT_FLAP]);
  auto& rightAileron  = registry.get<JetPartComponent>(core.partEntities[IDX_RIGHT_AILERON]);

  leftFlap    .localRotation = rightFlap.localRotation = core.animFlaps.currentRotation;
  airbrake    .localRotation = core.animAirbrake.currentRotation;
  leftElevator.localRotation = rightElevator.localRotation = core.animPitch.currentRotation;
  leftRudder  .localRotation = rightRudder.localRotation = core.animYaw.currentRotation;

  auto q = core.animRoll.currentRotation;
  leftAileron.localRotation = q;
  rightAileron.localRotation = {q.w, -q.x, q.y, q.z};

  core.animPitch.rotateBack(core.controlInput.x, 0.1f);
  core.animRoll.rotateBack (core.controlInput.z, 0.1f);
  core.animYaw.rotateBack  (core.controlInput.y, 0.1f);

  mat4 bodyTransform = glm::mat4_cast(phy.orientation * core.initialRotation);
  bodyTransform *= glm::scale(mat4(1.f), vec3(core.meshScale));

  for (auto partEnt : core.partEntities) {
    auto& part = registry.get<JetPartComponent>(partEnt);

    mat4 partTranslation = glm::translate(mat4(1.f), part.offset);
    mat4 partRotation = glm::mat4_cast(part.localRotation);

    part.modelRelative = bodyTransform * partTranslation * partRotation;
  }
}

// NOTE: Applying before the PhysicsSystem update
void updateCamera(JetCoreComponent& core, PhysicsComponent& phy, TransformComponent& trans, CameraComponent& cam, float dt) {
  float lerpFactor = 1.f - glm::exp2(-core.camFollowSpeed * dt);

  vec3 jetForward = glm::normalize(phy.orientation * core.localOrientation);
  vec3 jetUp = glm::normalize(phy.orientation * vec3(0.f, 1.f, 0.f));

  vec3 camOffsetDir = glm::normalize(jetUp * 0.34202f + jetForward * -0.93969f);
  vec3 relativePos = camOffsetDir * core.camDistance;
  vec3 currPos = cam.cam->position;
  vec3 targetPos = trans.position + relativePos;
  vec3 nextPos = glm::mix(currPos, targetPos, lerpFactor);

  vec3 lookAtEye = currPos + jetForward * core.camLookAheadDistance;
  vec3 lookAtCenter = lookAtEye - nextPos;

  cam.cam->orientation = glm::normalize(lookAtCenter);
  cam.cam->position = nextPos;
}

} // namespace

void init(entt::registry& registry) {
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();

  {
    core::Camera jetCamera{};

    assetManager.addCamera("Jet", std::move(jetCamera));
    assetManager.addShader("Jet", gfx::Shader("jet/f15.vert", "jet/f15.frag"));
  }

  gfx::fbx::Model jetModel = gfx::fbx::load("res/fbx/f15.fbx");
  assert(jetModel.meshes.size() == 17);

  auto entCore = createCoreEntity(registry);
  auto& core = registry.get<JetCoreComponent>(entCore);

  for (int i = 0; i < 17; i++) {
    auto& ufbxMesh = jetModel.meshes[i];
    assetManager.addMesh("FBX_" + ufbxMesh.name, std::move(ufbxMesh.mesh));
    core.partEntities[i] = createPartEntity(registry, ufbxMesh, i);
  }

  core.initialRotation = glm::angleAxis(PI, vec3{0.f, 1.f, 0.f});

  float flapsMaxAngle = glm::radians(-20.f);
  core.animFlaps.maxAngleRad = 0.f;
  core.animFlaps.minAngleRad = flapsMaxAngle;
  core.animFlaps.rotSpeedRad = flapsMaxAngle * 2.f;
  core.animFlaps.rotAxis     = {-1.f, 0.f, 0.f};

  float airbrakeMaxAngle = glm::radians(20.f);
  core.animAirbrake.maxAngleRad = airbrakeMaxAngle;
  core.animAirbrake.minAngleRad = 0.f;
  core.animAirbrake.rotSpeedRad = airbrakeMaxAngle * 2.f;
  core.animAirbrake.rotAxis     = {-1.f, 0.f, 0.f};

  float pitchMaxAngle = glm::radians(20.f);
  core.animPitch.maxAngleRad = pitchMaxAngle;
  core.animPitch.minAngleRad = -pitchMaxAngle;
  core.animPitch.rotSpeedRad = pitchMaxAngle * 2.f;
  core.animPitch.rotAxis     = {-1.f, 0.f, 0.f};

  float rollMaxAngle = glm::radians(20.f);
  core.animRoll.maxAngleRad = rollMaxAngle;
  core.animRoll.minAngleRad = -rollMaxAngle;
  core.animRoll.rotSpeedRad = rollMaxAngle * 2.f;
  core.animRoll.rotAxis     = {0.906308f, 0.0f, -0.422618f}; // The rotated Left by 25 degrees around the Up

  float yawMaxAngle = glm::radians(20.f);
  core.animYaw.maxAngleRad = yawMaxAngle;
  core.animYaw.minAngleRad = -yawMaxAngle;
  core.animYaw.rotSpeedRad = yawMaxAngle * 2.f;
  core.animYaw.rotAxis     = {0.f, 1.f, 0.f};
}

void update(entt::registry& registry, float dt) {
  auto& profiler = registry.ctx().get<ProfilerManager>();
  auto _task = profiler.startScopedTaskCpu("JetUpdatePass");

  for (auto entity : registry.view<JetCoreComponent, PhysicsComponent, TransformComponent, CameraComponent>()) {
    auto& core = registry.get<JetCoreComponent>(entity);
    auto& phy = registry.get<PhysicsComponent>(entity);
    auto& trans = registry.get<TransformComponent>(entity);
    auto& cam = registry.get<CameraComponent>(entity);

    calcState(core, phy);
    calcAngleOfAttack(core);

    updateThrust(core, phy);
    updateDrag(core, phy);
    updateLift(core, phy);
    updateSteering(core, phy, dt);
    updateTransforms(registry, core, phy, dt);
    updateCamera(core, phy, trans, cam, dt);

    PhysicsSystem::addForce(phy, {0.f, -9.81f * phy.mass * 2.f, 0.f});
    core.controlInput *= 0.9f;
  }
}

void render(entt::registry& registry) {
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();
  auto& ctx = registry.ctx().get<core::EngineContext>();
  auto& renderer = registry.ctx().get<gfx::Renderer>();

  for (auto coreEnt : registry.view<JetCoreComponent, TransformComponent, CameraComponent>()) {
    auto& core = registry.get<JetCoreComponent>(coreEnt);
    auto& trans = registry.get<TransformComponent>(coreEnt);
    auto& cam = registry.get<CameraComponent>(coreEnt);

    vec3 eye = activeCam.cam == cam.cam ? core.camRelativePos : cam.cam->position - trans.position;

    mat4 worldTrans = glm::translate(mat4(1.f), trans.position);
    mat4 localView = cam.cam->getLocalView(eye);

    for (auto partEnt : core.partEntities) {
      auto& part = registry.get<JetPartComponent>(partEnt);
      auto& meshComponent = registry.get<MeshComponent>(partEnt);

      mat4 model = worldTrans * part.modelRelative;
      mat4 modelView = localView * part.modelRelative;

      meshComponent.shader->setUniformMatrix4f("u_model", model);
      meshComponent.shader->setUniformMatrix4f("u_modelView", modelView);
      meshComponent.shader->setUniform3f("u_camPos", activeCam.cam->position);
      meshComponent.shader->setUniform3f("u_color", part.color);
      meshComponent.shader->setUniform1f("u_time", ctx.time);

      renderer.submit(gfx::Renderer::RenderCommand{
        .shader = meshComponent.shader,
        .mesh = meshComponent.mesh,
      });
    }
  }

  renderer.renderFrame();
}

}; // namespace ecs::JetSystem

