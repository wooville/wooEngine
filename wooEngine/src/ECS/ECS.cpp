#include "ECS.h"
#include "../Logger/Logger.h"
#include <string>

int IComponent::nextId = 0;

int Entity::GetId() const {
	return id;
}

void Entity::Kill() {
	registry->KillEntity(*this);
}

void System::AddEntityToSystem(Entity entity) {
	entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity) {
	entities.erase(std::remove_if(entities.begin(), entities.end(), [&entity](Entity other) {
		return entity == other;
		}), entities.end());
}

std::vector<Entity> System::GetSystemEntities() const {
	return entities;
}

int System::GetNumEntities() const {
	return entities.size();
}

const Signature& System::GetComponentSignature() const {
	return componentSignature;
}

Entity Registry::CreateEntity() {
	int entityId;
	
	if (freeIds.empty()) {
		entityId = numEntities++;

		if (entityId >= entityComponentSignatures.size()) {
			entityComponentSignatures.resize(entityId + 1);
		}
	}
	else {
		// reuse previously removed entity id
		entityId = freeIds.front();
		freeIds.pop_front();
	}

	Entity entity(entityId);
	entity.registry = this;
	entitiesToBeAdded.insert(entity);

	Logger::Log("Entity created with id = " + std::to_string(entityId));

	return entity;
}

void Registry::KillEntity(Entity entity) {
	entitiesToBeKilled.insert(entity);
}

void Registry::AddEntityToSystems(Entity entity) {
	const auto entityId = entity.GetId();
	const auto& entityComponentSignature = entityComponentSignatures[entityId];

	//check every system for one that matches signatures
	for (auto& system : systems) {
		const auto& systemComponentSignature = system.second->GetComponentSignature();

		bool componentsMatch = (entityComponentSignature & systemComponentSignature) == systemComponentSignature;	//check bitsets with bitwise AND

		if (componentsMatch) {
			system.second->AddEntityToSystem(entity);
		}
	}
}

void Registry::RemoveEntityFromSystems(Entity entity) {
	//check every system for one that matches signatures
	for (auto& system : systems) {
		system.second->RemoveEntityFromSystem(entity);
	}
}

void Registry::Update() {
	// Add entities that are waiting
	for (auto entity : entitiesToBeAdded) {
		AddEntityToSystems(entity);
	}
	entitiesToBeAdded.clear();

	//remove similarly
	for (auto entity : entitiesToBeKilled) {
		RemoveEntityFromSystems(entity);
		entityComponentSignatures[entity.GetId()].reset();

		freeIds.push_back(entity.GetId());
	}
	entitiesToBeKilled.clear();
}