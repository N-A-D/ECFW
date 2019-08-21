#include <benchmark/benchmark.h>
#include <ecfw/ecfw.hpp>
/*
#ifndef NDEBUG
#define NDEBUG
#endif // !NDEBUG
*/
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
		manager.create(entity_count);
	}
}

BENCHMARK(BM_EntityCreation);

static void BM_EntityCreation3Components(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		EntityManager manager(entity_count);
		state.ResumeTiming();
		manager.create<Position, Direction, Comp0>(entity_count);
	}
}

BENCHMARK(BM_EntityCreation3Components);

static void BM_EntityCreation5Components(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		EntityManager manager(entity_count);
		state.ResumeTiming();
		manager.create<Position, Direction, Comp0, Comp1, Comp2>(entity_count);
	}
}

BENCHMARK(BM_EntityCreation5Components);

static void BM_EntityCreation7Components(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		EntityManager manager(entity_count);
		state.ResumeTiming();
		manager.create<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4>(entity_count);
	}
}

BENCHMARK(BM_EntityCreation7Components);

static void BM_EntityCreation10Components(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		EntityManager manager(entity_count);
		state.ResumeTiming();
		manager.create<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4, Comp5, Comp6, Comp7>(entity_count);
	}
}

BENCHMARK(BM_EntityCreation10Components);

static void BM_EntityDestruction(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	manager.group_entites_with<Comp1>();
	manager.group_entites_with<Comp2>();
	manager.group_entites_with<Comp3>();
	manager.group_entites_with<Comp4>();
	manager.group_entites_with<Comp5>();
	manager.group_entites_with<Comp6>();
	manager.group_entites_with<Comp7>();
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

static void BM_Assigning3Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
	}
}

BENCHMARK(BM_Assigning3Components);

static void BM_Assigning5Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
	}
}

BENCHMARK(BM_Assigning5Components);

static void BM_Assigning7Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
	}
}

BENCHMARK(BM_Assigning7Components);

static void BM_Assigning10Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
		manager.assign<Comp5>(e);
		manager.assign<Comp6>(e);
		manager.assign<Comp7>(e);
	}
}

BENCHMARK(BM_Assigning10Components);

static void BM_Assigning3ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
	}
}

BENCHMARK(BM_Assigning3ComponentsWithExistingCaches);

static void BM_Assigning5ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	manager.group_entites_with<Comp1>();
	manager.group_entites_with<Comp2>();
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
	}
}

BENCHMARK(BM_Assigning5ComponentsWithExistingCaches);

static void BM_Assigning7ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	manager.group_entites_with<Comp1>();
	manager.group_entites_with<Comp2>();
	manager.group_entites_with<Comp3>();
	manager.group_entites_with<Comp4>();
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
	}
}

BENCHMARK(BM_Assigning7ComponentsWithExistingCaches);

static void BM_Assigning10ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	manager.group_entites_with<Comp1>();
	manager.group_entites_with<Comp2>();
	manager.group_entites_with<Comp3>();
	manager.group_entites_with<Comp4>();
	manager.group_entites_with<Comp5>();
	manager.group_entites_with<Comp6>();
	manager.group_entites_with<Comp7>();
	for (auto _ : state) {
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
		manager.assign<Comp5>(e);
		manager.assign<Comp6>(e);
		manager.assign<Comp7>(e);
	}
}

BENCHMARK(BM_Assigning10ComponentsWithExistingCaches);

static void BM_Removing3Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0>(e);
	}
}

BENCHMARK(BM_Removing3Components);

static void BM_Removing5Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0, Comp2>(e);
	}
}

BENCHMARK(BM_Removing5Components);

static void BM_Removing7Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4>(e);
	}
}

BENCHMARK(BM_Removing7Components);

static void BM_Removing10Components(benchmark::State& state) {
	EntityManager manager(entity_count);
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
		manager.assign<Comp5>(e);
		manager.assign<Comp6>(e);
		manager.assign<Comp7>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4, Comp5, Comp6, Comp7>(e);
	}
}

BENCHMARK(BM_Removing10Components);

static void BM_Removing3ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0>(e);
	}
}

BENCHMARK(BM_Removing3ComponentsWithExistingCaches);

static void BM_Removing5ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	manager.group_entites_with<Comp1>();
	manager.group_entites_with<Comp2>();
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0, Comp2>(e);
	}
}

BENCHMARK(BM_Removing5ComponentsWithExistingCaches);

static void BM_Removing7ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	manager.group_entites_with<Comp1>();
	manager.group_entites_with<Comp2>();
	manager.group_entites_with<Comp3>();
	manager.group_entites_with<Comp4>();
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4>(e);
	}
}

BENCHMARK(BM_Removing7ComponentsWithExistingCaches);

static void BM_Removing10ComponentsWithExistingCaches(benchmark::State& state) {
	EntityManager manager(entity_count);
	manager.group_entites_with<Position>();
	manager.group_entites_with<Direction>();
	manager.group_entites_with<Position, Direction>();
	manager.group_entites_with<Comp0>();
	manager.group_entites_with<Comp1>();
	manager.group_entites_with<Comp2>();
	manager.group_entites_with<Comp3>();
	manager.group_entites_with<Comp4>();
	manager.group_entites_with<Comp5>();
	manager.group_entites_with<Comp6>();
	manager.group_entites_with<Comp7>();
	for (auto _ : state) {
		state.PauseTiming();
		Entity e = manager.create();
		manager.assign<Position>(e, 1.337f, 1.337f);
		manager.assign<Direction>(e, 1.337f, 1.337f);
		manager.assign<Comp0>(e);
		manager.assign<Comp1>(e);
		manager.assign<Comp2>(e);
		manager.assign<Comp3>(e);
		manager.assign<Comp4>(e);
		manager.assign<Comp5>(e);
		manager.assign<Comp6>(e);
		manager.assign<Comp7>(e);
		state.ResumeTiming();
		manager.remove<Position, Direction, Comp0, Comp1, Comp2, Comp3, Comp4, Comp5, Comp6, Comp7>(e);
	}
}

BENCHMARK(BM_Removing10ComponentsWithExistingCaches);

static void BM_SequentialIteration(benchmark::State& state) {
	EntityManager manager;
	manager.create<Position, Direction>(entity_count);
	manager.group_entites_with<Position, Direction>();
	for (auto _ : state) {
		manager.entities_with<Position, Direction>([](auto& pos, auto& dir) {
			pos.x += dir.x * 1.337f;
			pos.y += dir.y * 1.337f;
		});
	}
}

BENCHMARK(BM_SequentialIteration);

BENCHMARK_MAIN();
