#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string_view>
#include <vector>

#include "bataille.h"

using bataille::Card;
using bataille::Deck;
using bataille::GameArena;
using bataille::Strategy;

void Exhaustive(std::span<Card> cards, Strategy strategy, GameArena& arena,
                std::ostream& os) {
  // Doubles can represent integers up to 2^52, which is enough for anything
  // we can search exhaustively.
  if (arena.stats().num_games > static_cast<double>(uint64_t{1} << 52)) {
    std::cerr << "too many games to explore, use the 'random' mode\n";
    return;
  }

  const auto start = std::chrono::system_clock::now();
  const uint64_t num_games =
      arena.stats().num_games + 1;  // +1 for numerical precision issues.
  auto stats_snapshot = arena.stats().snapshot();
  for (uint64_t i = 0; i < num_games; ++i) {
    arena.Play(cards, strategy);
    if ((arena.stats().num_played & 0xfffff) == 0 &&
        (arena.stats().snapshot() != stats_snapshot)) {
      arena.stats().Print(os);
      os << "time: "
         << std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - start)
         << "\n";
      os.flush();
      stats_snapshot = arena.stats().snapshot();
    }
    if (!std::next_permutation(cards.begin(), cards.end())) {
      break;  // Done.
    }
  }
  arena.stats().Print(os);
  os << "total time: "
     << std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - start)
     << "\n";
}

void Random(std::span<Card> cards, Strategy strategy, GameArena& arena,
            const unsigned seed, std::ostream& os) {
  std::mt19937 gen(seed);

  const auto start = std::chrono::system_clock::now();

  auto stats_snapshot = arena.stats().snapshot();
  while (true) {
    std::shuffle(cards.begin(), cards.end(), gen);
    arena.Play(cards, strategy);
    if ((arena.stats().num_played & 0xfffff) == 0 &&
        (arena.stats().snapshot() != stats_snapshot)) {
      arena.stats().Print(os);
      os << "time: "
         << std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - start)
         << "\n";
      os.flush();
      stats_snapshot = arena.stats().snapshot();
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 5) {
    std::cerr << argv[0] << " exhaustive|random natural|optimized C V [seed]\n";
    return 1;
  }

  bool exhaustive = false;
  if (argv[1] == std::string_view("exhaustive")) {
    exhaustive = true;
  } else if (argv[1] != std::string_view("random")) {
    std::cerr << "invalid exploration mode '" << argv[1] << "'\n";
  }

  Strategy strategy = Strategy::kNatural;
  if (argv[2] == std::string_view("optimized")) {
    strategy = Strategy::kOptimized;
  } else if (argv[2] != std::string_view("random")) {
    std::cerr << "invalid exploration mode '" << argv[1] << "'\n";
  }
  const Deck deck = {.colors = static_cast<unsigned>(std::atoi(argv[3])),
                     .values = static_cast<unsigned>(std::atoi(argv[4]))};
  const unsigned seed =
      exhaustive ? 0
                 : (argc >= 6 ? std::atoi(argv[5]) : std::random_device()());

  std::ofstream os("c" + std::to_string(deck.colors) + "v" +
                   std::to_string(deck.values) +
                   (strategy == Strategy::kNatural ? "" : "_opt") +
                   (exhaustive ? "" : "_" + std::to_string(seed)) + ".txt");
  if (!os) {
    std::cerr << "cannot open output file\n";
    return 1;
  }

  os << (exhaustive ? "exhaustive" : "random")
     << " exploration C=" << deck.colors << " V=" << deck.values << "\n\n";

  std::vector<Card> cards = deck.Make();
  GameArena arena(deck);
  if (exhaustive) {
    Exhaustive(cards, strategy, arena, os);
  } else {
    os << "seed=" << seed << "\n";
    Random(cards, strategy, arena, seed, os);
  }
  return 0;
}