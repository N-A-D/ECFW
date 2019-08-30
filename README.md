## ecfw

`ecfw` is a header-only C++17 entity component system.

## ECS Overview

***E**ntity **C**omponent **S**ystem* is a software 
architecture primarily used in game development. It
follows *composition over inheritance* to provide
greater flexibility when defining objects.

For more information on entity component systems:   
- [Evolve Your Hierarchy](http://cowboyprogramming.com/2007/01/05/evolve-your-heirachy/)
- [Entities, components, and systems](https://medium.com/ingeniouslysimple/entities-components-and-systems-89c31464240d)
- [ECS on Wikipedia](https://en.wikipedia.org/wiki/Entity_component_system)

## Learning objectives
- [Composition over inheritance](https://en.wikipedia.org/wiki/Composition_over_inheritance)
- [Object decomposition](https://en.wikipedia.org/wiki/Decomposition_(computer_science))
- [Data oriented design](https://en.wikipedia.org/wiki/Data-oriented_design)
- Design patterns
    - [Prototype](https://en.wikipedia.org/wiki/Prototype_pattern)
    - [Observer](https://en.wikipedia.org/wiki/Observer_pattern)
    - [Object pool](https://en.wikipedia.org/wiki/Object_pool_pattern)

## Usage

### Entities
Entities are created from `ecfw::entity_manager`s which are template classes
that accept an entity type (one of: [`uint16_t`, `uint32_t`, `uint64_t`]),
and a type list of components that your game or simulation requires.

For example:
```cpp
using Entity = uint32_t;
using CompList = ecfw::type_list<Position, Direction, Render>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;
```

To construct entities you may invoke any one of the `ecfw::entity_manager::create`
functions.   
For example:
```cpp
using Entity = uint32_t;
using CompList = ecfw::type_list<Position, Direction, Render>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

int main() {
    EntityManager mgr;

    // Create a plain entity without any components
    Entity e0 = mgr.create();

    // Create an entity initialized with the specified components
    // Note: Any component specified must be default constructible.
    Entity e1 = mgr.create<Position, Direction>();

    // Create 100 plain entities
    mgr.create(100);

    // Create 100 entities initialized with the specified components
    mgr.create<Position, Direction>(100);

    // Create and store entities in an external container
    std::vector<Entity> plain_entities(100);
    mgr.create(plain_entities.begin(), plain_entities.end());

    // Create and store entities initialized with components in an
    // external container
    std::vector<Entity> movable_entities(100);
    mgr.create<Position, Direction>(movable_entities.begin(), movable_entities.end());
}
```

Additionally, entities can be constructed as copies of an existing entity by
invoking any one of the `ecfw::entity_manager::clone` functions. The only caveat
with cloning is that it requires you to specify at least one component to copy 
as a template param. The rationale here is that if you're going to clone an
entity you're doing so to copy its current functionality. If not, then its 
better to just create a regular entity.   

Examples:
```cpp
using Entity = uint32_t;
using CompList = ecfw::type_list<Position, Direction, Render>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

int main() {
    EntityManager mgr;
    
    // Create the model entity
    Entity progenitor = mgr.create<Position, Direction>();

    // Create a single clone
    Entity clone0 = mgr.clone<Position, Direction>(progenitor);

    // Create 100 clones
    mgr.clone<Position, Direction>(progenitor, 100);

    // Create and store clones in an external container
    std::vector<Entity> clones(100);
    mgr.clone<Position, Direction>(progenitor, clones.begin(), clones.end());
}
```

Entity destruction is given by the `ecfw::entity_manager::destroy` members.
Destruction is not immediate and must be followed up with a call to 
`ecfw::entity_manager::update` to flush any entities that were *destroyed*
since the last update step. The rationale behind delayed destruction is
to give dependent systems the ability to run their logic on the entity
once more before cleaning up for it.   
Examples:
```cpp
using Entity = uint32_t;
using CompList = ecfw::type_list<Position, Direction, Render>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

int main() {
    EntityManager mgr;
    
    // Create some entities;
    Entity e = mgr.create<Position, Direction>();
    
    // Destroy a single entity
    mgr.destroy(e);

    // Create a vector of entities
    std::vector<Entity> entities(100);
    mgr.create<Position, Direction>(entities.begin(), entities.end());
    
    // Destroy a collection of entities
    mgr.destroy(entities.begin(), entities.end());
    
    // NOTE: Do not forget to update your entity_manager
    mgr.update(); // Completely destroys any entities 
}
```

To iterate over a subset of the entities that have a certain set of components, 
you must invoke an `ecfw::entity_manager`s `entities_with` member function. The
member function accepts a functor whose parameter values are component type
references, or the entity type followed by component type references.   
For example:
```cpp
using Entity = uint32_t;
using CompList = ecfw::type_list<Position, Direction, Render>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

int main() {
    EntityManager mgr;
    
    // Create some entities;
    mgr.create<Position, Direction, Render>(10'000);
    
    // Iterates over all entities that have a Position component
    // Note: the supplied functor only accepts the component reference
    mgr.entities_with<Position>([](Position& p){});
    
    // Iterates over all entities that have both a Position and Direction
    // Note: the supplied functor accepts an entity as well as the components
    mgr.entities_with<Position, Direction>([](Entity e, Position& p, Direction& d)
    {});
}
```

#### Aside   
Internally, an `ecfw::entity_manager` is a component matrix whose column
headings are the component types you specified as a template parameter list.
Each row in the component matrix is an *entity* and the components in that
row are associated with the entity. Whether or not the entity actually has
any of those components depends on whether they're assigned to it.

The depth of the component matrix is two. The first layer of the matrix is
filled with boolean values that indicate current entity-component assignments.
The second layer of the matrix is just the component data.
When iterating over the entities in the matrix, the boolean layer serves as
a way to filter the entities based on the components an entity is required to
have. It works identically to that of a spreadsheet with boolean data. You're
filtering the columns based on `TRUE` values and then iterating the result.

Each entity type is divided into two parts: an index and a version.
The index serves as a *row index* into the component matrix (entity manager), 
and the version discriminates between old and new indices.   
Entity composition breakdown:   

| entity type  | largest index  | largest version  |
|---|---|---|
| `uint16_t`  | 2^12 - 1  | 2^4 - 1  |
| `uint32_t`  | 2^20 - 1  | 2^12 - 1  |
| `uint64_t`  | 2^32 - 1  | 2^32 - 1  |

There is a time-space tradeoff when using smaller entity types. The smaller types
generally increase the speed of the library at the cost of using more memory. It 
is up to you whether or not the cost is worth it at the end. I would generally
recommend using `uint32_t` as the default entity type as it is large enough for
almost all situations and provides good performance when iterating.

### Components
Components are simply building blocks that come together to form a larger whole.
They should each provide a default constructor, as well as a constructor that
properly initialize its member variables.

**Note:** You may provide member functions with your components but they are 
not used by the library internally.

Position component example:
```cpp
struct Position {
    Position(float x=0.f, float y=0.f)
        : x(x), y(y) {}
    float x, y;
};
```

The default storage implementation for components uses `std::vector`s. If this
scheme wastes too much memory, you can instead provide an alternative storage
container on a per component basis. All that is required of you is to provide
a template specialization of `ecfw::underlying_storage` with a member type 
`type` as an alias for your storage container.   
Example:
```cpp
struct AI {};

namespace ecfw {
    template <>
    struct underlying_storage<AI> {
        using type = <your_storage_container>
    };
}
```

### Events
The only events that matter when using an entity component system are:
1. Entity construction events.
2. Entity destruction events.
3. Component assignment events.
4. Component removal events.

`ecfw` provides an event type for each of the four categories:
```cpp
template <class Entity>
struct entity_created {
    Entity entity;  
};

template <class Entity>
struct entity_destroyed {
    Entity entity;  
};

template <class Entity, class Component>
struct component_added {
    Entity entity;  
    Component& component;
};

template <class Entity, class Component>
struct component_removed  {
    Entity entity;  
    Component& component;
};
```

The following are examples of how to subscribe to entity events:
```cpp
using Entity = uint32_t;
using CompList = ecfw::type_list<Position, Direction, Render>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;
using EntityCreatedEvent = ecfw::entity_created<Entity>;
using EntityDestroyedEvent = ecfw::entity_destroyed<Entity>;
template <class Component>
using ComponentAddedEvent = ecfw::component_added<Entity, Component>;
template <class Component>
using ComponentRemovedEvent = ecfw::component_removed<Entity, Component>;

// In order to subscribe to events using member functions you
// must inherit from ecfw::event_receiver. 
// ecfw::event_receiver manages the connections made between
// event signals and their slots
struct Receiver : ecfw::event_receiver {
    
    // You can attach nonconst member functions
    void function0(const EntityCreatedEvent& e){}
    
    // You can also attach const member functions
    void function1(const EntityDestroyedEvent& e) const {}
};

// You can attach lambda functions as event receivers
auto lambda = [](const ComponentAdded<Position>& e){}

// You can attach free functions as event receivers as well
void free_function(const ComponentRemoved<Position>& e) {}

int main() {
    EntityManager mgr;
    Receiver receiver;
    mgr.events().subscribe<EntityCreatedEvent>(&receiver, &Receiver::function0);
    mgr.events().subscribe<EntityDestroyedEvent>(&receiver, &Receiver::function1);
    mgr.events().subcribe<ComponentAddedEvent<Position>>(lambda);
    mgr.events().subcribe<ComponentRemovedEvent<Position>>(free_function);
}
```

**Note:** Entity destruction events are only emitted to its receivers when
an `entity_manager`'s `update` method is invoked.

## CMake integration
If you use CMake and what to use `ecfw` as a subproject, you can either clone
the repository, or add it as a git submodule in a sub-folder within your project.
In your *CMakeLists.txt* you just need to add the following:   
```CMake
add_subdirectory(<path_to_ecfw>)
target_link_libraries(${PROJECT_NAME} ecfw)
```

## CMake build options
 The library provides the following CMake options:
- `BUILD_ECFW_TESTS`: Enables tests to be built
- `BUILD_ECFW_BENCHMARKS`: Enables benchmarks to be built

**Note:** Both options are set to `OFF` be default.

## Dependencies
- [googletest](https://github.com/google/googletest): For unit tests.
- [googlebenchmark](https://github.com/google/benchmark): For benchmarks.
- [proto](https://github.com/N-A-D/proto): For event emissions and subscriptions.

## License
`ecfw` is licensed under the [MIT License](https://opensource.org/licenses/MIT):   

Copyright (c) 2019 [Ned Datiles](https://github.com/N-A-D)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
