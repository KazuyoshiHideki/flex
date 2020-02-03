#pragma once
#include "stdafx.h"

// -----------------------
//   insertion sort
// -----------------------

// stable�ł��邱�Ƃ��ۏ؂��ꂽinsertion sort�B�w����I�[�_�����O�̂��߂Ɏg���B
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

	// ����table�̗v�f�̍ő�l
	static const Value Max = Value(1 << 28);

	// table�̗v�f�̒l�����o��
	const T* operator[](Point pt) const {
		return table[pt];
	}
	T* operator[](Point pt) {
		return table[pt];
	}

	// table��clear
	void clear() { std::memset(table, 0, sizeof(table)); }

	// table�Ɏw������i�[����B(T�̌^��Point�̂Ƃ�)
	void update(Color c, Point pt, Point m)
	{
		table[pt][c] = m;
	}

	// table�ɒl���i�[����(T�̌^��Value�̂Ƃ�)
	void update(Color c, Point pt, Value v) 
	{
		table[pt][c] -= table[pt][c] * abs(int(v));
		table[pt][c] += int(v);
	}

private:

	T table[PT_NB][COLOR_ALL];
};

// Stats�́Apc��sq�̏��Ɉړ�������w����ɑ΂���T�^�̒l��ۑ�����B
// T��Move�̂Ƃ��́A�w����ɑ΂���w����A���Ȃ킿�A"����"�ƂȂ�B
// T��Value�̂Ƃ��͎w����ɑ΂���X�R�A�ƂȂ�B���ꂪhistory table(HistoryStats��CounterMoveStats)
// ����Stats<CounterMoveStats>�́A���O�̎w����ɑ΂���A������w����ɑ΂���X�R�A�ł���B

typedef Stats<Point> MoveStats;
typedef Stats<Value> HistoryStats;

// -----------------------
//   �w����I�[�_�����O
// -----------------------

// �w�����i�K�I�ɐ������邽�߂Ɍ��݂ǂ̒i�K�ɂ��邩�̏�Ԃ�\���萔
enum Stages {

	MAIN_SEARCH_START,            // �u���\�̎�
	CAPTURES,                     // ����
	ESCAPES,                      // �������
	KILLERS,                      // KILLER�̎�
	LAST_AROUND,                  // ���O��̎���
	FOLLOW_AROUND,                // ����܂ł̒T�����̎�̎���̎�
	STOP,                         // �I�[
	BAD_FAR,                      // �����������Ȏ�
	OTHERS,                       // ����ȊO�̎�
	GOOD_FAR,                     // �����ǂ������Ȏ�i�܂�蔲���j

	
	//�ق��ɂ��AGOOD_FAR����ɂ���Ȃǂł��邩��
};
ENABLE_OPERATORS_ON(Stages); // ���̏�Ԃɂ��邽�߂ɃC���N�������g���g�������B

							 // �w����I�[�_�����O��
struct MovePicker
{
	// ���̃N���X�͎w���萶���o�b�t�@���傫���̂ŁA�R�s�[���Ďg���悤�Ȏg�����͋֎~�B
	MovePicker(const MovePicker&) = delete;
	MovePicker& operator=(const MovePicker&) = delete;

	// �ʏ�T������Ăяo�����Ƃ��p�B
	MovePicker(Position& pos_, Point ttMove_, Stack* ss_)
		: pos(pos_), ss(ss_), ttMove(ttMove_)
	{
		// ���̎w���萶���̒i�K
		stage = MAIN_SEARCH_START;
		stageBB = ZERO_BB;
		doneBB = ZERO_BB;
	}

	// ���̎w������ЂƂԂ�
	// �w���肪�s�����PT_NULL���Ԃ�B
	Point next_move();

private:

	Position& pos;

	Bitboard doneBB;

	Bitboard stageBB;

	// node stack
	Stack* ss;

	// �R���X�g���N�^�œn���ꂽttmove
	Point ttMove;

	// �w���萶���̒i�K
	Stages stage;

	// root�ǖʂł̍�����
	std::vector<RootMove>* rootMoves;

};