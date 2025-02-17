
#ifndef BATAILLE_H
#define BATAILLE

#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace bataille {

// Card values are in 1..255.
using Card = uint8_t;

struct Deck {
  unsigned colors;  // C
  unsigned values;  // V

  unsigned num_cards() const { return colors * values; }

  // Creates an unshuffled set of cards for the deck:
  // 1, 1, 1, 2, 2, 2, ..., values, values, values.
  std::vector<Card> Make() const;

  static Deck Seq(unsigned values) { return {.colors = 1, .values = values}; }
  static Deck Standard32() { return {.colors = 4, .values = 8}; }
  static Deck Standard54() { return {.colors = 4, .values = 13}; }
};

enum class Strategy {
  kNatural,  // Natural strategy.
  // kOptimized,  // Optimized strategy.
};

// A hand is a circular array.
class Hand {
 public:
  explicit Hand(unsigned max_cards)
      : buffer_(std::make_unique<Card[]>(max_cards + 1)),
        buffer_end_(buffer_.get() + max_cards + 1),
        start_(buffer_.get()),
        end_(buffer_.get()) {}

  Hand(const Hand& hand);
  Hand(Hand&&) = default;

  void Assign(const Card* cards, int n) {
    start_ = buffer_.get();
    end_ = start_ + n;
    assert(n < (buffer_end_ - buffer_.get()));
    memcpy(start_, cards, n * sizeof(Card));
  }

  friend bool operator==(const Hand& l, const Hand& r);

  bool empty() const { return start_ == end_; }

  std::string DebugString() const;

  // Remove the top card of the hand.
  Card Pop() {
    assert(!empty());
    const Card card = *start_;
    ++start_;
    if (start_ == buffer_end_) start_ = buffer_.get();
    return card;
  }

  // Add a card at the bottom.
  void Push(Card card) {
    // Add card.
    *end_ = card;
    ++end_;
    if (end_ == buffer_end_) end_ = buffer_.get();
  }

  // Add a bunch of cards at the bottom.
  void PushAll(Card hi, Card lo, std::span<const Card> cards,
               Strategy strategy);

 private:
  size_t buffer_size() const { return buffer_end_ - buffer_.get(); }
  const std::unique_ptr<Card[]> buffer_;
  const Card* const buffer_end_;
  Card* start_;
  Card* end_;
};

class Game {
 public:
  Game(Deck deck)
      : deck_(deck),
        l_(deck.num_cards()),
        r_(deck.num_cards()),
        ties_(std::make_unique<Card[]>(deck.num_cards())) {}

  // Splits the deck is evenly between left and right: the first half goes to
  // left player. If odd, the first player gets one card less.
  void Deal(std::span<const Card> cards) {
    assert(cards.size() == deck_.num_cards());
    l_.Assign(cards.data(), cards.size() / 2);
    r_.Assign(cards.data() + cards.size() / 2, cards.size() - cards.size() / 2);
  }

  const Hand& left() const { return l_; }
  const Hand& right() const { return r_; }

  friend bool operator==(const Game& lhs, const Game& rhs);

  enum class Winner {
    kLeft,
    kRight,
    kDraw,
    kCycle,
  };
  struct Result {
    Winner winner;
    unsigned num_steps;
  };

  // Does one round (incl. resolving ties) and returns true if any side is
  // empty.
  bool Step();

  // Given that at least one of the hands is empty, returns the winner.
  Winner GetWinner() const;

  Game(const Game& o);
  Deck deck() const { return deck_; }

 private:
  const Deck deck_;
  Hand l_;
  Hand r_;
  // We can represent each pair of ties with a single card since those
  // are equal by definition.
  std::unique_ptr<Card[]> ties_;
  unsigned num_ties_ = 0;
};

// A class that plays a bunch of games and computes stats.
// This reuses games in between calls to `Play` to avoid allocations.
class GameArena {
 public:
  GameArena(Deck deck);
  // Runs the game until the end or until we find a cycle.
  // Left player gets the first half, right player gets the second half. If odd,
  // the first player gets one card less.
  Game::Result Play(std::span<const Card> cards);

  struct Stats {
    Stats(Deck deck);

    void Print(std::ostream& os) const;

    // A snapshot of the length best games so far. Only for comparison, do not
    // rely on the type.
    auto snapshot() const {
      return std::make_pair(longest_len, shortest_with_cycle_len);
    }

    uint64_t num_played = 0;
    uint64_t num_played_with_cycle = 0;
    unsigned longest_len = 0;
    Game longest;
    unsigned shortest_with_cycle_len = std::numeric_limits<unsigned>::max();
    Game shortest_with_cycle;
    const double num_games;
  };
  const Stats& stats() const { return stats_; }

 private:
  Game::Result PlayImpl(std::span<const Card> cards);

  Game slow_;
  Game fast_;
  Stats stats_;
};

}  // namespace bataille

#endif  // BATAILLE_H