#ifndef ECS_H
#define ECS_H

#include <bitset>
#include <vector>

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

	Entity& operator =(const Entity& other) = default;
	bool operator ==(const Entity& other) const { return id == other.id; };
	bool operator !=(const Entity& other) const { return id != other.id; };
	bool operator >(const Entity& other) const { return id > other.id; };
	bool operator <(const Entity& other) const { return id < other.id; };
	bool operator >=(const Entity& other) const { return id >= other.id; };
	bool operator <=(const Entity& other) const { return id <= other.id; };
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
	const Signature& GetComponentSignature() const;

	// defines which kinds of components entities must have to be considered by system
	template <typename TComponent> void RequireComponent();
};

class Registry {

};

template <typename TComponent>
void System::RequireComponent() {
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

#endif