#include <benchmark/benchmark.h>
#include <ecfw/ecfw.hpp>

#ifndef NDEBUG
#define NDEBUG
#endif // !NDEBUG


constexpr size_t entity_count = 1'000'000;

struct Position {
	Position(float x = 1.f, float y = 1.f)
		: x(x), y(y)
	{}
	float x, y;
};

struct Direction {
	Direction(float x = 1.f, float y = 1.f)
		: x(x), y(y)
	{}
	float x, y;
};

template <size_t N>
struct Component {
	Component()
		: name("")
		, floaty(0.f)
		, doublely(0.0)
		, id(0)
		, active(false) {}
	std::string name;
	float floaty;
	double doublely;
	int id;
	bool active;
};

using Comp0 = Component<0>;
using Comp1 = Component<1>;
using Comp2 = Component<2>;
using Comp3 = Component<3>;
using Comp4 = Component<4>;
using Comp5 = Component<5>;
using Comp6 = Component<6>;
using Comp7 = Component<7>;

using Entity = uint32_t;
using CompList = ecfw::type_list<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4, Comp5, Comp6, Comp7>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

static void BM_EntityCreation(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		EntityManager manager(entity_count);
		state.ResumeTiming();
		manager.create<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4, Comp5, Comp6, Comp7>(entity_count);
	}
}

BENCHMARK(BM_EntityCreation);

static void BM_EntityDestruction(benchmark::State& state) {
	EntityManager manager(entity_count);
	// Create some entity groups to guage the time to remove entities from them
	manager.entities_with<Position>([](auto&) {});
	manager.entities_with<Direction>([](auto&) {});
	manager.entities_with<Position, Direction>([](auto&, auto&) {});
	manager.entities_with<Comp0>([](auto&) {});
	manager.entities_with<Comp1>([](auto&) {});
	manager.entities_with<Comp2>([](auto&) {});
	manager.entities_with<Comp3>([](auto&) {});
	manager.entities_with<Comp4>([](auto&) {});
	manager.entities_with<Comp5>([](auto&) {});
	manager.entities_with<Comp6>([](auto&) {});
	manager.entities_with<Comp7>([](auto&) {});
	std::vector<Entity> entities(entity_count);
	for (auto _ : state) {
		state.PauseTiming();
		manager.create<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4, Comp5, Comp6, Comp7>(entities.begin(), entities.end());
		state.ResumeTiming();
		manager.destroy(entities.begin(), entities.end());
		manager.update();
	}
}

BENCHMARK(BM_EntityDestruction);

static void BM_SequentialIteration(benchmark::State& state) {
	EntityManager manager;
	manager.create<Position, Direction>(entity_count);
	manager.entities_with<Position, Direction>([](auto&, auto&) {});
	for (auto _ : state) {
		manager.entities_with<Position, Direction>([](auto& pos, auto& dir) {
			pos.x += dir.x * 1.337f;
			pos.y += dir.y * 1.337f;
		});
	}
}

BENCHMARK(BM_SequentialIteration);

static void BM_ParallelIteration(benchmark::State& state) {
	EntityManager manager;
	manager.create<Position, Direction>(entity_count);
	manager.entities_with<Position, Direction>([](auto&, auto&) {});
	for (auto _ : state) {
		manager.entities_with<Position, Direction>(std::execution::par, 
		[](auto& pos, auto& dir) {
			pos.x += dir.x * 1.337f;
			pos.y += dir.y * 1.337f;
		});
	}
}

BENCHMARK(BM_ParallelIteration);

BENCHMARK_MAIN();
