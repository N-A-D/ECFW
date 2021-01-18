``` 
                                          ___    __  _____  __    __ 
                                         /  _]  /  ]|     ||  |__|  |
                                        /  [_  /  / |   __||  |  |  |
                                       |    _]/  /  |  |_  |  |  |  |
                                       |   [_/   \_ |   _] |  `  '  |
                                       |     \     ||  |    \      / 
                                       |_____|\____||__|     \_/\_/  
                               
```
## ecfw
`ecfw` is a header-only C++20 entity component system.

An entity component system is a type of component based system in which objects
(entities) span multiple domains by including components. Components are defined
as structures that are applicable to certain domains. For example, a `Physics`
component may contain, among other things, the attributes necessary to describe
how something moves.

For more information on entity component systems, see the following:
- [Wikipedia](https://en.wikipedia.org/wiki/Entity_component_system)
- [gameprogrammingpatterns](http://gameprogrammingpatterns.com/component.html)

## Overview
```cpp
#include <algorithm>
#include <ecfw/ecfw.hpp>

// ecfw represents entities as one of std::uint16_t, std::uint32_t, or std::uint64_t.
using Entity = std::uint16_t;

// ecfw::entity_traits defines the operations instances of ecfw::basic_world may
// perform on an entity's representation throughout the lifetime of an entity.
using EntityTraits = ecfw::entity_traits<Entity>;

// The class template ecfw::basic_world stores and manipulates entities and
// their components.
using World = ecfw::basic_world<Entity, EntityTraits>;

// Basic components to assign.
struct Position { float x, y; };
struct Velocity { float x, y; };

int main() {
    World world;

    // To create new entities use ecfw::basic_world::create.
    Entity entity = world.create();

    // To assign components to an entity, use template member function 
    // ecfw::basic_world::assign.
    world.assign<Position>(1.0f, 1.0f);
    world.assign<Velocity>(1.0f, 1.0f);

    // To obtain only the entities with a particular set of components, 
    // construct an instance of ecfw::basic_view using ecfw::basic_world::view.
    auto entities = world.view<Position, Direction>();

    // ecfw does not provide a concrete definition of what a system is. So long
    // as it can transform an entity's components, it can be called a system. To 
    // support may kinds of 'systems' class template ecfw::basic_view provides
    // begin/end member functions to allow iterating over the entities its
    // instances view.
    // Here, the lambda function given to std::ranges::for_each is our system.
    std::ranges::for_each(entities, [&entities](Entity e){
        // To obtain an entity's components use template member function 
        // ecfw::basic_view::get. You may choose to supply one or more
        // components, or none at all. In case you supply a single component,
        // ecfw::basic_view::get will return a reference. Supplying multiple
        // components returns a tuple of references ordered according to how
        // the templates types are given. When not supplying any components, 
        // a tuple of references is also returned, but the element order is
        // determined by the class template parameters.
        auto&& [p, v] = entities.get(e);
        p.x += v.x;
        p.y += v.y;
    });

    // To remove components from an entity, use template member function 
    // ecfw::basic_world::remove.
    world.remove<Position, Direction>(entity);

    // To destroy an entity, use ecfw::basic_world::destroy.
    world.destroy(entity);
}
```

## Dependencies
- Boost 1.74.
- Google Test (tests only).
- Google Benchmark (benchmarks only).

## Build
`ecfw` is a header-only library. It includes tests and benchmarks that can be
compiled as well, should you choose to do so.

To compile the tests and/or the benchmarks:
```sh
mkdir build
cd build
cmake -DBOOST_ROOT=/path/to/boost -DBUILD_ECFW_TESTS=True -DBUILD_ECFW_BENCHMARKS=True ..
cmake --build .
```

## License
Please see [LICENSE.md](LICENSE.md)
