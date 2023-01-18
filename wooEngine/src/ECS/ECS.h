#pragma once

#include "../Logger/Logger.h"

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <deque>
#include <memory>

const unsigned int MAX_COMPONENTS = 32;

///////////////////////////////////////////////////
// Signature
///////////////////////////////////////////////////
// a bitset to keep track of which components an entity has
// and which entities a given system is interested in
///////////////////////////////////////////////////

typedef std::bitset<MAX_COMPONENTS> Signature;

// "interface component"
struct IComponent {
protected:
	static int nextId;
};

// used to assign a unique id to a component type
template <typename T>
class Component: public IComponent {
public:
	static int GetId() {
		static auto id = nextId++;
		return id;
	}
};

class Entity {
private:
	int id;
public:
	Entity(int id) : id(id) {};
	Entity(const Entity& entity) = default;
	int GetId() const;

	// manage tags and groups
	void Tag(const std::string& tag);
	bool HasTag(const std::string& tag) const;
	void Group(const std::string& group);
	bool BelongsToGroup(const std::string& group) const;

	Entity& operator =(const Entity& other) = default;
	bool operator ==(const Entity& other) const { return id == other.id; };
	bool operator !=(const Entity& other) const { return id != other.id; };
	bool operator >(const Entity& other) const { return id > other.id; };
	bool operator <(const Entity& other) const { return id < other.id; };
	bool operator >=(const Entity& other) const { return id >= other.id; };
	bool operator <=(const Entity& other) const { return id <= other.id; };

	template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
	template <typename TComponent> void RemoveComponent();
	template <typename TComponent> bool HasComponent() const;
	template <typename TComponent> TComponent& GetComponent() const;

	void Kill();

	// reference to entity's registry
	// used to allow function calls (kill, component management) on entity object instead of directly from registry
	class Registry* registry;
};

///////////////////////////////////////////////////
// System
///////////////////////////////////////////////////
// The system processes entities that contain a specific signature
///////////////////////////////////////////////////

class System {
private:
	// which components an entity must have for system to consider the entity
	Signature componentSignature;
	std::vector<Entity> entities;

public:
	System() = default;
	~System() = default;

	void AddEntityToSystem(Entity entity);
	void RemoveEntityFromSystem(Entity entity);
	std::vector<Entity> GetSystemEntities() const;
	int GetNumEntities() const;
	const Signature& GetComponentSignature() const;

	// defines which kinds of components entities must have to be considered by system
	template <typename TComponent> void RequireComponent();
};

///////////////////////////////////////////////////
// Pool
///////////////////////////////////////////////////
// A pool is a vector of objects of type T
///////////////////////////////////////////////////
class IPool {
public:
	virtual ~IPool() {}
	virtual void RemoveEntityFromPool(int entityId) = 0;
};

template <typename T>
class Pool: public IPool {
private:
	std::vector<T> data;
	int size;

	std::unordered_map<int, int> entityIdToIndex;
	std::unordered_map<int, int> indexToEntityId;

public:
	Pool(int capacity = 100) {
		size = 0;
		data.resize(capacity);
	}

	~Pool() = default;

	bool IsEmpty() const {
		return size == 0;
	}

	int GetSize() const {
		return size;
	}

	void Resize(int n) {
		data.resize(n);
	}

	void Clear() {
		data.clear();
		size = 0;
	}

	void Add(T object) {
		data.push_back(object);
	}

	void Set(int entityId, T object) {
		if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
			// if element exists already, replace it
			int index = entityIdToIndex[entityId];
			data[index] = object;
		}
		else {
			// keep track of entity id and its vector index
			int index = size;
			entityIdToIndex.emplace(entityId, index);
			indexToEntityId.emplace(index, entityId);

			//resize by doubling if necessary
			if (index >= data.capacity()) {
				data.resize(size * 2);
			}

			data[index] = object;
			size++;
		}
	}

	void Remove(int entityId) {
		// copy last element to deleted position for contiguity
		int indexOfRemoved = entityIdToIndex[entityId];
		int indexOfLast = size - 1;
		data[indexOfRemoved] = data[indexOfLast];

		// update index-entity maps
		int entityOfLastElement = indexToEntityId[indexOfLast];
		entityIdToIndex[entityOfLastElement] = indexOfRemoved;
		indexToEntityId[indexOfRemoved] = entityOfLastElement;

		entityIdToIndex.erase(entityId);
		indexToEntityId.erase(indexOfLast);

		size--;
	}

	void RemoveEntityFromPool(int entityId) override {
		if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
			Remove(entityId);
		}
	}

	T& Get(int entityId) {
		int index = entityIdToIndex[entityId];
		return static_cast<T&>(data[index]);
	}

	bool operator [](unsigned int index) {
		return data[index];
	}
};


///////////////////////////////////////////////////
// Registry
///////////////////////////////////////////////////
// The registry manages creation and destruction of entities, systems and components
///////////////////////////////////////////////////

class Registry {
private:
	int numEntities = 0;
	std::set<Entity> entitiesToBeAdded;
	std::set<Entity> entitiesToBeKilled;

	// Vector of component pools, each containing all of the data for a specific component type
	// vector index = component type id
	// pool index = entity id
	std::vector<std::shared_ptr<IPool>> componentPools;

	// signatures denote which components are enabled for each entity
	// vector index = entity id
	std::vector<Signature> entityComponentSignatures;

	// map of active systems
	// index = system type id
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

	// Entity tags (one per entity)
	std::unordered_map<std::string, Entity> entityPerTag;
	std::unordered_map<int, std::string> tagPerEntity;

	// Entity groups
	std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
	std::unordered_map<int, std::string> groupPerEntity;

	// list of available entity ids previously removed
	std::deque<int> freeIds;

public:
	Registry() {
		Logger::Log("Registry constructor called.");
	}
	~Registry() {
		Logger::Log("Registry destructor called.");
	}

	void Update();

	// entity management
	Entity CreateEntity();
	void KillEntity(Entity entity);
	
	// component management
	template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
	template <typename TComponent> void RemoveComponent(Entity entity);
	template <typename TComponent> bool HasComponent(Entity entity) const;
	template <typename TComponent> TComponent& GetComponent(Entity entity) const;

	//system management
	template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
	template <typename TSystem> void RemoveSystem();
	template <typename TSystem> bool HasSystem() const;
	template <typename TSystem> TSystem& GetSystem() const;

	void AddEntityToSystems(Entity entity);
	void RemoveEntityFromSystems(Entity entity);

	// tag management
	void TagEntity(Entity entity, const std::string& tag);
	bool EntityHasTag(Entity entity, const std::string& tag) const;
	Entity GetEntityByTag(const std::string& tag) const;
	void RemoveEntityTag(Entity entity);

	//group management
	void GroupEntity(Entity entity, const std::string& group);
	bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
	std::vector<Entity> GetEntitiesByGroup(const std::string& group) const;
	void RemoveEntityGroup(Entity entity);
	
};

template <typename TComponent>
void System::RequireComponent() {
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem>
void Registry::RemoveSystem() {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	systems.erase(system);
}

template <typename TSystem>
bool Registry::HasSystem() const {
	return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem>
TSystem& Registry::GetSystem() const {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	return *(std::static_pointer_cast<TSystem> (system->second));
}

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	// resize pools to accommodate new component pool
	if (componentId >= componentPools.size()) {
		componentPools.resize(componentId + 1, nullptr);
	}

	// create new component pool and add to list of pools
	if (!componentPools[componentId]) {
		std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
		componentPools[componentId] = newComponentPool;
	}

	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

	// create a new component and forward given arguments
	TComponent newComponent(std::forward<TArgs>(args)...);

	// add to component pool list for entity
	componentPool->Set(entityId, newComponent);

	// set component in corresponding signature
	entityComponentSignatures[entityId].set(componentId);

	Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id = " + std::to_string(entityId));
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	entityComponentSignatures[entityId].set(componentId, false);

	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	componentPool->remove(entityId);

	entityComponentSignatures[entityId].set(componentId, false);

	Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id = " + std::to_string(entityId));
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	return entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	auto componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	return componentPool->Get(entityId);
}




template <typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args) {
	registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template <typename TComponent>
void Entity::RemoveComponent() {
	registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const {
	return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const {
	return registry->GetComponent<TComponent>(*this);
}