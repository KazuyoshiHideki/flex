#pragma once
#include"stdafx.h"
#include"position.h"
#include"time.h"



struct Playout {

	Playout(Position& thinkPos);

	//まずこの関数が呼び出される
	Point move_playout();

	//プレイアウトの精度を見るのに使える
	void playout_check();

	//Bitboardの中からランダムに着手点を決める
	Point pick_random(Bitboard bb);

	//ランダムゲーム
	Point random_play();

	//プレイアウト
	Color playout();

	Bitboard all_moves(Color c) { return allMovesBB[c]; }


private:

	//作業用局面
	Position pos;

	//rave用Bitboard 打った手の記録
	Bitboard allMovesBB[COLOR_ALL];

	//とった石
	Bitboard takeStoneBB[COLOR_ALL];

};

extern Color think_side;