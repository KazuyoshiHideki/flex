#pragma once
#include"stdafx.h"
#include"position.h"
#include"time.h"



struct Playout {

	Playout(Position& thinkPos);

	//�܂����̊֐����Ăяo�����
	Point move_playout();

	//�v���C�A�E�g�̐��x������̂Ɏg����
	void playout_check();

	//Bitboard�̒����烉���_���ɒ���_�����߂�
	Point pick_random(Bitboard bb);

	//�����_���Q�[��
	Point random_play();

	//�v���C�A�E�g
	Color playout();

	Bitboard all_moves(Color c) { return allMovesBB[c]; }


private:

	//��Ɨp�ǖ�
	Position pos;

	//rave�pBitboard �ł�����̋L�^
	Bitboard allMovesBB[COLOR_ALL];

	//�Ƃ�����
	Bitboard takeStoneBB[COLOR_ALL];

};

extern Color think_side;