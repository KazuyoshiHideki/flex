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

	//float型のデータを8個並べる
	Ucb(float i) { mf = _mm256_broadcast_ss(&i); }

	//float型のデータを8個並べる
	void set(float i) { mf = _mm256_broadcast_ss(&i); }

	//代入型演算子
	//Ucb& operator = (const Ucb& rhs) { _mm256_store_ps(this->p, rhs.mf); return *this; }
	Ucb& operator += (const Ucb& b1) { this->mf = _mm256_add_ps(mf, b1.mf); return *this; }
	Ucb& operator -= (const Ucb& b1) { this->mf = _mm256_sub_ps(mf, b1.mf); return *this; }
	Ucb& operator *= (const Ucb& b1) { this->mf = _mm256_mul_ps(mf, b1.mf); return *this; }
	Ucb& operator /= (const Ucb& b1) { this->mf = _mm256_div_ps(mf, b1.mf); return *this; }


	//2項演算子
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

//ビット列に応じてfloat型が8つ並ぶ
extern Ucb MASK[256];

//ノードの構造体
struct Node
{
	Node() {}

	//ノードの初期値を与える
	//pos:局面　pt:直前手 flag:さらに探索木を伸ばすか
	bool set_node(Position& pos, Point pt, bool* flag);

	//UCB値などから手を選ぶ
	Point select();

	//ucb値を更新
	void update_ucb(Color winner, int d);

	//raveを更新
	void update_rave(Color winner, Bitboard allMoves);

	//末端のノード（葉）か
	bool is_leef() const { return (child_node[best_move] == NULL)? true : false; }

	//選択した手の子ノードを返す
	Node* get_child_node(Point pt) const { return child_node[pt]; }

	//子ノードへのポインタをセット
	void set_child_node(Node* node, Point pt) { child_node[pt] = node; }

	//親ノードのポインタをセット
	void set_parent_node(Node* node) { parent_node = node; }

	//深さを返す
	int get_depth() const { return depth; }

	//最善手を選ぶ
	Point get_best_move() const;

	int get_allGames() const { return allGames; }

	Bitboard get_childBB() const { return childBB; }

	//ノード情報を表示
	void node_print() const;

	//ノードの勝率を返す
	float get_rate() const { return (float)wins[side] / (wins[BLACK] + wins[WHITE]); }

	//keyを返す
	Key get_key() const { return key; }

	void koho(Position& pos) const;

	// 64bitのパートにつき６個ずつUcb構造体を持つ
	Ucb games[26];
	Ucb rate[26];
	Ucb raveGames[26];
	Ucb raveRate[26];

private:

	//子ノードのビットが1のBitboard
	Bitboard childBB;

	//子ノードの数
	uint32_t child_num;

	//ノードの訪問回数
	uint32_t allGames;

	//各色の勝利数
	uint32_t wins[COLOR_ALL];

	//子ノードのポインタ
	Node* child_node[PT_NB];

	//親ノードのポインタ
	Node* parent_node;

	//最新の選んだ手
	Point best_move;

	//手番
	Color side;

	//深さ
	uint32_t depth;

	//Key
	Key key;

};

