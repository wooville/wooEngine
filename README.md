# wooEngine
Custom ECS game engine built from scratch using C++, SDL, Lua, design based on this wonderful course by Gustavo Pezzi:
https://pikuma.com/courses/cpp-2d-game-engine-development

## ECS Implementation
This engine uses a basic ECS implementation of Entity, Component, and System classes. Components and Systems use template types so that their functions can be used generically for each specific Component/System. A Registry class organizes the E/C/S in use. The Game class provides structure to the basic loop of the program.

Each Entity has a unique ID generated for it upon creation. If an Entity is killed, its ID will be reused as the next Entity is created. Entities can be tagged, grouped, compared by ID (using overloaded operators), or have Components added to them. To enable access to the tag, group, and Component management functionality of the Registry from an instance of the Entity class, a reference to the Entity's Registry is stored as a class variable. 

The Registry manages Systems and entities as well as their Components, tags, and groups. The Registry will synchronize the addition and removal of entities with the next frame by keeping a sets of entities to be added & removed. 

The Registry tracks Components using Pools. The Pool class is a wrapper on top of a vector which is used to identify which Components each Entity has enabled. In the spirit of data-oriented design, each Pool is a vector representing one Component's list of entities contiguously in memory, using hash maps to allow the user to index the data vector by the Entity ID as well as retrieve the ID of an Entity at a given index.

The Game class has three main functions that form the backbone of the program: Initialize(), Run(), and Destroy(). It also stores a pointer to the Registry, asset store, and Event bus.

Initialize() constructs the SDL window that the game runs in and creates the ImGui context.
Run() first runs a setup function that adds the engine's Systems to the Registry, creates lua bindings, and loads the current level with the Level Loader. It then processes user input, updates deltaTime/EventBus/Systems, and renders the current frame until the game stops running.
Destroy() destroys SDL's window and our ImGui renderer before quitting.

### Events & Event Bus
There are two existing Event classes: CollisionEvent and OnKeyPressedEvent. Each Event type has its own list of arguments. The EventBus handles the emitting of Events and subscription of Systems to different types of Events. When an Event is emitted through the Event Bus, any subscribed Systems will execute the callback function that is pointed to for that Event by that System in its SubscribeToEvents method. 

### Asset Store
The Asset Store uses SDL to create textures and fonts from a given filepath and emplaces them into maps with user-defined texture IDs as keys. These assets are rendered as sprites and text by RenderSystem and RenderTextSystem.

### Level Loader
The Level Loader uses Sol to read a formatted Lua script which defines a level, creating corresponding entities and assigning Components as defined in the script. Entire levels, tilemaps, and groups of entities with scripted behaviours can be written in Lua and interpreted by the Level Loader.

## Components
The a list of Components is shown below with their respective parameters:
#### Animation Component
int numFrames;
int currentFrame;
int frameSpeedRate;
bool isLoop;
int startTime;
#### BoxCollider Component
int width;
int height;
glm::vec2 offset;
#### CameraFollow Component
#### Health Component
int health_val;
#### KeyboardController Component
glm::vec2 upVelocity;
glm::vec2 rightVelocity;
glm::vec2 downVelocity;
glm::vec2 leftVelocity;
#### Projectile Component
bool isFriendly;
int hitDamage;
int duration;
int startTime;
#### ProjectileEmitter Component
glm::vec2 projectileVelocity;
int repeatFreq;
int projectileDuration;
int hitDamage;
bool isFriendly;
int lastEmissionTime;
#### RigidBody Component
glm::vec2 velocity;
#### Script Component
sol::function func;
#### Sprite Component
std::string assetId;
int width;
int height;
int zIndex;
SDL_RendererFlip flip;
bool isFixed;
SDL_Rect srcRect;
#### TextLabel Component
glm::vec2 position;
std::string text;
std::string assetId;
SDL_Color color;
bool isFixed;
#### Transform Component
glm::vec2 position;
glm::vec2 scale;
double rotation;

## Systems
### Animation System
#### Required Components: AnimationComponent, SpriteComponent
This System uses the framerate defined by each relevant Entity's AnimationComponent to update the currentFrame, then decides the new srcRect of its SpriteComponent based on the currentFrame.

### CameraMovement System
#### Required Components: TransformComponent, CameraFollowComponent
This System moves the camera to follow the relevant entities once they reach halfway across the screen in the x or y directions. It will also force the camera to remain within bounds.

### Collision System
#### Required Components: BoxColliderComponent, TransformComponent
This System checks for AABB collision on update and emits a CollisionEvent if it finds one.

### Damage System
#### Required Components: BoxColliderComponent
This System is subscribed to CollisionEvents and handles projectile collisions with players and enemies, reducing their HealthComponent health values.

### KeyboardControl System
#### Required Components: KeyboardControllerComponent, SpriteComponent, RigidBodyComponent
This System is subscribed to KeyPressedEvents, changing the Entity's velocity and spritesheet position based on the key pressed. Currently supports WASD and arrow keys.

### Movement System
#### Required Components: TransformComponent, RigidBodyComponent
This System handles the movement of relevant entities based on their velocity (scaled by deltaTime) and current position. Entities with the tag "player" are padded by 8px around the edges of the map. Entities that do not have the tag "player" and are found outside of the map are killed.

This System is also subscribed to CollisionEvents and handles collisions with obstacles. Enemies colliding with obstacles have their velocities and sprites flipped.

### ProjectileEmit System
#### Required Components: ProjectileEmitterComponent, TransformComponent
This System emits projectiles from relevant entities to the specification of their ProjectileEmitterComponent values.

This System is also subscribed to KeyPressedEvents, and emits projectiles for relevant "player" tagged entities on spacebar press in the direction that the player is facing.

### ProjectileLifecycle System
#### Required Components: ProjectileComponent
This System kills projectile entities after their duration has expired.

### RenderCollider System
#### Required Components: BoxColliderComponent, TransformComponent
This System renders a red rectangle around relevant entities illustrating their BoxCollider bounds. Only rendered in Debug mode (F1).

### RenderGUI System
#### Required Components: None
This System is not applied to entities, but is instead used to display an immediate-mode debug GUI using ImGui and SDL. This GUI currently only displays cursor coordinates and enemy spawner window. Only rendered in Debug mode (F1).

### RenderHealthBar System
#### Required Components: HealthComponent, TransformComponent, SpriteComponent
This System renders red rectangle and text for health bar display. Only rendered in Debug mode (F1).

### Render System
#### Required Components: TransformComponent, SpriteComponent
This System sorts and then renders relevant entities in Z-Index order using SDL. Entities outside of camera bounds are excluded from rendering. 

### RenderText System
#### Required Components: TextLabelComponent
This System renders text attached to relevant entities on the screen using SDL.

### Script System
#### Required Components: SpriteComponent
This System creates the bindings between the engine's C++ implementation and the game's Lua scripts using Sol and executes each script on update. The functions being exposed to Lua are defined in this System. Currently bound functions expose Entity position/velocity/rotation, as well as projectile velocity and animation frame.

