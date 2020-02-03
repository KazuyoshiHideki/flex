#include "stdafx.h"
#include "playout.h"

#include"time.h"

using namespace std;

//�����e�J�����؂̓O���[�o���ɒu��
Color think_side = NEW;

//Playout�̃R���X�g���N�^
Playout::Playout(Position& thinkPos)
{
	pos = thinkPos;
	allMovesBB[BLACK] = ZERO_BB;
	allMovesBB[WHITE] = ZERO_BB;
	takeStoneBB[BLACK] = ZERO_BB;
	takeStoneBB[WHITE] = ZERO_BB;
}

//�őP����l����
//�����Ŏg���A���S���Y�������߂�
Point Playout::move_playout()
{

	Point move = random_play();
	//pv_print();

	return move;
}

//�v���C�A�E�g�̎��̃`�F�b�N
void Playout::playout_check()
{
	Color winner_maybe = BLACK;
	int loop;
	for (loop = pos.game_play(); loop < MAX_MOVES; loop++)
		//for (int loop = 0; loop < 169; loop++)
	{
		Point move = PT_NULL;

		//�m���������������_���Ŏ�����߂�
		move = random_play();

		//���҃p�X�������͓����Ȃ甲����
		if (move == PASS && pos.last_move() == PASS) break;
		if (move == RESIGN) break;

		//rave��update�p�ɒ���_��Bitboard�ɋL�^
		Color color = pos.side_to_move();
		allMovesBB[color] |= (PointBB[move] ^ allMovesBB[~color]);

		pos.print();
		pos.do_move(move);

		if (loop == 200)
		{
			winner_maybe = pos.result_game_maybe(takeStoneBB);
			break;
		}
	}
	int a;
	std::cin >> a;
}

//���@��̎�������_���ɂP��i�߂�
Point Playout::pick_random(Bitboard bb)
{
	Bitboard legalBB = bb;
	int n = legalBB.pop_count();

	while (n)
	{
		Point pt;
		Bitboard b = legalBB;
		uint64_t r = 1 + rand64() % n;

		while (r--) pt = b.pop();

		if (pos.is_eye(pt) == false && pos.legal(pt))
			return pt;

		legalBB ^= PointBB[pt];
		n--;
	}
	return PT_NULL;
}

//�v���C�A�E�g���Ƀ����_���ɒ�������߂�
//�����_���ł͂�����̂́A���悳�����Ȏ肪�I�΂�₷������Ȃ�v���C�A�E�g�̎��͍��܂�
Point Playout::random_play()
{
	Bitboard brank_bb = pos.poss_bb() ^ (takeStoneBB[BLACK] | takeStoneBB[WHITE]);
	Point move;
	
	/*
	//����͓r���Ńv���C�A�E�g��ł��؂�ꍇ�A�s�v�B�Ō�܂őłꍇ���p�[�Z���g�قǑ����Ȃ�B
	//���肪���ׂĎ����̐΂��ǂ̏ꍇ�A��Ȃ̂ŏ���
	Bitboard bb = ALL_BB;
	Bitboard bb1;
	Bitboard stone = WALL_BB | pos.stones_color(pos.side_to_move());
	bb &= stone >> 1;
	bb &= stone << 1;
	ShiftS(bb1, stone);
	bb &= bb1;
	bb &= bb1 << 1;
	bb &= bb1 >> 1;
	ShiftN(bb1, stone);
	bb &= bb1;
	bb &= bb1 << 1;
	bb &= bb1 >> 1;
	brank_bb ^= bb;
	*/

	Bitboard rand_bb = brank_bb;

	//���O����Ƃ��Ȃ�ⓚ���p�ł��̎��I��
	move = pick_random(pos.move_catch(pos.last_move()) & brank_bb);
	if (move != PT_NULL)
		return move;
	
	//���O��̎���ł悳�����Ȏ肪����΍��m���őI�ԁi������������͋����Ă���_�j
	Bitboard stick = pos.stick();
	rand_bb &= AroundBB[pos.last_move()] & stick;
	rand_bb &= Bitboard(rand64());
	rand_bb |= pos.move_escape(pos.last_move());

	//�����I��
	move = pick_random(rand_bb);
	if (move != PT_NULL)
		return move;

	//�ՖʑS�̂ł悳�����Ȏ肪����΍��m���őI�ԁi�����Ă���_���P���łȂ��j
	brank_bb ^= rand_bb;
	rand_bb = brank_bb;
	rand_bb ^= pos.breath(BLACK) | pos.breath(WHITE);
	rand_bb |= stick;
	rand_bb &= MASK1_BB;
	rand_bb &= Bitboard(rand64()&rand64());
	rand_bb &= brank_bb;
	
	//�����I��
	move = pick_random(rand_bb);
	if (move != PT_NULL)
		return move;

	//�a��Bitboard�ł�������ł���_������Ȃ�������
	//���ׂĂ̓_�ŒT��
	brank_bb ^= rand_bb;
	move = pick_random(brank_bb);
	if (move != PT_NULL)
		return move;

	//��������ł���_������Ȃ�������PASS
	return PASS;
}


//�v���C�A�E�g
Color Playout::playout()
{
	for (int loop = pos.game_play(); loop < MAX_MOVES; loop++)
	//for (int loop = 0; loop < 200 && pos.game_play() < MAX_MOVES; loop++)
	{
		Point move = PT_NULL;

		//�m���������������_���Ŏ�����߂�
		move = random_play();

		//���҃p�X�������͓����Ȃ甲����
		if (move == PASS && pos.last_move() == PASS) break;
		if (move == RESIGN) break;

		//rave��update�p�ɒ���_��Bitboard�ɋL�^
		Color color = pos.side_to_move();
		allMovesBB[color] |= (PointBB[move] ^ allMovesBB[~color]);

		//�Ƃ����΂�Bitboard�̍X�V�̂��߂̏��
		Color yourSide = ~pos.side_to_move();
		Bitboard yourStone = pos.get_stones(yourSide);

		pos.do_move(move);
		
		//�Ƃ����΂�Bitboard���X�V
		takeStoneBB[yourSide] |= yourStone ^ pos.get_stones(yourSide);
	}
	//pos.print();
	//std::cout << dec << pos.game_play();
	
	return pos.result_game_maybe(takeStoneBB);
}





