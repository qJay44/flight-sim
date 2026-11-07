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
  vec3 initPos = {0.f, 0.f, 1e6f + 1e5f};
  float totalMass = 13000.f;

  JetCoreComponent jetCoreComponent{
    .localOrientation = vec3(0.f, 0.f, 1.f),
    .camDistance = 100.f,
    .maxThrust = 6e5f,
    .stiffness = 500.f,
    .airbrakeDrag = 10.f,
    .flapsLiftPower = 2.f,
    .flapsAOABias = 10.f,
    .inducedDrag = 35.5f,
    .liftPower = 75.f,
    .rudderPower = 50.f,
    .gLimit = 20.f, // feels too heavy when low
    .gLimitPitch = -20.f, // same about this
    .turnSpeed = vec3(40.f, 20.f, 120.f),
    .turnAcceleration = vec3(5e5f, 2e5f, 1e6f),
  };

  PhysicsComponent phyComponent{
    .mass = totalMass,
    .inverseMass = totalMass == 0.f ? 0.f : 1.f / totalMass,
    .drag = 0.02f,
    .localInertia = { 471906.f, 684784.f, 212878.f }, // Calculate at runtime?
  };

  TransformComponent transformComponent{
    .position = initPos,
    .scale = vec3(0.05f),
  };

  CameraComponent camComponent{
    .cam = assetManager.getCamera("Jet"),
    .isDetached = false,
  };

  InputComponent inputComponent{
    .disabled = false,
    .shiftMultiplier = 10.f
  };

  camComponent.cam->position = initPos;

  registry.emplace<JetCoreComponent>(entity, std::move(jetCoreComponent));
  registry.emplace<PhysicsComponent>(entity, std::move(phyComponent));
  registry.emplace<TransformComponent>(entity, std::move(transformComponent));
  registry.emplace<CameraComponent>(entity, std::move(camComponent));
  registry.emplace<InputComponent>(entity, std::move(inputComponent));

  return entity;
}

entt::entity createPartEntity(entt::registry& registry, gfx::fbx::UfbxMesh& ufbxMesh, float weight) {
  auto entity = registry.create();
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();

  JetPartComponent jetPartComponent{
    .massWeight = weight,
    .offset = ufbxMesh.offset,
    .localBoxMin = ufbxMesh.minPos,
    .localBoxMax = ufbxMesh.maxPos,
  };

  TransformComponent transformComponent{
    .position = {0.f, 0.f, 1e6f + 1e5f}
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

void updateInput(JetCoreComponent& core, InputComponent& input, float dt) {
  float up        = input.keyboardKeys[GLFW_KEY_SPACE];
  float down      = input.keyboardKeys[GLFW_KEY_LEFT_CONTROL];
  float left      = input.keyboardKeys[GLFW_KEY_A];
  float right     = input.keyboardKeys[GLFW_KEY_D];
  float rollLeft  = input.keyboardKeys[GLFW_KEY_Q];
  float rollRight = input.keyboardKeys[GLFW_KEY_E];
  float forward   = input.keyboardKeys[GLFW_KEY_W];

  core.throttle = forward;
  core.controlInput.x += down + -up;
  core.controlInput.y += left + -right;
  core.controlInput.z += (rollRight + -rollLeft) * dt * 5.f;
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

void updateTransforms(entt::registry& registry, JetCoreComponent& core, PhysicsComponent& phy, TransformComponent& trans, float dt) {
  using namespace weights;

  core.animFlaps.rotate(core.flapsDeployed * 2.f - 1.f, dt);
  core.animAirbrake.rotate(core.airbrakeDeployed * 2.f - 1.f, dt);
  core.animPitch.rotate(core.controlInput.x, dt);
  core.animYaw.rotate(core.controlInput.y, dt);
  core.animRoll.rotate(core.controlInput.z, dt);

  auto& leftFlap      = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_FLAP]);
  auto& airbrake      = registry.get<JetPartComponent>(core.partEntities[IDX_AIRBRAKE]);
  auto& leftElevator  = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_ELEVATOR]);
  auto& leftRudder    = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_RUDDER]);
  auto& leftAileron   = registry.get<JetPartComponent>(core.partEntities[IDX_LEFT_AILERON]);
  auto& rightFlap     = registry.get<JetPartComponent>(core.partEntities[IDX_RIGHT_FLAP]);
  auto& rightElevator = registry.get<JetPartComponent>(core.partEntities[IDX_RIGHT_ELEVATOR]);
  auto& rightRudder   = registry.get<JetPartComponent>(core.partEntities[IDX_RIGHT_RUDDER]);
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
  bodyTransform *= glm::scale(mat4(1.f), trans.scale);

  for (auto partEnt : core.partEntities) {
    auto& part = registry.get<JetPartComponent>(partEnt);

    mat4 partTranslation = glm::translate(mat4(1.f), part.offset);
    mat4 partRotation = glm::mat4_cast(part.localRotation);

    part.modelRelative = bodyTransform * partTranslation * partRotation;
  }
}

// NOTE: Applying before the PhysicsSystem update
void updateCamera(JetCoreComponent& core, PhysicsComponent& phy, TransformComponent& trans, CameraComponent& cam, float aspect, float dt) {
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

  core.camRelativePos = relativePos;
  cam.cam->orientation = glm::normalize(lookAtCenter);
  cam.cam->orientation = jetForward;
  cam.cam->position = nextPos;
  cam.cam->up = jetUp;
  cam.cam->updateProj(aspect);
  cam.cam->cachedView = cam.cam->getLocalView(relativePos);
}

void updatePlanetForce(PhysicsComponent& phy, TransformComponent& trans) {
  constexpr vec3 planetPos{0.f};

  vec3 planetDir = glm::normalize(planetPos - trans.position);
  vec3 planetForce = planetDir * 9.81f * phy.mass;

  PhysicsSystem::addForce(phy, planetForce);
}

} // namespace

void init(entt::registry& registry) {
  auto& assetManager = registry.ctx().get<gfx::AssetManager>();

  {
    core::Camera jetCamera{
      .farPlane = 1e6f,
      .position = {0.f, 0.f, 1e5f + 25.f},
    };

    assetManager.addCamera("Jet", std::move(jetCamera));
    assetManager.addShader("Jet", gfx::Shader("jet/f15.vert", "jet/f15.frag"));
  }

  gfx::fbx::Model jetModel = gfx::fbx::load("res/fbx/f15.fbx");
  assert(jetModel.meshes.size() == 17);

  auto entCore = createCoreEntity(registry);
  auto& core = registry.get<JetCoreComponent>(entCore);
  auto& phy = registry.get<PhysicsComponent>(entCore);

  for (auto& ufbxMesh : jetModel.meshes) {
    auto partIdx = weights::strToPart.at(ufbxMesh.name);
    float weight = phy.mass * weights::weights[partIdx];
    assetManager.addMesh("FBX_" + ufbxMesh.name, std::move(ufbxMesh.mesh));
    core.partEntities[partIdx] = createPartEntity(registry, ufbxMesh, weight);
  }

  core.initialRotation = glm::angleAxis(PI, vec3{0.f, 1.f, 0.f});
  registry.get<JetPartComponent>(core.partEntities[weights::IDX_CANOPY]).color = vec3(1.f);

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
  auto& ctx = registry.ctx().get<core::EngineContext>();
  auto _task = profiler.startScopedTaskCpu("JetUpdatePass");
  float aspect = ctx.getAspectRatio_WidthOverHeight();

  for (auto entity : registry.view<JetCoreComponent, PhysicsComponent, TransformComponent, CameraComponent>()) {
    auto& core  = registry.get<JetCoreComponent>(entity);
    auto& phy   = registry.get<PhysicsComponent>(entity);
    auto& trans = registry.get<TransformComponent>(entity);
    auto& cam   = registry.get<CameraComponent>(entity);
    auto& input = registry.get<InputComponent>(entity);

    calcState(core, phy);
    calcAngleOfAttack(core);

    updateInput(core, input, dt);
    updateThrust(core, phy);
    updateDrag(core, phy);
    updateLift(core, phy);
    updateSteering(core, phy, dt);
    updateTransforms(registry, core, phy, trans, dt);
    updateCamera(core, phy, trans, cam, aspect, dt);
    updatePlanetForce(phy, trans);

    core.controlInput *= 0.9f;
  }
}

void render(entt::registry& registry) {
  auto& activeCam = registry.ctx().get<core::ActiveCamera>();
  auto& renderer = registry.ctx().get<gfx::Renderer>();

  for (auto coreEnt : registry.view<JetCoreComponent, TransformComponent, CameraComponent>()) {
    auto& core = registry.get<JetCoreComponent>(coreEnt);
    auto& trans = registry.get<TransformComponent>(coreEnt);
    auto& cam = registry.get<CameraComponent>(coreEnt);

    mat4 worldTrans = glm::translate(mat4(1.f), trans.position);
    mat4 localView = activeCam.cam == cam.cam ? cam.cam->cachedView : activeCam.cam->getLocalView(activeCam.cam->position - trans.position);

    for (auto partEnt : core.partEntities) {
      auto& part = registry.get<JetPartComponent>(partEnt);
      auto& meshComponent = registry.get<MeshComponent>(partEnt);

      mat4 model = worldTrans * part.modelRelative;
      mat4 modelView = localView * part.modelRelative;

      meshComponent.shader->setUniformMatrix4f("u_proj", activeCam.cam->cachedProj);
      meshComponent.shader->setUniformMatrix4f("u_model", model);
      meshComponent.shader->setUniformMatrix4f("u_modelView", modelView);
      meshComponent.shader->setUniform3f("u_camPos", activeCam.cam->position);
      meshComponent.shader->setUniform3f("u_color", part.color);
      meshComponent.shader->setUniform1f("u_camFar", activeCam.cam->farPlane);

      renderer.submit(gfx::Renderer::RenderCommand{
        .shader = meshComponent.shader,
        .mesh = meshComponent.mesh,
      });
    }
  }

  renderer.renderFrame();
}

}; // namespace ecs::JetSystem

