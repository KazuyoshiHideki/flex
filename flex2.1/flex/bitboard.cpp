#include "bitboard.h"
#include"stdafx.h"

inline int lsb64(uint64_t v) { unsigned long index; _BitScanForward64(&index, v); return index; }

using namespace std;

// ----- Bitboard tables

// sqの升が1であるbitboard
Bitboard PointBB[PT_NB];

// ptの呼吸点が1であるbitboard
Bitboard BreathBB[PT_NB];

// ptの周り８か所が1であるBitboard
Bitboard AroundBB[PT_NB];

// Bitboard関連の各種テーブルの初期化。
void Bitboards::init()
{
	// PointBBの初期化。
	PointBB[PT_NULL] = ZERO_BB;
	for (Point pt = PT_ZERO; pt < PT_NB; ++pt)
	{
		int r = 1 + (int)(pt - PT_1A) / BOARD_SIZE;
		int f = 1 + (int)(pt - PT_1A) % BOARD_SIZE;
		PointBB[pt] = ZERO_BB;
		if(r > 0 && r <= 3 )PointBB[pt].p[0] = (uint64_t)1 << (r * 16 + f);
		if(r > 3 && r <= 7 )PointBB[pt].p[1] = (uint64_t)1 << ((r - 4) * 16 + f);
		if(r > 7 && r <= 11)PointBB[pt].p[2] = (uint64_t)1 << ((r - 8) * 16 + f);
		if(r > 11&& r <= 13)PointBB[pt].p[3] = (uint64_t)1 << ((r - 12) * 16 + f);
	}

	// BreathBBの初期化。
	BreathBB[PT_NULL] = ZERO_BB;
	for (Point pt = PT_ZERO; pt < PT_NB; ++pt)
	{
		BreathBB[pt] = ZERO_BB;
		BreathBB[pt] |= PointBB[pt] << 1;
		BreathBB[pt] |= PointBB[pt] >> 1;
		BreathBB[pt] |= PointBB[((int)pt - BOARD_SIZE > (int)PT_NULL) ? (int)pt - BOARD_SIZE : (int)PT_NULL];
		BreathBB[pt] |= PointBB[((int)pt + BOARD_SIZE < (int)PT_NB) ? (int)pt + BOARD_SIZE : (int)PT_NULL];
		BreathBB[pt] &= ALL_BB;
	}
	
	// AroundBBの初期化。
	AroundBB[PT_NULL] = ZERO_BB;
	for (Point pt = PT_ZERO; pt < PT_NB; ++pt)
	{
		AroundBB[pt] = ZERO_BB;
		AroundBB[pt] |= PointBB[((int)pt - BOARD_SIZE > (int)PT_NULL) ? (int)pt - BOARD_SIZE : (int)PT_NULL];
		AroundBB[pt] |= PointBB[((int)pt + BOARD_SIZE < (int)PT_NB) ? (int)pt + BOARD_SIZE : (int)PT_NULL];
		AroundBB[pt] |= AroundBB[pt] << 1;
		AroundBB[pt] |= AroundBB[pt] >> 1;
		AroundBB[pt] |= PointBB[pt] << 1;
		AroundBB[pt] |= PointBB[pt] >> 1;
		AroundBB[pt] &= ALL_BB;
	}
}

Point Bitboard::pop()
{
	static int i;
	//立っているビットがない場合、PT_NULLを返す
	if(*this == ZERO_BB)
		return PT_NULL;
	
	//立っているビットのあるp[i]を探す
	while (p[i] == 0) { i = (i + 1) & 3; }

	int index = lsb64(p[i]);
	p[i] &= p[i] - 1;

	return BitToPoint[index + (i<<6)];
}

Point Bitboard::pop_const() const
{
	static int i;
	//立っているビットがない場合、PT_NULLを返す
	if (*this == ZERO_BB)
		return PT_NULL;

	//立っているビットのあるp[i]を探す
	while (p[i] == 0) { i = (i + 1) & 3; }

	int index = lsb64(p[i]);

	return BitToPoint[index + (i << 6)];
}

// Bitboardを表示する
std::ostream& operator<<(std::ostream& os, const Bitboard& board)
{
	for (int i = 0; i < 4; i++){
		int shiftsize = 0;
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 16; k++)
				os << ((board.p[i] & ((uint64_t)1 << shiftsize++)) ? "1" : "0") << " ";
			os << endl;
		}
	}

	// 連続して表示させるときのことを考慮して改行を最後に入れておく。
	os << endl;
	return os;
}