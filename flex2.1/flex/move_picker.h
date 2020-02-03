#pragma once
#include "stdafx.h"

// -----------------------
//   insertion sort
// -----------------------

// stableであることが保証されたinsertion sort。指し手オーダリングのために使う。
inline void insertion_sort(ExtMove* begin, ExtMove* end)
{
	ExtMove tmp, *p, *q;

	for (p = begin + 1; p < end; ++p)
	{
		tmp = *p;
		for (q = p; q != begin && *(q - 1) < tmp; --q)
			*q = *(q - 1);
		*q = tmp;
	}
}

// -----------------------
//  history , counter move
// -----------------------

template<typename T>
struct Stats {

	// このtableの要素の最大値
	static const Value Max = Value(1 << 28);

	// tableの要素の値を取り出す
	const T* operator[](Point pt) const {
		return table[pt];
	}
	T* operator[](Point pt) {
		return table[pt];
	}

	// tableのclear
	void clear() { std::memset(table, 0, sizeof(table)); }

	// tableに指し手を格納する。(Tの型がPointのとき)
	void update(Color c, Point pt, Point m)
	{
		table[pt][c] = m;
	}

	// tableに値を格納する(Tの型がValueのとき)
	void update(Color c, Point pt, Value v) 
	{
		table[pt][c] -= table[pt][c] * abs(int(v));
		table[pt][c] += int(v);
	}

private:

	T table[PT_NB][COLOR_ALL];
};

// Statsは、pcをsqの升に移動させる指し手に対してT型の値を保存する。
// TがMoveのときは、指し手に対する指し手、すなわち、"応手"となる。
// TがValueのときは指し手に対するスコアとなる。これがhistory table(HistoryStatsとCounterMoveStats)
// このStats<CounterMoveStats>は、直前の指し手に対する、あらゆる指し手に対するスコアである。

typedef Stats<Point> MoveStats;
typedef Stats<Value> HistoryStats;

// -----------------------
//   指し手オーダリング
// -----------------------

// 指し手を段階的に生成するために現在どの段階にあるかの状態を表す定数
enum Stages {

	MAIN_SEARCH_START,            // 置換表の手
	CAPTURES,                     // 取る手
	ESCAPES,                      // 逃げる手
	KILLERS,                      // KILLERの手
	LAST_AROUND,                  // 直前手の周り
	FOLLOW_AROUND,                // これまでの探索中の手の周りの手
	STOP,                         // 終端
	BAD_FAR,                      // 遠い悪そうな手
	OTHERS,                       // それ以外の手
	GOOD_FAR,                     // 遠い良さそうな手（つまり手抜き）

	
	//ほかにも、GOOD_FARを大場にするなどできるかも
};
ENABLE_OPERATORS_ON(Stages); // 次の状態にするためにインクリメントを使いたい。

							 // 指し手オーダリング器
struct MovePicker
{
	// このクラスは指し手生成バッファが大きいので、コピーして使うような使い方は禁止。
	MovePicker(const MovePicker&) = delete;
	MovePicker& operator=(const MovePicker&) = delete;

	// 通常探索から呼び出されるとき用。
	MovePicker(Position& pos_, Point ttMove_, Stack* ss_)
		: pos(pos_), ss(ss_), ttMove(ttMove_)
	{
		// 次の指し手生成の段階
		stage = MAIN_SEARCH_START;
		stageBB = ZERO_BB;
		doneBB = ZERO_BB;
	}

	// 次の指し手をひとつ返す
	// 指し手が尽きればPT_NULLが返る。
	Point next_move();

private:

	Position& pos;

	Bitboard doneBB;

	Bitboard stageBB;

	// node stack
	Stack* ss;

	// コンストラクタで渡されたttmove
	Point ttMove;

	// 指し手生成の段階
	Stages stage;

	// root局面での差し手
	std::vector<RootMove>* rootMoves;

};