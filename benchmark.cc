
#include <benchmark/benchmark.h>

#include "bataille.h"

namespace bataille {
namespace {

template <Strategy strategy>
void BM_Play(benchmark::State& state) {
  GameArena arena({.colors = 4, .values = 8});
  const std::vector<Card> cards = {
      3, 3, 5, 4, 2, 2, 5, 8, 7, 2, 6, 3, 7, 6, 5, 6,
      //
      1, 8, 2, 4, 8, 1, 5, 1, 1, 6, 7, 3, 7, 4, 8, 4};

  for (const auto s : state) {
    auto result = arena.Play(cards, strategy);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_Play<Strategy::kNatural>);
BENCHMARK(BM_Play<Strategy::kOptimized>);

}  // namespace
}  // namespace bataille