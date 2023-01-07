#pragma once

#include "../Logger/Logger.h"
#include "Event.h"
#include <map>
#include <typeindex>
#include <memory>
#include <list>

//interface to abstract event callbacks of different types
class IEventCallback {
private:
	virtual void Call(Event& e) = 0;

public:
	virtual ~IEventCallback() = default;

	void Execute(Event& e) {
		Call(e);
	}
};

// wrapper for function ptr
template <typename TOwner, typename TEvent>
class EventCallback : public IEventCallback {
private:
	typedef void (TOwner::* CallbackFunction)(TEvent&);

	TOwner* ownerInstance;
	CallbackFunction callbackFunction;

	virtual void Call(Event& e) override {
		std::invoke(callbackFunction, ownerInstance, static_cast<TEvent&>(e));
	}

public:
	EventCallback(TOwner* ownerInstance, CallbackFunction callbackFunction) {
		this->ownerInstance = ownerInstance;
		this->callbackFunction = callbackFunction;
	}

	virtual ~EventCallback() override = default;
};

typedef std::list<std::unique_ptr<IEventCallback>> HandlerList;

class EventBus {
private:
	std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;

public:
	EventBus() {
		Logger::Log("EventBus constructor called.");
	}
	
	~EventBus() {
		Logger::Log("EventBus destructor called.");
	}

	// clears list of subscribers
	void Reset() {
		subscribers.clear();
	}

	///////////////////////////////////////////////
	// Subscribe to event type T
	// listener subscribes to event
	// ie: eventBus->SubscribeToEvent<CollisionEvent>(this, &Game::onCollision)
	///////////////////////////////////////////////
	template <typename TEvent, typename TOwner>
	void SubscribeToEvent(TOwner* ownerInstance, void (TOwner::* callbackFunction)(TEvent&)) {
		if (!subscribers[typeid(TEvent)].get()) {
			subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
		}

		auto subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(ownerInstance, callbackFunction);
		subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
	}

	///////////////////////////////////////////////
	// Emit event of type T
	// as soon as something emits an event, execute all listener callback functions
	// ie: eventBus->EmitEvent<CollisionEvent>(player, enemy);
	///////////////////////////////////////////////
	template <typename TEvent, typename ...TArgs>
	void EmitEvent(TArgs&& ...args) {
		auto handlers = subscribers[typeid(TEvent)].get();
		if (handlers) {
			TEvent event(std::forward<TArgs>(args)...);
			for (auto it = handlers->begin(); it != handlers->end(); it++) {
				auto handler = it->get();
				handler->Execute(event);
			}
		}
	}
};