## ecfw

### Overview
A header only C++17 entity component system implementation.

Entity component systems are object factories that
follow composition over inheritance which that allows
greater flexibility in defining objects. Every entity is 
composed of one or more components that each add unique behaviour.
At runtime, an entity can change its own behaviour by adding
or removing components.

Entity component systems are often combined with data-oriented
design techniques to enable efficient usage of CPU resources. 

### Learning objectives
- [Data oriented design techniques](https://en.wikipedia.org/wiki/Data-oriented_design)
- [Prototype pattern](https://en.wikipedia.org/wiki/Prototype_pattern)
- [Factory method pattern](https://en.wikipedia.org/wiki/Factory_method_pattern)
- [Template metaprogramming](https://en.wikipedia.org/wiki/Template_metaprogramming)

### Requirements
1. A C++17 compliant compiler
1. CMake 3.8 or later.

### Dependencies
[proto](https://github.com/N-A-D/proto)

### Installation
You can add **ecfw** in your root CMake project:
```CMake
add_subdirectory(extern/ecfw)
target_link_libraries(${PROJECT_NAME} ecfw)
```

### Usage

#### Entities
Entities are unsigned integers created from an `ecfw::entity_manager`.   
Two template parameters are required to create an `ecfw::entity_manager`:
1. An entity type (one of `std::uint16_t`, `std::uint32_t`, `std::uint64_t`)
2. A unqiue `ecfw::type_list` of component types.   

Example:
```cpp
using Entity = std::uint32_t;    
using CompList = ecfw::type_list<RigidBody, Render, AI>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;

Entity e = manager.create();
```

Each entity type defines an explicit maximum number of entities 
any entity manager can create. Each entity identifier contains two numbers, 
an index and a version. The index is used for component retrieval in a
background component database, whereas the version discriminates between
live/deceased entities.


| Entity Type | Max entities (also max index) | Max version |
| ----------- | --------------------------- | ---------- |
| std::uint16_t | 4095 (2 ^ 12 - 1) |  15 (2 ^ 4 - 1) |
| std::uint32_t | 1048575 (2 ^ 20 - 1) |  4095 (2 ^ 12 - 1) |
| std::uint64_t | 4294967295 (2 ^ 32 - 1) |  4294967295 (2 ^ 32 - 1) |

To create entities, invoke any one of the `ecfw::entity_manger::create` member functions.
Example:
```cpp
using Entity = std::uint32_t;    
using CompList = ecfw::type_list<RigidBody, Render, AI>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;

// Create a single entity with no components
auto e0 = manager.create();

// Create a single entity with a given list of components
// Any supplied types must be a subset of the types given 
// as a list to the entity_manager type
// Note also that each type must be default constructible
auto e1 = manager.create<RigidBody, Render, AI>();

// Create N many entities with no components
manager.create(10'000);

// Create N many entities with a given list of components
manager.create<RigidBody, Render, AI>(10'000);

std::vector<Entity> entities(10'000);

// Create entities with no components and store them in a container
manager.create(entities.begin(), entities.end());

// Create entities with a given list of components and store them in a container
manager.create<RigidBody, Render, AI>(entities.begin(), entities.end());
```

To clone an entity and its components, use any of the `ecfw::entity_manager::clone` member functions.
```cpp
using Entity = std::uint32_t;    
using CompList = ecfw::type_list<RigidBody, Render, AI>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;

auto prototype = manager.create<RigidBody, Render, AI>();

// Clone an existing entity
// To clone an entity, you must provide at least one component to copy from the base entity
auto clone0 = manager.clone<Render, AI>(prototype);

// Make N many clones of an entity
manager.clone<Render, AI>(prototype, 10'000);

// Make N many clones an store them in a container
manage.clone<Render, AI>(prototype, entities.begin(), entities.end());
```

For entity destruction, **ecfw** provides the `ecfw::entity_manager::destroy` members.
Example:
```cpp
using Entity = std::uint32_t;    
using CompList = ecfw::type_list<RigidBody, Render, AI>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;

auto e = manager.create();

// Destroy a single entity
manager.destroy(e);

std::vector<Entity> entities(10'000);

// Destroy a container of entities
manager.create(entities.begin(), entities.end());
manager.destroy(entities.begin(), entities.end());
```

To iterate over a set of entities that each possess a specific set of components, an `ecfw::entity_manager`
provides the `ecfw::entity_manager::entities_with` member function.
Example:
```cpp
using Entity = std::uint32_t;    
using CompList = ecfw::type_list<RigidBody, Render, AI>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;

manager.create<RigidBody, Render, AI>(10'000);

// Iterates over all entities with RigidBody, Render, and AI components
manager.entities_with<RigidBody, Render, AI>([](RigidBody& body, Render& render, AI& ai){ /* Logic */ });
```

#### Components
Each component type must provide a default constructor as well as a constructor that
properly initializes its member variables.   
Example:
```cpp
struct Position {
    Position()
        : x(0.f), y(0.f) {}
    Position(float x, float y)
        : x(x), y(y) {}
    float x;
    float y;
};
```

The default component storage implementation uses `std::vector`s for each
component type.

To provide an alternative storage container for a given component type,
you must provide a specialization of `ecfw::underlying_storage` with
a member type `type` denoting the storage container.   
Example:
```cpp
struct SomeFairlyUsedComponent {
    // Implementation
};

namespace ecfw {
    template <>
    struct underlying_storage<SomeFairlyUsedComponent> {
        using type = /*Your storage container*/;
    };
}
```

**NOTE:** Any alternative storage container must satisfy the following interface:
```cpp
class AlternativeStorage {
public:

    C& get(size_t);
    const C& get(size_t) const;

    bool empty() const;
    size_t size() const;
    void reserve(size_t);

    template <class... Args>
    C& construct(size_t, Args&&...);

    destroy(size_t);

    void clear();        
};
```

To assign components to an entity, use the `ecfw::entity_manager::assign` 
member.   
Example:
```cpp
struct Position {
    Position()
        : x(0.f), y(0.f) {}
    Position(float x, float y)
        : x(x), y(y) {}
    float x;
    float y;
};

using Entity = std::uint32_t;    
using CompList = ecfw::type_list<Position>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;
Entity e = manager.create();

manager.assign<Position>(e, (float)std::rand(), (float)std::rand());
```

To fetch an entity's component, use the `ecfw::entity_manager::component` 
member.   
Example:
```cpp
struct Position {
    Position()
        : x(0.f), y(0.f) {}
    Position(float x, float y)
        : x(x), y(y) {}
    float x;
    float y;
};

using Entity = std::uint32_t;    
using CompList = ecfw::type_list<Position>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;
Entity e = manager.create();

manager.assign<Position>(e, (float)std::rand(), (float)std::rand());

Position& position = manager.component<Position>(e);
```

To remove components from an entity, use the `ecfw::entity_manager::remove`
member.   
Example:
```cpp
struct Position {
    Position()
        : x(0.f), y(0.f) {}
    Position(float x, float y)
        : x(x), y(y) {}
    float x;
    float y;
};

using Entity = std::uint32_t;    
using CompList = ecfw::type_list<Position>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

EntityManager manager;
Entity e = manager.create();

manager.assign<Position>(e, (float)std::rand(), (float)std::rand());

manager.remove<Position>(e);
```

#### Events
**ecfw** provides four event categories:
1. `ecfw::entity_created`
2. `ecfw::entity_destroyed`
3. `ecfw::component_added`
4. `ecfw::component_removed`

Each `ecfw::entity_manager` instantiates each of the given component types
according to the entity type it was given as well as the type list of components
it was given.

To subscribe to an entity related event, call the `ecfw::entity_manager::events`
member on an existing entity_manager and then call the `ecfw::event_dispatcher`'s
`subscribe` function. 

You may subscribe to an event using a lambda or free function for which the 
`ecfw::event_dispatcher::subscribe` function will return an instance of 
`ecfw::event_subscription`. Or you can subscribe using a member function
of an existing object so long as it inherits from `ecfw::event_receiver`.

**NOTE** Entity event subscription does not allow for the collection of event
emissions.

Example:
```cpp
using Entity = std::uint32_t;    
using CompList = ecfw::type_list<RigidBody, Render, AI>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

void free_function0(const entity_create<Entity>& e) {}

struct SomeReceiver : ecfw::event_receiver {
    void function0(const component_added<Entity, AI>& e) {}
    void function1(const component_added<Entity, Render>& e) const {}
    void function2(const component_removed<Entity, AI>& e) {}
    void function3(const component_removed<Entity, Render>& e) const {}
};

EntityManager manager;

// The returned event_subcription object can be used to disconnect the connected funcion.
ecfw::event_subscription = manager.events().subscribe<entity_created<Entity>>(free_fucntion0);

SomeReceiver reciever;

manager.events().subscribe<component_added<Entity, AI>>(&receiver, &SomeReceiver::function0);
manager.events().subscribe<component_added<Entity, Render>>(&receiver, &SomeReceiver::function1);
manager.events().subscribe<component_removed<Entity, AI>>(&receiver, &SomeReceiver::function2);
manager.events().subscribe<component_removed<Entity, Render>>(&receiver, &SomeReceiver::function3);
```
