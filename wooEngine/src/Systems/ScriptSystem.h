#pragma once

#include "../ECS/ECS.h"
#include "../Components/ScriptComponent.h"
#include "../Components/TransformComponent.h"

// native cpp functions for binding with lua
std::tuple<double, double> GetEntityPosition(Entity entity) {
	if (entity.HasComponent<TransformComponent>()) {
		const auto transform = entity.GetComponent<TransformComponent>();
		return std::make_tuple(transform.position.x, transform.position.y);
	}
	else {
		Logger::Err("Attempt to get position of entity with no transform component");
	}
}


std::tuple<double, double> GetEntityVelocity(Entity entity) {
	if (entity.HasComponent<RigidBodyComponent>()) {
		const auto rigidbody = entity.GetComponent<RigidBodyComponent>();
		return std::make_tuple(rigidbody.velocity.x, rigidbody.velocity.y);
	}
	else {
		Logger::Err("Attempt to get velocity of entity with no rigidbody component");
	}
}

void SetEntityPosition(Entity entity, double x, double y) {
	if (entity.HasComponent<TransformComponent>()) {
		auto& transform = entity.GetComponent<TransformComponent>();
		transform.position.x = x;
		transform.position.y = y;
	}
	else {
		Logger::Err("Attempt to set position of entity with no transform component");
	}
}

void SetEntityVelocity(Entity entity, double x, double y) {
	if (entity.HasComponent<RigidBodyComponent>()) {
		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
		rigidbody.velocity.x = x;
		rigidbody.velocity.y = y;
	}
	else {
		Logger::Err("Attempt to set velocity of entity with no rigidbody component");
	}
}

void SetEntityRotation(Entity entity, double angle) {
	if (entity.HasComponent<TransformComponent>()) {
		auto& transform = entity.GetComponent<TransformComponent>();
		transform.rotation = angle;
	}
	else {
		Logger::Err("Attempt to set rotation of entity with no transform component");
	}
}

void SetEntityAnimationFrame(Entity entity, int frame) {
	if (entity.HasComponent<AnimationComponent>()) {
		auto& animation = entity.GetComponent<AnimationComponent>();
		animation.currentFrame = frame;
	}
	else {
		Logger::Err("Attempt to set animation frame of aentity with no animation component");
	}
}

void SetProjectileVelocity(Entity entity, double x, double y) {
	if (entity.HasComponent<ProjectileEmitterComponent>()) {
		auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
		projectileEmitter.projectileVelocity.x = x;
		projectileEmitter.projectileVelocity.y = y;
	}
	else {
		Logger::Err("Attempt to set projectile velocity of entity with no projectile emitter component");
	}
}

class ScriptSystem : public System {
public:
	ScriptSystem() {
		RequireComponent<ScriptComponent>();
	}

	void CreateLuaBindings(sol::state& lua) {
		// create entity usertype for Lua
		lua.new_usertype<Entity>(
			"entity",
			"get_id", &Entity::GetId,
			"destroy", &Entity::Kill,
			"has_tag", &Entity::HasTag,
			"belongs_to_group", &Entity::BelongsToGroup
			);

		lua.set_function("set_position", SetEntityPosition);
		lua.set_function("get_position", GetEntityPosition);
		lua.set_function("set_velocity", SetEntityVelocity);
		lua.set_function("get_velocity", GetEntityVelocity);
		lua.set_function("set_rotation", SetEntityRotation);
		lua.set_function("set_projectile_velocity", SetProjectileVelocity);
		lua.set_function("set_animation_frame", SetEntityAnimationFrame);
	}

	void Update(double deltaTime, int elapsedTime) {
		// invoke lua function of each entity with script component
		for (auto entity: GetSystemEntities()) {
			const auto script = entity.GetComponent<ScriptComponent>();
			script.func(entity, deltaTime, elapsedTime); // invoke function
		}
	}
};