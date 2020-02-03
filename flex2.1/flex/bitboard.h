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

	// ���������Ȃ��B���̂Ƃ����g�͕s��B
	Bitboard() {}

	// ������ɂ����Ă�avx2���g�����R�s�[���Ȃ���ė~����
	Bitboard(const Bitboard& bb) { _mm256_store_si256(&this->m, bb.m); }

	// p[0],p[1],p[2],p[3]�̒l�𒼐ڎw�肵�Ă̏������B(Bitboard�萔�̏������̂Ƃ��̂ݗp����)
	Bitboard(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3) { p[0] = p0; p[1] = p1; p[2] = p2; p[3] = p3; }

	// p0�̒l��256bit�S�̂�
	Bitboard(uint64_t p0) { m = _mm256_set1_epi64x(p0); }

	// sq�̏���1��Bitboard�Ƃ��ď���������B
	Bitboard(Point pt);

	//����^���Z�q
	Bitboard& operator = (const Bitboard& rhs) { _mm256_store_si256(&this->m, rhs.m); return *this; }
	Bitboard& operator |= (const Bitboard& b1) { this->m = _mm256_or_si256(m, b1.m);  return *this; }
	Bitboard& operator &= (const Bitboard& b1) { this->m = _mm256_and_si256(m, b1.m); return *this; }
	Bitboard& operator ^= (const Bitboard& b1) { this->m = _mm256_andnot_si256(b1.m, m); return *this; }
	Bitboard& operator += (const Bitboard& b1) { this->m = _mm256_add_epi64(m, b1.m); return *this; }
	Bitboard& operator -= (const Bitboard& b1) { this->m = _mm256_sub_epi64(m, b1.m); return *this; }
	Bitboard& operator <<= (int shift) { m = _mm256_slli_epi64(m, shift); return *this; }
	Bitboard& operator >>= (int shift) { m = _mm256_srli_epi64(m, shift); return *this; }


	//2�����Z�q
	Bitboard operator & (const Bitboard& rhs) const { return Bitboard(*this) &= rhs; }
	Bitboard operator | (const Bitboard& rhs) const { return Bitboard(*this) |= rhs; }
	Bitboard operator ^ (const Bitboard& rhs) const { return Bitboard(*this) ^= rhs; }
	Bitboard operator + (const Bitboard& rhs) const { return Bitboard(*this) += rhs; }
	Bitboard operator << (const int i) const { return Bitboard(*this) <<= i; }
	Bitboard operator >> (const int i) const { return Bitboard(*this) >>= i; }

	//��r���Z�q�@������Bitboard��1�̃r�b�g�Ɋւ��ē��ꂩ ���Ԃɒ���
	bool operator == (const Bitboard& rhs) const { return (_mm256_testc_si256(rhs.m, this->m) ? true : false); }
	//��r���Z�q�@������Bitboard��1�̃r�b�g�Ɋւ��Ē������Ă��邩 ���Ԃɒ���
	bool operator != (const Bitboard& rhs) const { return (_mm256_testz_si256(rhs.m, this->m) ? true : false); }

	//1�ł���r�b�g�𐔂���
	int pop_count() const { return (int)(__popcnt64(p[0]) + __popcnt64(p[1]) + __popcnt64(p[2]) + __popcnt64(p[3])); }

	int pop_count(int i) const { return (int)__popcnt64(p[i]); }

	//1�ł���r�b�g������o��
	Point pop();

	Point pop_const() const;

	//p[0],p[1],p[2],p[3]�� OR ���Ƃ�
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

// pt�̓_��1�ł���bitboard
extern Bitboard PointBB[PT_NB];
inline Bitboard::Bitboard(Point pt) { *this = PointBB[pt]; }

// pt�̌ċz�_��1�ł���Bitboard
extern Bitboard BreathBB[PT_NB];

// pt�̎���W������1�ł���Bitboard
extern Bitboard AroundBB[PT_NB];

// �S�_��1�ŔՊO��0��Bitboard
const Bitboard ALL_BB = Bitboard((uint64_t)0x3ffe3ffe3ffe0000, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x000000003ffe3ffe);

// �S�_��0�ŔՊO��1��Bitboard
const Bitboard WALL_BB = Bitboard((uint64_t)0xffffffffffffffff, (uint64_t)0xffffffffffffffff, (uint64_t)0xffffffffffffffff, (uint64_t)0xffffffffffffffff) ^ ALL_BB;

// �S����0�ł���Bitboard
const Bitboard ZERO_BB = Bitboard(0, 0, 0, 0);

// �P�����O�ɂ���MASK
const Bitboard MASK1_BB = Bitboard((uint64_t)0x1ffc1ffc00000000, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x0000000000001ffc);

// 1���̍s��0�ɂ���MASK
const Bitboard MASK1R_BB = Bitboard((uint64_t)0x3ffe3ffe00000000, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x3ffe3ffe3ffe3ffe, (uint64_t)0x0000000000003ffe);

// 1���̗��0�ɂ���MASK
const Bitboard MASK1F_BB = Bitboard((uint64_t)0x1ffc1ffc1ffc0000, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x1ffc1ffc1ffc1ffc, (uint64_t)0x000000001ffc1ffc);

// 2,3,4���ȊO��0�ɂ���MASK
const Bitboard UNMASK234_BB = Bitboard((uint64_t)0x1ffc1ffc00000000, (uint64_t)0x1c1c1c1c1c1c1ffc, (uint64_t)0x1ffc1ffc1c1c1c1c, (uint64_t)0x0000000000001ffc);

// 2,3,4���ȊO�̗��0�ɂ���MASK
const Bitboard UNMASK234R_BB = Bitboard((uint64_t)0x3ffe3ffe00000000, (uint64_t)0x0000000000003ffe, (uint64_t)0x3ffe3ffe00000000, (uint64_t)0x0000000000003ffe);

// 2,3,4���ȊO�̍s��0�ɂ���MASK
const Bitboard UNMASK234F_BB = Bitboard((uint64_t)0x1c1c1c1c1c1c0000, (uint64_t)0x1c1c1c1c1c1c1c1c, (uint64_t)0x1c1c1c1c1c1c1c1c, (uint64_t)0x000000001c1c1c1c);

//�s���͗l
const Bitboard ICHIMATSU[2] = { Bitboard((uint64_t)0x2aaa15542aaa0000, (uint64_t)0x2aaa15542aaa1554, (uint64_t)0x2aaa15542aaa1554, (uint64_t)0x000000002aaa1554),
								Bitboard((uint64_t)0x15542aaa15540000, (uint64_t)0x15542aaa15542aaa, (uint64_t)0x15542aaa15542aaa, (uint64_t)0x0000000015542aaa) };

// Bitboard��\������B
std::ostream& operator<<(std::ostream& os, const Bitboard& board);
