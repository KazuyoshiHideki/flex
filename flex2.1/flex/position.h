#pragma once
#include"stdafx.h"

//�A�̍\����
struct String
{
	//�A�̐΂�Bitboard
	Bitboard StringBB;

	//�A�̌ċz�_��Bitboard
	Bitboard StringBreathBB;
	
	//�e�̘A�i���o�[ �e�����Ȃ����PT_NULL
	Point parent_num;


	String() {}

	String(Point pt)
	{
		StringBB = PointBB[pt]; 
		StringBreathBB = BreathBB[pt]; 
		parent_num = PT_NULL;
	}
};

const String ZERO_ST = String(PT_NULL);

struct Stateinfo
{
	Point myRoot[4];
	Point yourRoot[4];

	Point ko;
	
	Key key;

	Stateinfo()
	{
		std::memset(this, 0, sizeof(Stateinfo));
	}
};

//�ǖ�
struct Position
{
	// �R���X�g���N�^�ł͕���̊J�n�ǖʂɂ���B
	Position() { clear(); set_start_board(); }

	// ������
	void clear() { std::memset(this, 0, sizeof(Position)); };

	//�J�n�ǖʂɃZ�b�g����
	void set_start_board();

	//���@��̃`�F�b�N
	bool legal(Point move) const;

	//����
	void do_move(Point move);

	//����i�߂����Ƃ�����ꍇ�j
	void do_move(Point move, Stateinfo& si);

	//���@��`�F�b�N������
	bool legal_do_move(Point move);

	//�ǖʂ�߂�
	void undo_move(Point move, Stateinfo& si);

	//�p�X
	void do_null_move();

	//�p�X��߂�
	void undo_null_move();

	//�I�ǋǖʂ̌���(���҂�Color)��Ԃ�
	Color result_game() const;

	//�r���ǖʂ���\�z����錋��(���҂�Color)��Ԃ��B�v���C�A�E�g��r���őł��؂肷��Ƃ��Ɏg��
	Color result_game_maybe(Bitboard* take);

	//��Ԃ�Ԃ�
	Color side_to_move() const { return side; }

	//�J�n�ǖʂ���̎萔��Ԃ��B
	int game_play() const { return moves; }

	//��ԑ���Bitboard��Ԃ�
	Bitboard stones_side() const { return StoneBB[side]; }

	//c����Bitboard��Ԃ�
	Bitboard get_stones(Color c) const { return StoneBB[c]; }

	//���҂̐΂������Ă���Bitboard��Ԃ�
	Bitboard stick() const;

	//�΂̌ċz�_��Bitboard��Ԃ�
	Bitboard breath(Color c) const;

	//�΂̎���̓_��Bitboard��Ԃ�
	Bitboard around(Color c) const;

	//���O�̒����Ԃ�
	Point last_move() const { return (moves > 0) ? record[moves - 1] : PT_NULL; }

	//���O�̒����Ԃ�
	Point last_last_move() const { return (moves > 1) ? record[moves - 2] : PT_NULL; }

	//�A�̃��[�g��T��
	Point root_num(Point pt);

	//�A�̃��[�g��T���i�����T�����j
	Point root_num_c(Point pt) const;

	//�ǖʂ̕\��
	void print() const;

	//����̕\��
	void print_move() const;

	//����\�ȓ_��Bitboard
	Bitboard poss_bb() const { return BrankBB ^ KoBB; }

	//���@��̓_��Bitboard
	Bitboard legal_bb();

	//����\�ȓ_�̐���Ԃ��i�񍇖@����܂ށj
	int poss_count() const { Bitboard bb = poss_bb(); return bb.pop_count(); }

	//��Ԃ�ς���
	void flip_color() { side = ~side; }

	//�Ⴉ�ǂ���
	bool is_eye(Point pt);

	//pt�̘A��Ԃ�
	String* get_string(Point pt) { return &string[root_num_c(pt)]; }

	//���O�肪�Ƃ�邩�@pt:�i�Ō�́j����̎�
	Bitboard move_catch(Point pt);

	//�A�^�����ǂ����@pt:�i�Ō�́j����̎�
	Bitboard move_escape(Point pt);

	//�V�`���E���ǂ����B�ȈՓI�Ȃ��́@pt:�i�Ō�́j����̎�
	//�V�`���E�̏ꍇ�A�Ƃ�鎟�̎��Bitboard���Ԃ�B�V�`���E�łȂ��ꍇ�APT_NULL���Ԃ�
	Bitboard move_shichou(Point pt);

	Key get_key() const { return key; }

private:
	
	//��_��1��Bitboard
	Bitboard BrankBB;
	
	//���E�����ꂼ��̐΂��u����Ă���_��1��Bitboard
	Bitboard StoneBB[COLOR_NB];

	//�R�E(�������͒���֎~�_)�̑��݂���Bitboard
	Bitboard KoBB;

	//���
	Color side;

	//�J�n�ǖʂ���̎萔
	int moves;

	//�J�n�ǖʂ���̒����
	Point record[MAX_MOVES];

	//�A
	String string[PT_NB];

	//���ǖʂɑΉ�����Stateinfo
	//Stateinfo* stateinfo;

	//�ǖʂ̃n�b�V���L�[
	Key key;

};

bool shichou_search(Bitboard breath, Bitboard brank, Point pt);

