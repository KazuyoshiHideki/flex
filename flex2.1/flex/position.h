#pragma once
#include"stdafx.h"

//連の構造体
struct String
{
	//連の石のBitboard
	Bitboard StringBB;

	//連の呼吸点のBitboard
	Bitboard StringBreathBB;
	
	//親の連ナンバー 親がいなければPT_NULL
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

//局面
struct Position
{
	// コンストラクタでは平手の開始局面にする。
	Position() { clear(); set_start_board(); }

	// 初期化
	void clear() { std::memset(this, 0, sizeof(Position)); };

	//開始局面にセットする
	void set_start_board();

	//合法手のチェック
	bool legal(Point move) const;

	//着手
	void do_move(Point move);

	//着手（戻すことがある場合）
	void do_move(Point move, Stateinfo& si);

	//合法手チェック＆着手
	bool legal_do_move(Point move);

	//局面を戻す
	void undo_move(Point move, Stateinfo& si);

	//パス
	void do_null_move();

	//パスを戻す
	void undo_null_move();

	//終局局面の結果(勝者のColor)を返す
	Color result_game() const;

	//途中局面から予想される結果(勝者のColor)を返す。プレイアウトを途中で打ち切りするときに使う
	Color result_game_maybe(Bitboard* take);

	//手番を返す
	Color side_to_move() const { return side; }

	//開始局面からの手数を返す。
	int game_play() const { return moves; }

	//手番側のBitboardを返す
	Bitboard stones_side() const { return StoneBB[side]; }

	//c側のBitboardを返す
	Bitboard get_stones(Color c) const { return StoneBB[c]; }

	//両者の石が競っているBitboardを返す
	Bitboard stick() const;

	//石の呼吸点のBitboardを返す
	Bitboard breath(Color c) const;

	//石の周りの点のBitboardを返す
	Bitboard around(Color c) const;

	//一手前の着手を返す
	Point last_move() const { return (moves > 0) ? record[moves - 1] : PT_NULL; }

	//二手前の着手を返す
	Point last_last_move() const { return (moves > 1) ? record[moves - 2] : PT_NULL; }

	//連のルートを探す
	Point root_num(Point pt);

	//連のルートを探す（αβ探索中）
	Point root_num_c(Point pt) const;

	//局面の表示
	void print() const;

	//着手の表示
	void print_move() const;

	//着手可能な点のBitboard
	Bitboard poss_bb() const { return BrankBB ^ KoBB; }

	//合法手の点のBitboard
	Bitboard legal_bb();

	//着手可能な点の数を返す（非合法手も含む）
	int poss_count() const { Bitboard bb = poss_bb(); return bb.pop_count(); }

	//手番を変える
	void flip_color() { side = ~side; }

	//眼かどうか
	bool is_eye(Point pt);

	//ptの連を返す
	String* get_string(Point pt) { return &string[root_num_c(pt)]; }

	//直前手がとれるか　pt:（最後の）相手の手
	Bitboard move_catch(Point pt);

	//アタリかどうか　pt:（最後の）相手の手
	Bitboard move_escape(Point pt);

	//シチョウかどうか。簡易的なもの　pt:（最後の）相手の手
	//シチョウの場合、とれる次の手のBitboardが返る。シチョウでない場合、PT_NULLが返る
	Bitboard move_shichou(Point pt);

	Key get_key() const { return key; }

private:
	
	//空点が1のBitboard
	Bitboard BrankBB;
	
	//黒・白それぞれの石が置かれている点が1のBitboard
	Bitboard StoneBB[COLOR_NB];

	//コウ(もしくは着手禁止点)の存在するBitboard
	Bitboard KoBB;

	//手番
	Color side;

	//開始局面からの手数
	int moves;

	//開始局面からの着手歴
	Point record[MAX_MOVES];

	//連
	String string[PT_NB];

	//現局面に対応するStateinfo
	//Stateinfo* stateinfo;

	//局面のハッシュキー
	Key key;

};

bool shichou_search(Bitboard breath, Bitboard brank, Point pt);

