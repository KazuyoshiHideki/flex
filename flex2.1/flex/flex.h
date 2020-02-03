#pragma once
#include"stdafx.h"

#define BOARD_SIZE 13
#define BITBOARD_SIZE 256
#define MAX_MOVES 250
#define KOMI 4  //持碁白勝ち
#define MAX_PLAYOUTS 200
#define MAX_DEPTH 64
#define MAX_PLY 30
#define NODEMOVES 16 //ここを変えるときはNodeMovesも変えるべき
#define VALUE_INFINITE 1000
#define VALUE_NONE -1000


typedef int16_t Value;
typedef int32_t Depth;
typedef uint64_t Key;
typedef uint64_t Time;


//手番
enum Color { BLACK/*先手*/, WHITE/*後手*/, COLOR_NB, COLOR_ALL = 2 /*先後共通の何か*/, COLOR_ZERO = 0, NEW = 0, OLD = 1,};

// 相手番を返す
constexpr Color operator ~(Color c) { return (Color)(c ^ 1); }

//  例) FILE_3なら3筋。RANK_4なら4段
enum File { FILE_1, FILE_2, FILE_3, FILE_4, FILE_5, FILE_6, FILE_7, FILE_8, FILE_9, FILE_10, FILE_11, FILE_12, FILE_13, FILE_NB, FILE_ZERO = 0 };
enum Rank { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_9, RANK_10, RANK_11, RANK_12, RANK_13, RANK_NB, RANK_ZERO = 0 };



//交点・着手点
enum Point : uint16_t
{
	//着手なし
	PT_NULL = 0,

	// 以下、盤面の右上から左下までの定数。
	PT_1A, PT_1B, PT_1C, PT_1D, PT_1E, PT_1F, PT_1G, PT_1H, PT_1J, PT_1K, PT_1L, PT_1M, PT_1N,
	PT_2A, PT_2B, PT_2C, PT_2D, PT_2E, PT_2F, PT_2G, PT_2H, PT_2J, PT_2K, PT_2L, PT_2M, PT_2N,
	PT_3A, PT_3B, PT_3C, PT_3D, PT_3E, PT_3F, PT_3G, PT_3H, PT_3J, PT_3K, PT_3L, PT_3M, PT_3N,
	PT_4A, PT_4B, PT_4C, PT_4D, PT_4E, PT_4F, PT_4G, PT_4H, PT_4J, PT_4K, PT_4L, PT_4M, PT_4N,
	PT_5A, PT_5B, PT_5C, PT_5D, PT_5E, PT_5F, PT_5G, PT_5H, PT_5J, PT_5K, PT_5L, PT_5M, PT_5N,
	PT_6A, PT_6B, PT_6C, PT_6D, PT_6E, PT_6F, PT_6G, PT_6H, PT_6J, PT_6K, PT_6L, PT_6M, PT_6N,
	PT_7A, PT_7B, PT_7C, PT_7D, PT_7E, PT_7F, PT_7G, PT_7H, PT_7J, PT_7K, PT_7L, PT_7M, PT_7N,
	PT_8A, PT_8B, PT_8C, PT_8D, PT_8E, PT_8F, PT_8G, PT_8H, PT_8J, PT_8K, PT_8L, PT_8M, PT_8N,
	PT_9A, PT_9B, PT_9C, PT_9D, PT_9E, PT_9F, PT_9G, PT_9H, PT_9J, PT_9K, PT_9L, PT_9M, PT_9N,
	PT_10A, PT_10B, PT_10C, PT_10D, PT_10E, PT_10F, PT_10G, PT_10H, PT_10J, PT_10K, PT_10L, PT_10M, PT_10N,
	PT_11A, PT_11B, PT_11C, PT_11D, PT_11E, PT_11F, PT_11G, PT_11H, PT_11J, PT_11K, PT_11L, PT_11M, PT_11N,
	PT_12A, PT_12B, PT_12C, PT_12D, PT_12E, PT_12F, PT_12G, PT_12H, PT_12J, PT_12K, PT_12L, PT_12M, PT_12N,
	PT_13A, PT_13B, PT_13C, PT_13D, PT_13E, PT_13F, PT_13G, PT_13H, PT_13J, PT_13K, PT_13L, PT_13M, PT_13N,

	// 末尾とゼロ
	PT_NB, PT_ZERO = 1,

	//盤外
	PT_OUT = 0,
	
	//パスと投了
	PASS = 0, RESIGN = 171 
};

void point_print(Point pt);

const Point BitToPoint[BITBOARD_SIZE] = {
	PT_OUT, PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,
	PT_OUT, PT_1A, PT_1B, PT_1C, PT_1D, PT_1E, PT_1F, PT_1G, PT_1H, PT_1J, PT_1K, PT_1L, PT_1M, PT_1N, PT_OUT,PT_OUT,
	PT_OUT, PT_2A, PT_2B, PT_2C, PT_2D, PT_2E, PT_2F, PT_2G, PT_2H, PT_2J, PT_2K, PT_2L, PT_2M, PT_2N, PT_OUT,PT_OUT,
	PT_OUT, PT_3A, PT_3B, PT_3C, PT_3D, PT_3E, PT_3F, PT_3G, PT_3H, PT_3J, PT_3K, PT_3L, PT_3M, PT_3N, PT_OUT,PT_OUT,
	PT_OUT, PT_4A, PT_4B, PT_4C, PT_4D, PT_4E, PT_4F, PT_4G, PT_4H, PT_4J, PT_4K, PT_4L, PT_4M, PT_4N, PT_OUT,PT_OUT,
	PT_OUT, PT_5A, PT_5B, PT_5C, PT_5D, PT_5E, PT_5F, PT_5G, PT_5H, PT_5J, PT_5K, PT_5L, PT_5M, PT_5N, PT_OUT,PT_OUT,
	PT_OUT, PT_6A, PT_6B, PT_6C, PT_6D, PT_6E, PT_6F, PT_6G, PT_6H, PT_6J, PT_6K, PT_6L, PT_6M, PT_6N, PT_OUT,PT_OUT,
	PT_OUT, PT_7A, PT_7B, PT_7C, PT_7D, PT_7E, PT_7F, PT_7G, PT_7H, PT_7J, PT_7K, PT_7L, PT_7M, PT_7N, PT_OUT,PT_OUT,
	PT_OUT, PT_8A, PT_8B, PT_8C, PT_8D, PT_8E, PT_8F, PT_8G, PT_8H, PT_8J, PT_8K, PT_8L, PT_8M, PT_8N, PT_OUT,PT_OUT,
	PT_OUT, PT_9A, PT_9B, PT_9C, PT_9D, PT_9E, PT_9F, PT_9G, PT_9H, PT_9J, PT_9K, PT_9L, PT_9M, PT_9N, PT_OUT,PT_OUT,
	PT_OUT, PT_10A,PT_10B,PT_10C,PT_10D,PT_10E,PT_10F,PT_10G,PT_10H,PT_10J,PT_10K,PT_10L,PT_10M,PT_10N,PT_OUT,PT_OUT,
	PT_OUT, PT_11A,PT_11B,PT_11C,PT_11D,PT_11E,PT_11F,PT_11G,PT_11H,PT_11J,PT_11K,PT_11L,PT_11M,PT_11N,PT_OUT,PT_OUT,
	PT_OUT, PT_12A,PT_12B,PT_12C,PT_12D,PT_12E,PT_12F,PT_12G,PT_12H,PT_12J,PT_12K,PT_12L,PT_12M,PT_12N,PT_OUT,PT_OUT,
	PT_OUT, PT_13A,PT_13B,PT_13C,PT_13D,PT_13E,PT_13F,PT_13G,PT_13H,PT_13J,PT_13K,PT_13L,PT_13M,PT_13N,PT_OUT,PT_OUT,
	PT_OUT, PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,
	PT_OUT, PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT,PT_OUT
};

//各点の状態
enum State { B_STONE, W_STONE, BRANK, OUT};

//列挙型enumに対するoperator
#define ENABLE_OPERATORS_ON(T)                                                  \
  inline T operator+(const T d1, const T d2) { return T(int(d1) + int(d2)); }   \
  inline T operator-(const T d1, const T d2) { return T(int(d1) - int(d2)); }   \
  inline T operator*(const int i, const T d) { return T(i * int(d)); }          \
  inline T operator*(const T d, const int i) { return T(int(d) * i); }          \
  inline T operator-(const T d) { return T(-int(d)); }                          \
  inline T& operator+=(T& d1, const T d2) { return d1 = d1 + d2; }              \
  inline T& operator-=(T& d1, const T d2) { return d1 = d1 - d2; }              \
  inline T& operator*=(T& d, const int i) { return d = T(int(d) * i); }         \
  inline T& operator++(T& d) { return d = T(int(d) + 1); }                      \
  inline T& operator--(T& d) { return d = T(int(d) - 1); }                      \
  inline T operator++(T& d,int) { T prev = d; d = T(int(d) + 1); return prev; } \
  inline T operator--(T& d,int) { T prev = d; d = T(int(d) - 1); return prev; } \
  inline T operator/(const T d, const int i) { return T(int(d) / i); }          \
  inline T& operator/=(T& d, const int i) { return d = T(int(d) / i); }

ENABLE_OPERATORS_ON(Color)
ENABLE_OPERATORS_ON(File)
ENABLE_OPERATORS_ON(Rank)
ENABLE_OPERATORS_ON(Point)
ENABLE_OPERATORS_ON(State);

// 指し手とオーダリングのためのスコアがペアになっている構造体。
// オーダリングのときにスコアで並べ替えしたいが、一つになっているほうが並び替えがしやすいのでこうしてある。
struct ExtMove {

	Point move;   // 指し手
	Value value; // これはMovePickerが指し手オーダリングのために並び替えるときに用いる値(≠評価値)。

				 // Move型とは暗黙で変換できていい。

	operator Point() const { return move; }
	void operator=(Point m) { move = m; }
};

// ExtMoveの並べ替えを行なうので比較オペレーターを定義しておく。
inline bool operator<(const ExtMove& first, const ExtMove& second) {
	return first.value < second.value;
}

inline std::ostream& operator<<(std::ostream& os, ExtMove m) { os << m.move << '(' << m.value << ')'; return os; }



