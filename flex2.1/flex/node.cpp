#include"stdafx.h"
#include "node.h"

#define UcbSqrt(b,b1)   (b).mf = _mm256_sqrt_ps( (b1).mf )

using namespace std;

Ucb MASK[256];

void Ucts::init()
{
	for (int i = 0; i < 256; i++) for (int j = 0; j < 8; j++)
		MASK[i].p[j] = (float)((i >> j) & 1);	
	
}

//ノードの初期値を与える
//pos:局面　pt:直前手　flag:さらに探索木をのばすか
bool Node::set_node(Position& pos, Point pt, bool* flag)
{

	//着手可能な点を全部ピックアップする
	childBB |= pos.poss_bb();
	Bitboard bb = childBB;
	child_num = 0;
	while (Point pt = bb.pop())
	{
		if (pos.legal(pt))
			child_num++;
		else {
			childBB ^= PointBB[pt];
		}
	}

	if (child_num == 0)
	{
		*flag = false;
		return false;
	}

	//keyの計算
	key = pos.get_key();

	//TTのエントリーを探す
	//もしあればそれをrave値とする
	*flag = false;
	if(*flag == false)
	{
		//ボーナスを与える
		Bitboard bonus;
		do
		{
			//第三優先までなら探索木を伸ばす
			*flag = false;
			//第一優先　直前手をとる手がある
			bonus = pos.move_catch(pt);
			if (!(ALL_BB != bonus)) break;
			//第二優先　アタリを逃げる手がある
			bonus = pos.move_escape(pt);
			if (!(ALL_BB != bonus)) break;
			//第三優先　シチョウかもしれないアタリを打つ手がある
			bonus = pos.move_shichou(pt);
			if (!(ALL_BB != bonus)) break;

			*flag = false;
			//第四優先　直前手の周りの手
			bonus = AroundBB[pt];
		} while (0);

		//各手に対してgamesとrateの初期値を与える
		for (int i = 0; i < 26; i++)
		{
			Ucb mask = MASK[bonus.b[i + 2]];
			games[i] = GAMES + BONUS_GAMES * mask;
			rate[i] = RATE + BONUS_RATE * mask;
			raveGames[i] = RAVE_GAMES + BONUS_GAMES * mask;
			raveRate[i] = RAVE_RATE + BONUS_RATE * mask;
		}
	}

	side = pos.side_to_move();

	//ノード全体のゲーム数、勝利数
	allGames = INIT_ALLGAMES;
	wins[BLACK] = 0;
	wins[WHITE] = 0;

	return true;
}

//ucbやraveなどから今回の着手を選ぶ
Point Node::select()
{
	//Ucb kn = Ucb((float)UCB_PARA * log((float)allGames));

	float maxUcb = -99.0;
	int sel_i = 0;
	int sel_j = 0;

	for (int i = 0; i < 26; i++)
	{
		//Ucb bonus;
		//UcbSqrt(bonus, (kn / games[i]));
		Ucb ucb = rate[i];// +bonus;
		Ucb beta = raveGames[i] / (raveGames[i] + games[i] + ((raveGames[i] * games[i]) / BETA_UCB));
		Ucb ucb_rave = (beta * raveRate[i]) + ((ONE_UCB[0] - beta) * ucb);
		ucb_rave *= MASK[childBB.b[i + 2]];
		ucb_rave += MASK[childBB.b[i + 2]];

		//一つずつ最大値を探していく
		//SSE命令使ったら早いかも
		for (int j = 0; j < 8; j++)
		{
			if (ucb_rave.p[j] > maxUcb)
			{
				maxUcb = ucb_rave.p[j];
				sel_i = i;
				sel_j = j;
			}
		}
	}

	//NodeRecordを記録する
	//勝者のupdateに使う

	best_move = BitToPoint[16 + (sel_i << 3) | sel_j];
	return best_move;
}

// ucb値を更新
void Node::update_ucb(Color winner, int d)
{

	//pointは、手番==勝者(side == winner)なら+1,そうでないなら0
	float point = (float)(1 - (side ^ winner));

	allGames++;
	wins[winner]++;
}

//raveを更新
void Node::update_rave(Color winner, Bitboard allMoves)
{
	Bitboard bb = childBB & allMoves;
	int point = (int)side ^ (int)winner;
	for (int i = 0; i < 26; i++)
	{
		raveRate[i] *= raveGames[i];
		raveRate[i] += (ONE_UCB[point] * MASK[bb.b[i+2]]);
		raveGames[i] += MASK[bb.b[i+2]];
		raveRate[i] /= raveGames[i];
	}
}

//ノードの最善手を選ぶ
//探索後に最善手を選んだり、pv列を求めたりするのに使う
Point Node::get_best_move() const
{
	int select_i, select_j;
	float count = 0.0;
	//子ノードのなかで最も到達回数（選択回数）の多いものを最善手とする
	//勝率よりもよいとされる
	for (int i = 0; i < 26; i++)
	{
		Ucb game_num = games[i] * MASK[childBB.b[i + 2]];
		for (int j = 0; j < 8; j++)
		{
			if (count < game_num.p[j])
			{
				count = game_num.p[j];
				select_i = i;
				select_j = j;
			}
		}
	}

	//選んだ子の手を返す
	return BitToPoint[16 + (select_i << 3) | select_j];
}

//ノードの情報を表示する
void Node::node_print() const
{
	float r = (float)wins[side] / (wins[BLACK] + wins[WHITE]);
	cout << "黒 " << wins[BLACK] << "勝 ";
	cout << "白 " << wins[WHITE] << "勝 " << endl;
	cout << "勝率 " << r << endl;

	Ucb kn = Ucb((float)UCB_PARA * log((float)allGames));
	for (int i = 0; i < 26; i++)
	{
		Ucb bonus;
		UcbSqrt(bonus, (kn / games[i]));
		Ucb ucb = rate[i] + bonus;
		//Ucb beta = raveGames[n] / (raveGames[n] + games[n] + ((raveGames[n] * games[n]) / PARA_UCB));
		//Ucb ucb_rave = (beta * raveRate[n]) + ((ONE_UCB[0] - beta) * ucb);
		ucb *= MASK[childBB.b[i+2]];

		//一つずつ最大値を探していく
		//SSE命令使ったら早いかも
		for (int j = 0; j < 8; j++)
		{
			if (ucb.p[j] == 0.0) continue;

			Point move = BitToPoint[16 + (i << 3) | j];
			cout << "move = ";
			cout << hex << (1 + (move - 1) / 13) << " ";
			cout << hex << (1 + (move - 1) % 13) << " " << dec;
			cout << "ucb = " << ucb.p[j] << " ";
			cout << "games = " << games[i].p[j] << " ";
			cout << "rate = " << rate[i].p[j] << " ";
			cout << " RAVE games = " << (int)raveGames[i].p[j] << " ";
			cout << "rate = " << raveRate[i].p[j] << " ";
			cout << endl;
		}
	}
}


void Node::koho(Position& pos) const
{
	Bitboard bb = childBB;
	for (int num = 0; num < 16; num++)
	{
		float max = 0.0;
		int sel_i = 0;
		int sel_j = 0;
		for (int i = 0; i < 26; i++)
		{
			Ucb g = raveRate[i];
			g += Ucb((float)0.01) * MASK[MASK1F_BB.b[i + 2]];
			g += Ucb((float)0.01) * MASK[MASK1R_BB.b[i + 2]];
			g += Ucb((float)0.01) * MASK[UNMASK234F_BB.b[i + 2]];
			g += Ucb((float)0.01) * MASK[UNMASK234R_BB.b[i + 2]];
			g += Ucb((float)0.03) * MASK[AroundBB[pos.last_move()].b[i + 2]];
			g += Ucb((float)0.02) * MASK[AroundBB[pos.last_last_move()].b[i + 2]];
			g += Ucb((float)0.01) * MASK[pos.around(pos.side_to_move()).b[i + 2]];
			g += Ucb((float)0.03) * MASK[pos.around(~pos.side_to_move()).b[i + 2]];
			g *= MASK[bb.b[i + 2]];
			for (int j = 0; j < 8; j++)
			{
				if (max < g.p[j])
				{
					max = g.p[j];
					sel_i = i;
					sel_j = j;
				}
			}
		}
		Point move = BitToPoint[16 + (sel_i << 3) | sel_j];
		cout << "move = ";
		cout << hex << (1 + (move - 1) / 13) << " ";
		cout << hex << (1 + (move - 1) % 13) << " " << dec;
		cout << " games = " << (int)games[sel_i].p[sel_j] << " ";
		cout << " RAVE games = " << (int)raveGames[sel_i].p[sel_j] << " ";
		cout << "rate = " << raveRate[sel_i].p[sel_j] << " ";
		cout << endl;
		bb ^= PointBB[move];
	}
}
