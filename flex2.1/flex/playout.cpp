#include "stdafx.h"
#include "playout.h"

#include"time.h"

using namespace std;

//モンテカルロ木はグローバルに置く
Color think_side = NEW;

//Playoutのコンストラクタ
Playout::Playout(Position& thinkPos)
{
	pos = thinkPos;
	allMovesBB[BLACK] = ZERO_BB;
	allMovesBB[WHITE] = ZERO_BB;
	takeStoneBB[BLACK] = ZERO_BB;
	takeStoneBB[WHITE] = ZERO_BB;
}

//最善手を考える
//ここで使うアルゴリズムを決める
Point Playout::move_playout()
{

	Point move = random_play();
	//pv_print();

	return move;
}

//プレイアウトの質のチェック
void Playout::playout_check()
{
	Color winner_maybe = BLACK;
	int loop;
	for (loop = pos.game_play(); loop < MAX_MOVES; loop++)
		//for (int loop = 0; loop < 169; loop++)
	{
		Point move = PT_NULL;

		//確率調整したランダムで手を決める
		move = random_play();

		//両者パスもしくは投了なら抜ける
		if (move == PASS && pos.last_move() == PASS) break;
		if (move == RESIGN) break;

		//raveのupdate用に着手点をBitboardに記録
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

//合法手の手をランダムに１手進める
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

//プレイアウト中にランダムに着手を決める
//ランダムではあるものの、よりよさそうな手が選ばれやすくするならプレイアウトの質は高まる
Point Playout::random_play()
{
	Bitboard brank_bb = pos.poss_bb() ^ (takeStoneBB[BLACK] | takeStoneBB[WHITE]);
	Point move;
	
	/*
	//これは途中でプレイアウトを打ち切る場合、不要。最後まで打つ場合数パーセントほど早くなる。
	//周りがすべて自分の石か壁の場合、眼なので消す
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

	//直前手をとれるなら問答無用でその手を選ぶ
	move = pick_random(pos.move_catch(pos.last_move()) & brank_bb);
	if (move != PT_NULL)
		return move;
	
	//直前手の周りでよさそうな手があれば高確率で選ぶ（逃げるもしくは競っている点）
	Bitboard stick = pos.stick();
	rand_bb &= AroundBB[pos.last_move()] & stick;
	rand_bb &= Bitboard(rand64());
	rand_bb |= pos.move_escape(pos.last_move());

	//着手を選ぶ
	move = pick_random(rand_bb);
	if (move != PT_NULL)
		return move;

	//盤面全体でよさそうな手があれば高確率で選ぶ（競っている点かつ１線でない）
	brank_bb ^= rand_bb;
	rand_bb = brank_bb;
	rand_bb ^= pos.breath(BLACK) | pos.breath(WHITE);
	rand_bb |= stick;
	rand_bb &= MASK1_BB;
	rand_bb &= Bitboard(rand64()&rand64());
	rand_bb &= brank_bb;
	
	//着手を選ぶ
	move = pick_random(rand_bb);
	if (move != PT_NULL)
		return move;

	//疎なBitboardでもし着手できる点が一つもなかったら
	//すべての点で探す
	brank_bb ^= rand_bb;
	move = pick_random(brank_bb);
	if (move != PT_NULL)
		return move;

	//もし着手できる点が一つもなかったらPASS
	return PASS;
}


//プレイアウト
Color Playout::playout()
{
	for (int loop = pos.game_play(); loop < MAX_MOVES; loop++)
	//for (int loop = 0; loop < 200 && pos.game_play() < MAX_MOVES; loop++)
	{
		Point move = PT_NULL;

		//確率調整したランダムで手を決める
		move = random_play();

		//両者パスもしくは投了なら抜ける
		if (move == PASS && pos.last_move() == PASS) break;
		if (move == RESIGN) break;

		//raveのupdate用に着手点をBitboardに記録
		Color color = pos.side_to_move();
		allMovesBB[color] |= (PointBB[move] ^ allMovesBB[~color]);

		//とった石のBitboardの更新のための情報
		Color yourSide = ~pos.side_to_move();
		Bitboard yourStone = pos.get_stones(yourSide);

		pos.do_move(move);
		
		//とった石のBitboardを更新
		takeStoneBB[yourSide] |= yourStone ^ pos.get_stones(yourSide);
	}
	//pos.print();
	//std::cout << dec << pos.game_play();
	
	return pos.result_game_maybe(takeStoneBB);
}





