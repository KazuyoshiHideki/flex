#pragma once
#include"stdafx.h"

#define NODE_MAX 100
#define INIT_GAMES 1
#define INIT_RATE 0.0
#define INIT_RAVE_GAMES 1
#define INIT_RAVE_RATE 0.0
#define INIT_BONUS_GAMES 1
#define INIT_BONUS_RATE 1.0
#define UCB_PARA 0.0
#define BETA_PARA 3000.0
#define INIT_ALLGAMES 1

namespace Ucts
{
	void init();
}

struct Ucb
{
	union
	{
		float    p[8];
		__m256   mf;
	};

	Ucb() {}

	//float�^�̃f�[�^��8���ׂ�
	Ucb(float i) { mf = _mm256_broadcast_ss(&i); }

	//float�^�̃f�[�^��8���ׂ�
	void set(float i) { mf = _mm256_broadcast_ss(&i); }

	//����^���Z�q
	//Ucb& operator = (const Ucb& rhs) { _mm256_store_ps(this->p, rhs.mf); return *this; }
	Ucb& operator += (const Ucb& b1) { this->mf = _mm256_add_ps(mf, b1.mf); return *this; }
	Ucb& operator -= (const Ucb& b1) { this->mf = _mm256_sub_ps(mf, b1.mf); return *this; }
	Ucb& operator *= (const Ucb& b1) { this->mf = _mm256_mul_ps(mf, b1.mf); return *this; }
	Ucb& operator /= (const Ucb& b1) { this->mf = _mm256_div_ps(mf, b1.mf); return *this; }


	//2�����Z�q
	Ucb operator + (const Ucb& rhs) const { return Ucb(*this) += rhs; }
	Ucb operator - (const Ucb& rhs) const { return Ucb(*this) -= rhs; }
	Ucb operator * (const Ucb& rhs) const { return Ucb(*this) *= rhs; }
	Ucb operator / (const Ucb& rhs) const { return Ucb(*this) /= rhs; }
};

const Ucb ZERO_UCB = Ucb((float)0.0);
const Ucb ONE_UCB[2] = { Ucb((float)1.0) , Ucb((float)(0.0)) };
const Ucb BETA_UCB = Ucb((float)BETA_PARA);
const Ucb BONUS_GAMES = Ucb((float)INIT_BONUS_GAMES);
const Ucb BONUS_RATE = Ucb((float)INIT_BONUS_RATE);
const Ucb GAMES = Ucb((float)INIT_GAMES);
const Ucb RATE = Ucb((float)INIT_RATE);
const Ucb RAVE_GAMES = Ucb((float)INIT_RAVE_GAMES);
const Ucb RAVE_RATE = Ucb((float)INIT_RAVE_RATE);

//�r�b�g��ɉ�����float�^��8����
extern Ucb MASK[256];

//�m�[�h�̍\����
struct Node
{
	Node() {}

	//�m�[�h�̏����l��^����
	//pos:�ǖʁ@pt:���O�� flag:����ɒT���؂�L�΂���
	bool set_node(Position& pos, Point pt, bool* flag);

	//UCB�l�Ȃǂ�����I��
	Point select();

	//ucb�l���X�V
	void update_ucb(Color winner, int d);

	//rave���X�V
	void update_rave(Color winner, Bitboard allMoves);

	//���[�̃m�[�h�i�t�j��
	bool is_leef() const { return (child_node[best_move] == NULL)? true : false; }

	//�I��������̎q�m�[�h��Ԃ�
	Node* get_child_node(Point pt) const { return child_node[pt]; }

	//�q�m�[�h�ւ̃|�C���^���Z�b�g
	void set_child_node(Node* node, Point pt) { child_node[pt] = node; }

	//�e�m�[�h�̃|�C���^���Z�b�g
	void set_parent_node(Node* node) { parent_node = node; }

	//�[����Ԃ�
	int get_depth() const { return depth; }

	//�őP���I��
	Point get_best_move() const;

	int get_allGames() const { return allGames; }

	Bitboard get_childBB() const { return childBB; }

	//�m�[�h����\��
	void node_print() const;

	//�m�[�h�̏�����Ԃ�
	float get_rate() const { return (float)wins[side] / (wins[BLACK] + wins[WHITE]); }

	//key��Ԃ�
	Key get_key() const { return key; }

	void koho(Position& pos) const;

	// 64bit�̃p�[�g�ɂ��U����Ucb�\���̂�����
	Ucb games[26];
	Ucb rate[26];
	Ucb raveGames[26];
	Ucb raveRate[26];

private:

	//�q�m�[�h�̃r�b�g��1��Bitboard
	Bitboard childBB;

	//�q�m�[�h�̐�
	uint32_t child_num;

	//�m�[�h�̖K���
	uint32_t allGames;

	//�e�F�̏�����
	uint32_t wins[COLOR_ALL];

	//�q�m�[�h�̃|�C���^
	Node* child_node[PT_NB];

	//�e�m�[�h�̃|�C���^
	Node* parent_node;

	//�ŐV�̑I�񂾎�
	Point best_move;

	//���
	Color side;

	//�[��
	uint32_t depth;

	//Key
	Key key;

};

