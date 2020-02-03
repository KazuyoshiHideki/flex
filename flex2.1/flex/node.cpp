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

//�m�[�h�̏����l��^����
//pos:�ǖʁ@pt:���O��@flag:����ɒT���؂��̂΂���
bool Node::set_node(Position& pos, Point pt, bool* flag)
{

	//����\�ȓ_��S���s�b�N�A�b�v����
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

	//key�̌v�Z
	key = pos.get_key();

	//TT�̃G���g���[��T��
	//��������΂����rave�l�Ƃ���
	*flag = false;
	if(*flag == false)
	{
		//�{�[�i�X��^����
		Bitboard bonus;
		do
		{
			//��O�D��܂łȂ�T���؂�L�΂�
			*flag = false;
			//���D��@���O����Ƃ�肪����
			bonus = pos.move_catch(pt);
			if (!(ALL_BB != bonus)) break;
			//���D��@�A�^���𓦂���肪����
			bonus = pos.move_escape(pt);
			if (!(ALL_BB != bonus)) break;
			//��O�D��@�V�`���E��������Ȃ��A�^����ł肪����
			bonus = pos.move_shichou(pt);
			if (!(ALL_BB != bonus)) break;

			*flag = false;
			//��l�D��@���O��̎���̎�
			bonus = AroundBB[pt];
		} while (0);

		//�e��ɑ΂���games��rate�̏����l��^����
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

	//�m�[�h�S�̂̃Q�[�����A������
	allGames = INIT_ALLGAMES;
	wins[BLACK] = 0;
	wins[WHITE] = 0;

	return true;
}

//ucb��rave�Ȃǂ��獡��̒����I��
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

		//����ő�l��T���Ă���
		//SSE���ߎg�����瑁������
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

	//NodeRecord���L�^����
	//���҂�update�Ɏg��

	best_move = BitToPoint[16 + (sel_i << 3) | sel_j];
	return best_move;
}

// ucb�l���X�V
void Node::update_ucb(Color winner, int d)
{

	//point�́A���==����(side == winner)�Ȃ�+1,�����łȂ��Ȃ�0
	float point = (float)(1 - (side ^ winner));

	allGames++;
	wins[winner]++;
}

//rave���X�V
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

//�m�[�h�̍őP���I��
//�T����ɍőP���I�񂾂�Apv������߂��肷��̂Ɏg��
Point Node::get_best_move() const
{
	int select_i, select_j;
	float count = 0.0;
	//�q�m�[�h�̂Ȃ��ōł����B�񐔁i�I���񐔁j�̑������̂��őP��Ƃ���
	//���������悢�Ƃ����
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

	//�I�񂾎q�̎��Ԃ�
	return BitToPoint[16 + (select_i << 3) | select_j];
}

//�m�[�h�̏���\������
void Node::node_print() const
{
	float r = (float)wins[side] / (wins[BLACK] + wins[WHITE]);
	cout << "�� " << wins[BLACK] << "�� ";
	cout << "�� " << wins[WHITE] << "�� " << endl;
	cout << "���� " << r << endl;

	Ucb kn = Ucb((float)UCB_PARA * log((float)allGames));
	for (int i = 0; i < 26; i++)
	{
		Ucb bonus;
		UcbSqrt(bonus, (kn / games[i]));
		Ucb ucb = rate[i] + bonus;
		//Ucb beta = raveGames[n] / (raveGames[n] + games[n] + ((raveGames[n] * games[n]) / PARA_UCB));
		//Ucb ucb_rave = (beta * raveRate[n]) + ((ONE_UCB[0] - beta) * ucb);
		ucb *= MASK[childBB.b[i+2]];

		//����ő�l��T���Ă���
		//SSE���ߎg�����瑁������
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
