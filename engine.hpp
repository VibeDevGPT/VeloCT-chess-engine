#pragma once
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <thread>
#include <optional>
#include <variant>
#include <functional>
#include <filesystem>

namespace VeloCT {

// Basic Type Aliases
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = std::size_t;
using Depth = int;

constexpr bool Is64Bit = sizeof(void*) == 8;
constexpr int TT_SIZE_PER_THREAD = 16;
constexpr const char* StartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr int MaxThreads = 1024;
constexpr int MaxHashMB = 33554432;
constexpr int MAX_MOVES = 256;

using Key = uint64_t;
using Bitboard = uint64_t;

enum Direction : int {
  NORTH = 8,  NORTH_EAST = 9,  EAST = 1,  SOUTH_EAST = -7,
  SOUTH = -8, SOUTH_WEST = -9, WEST = -1, NORTH_WEST = 7
};

enum Rank : int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB = 8
};

enum File : int {
  FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB = 8
};

enum Square : int {
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_NONE,
  SQUARE_ZERO = 0,
  SQUARE_NB   = 64
};

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileHBB = FileABB << 7;
constexpr Bitboard Rank1BB = 0xFFULL;
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

inline bool is_ok(Square s) { return s >= SQ_A1 && s <= SQ_H8; }
inline Bitboard square_bb(Square s) { assert(is_ok(s)); return 1ULL << s; }

inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
inline Square operator++(Square& s, int) { Square prev = s; s = Square(int(s) + int(1)); return prev; }
inline Square& operator+=(Square& s, Direction d) { return s = Square(int(s) + int(d)); }
inline Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }

inline Rank& operator--(Rank& r) { return r = Rank(int(r) - 1); }
inline File& operator++(File& f) { return f = File(int(f) + 1); }

inline Rank rank_of(Square s) { return Rank(s >> 3); }
inline File file_of(Square s) { return File(s & 7); }
inline Bitboard rank_bb(Square s) { return Rank1BB << (8 * rank_of(s)); }
inline Bitboard file_bb(Square s) { return FileABB << file_of(s); }
inline Square make_square(File f, Rank r) { return Square((r << 3) + f); }

#if defined(__GNUG__) || defined(__clang__)
inline int popcount(Bitboard b) { return __builtin_popcountll(b); }
#else
inline int popcount(Bitboard b) {
    int c = 0;
    for (; b; b &= b - 1) c++;
    return c;
}
#endif

enum PieceType {
  NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
  ALL_PIECES = 0, PIECE_TYPE_NB = 8
};

extern Bitboard PseudoAttacks[PIECE_TYPE_NB][SQUARE_NB];
extern Bitboard LineBB[SQUARE_NB][SQUARE_NB];
Bitboard attacks_bb(PieceType pt, Square sq, Bitboard occupied);

struct Magic {
  Bitboard  mask;
  Bitboard  magic;
  Bitboard* attacks;
  unsigned  shift;

  unsigned index(Bitboard b) const {
    if (Is64Bit)
        return unsigned(((b & mask) * magic) >> shift);
    unsigned lo = unsigned(b & mask);
    unsigned hi = unsigned((b & mask) >> 32);
    return unsigned((lo * unsigned(magic) ^ hi * unsigned(magic >> 32)) >> shift);
  }
};

class PRNG {
  uint64_t s;
  uint64_t rand64() {
    s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
    return s * 2685821657736338717ULL;
  }
public:
  PRNG(uint64_t seed) : s(seed) {}
  template<typename T> T rand() { return T(rand64()); }
  template<typename T = Bitboard>
  T sparse_rand() { return rand<T>() & rand<T>() & rand<T>(); }
};

Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied);

inline Square safe_destination(Square s, Direction d) {
  int dest = int(s) + int(d);
  if (dest < 0 || dest >= 64) return SQ_NONE;
  int file_diff = (dest % 8) - (int(s) % 8);
  if (std::abs(file_diff) > 2) return SQ_NONE;
  return Square(dest);
}

inline unsigned get_hardware_concurrency() {
  unsigned n = std::thread::hardware_concurrency();
  return n ? n : 1;
}

namespace Bitboards {
  std::string pretty(Bitboard b);
  void init();
}

namespace Eval::NNUE {
}

struct BundledL3Policy { int threads; };
using NumaAutoPolicy = BundledL3Policy;

namespace Search {
  struct LimitsType {};
  namespace Skill {
    constexpr int LowestElo = 1320;
    constexpr int HighestElo = 3190;
  }
}

struct PositionSetError {};

struct StateInfo {};

struct Option {
  using OnChange = std::function<std::optional<std::string>(const Option&)>;
  Option() = default;
  Option(bool) {}
  Option(int, int = 0, int = 0) {}
  Option(const char*, OnChange = nullptr) {}
  Option(const std::string&, OnChange = nullptr) {}
  Option(OnChange) {}
  Option(const char*, OnChange) {}
  Option(const std::string&, OnChange) {}
  Option(int, int, int, OnChange) {}

  template<typename F, typename = std::enable_if_t<std::is_invocable_v<F, const Option&>>>
  Option(F) {}

  template<typename F, typename = std::enable_if_t<std::is_invocable_v<F, const Option&>>>
  Option(const char*, F) {}

  template<typename F, typename = std::enable_if_t<std::is_invocable_v<F, const Option&>>>
  Option(int, int, int, F) {}

  operator std::string() const { return ""; }
  operator int() const { return 0; }
  operator bool() const { return false; }
};

struct CommandLine {
  static std::filesystem::path get_binary_directory(const std::filesystem::path& p) { return p.parent_path(); }
};

struct NumaConfig {
  static NumaConfig from_system(NumaAutoPolicy) { return {}; }
};

inline bool set_numa_config_from_option(const Option&) { return true; }
inline std::string numa_config_information_as_string() { return ""; }
inline std::string thread_allocation_information_as_string() { return ""; }

inline std::filesystem::path path_from_utf8(const std::string& str) { return std::filesystem::path(str); }
inline void start_logger(const std::filesystem::path&) {}

struct NetworkPair {
  std::optional<std::string> opt;
  std::string name;
};

struct NetworkEngine {
  NetworkEngine(const NumaConfig&) {}
};

class Engine {
public:
  struct InfoShort {};
  struct InfoFull {};
  struct InfoIter {};

  struct Threads { bool stop{false}; };

  std::filesystem::path binaryDirectory;
  NumaConfig numaContext;
  std::unique_ptr<std::deque<StateInfo>> states;
  Threads threads;
  NetworkPair networkFile;
  NetworkEngine network;
  
  struct Position {
    void set(const char*, bool, StateInfo*) {}
  } pos;

  struct Options {
    void add(const std::string&, const Option&) {}
  } options;

  Engine(std::optional<std::filesystem::path> path = std::nullopt);
  
  void resize_threads() {}
  void set_tt_size(int) {}

  std::variant<u64, PositionSetError> perft(const std::string& fen, Depth depth, bool isChess960);
  void go(Search::LimitsType& limits);
  void stop();
  void search_clear();
  
  void set_on_update_no_moves(std::function<void(const InfoShort&)>&& f);
  void set_on_update_full(std::function<void(const InfoFull&)>&& f);
  void set_on_iter(std::function<void(const InfoIter&)>&& f);
};

struct BenchmarkSetup {
  std::string fen;
  int depth{0};
  int threads{1};
  int ttSize{16};
  std::string originalInvocation;
  std::string filledInvocation;
  std::vector<std::string> commands;
};

}

namespace Stockfish = VeloCT;
using namespace VeloCT;
