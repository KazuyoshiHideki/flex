#pragma once
#include"stdafx.h"
#include"flex.h"

namespace Bitboards {
	void init();
}

struct Bitboard
{
	union {
		uint64_t p[4];
		uint8_t b[32];
		__m256i m;
	};

	// 初期化しない。このとき中身は不定。
	Bitboard() {}

	// 代入等においてはavx2を使ったコピーがなされて欲しい
	Bitboard(const Bitboard& bb) { _mm256_store_si256(&this->m, bb.m); }

	// p[0],p[1],p[2],p[3]の値を直接指定しての初期化。(Bitboard定数の初期化のときのみ用いる)
	Bitboard(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3) { p[0] = p0; p[1] = p1; p[2] = p2; p[3] = p3; }

	// p0の値を256bit全体に
	Bitboard(uint64_t p0) { m = _mm256_set1_epi64x(p0); }

	// sqの升が1のBitboardとして初期化する。
	Bitboard(Point pt);

	//代入型演算子
	Bitboard& operator = (const Bitboard& rhs) { _mm256_store_si256(&this->m, rhs.m); return *this; }
	Bitboard& operator |= (const Bitboard& b1) { this->m = _mm256_or_si256(m, b1.m);  return *this; }
	Bitboard& operator &= (const Bitboard& b1) { this->m = _mm256_and_si256(m, b1.m); return *this; }
	Bitboard& operator ^= (const Bitboard& b1) { this->m = _mm256_andnot_si256(b1.m, m); return *this; }
	Bitboard& operator += (const Bitboard& b1) { this->m = _mm256_add_epi64(m, b1.m); return *this; }
	Bitboard& operator -= (const Bitboard& b1) { this->m = _mm256_sub_epi64(m, b1.m); return *this; }
	Bitboard& operator <<= (int shift) { m = _mm256_slli_epi64(m, shift); return *this; }
	Bitboard& operator >>= (int shift) { m = _mm256_srli_epi64(m, shift); return *this; }


	//2項演算子
	Bitboard operator & (const Bitboard& rhs) const { return Bitboard(*this) &= rhs; }
	Bitboard operator | (const Bitboard& rhs) const { return Bitboard(*this) |= rhs; }
	Bitboard operator ^ (const Bitboard& rhs) const { return Bitboard(*this) ^= rhs; }
	Bitboard operator + (const Bitboard& rhs) const { return Bitboard(*this) += rhs; }
	Bitboard operator << (const int i) const { return Bitboard(*this) <<= i; }
	Bitboard operator >> (const int i) const { return Bitboard(*this) >>= i; }

	//比較演算子　左側のBitboardの1のビットに関して同一か 順番に注意
	bool operator == (const Bitboard& rhs) const { return (_mm256_testc_si256(rhs.m, this->m) ? true : false); }
	//比較演算子　左側のBitboardの1のビットに関して直交しているか 順番に注意
	bool operator != (const Bitboard& rhs) const { return (_mm256_testz_si256(rhs.m, this->m) ? true : false); }

	//1であるビットを数える
	int pop_count() const { return (int)(__popcnt64(p[0]) + __popcnt64(p[1]) + __popcnt64(p[2]) + __popcnt64(p[3])); }

	int pop_count(int i) const { return (int)__popcnt64(p[i]); }

	//1であるビットを一つ取り出す
	Point pop();

	Point pop_const() const;

	//p[0],p[1],p[2],p[3]の OR をとる
	uint64_t part_or() const { return p[0] | p[1] | p[2] | p[3]; }
};

#define ShiftE(b,b1) (b).m = _mm256_slli_epi64((b1).m, 1)
#define ShiftW(b,b1) (b).m = _mm256_srli_epi64((b1).m, 1)
#define ShiftN(b,b1) (b).m = _mm256_slli_si256((b1).m, 2)
#define ShiftS(b,b1) (b).m = _mm256_srli_si256((b1).m, 2)

inline Bitboard shiftN(Bitboard bb)
{
	Bitboard b1;
	ShiftN(b1, bb);
	b1.b[17] = bb.b[15];
	b1.b[18] = bb.b[16];
	return b1;
}

inline Bitboard shiftS(Bitboard bb)
{
	Bitboard b1;
	ShiftS(b1, bb);
	b1.b[15] = bb.b[17];
	b1.b[16] = bb.b[18];
	return b1;
}

// ptの点が1であるbitboard
extern Bitboard PointBB[PT_NB];
inline Bitboard::Bitboard(Point pt) { *this = PointBB[pt]; }

// ptの呼吸点が1であるBitboard
extern Bitboard BreathBB[PT_NB];

// ptの周り８か所が1であるBitboard
extern Bitboard AroundBB[PT_NB];

// 全点が1で盤外が0のBitboard
const Bitboard ALL_BB = Bitboard((uint64_t)0x3ffe3ffe3ffe0000, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x000000003ffe3ffe);

// 全点が0で盤外が1のBitboard
const Bitboard WALL_BB = Bitboard((uint64_t)0xffffffffffffffff, (uint64_t)0xffffffffffffffff, (uint64_t)0xffffffffffffffff, (uint64_t)0xffffffffffffffff) ^ ALL_BB;

// 全升が0であるBitboard
const Bitboard ZERO_BB = Bitboard(0, 0, 0, 0);

// １線を０にするMASK
const Bitboard MASK1_BB = Bitboard((uint64_t)0x1ffc1ffc00000000, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x0000000000001ffc);

// 1線の行を0にするMASK
const Bitboard MASK1R_BB = Bitboard((uint64_t)0x3ffe3ffe00000000, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x0000000000003ffe);

// 1線の列を0にするMASK
const Bitboard MASK1F_BB = Bitboard((uint64_t)0x1ffc1ffc1ffc0000, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x000000001ffc1ffc);

// 2,3,4線以外を0にするMASK
const Bitboard UNMASK234_BB = Bitboard((uint64_t)0x1ffc1ffc00000000, (uint64_t)0x1c1c1c1c1c1c1ffc, (uint64_t)0x1ffc1ffc1c1c1c1c, (uint64_t)0x0000000000001ffc);

// 2,3,4線以外の列を0にするMASK
const Bitboard UNMASK234R_BB = Bitboard((uint64_t)0x3ffe3ffe00000000, (uint64_t)0x0000000000003ffe, (uint64_t)0x3ffe3ffe00000000, (uint64_t)0x0000000000003ffe);

// 2,3,4線以外の行を0にするMASK
const Bitboard UNMASK234F_BB = Bitboard((uint64_t)0x1c1c1c1c1c1c0000, (uint64_t)0x1c1c1c1c1c1c1c1c, (uint64_t)0x1c1c1c1c1c1c1c1c, (uint64_t)0x000000001c1c1c1c);

//市松模様
const Bitboard ICHIMATSU[2] = { Bitboard((uint64_t)0x2aaa15542aaa0000, (uint64_t)0x2aaa15542aaa1554, (uint64_t)0x2aaa15542aaa1554, (uint64_t)0x000000002aaa1554),
								Bitboard((uint64_t)0x15542aaa15540000, (uint64_t)0x15542aaa15542aaa, (uint64_t)0x15542aaa15542aaa, (uint64_t)0x0000000015542aaa) };

// Bitboardを表示する。
std::ostream& operator<<(std::ostream& os, const Bitboard& board);
