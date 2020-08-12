#include <iterator>
#include <execution>
#include <benchmark/benchmark.h>
#include <ecfw/ecfw.hpp>

const size_t MAX_ENTITIES = 1'000'000;

static void BM_CreateSingleEmptyEntity(benchmark::State& state) {

}
BENCHMARK(BM_CreateSingleEmptyEntity);
