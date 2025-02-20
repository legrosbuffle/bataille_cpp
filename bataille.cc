#include "bataille.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

namespace bataille {

std::vector<Card> Deck::Make() const {
  std::vector<Card> cards;
  cards.reserve(colors * values);
  for (unsigned i = 1; i <= values; ++i) {
    for (unsigned j = 0; j < colors; ++j) {
      cards.push_back(i);
    }
  }
  return cards;
}

Hand::Hand(const Hand& other)
    : buffer_(std::make_unique<Card[]>(other.buffer_size())),
      buffer_end_(buffer_.get() + other.buffer_size()),
      start_(buffer_.get() + (other.start_ - other.buffer_.get())),
      end_(buffer_.get() + (other.end_ - other.buffer_.get())) {
  memcpy(buffer_.get(), other.buffer_.get(), buffer_size() * sizeof(Card));
}

bool operator==(const Hand& l, const Hand& r) {
  assert(l.buffer_size() == r.buffer_size());

  // Otherwise do a full comparison.
  const Card* lc = l.start_;
  const Card* rc = r.start_;
  const Card* const lend = l.end_;
  const Card* const rend = r.end_;
  const Card* const lbend = l.buffer_end_;
  const Card* const rbend = r.buffer_end_;
  for (; lc != lend && rc != rend;) {
    if (*lc != *rc) {
      return false;
    }
    ++lc;
    if (lc == lbend) lc = l.buffer_.get();
    ++rc;
    if (rc == rbend) rc = r.buffer_.get();
  }
  return lc == lend && rc == rend;
}

void Hand::PushAll(Card hi, Card lo, std::span<Card> cards, Strategy strategy) {
  assert(lo < hi);
  if (strategy == Strategy::kNatural) {
    Push(hi);
    Push(lo);
    // Then in reverse order.
    if (!cards.empty()) {
      const Card* const begin = cards.data();
      const Card* c = begin + cards.size();
      do {
        --c;
        Push(*c);
        Push(*c);
      } while (c != begin);
    }
  } else {
    assert(strategy == Strategy::kOptimized);
    if (cards.empty()) {
      Push(hi);
      Push(lo);
    } else {
      std::sort(cards.begin(), cards.end(), std::greater<>());
      const Card* c = cards.data();
      const Card* const end = c + cards.size();
      while (c != end && hi < *c) {
        Push(*c);
        Push(*c);
        ++c;
      }
      Push(hi);
      while (c != end && lo < *c) {
        Push(*c);
        Push(*c);
        ++c;
      }
      Push(lo);
      while (c != end && c != end) {
        Push(*c);
        Push(*c);
        ++c;
      }
    }
  }
}

std::string Hand::DebugString() const {
  std::string str = "[";
  for (const Card* c = start_; c != end_;) {
    str.append(std::to_string(*c));
    str.push_back(',');
    ++c;
    if (c == buffer_end_) c = buffer_.get();
  }
  str.push_back(']');
  return str;
}

// Precondition: num_ties_ == 0.
// Postcondition: num_ties_ == 0.
bool Game::Step(Strategy strategy) {
  assert(num_ties_ == 0);

  Card cl = l_.Pop();
  Card cr = r_.Pop();
  Card* const ties = ties_.get();
  while (cl == cr) {
    // Tie.
    ties[num_ties_] = cl;
    ++num_ties_;
    if (l_.empty() || r_.empty()) {
      num_ties_ = 0;
      return true;
    }
    cl = l_.Pop();
    cr = r_.Pop();
  }
  if (cr < cl) {
    l_.PushAll(cl, cr, std::span(ties, num_ties_), strategy);
  } else {
    // cr > cl.
    r_.PushAll(cr, cl, std::span(ties, num_ties_), strategy);
  }
  num_ties_ = 0;
  return l_.empty() || r_.empty();
}

Game::Game(const Game& o)
    : deck_(o.deck_),
      l_(o.l_),
      r_(o.r_),
      ties_(std::make_unique<Card[]>(deck_.num_cards())) {}

bool operator==(const Game& lhs, const Game& rhs) {
  assert(lhs.num_ties_ == 0);
  assert(rhs.num_ties_ == 0);
  assert(lhs.deck_.colors == rhs.deck_.colors);
  assert(lhs.deck_.values == rhs.deck_.values);
  return lhs.l_ == rhs.l_ && lhs.r_ == rhs.r_;
}

Game::Winner Game::GetWinner() const {
  return l_.empty() && r_.empty()
             ? Winner::kDraw
             : (l_.empty() ? Winner::kRight : Winner::kLeft);
}

GameArena::Stats::Stats(Deck deck)
    : longest(deck),
      shortest_with_cycle(deck),
      // number of games: (C*V)!/(C!)^V
      // log((C*V)!/(C!)^V) = log((C*V)!) - V * log(C!)
      //                    = lgamma(C*V + 1) - V* log(C + 1)
      num_games(std::exp(std::lgamma(deck.colors * deck.values + 1) -
                                   deck.values * std::lgamma(deck.colors + 1)) /
                          // Note: in the case of an even number of cards, we
                          // divide by two as right and left are symmetric.
                          (deck.num_cards() % 2 == 0 ? 2 : 1)) {}

void GameArena::Stats::Print(std::ostream& os) const {
  os << num_played_with_cycle << " loops found after " << num_played << "/"
     << num_games << "\n";
  if (shortest_with_cycle_len < std::numeric_limits<unsigned>::max()) {
    os << "shortest game with cycle (" << shortest_with_cycle_len
       << "):\ncartes_joueur1=" << shortest_with_cycle.left().DebugString()
       << "\ncartes_joueur2=" << shortest_with_cycle.right().DebugString()
       << "\n";
  }
  os << "longest game (" << longest_len
     << "):\ncartes_joueur1=" << longest.left().DebugString()
     << "\ncartes_joueur2=" << longest.right().DebugString() << "\n";
}

GameArena::GameArena(Deck deck) : slow_(deck), fast_(deck), stats_(deck) {}

Game::Result GameArena::PlayImpl(std::span<const Card> cards,
                                 Strategy strategy) {
  if (cards.size() == 0) {
    return {.winner = Game::Winner::kDraw, .num_steps = 0};
  }
  if (cards.size() == 1) {
    return {.winner = Game::Winner::kRight, .num_steps = 0};
  }
  // Cycle detection is done using Stepanov's "collision_point" method.
  slow_.Deal(cards);
  fast_.Deal(cards);

  if (fast_.Step(strategy))
    return {.winner = fast_.GetWinner(), .num_steps = 1};

  unsigned steps = 1;
  while (slow_ != fast_) {
    slow_.Step(strategy);
    if (fast_.Step(strategy))
      return {.winner = fast_.GetWinner(), .num_steps = 2 * steps};
    if (fast_.Step(strategy))
      return {.winner = fast_.GetWinner(), .num_steps = 2 * steps + 1};
    ++steps;
  }

  return {.winner = Game::Winner::kCycle, .num_steps = steps};
}

Game::Result GameArena::Play(std::span<const Card> cards, Strategy strategy) {
  const Game::Result result = PlayImpl(cards, strategy);
  if (result.winner == Game::Winner::kCycle) {
    ++stats_.num_played_with_cycle;
    if (result.num_steps < stats_.shortest_with_cycle_len) {
      stats_.shortest_with_cycle_len = result.num_steps;
      stats_.shortest_with_cycle.Deal(cards);
    }
  } else if (result.num_steps > stats_.longest_len) {
    stats_.longest_len = result.num_steps;
    stats_.longest.Deal(cards);
    std::cout << "new longest game (" << stats_.longest_len
              << "): " << stats_.longest.left().DebugString() << " "
              << stats_.longest.right().DebugString() << "\n";
  }
  ++stats_.num_played;
  return result;
}

}  // namespace bataille
