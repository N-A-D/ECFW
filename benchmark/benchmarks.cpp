#include <benchmark/benchmark.h>
#include <ecfw/ecfw.hpp>

#ifndef NDEBUG
#define NDEBUG
#endif // !NDEBUG


constexpr size_t entity_count = 30'000;

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

using Entity = uint64_t;
using CompList = ecfw::type_list<Position, Direction>;
using EntityManager = ecfw::entity_manager<Entity, CompList>;

static void BM_EntityCreation(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		manager.create<Position, Direction>(entity_count);
	}
}

BENCHMARK(BM_EntityCreation);

static void BM_EntityCreationAndDestruction(benchmark::State& state) {
	EntityManager manager;
	// Create some entity groups to guage the time to remove entities from them
	manager.entities_with<Position>([](auto&) {});
	manager.entities_with<Direction>([](auto&) {});
	manager.entities_with<Position, Direction>([](auto&, auto&) {});
	std::vector<Entity> storage(entity_count);
	for (auto _ : state) {
		manager.create<Position, Direction>(storage.begin(), storage.end());
		manager.destroy(storage.begin(), storage.end());
		manager.update();
	}
}

BENCHMARK(BM_EntityCreationAndDestruction);

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
