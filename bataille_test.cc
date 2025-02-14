#include "bataille.h"

#include <gtest/gtest.h>

namespace bataille {
namespace {

std::vector<Card> Cards(std::initializer_list<int> left,
                        std::initializer_list<int> right) {
  std::vector<Card> cards(left.begin(), left.end());
  cards.insert(cards.end(), right.begin(), right.end());
  return cards;
}

TEST(StatsTest, NumGames) {
  EXPECT_NEAR(GameArena::Stats({.colors = 1, .values = 5}).num_games, 120,
              0.01);
  EXPECT_NEAR(GameArena::Stats({.colors = 4, .values = 5}).num_games,
              152770117500ull, 0.01);
}

TEST(DeckTest, Make) {
  // Make sure that std::next_permutation enumerates multisets correctly (i.e.
  // just once for each duplicate value).
  // We have (C*V)!/(C!)^V permutations 6!/(2!)^3
  const Deck deck{.colors = 2, .values = 3};
  auto cards = deck.Make();
  EXPECT_EQ(cards, Cards({1, 1, 2, 2, 3, 3}, {}));
  int num_permutations = 1;
  while (std::next_permutation(cards.begin(), cards.end())) ++num_permutations;
  EXPECT_EQ(num_permutations, 6 * 5 * 4 * 3 * 2 / 8);
}

TEST(Hand, Basic) {
  Hand hand(3);
  EXPECT_TRUE(hand.empty());

  // 1
  hand.Push(1);
  ASSERT_FALSE(hand.empty());
  EXPECT_EQ(hand.Pop(), 1);
  EXPECT_TRUE(hand.empty());

  // 2 3
  hand.Push(2);
  hand.Push(3);
  ASSERT_FALSE(hand.empty());
  EXPECT_EQ(hand.Pop(), 2);
  ASSERT_FALSE(hand.empty());
  EXPECT_EQ(hand.Pop(), 3);
  EXPECT_TRUE(hand.empty());

  // 1 2 3
  hand.Push(1);
  hand.Push(2);
  hand.Push(3);
  ASSERT_FALSE(hand.empty());
  EXPECT_EQ(hand.Pop(), 1);
  ASSERT_FALSE(hand.empty());
  EXPECT_EQ(hand.Pop(), 2);
  ASSERT_FALSE(hand.empty());
  EXPECT_EQ(hand.Pop(), 3);
  EXPECT_TRUE(hand.empty());
}

TEST(Hand, CopyEquality) {
  Hand hand(4);
  hand.Push(1);
  hand.Push(2);
  EXPECT_TRUE(hand == hand);

  Hand hand2(4);
  hand2.Push(1);
  hand2.Push(2);
  EXPECT_TRUE(hand == hand2);

  Hand copy = hand;
  EXPECT_TRUE(hand == copy);

  copy.Pop();
  EXPECT_FALSE(hand == copy);

  copy.Pop();
  EXPECT_FALSE(hand == copy);

  copy.Push(1);
  EXPECT_FALSE(hand == copy);

  copy.Push(2);
  EXPECT_TRUE(hand == copy);
}

TEST(Hand, SimpleSeq2a) {
  GameArena arena(Deck::Seq(2));
  const auto result = arena.Play(Cards({1}, {2}));
  EXPECT_EQ(result.winner, Game::Winner::kRight);
  EXPECT_EQ(result.num_steps, 1);
}

TEST(Hand, SimpleSeq2b) {
  GameArena arena(Deck::Seq(2));
  const auto result = arena.Play(Cards({2}, {1}));
  EXPECT_EQ(result.winner, Game::Winner::kLeft);
  EXPECT_EQ(result.num_steps, 1);
}

TEST(Hand, SimpleWithTie) {
  GameArena arena({.colors = 2, .values = 1});
  const auto result = arena.Play(Cards({1}, {1}));
  EXPECT_EQ(result.winner, Game::Winner::kDraw);
  EXPECT_EQ(result.num_steps, 1);
}

TEST(Hand, SimpleWithTieThen) {
  GameArena arena({.colors = 2, .values = 3});
  const auto result = arena.Play(Cards({1, 3, 3}, {1, 2, 2}));
  // 1 1; 3 2 -> {3,3,2,1,1} {2}
  // 3 2 -> {3,2,1,1,3,2} {}
  EXPECT_EQ(result.winner, Game::Winner::kLeft);
  EXPECT_EQ(result.num_steps, 2);
}

TEST(Hand, SimpleDraw) {
  GameArena arena({.colors = 4, .values = 2});
  const auto result = arena.Play(Cards({1, 1, 2, 2}, {2, 2, 1, 1}));
  EXPECT_EQ(result.winner, Game::Winner::kDraw);
  EXPECT_EQ(result.num_steps, 5);
}

TEST(Hand, LoopC1V5) {
  GameArena arena(Deck::Seq(5));
  const auto result = arena.Play(Cards({5, 3}, {2, 4, 1}));
  EXPECT_EQ(result.winner, Game::Winner::kCycle);
  EXPECT_EQ(result.num_steps, 6);
}

TEST(Hand, LoopC1V7) {
  GameArena arena(Deck::Seq(7));
  const auto result = arena.Play(Cards({1, 7, 4}, {5, 3, 6, 2}));
  EXPECT_EQ(result.winner, Game::Winner::kCycle);
  EXPECT_EQ(result.num_steps, 8);
}

TEST(Hand, LoopC1V9) {
  GameArena arena(Deck::Seq(9));
  const auto result = arena.Play(Cards({9, 5, 8, 1}, {3, 7, 2, 6, 1}));
  EXPECT_EQ(result.winner, Game::Winner::kCycle);
  EXPECT_EQ(result.num_steps, 20);
}

TEST(Hand, LoopC1V10) {
  GameArena arena(Deck::Seq(10));
  const auto result = arena.Play(Cards({8, 6, 3, 10, 5}, {2, 9, 7, 4, 1}));
  EXPECT_EQ(result.winner, Game::Winner::kCycle);
  EXPECT_EQ(result.num_steps, 60);
}

TEST(Hand, NoLoopC1V16) {
  GameArena arena(Deck::Seq(16));
  const auto result = arena.Play(
      Cards({1, 3, 2, 8, 10, 15, 11, 12}, {4, 6, 16, 13, 9, 14, 5, 7}));
  EXPECT_EQ(result.winner, Game::Winner::kRight);
  EXPECT_EQ(result.num_steps, 90);
}

}  // namespace
}  // namespace bataille