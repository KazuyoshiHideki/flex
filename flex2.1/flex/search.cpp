#include"stdafx.h"
#include"search.h"

using namespace std;

bool check = false;
int evalCount = 0;

// PV line���R�s�[����B
// pv �� move(1��) + childPv(������,����MOVE_NONE)���R�s�[����B
// �ԕ��Ƃ��Ė�����MOVE_NONE�ɂ��邱�ƂɂȂ��Ă���B
inline void update_pv(Point* pv, Point move, Point* childPv) {

	for (*pv++ = move; childPv && *childPv != PT_NULL; )
		*pv++ = *childPv++;
	*pv = PT_NULL;
}

void Search::set_rootMoves()
{
	Bitboard bb;
	bb = UNMASK234_BB;
	bb |= rootPos.stick();
	bb &= rootPos.legal_bb();
	while (Point move = bb.pop())
	{
		rootMoves.push_back(RootMove(move));
	}
}

Value evaluate(Position& pos, Point& bestMove)
{

	Value eval = 0;
	Bitboard brank = pos.poss_bb();
	Ucb games[2][26];
	Ucb rate[26];
	for (int i = 0; i < 26; i++)
	{
		games[0][i] = GAMES;
		games[1][i] = GAMES;
	}

	for (int loop = 0; loop < MAX_PLAYOUTS; loop++)
	{
		Playout po(pos);
		Color winner = po.playout();
		eval += (int)~winner; 

		Color color = pos.side_to_move();
		Bitboard bb = brank & po.all_moves(color);
		for (int i = 0; i < 26; i++)
		{
			games[winner ^ color][i] += MASK[bb.b[i + 2]];
		}
	}
	while (true)
	{
		float max = 0.0;
		int sel_i = 0;
		int sel_j = 0;
		for (int i = 0; i < 26; i++)
		{
			Ucb g = games[0][i] / (games[0][i] + games[1][i]);

			g += Ucb((float)0.01) * MASK[MASK1F_BB.b[i + 2]];
			g += Ucb((float)0.01) * MASK[MASK1R_BB.b[i + 2]];
			g += Ucb((float)0.01) * MASK[UNMASK234F_BB.b[i + 2]];
			g += Ucb((float)0.01) * MASK[UNMASK234R_BB.b[i + 2]];
			g += Ucb((float)0.03) * MASK[AroundBB[pos.last_move()].b[i + 2]];
			g += Ucb((float)0.02) * MASK[AroundBB[pos.last_last_move()].b[i + 2]];
			g += Ucb((float)0.01) * MASK[pos.around(pos.side_to_move()).b[i + 2]];
			g += Ucb((float)0.03) * MASK[pos.around(~pos.side_to_move()).b[i + 2]];
			g *= MASK[brank.b[i + 2]];
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

		if (pos.legal(move) && !pos.is_eye(move))
		{
			bestMove = move;
			break;
		}
		brank ^= PointBB[move];
	}
	eval -= MAX_PLAYOUTS >> 1;
	return (pos.side_to_move() == BLACK) ? eval : -eval;
}

inline void update_stats(const Position& pos, Stack* ss, Point move)
{
	//   killer��update

	// killer 2�{�����Ȃ��̂�[0]�ƈႤ�Ȃ炢�܂�[0]��[1]�ɍ~�i������[0]�ƍ����ւ�
	if (ss->killers[0] != move)
	{
		ss->killers[1] = ss->killers[0];
		ss->killers[0] = move;
	}
}

template <NodeType NT>
Value Search::search(Position& pos, Stack* ss, Value alpha, Value beta, Depth depth, bool cutNode)
{
	// -----------------------
	// node�̎�ނƕϐ��̐錾
	// -----------------------

	// root node�ł��邩
	const bool RootNode = NT == Root;

	// PV node�ł��邩(root node��PV node�Ɋ܂܂��)
	const bool PvNode = NT == PV || NT == Root;

	//�ǖʕ]���l
	Value eval;

	// ����node�����PV line(�ǂ݋�)
	Point pv[MAX_PLY + 1];

	// ���̋ǖʂ�do_move()���ꂽ���@��̐�
	int moveCount = 0;

	// LMR�̂Ƃ���fail high���N����Ȃǂ����̂Ō��̎c��T���[���ŒT�����邱�Ƃ������t���O
	//bool doFullDepthSearch;

	// ���̋ǖʂł̃x�X�g�̃X�R�A
	Value bestValue = -VALUE_INFINITE;

	// search()�̖߂�l���󂯂�ꎞ�ϐ�
	Value value = bestValue;

	// ���̋ǖʂł̍őP��
	Point bestMove = PT_NULL;

	// ��̈ꎞ�ϐ�
	Point move;

	// �T�������̐��i���j
	int moves = (RootNode) ? (int)rootMoves.size() : 100;

	// ����ڂɎ������肪�ł��悩������
	int bestCount = 0;


	// -----------------------
	//  �T��Stack�̏�����
	// -----------------------

	// 1����skipEarlyPruning�t���O�̏������B
	(ss + 1)->skipEarlyPruning = false;

	// 2����killer�̏������B
	(ss + 2)->killers[0] = (ss + 2)->killers[1] = PT_NULL;

	// ���̋ǖʂŎ�������̐�
	ss->moveCount = 0;

	// root����̎萔
	ss->ply = (ss - 1)->ply + 1;

	// -----------------------
	//  �u���\����T��
	// -----------------------

	auto key = pos.get_key();

	bool ttHit;    // �u���\��hit������
	TTEntry* tte = TT.probe(key, ttHit);

	// �u���\��̃X�R�A
	// �u���\��hit���Ȃ����VALUE_NONE
	Value ttValue = ttHit ? tte->value : VALUE_NONE;

	// �u���\�̎w����
	// �u���\��hit���Ȃ����PT_NULL
	Point ttMove = RootNode ? rootMoves[0].pv[0] : ttHit ? (Point)tte->move : PT_NULL;

	// RootNode�ł���Ȃ�A�w����͌��ݒ��ڂ��Ă���1�肾���ł��邩��A���ꂪ�u���\�ɂ��������̂Ƃ��Ďw�����i�߂�B

	// �u���\�̒l�ɂ��}����
	if (!PvNode        // PV node�ł͒u���\�̎w����ł͎}���肵�Ȃ�(PV node�͂����킸�������Ȃ��̂�..)
		&& ttHit         // �u���\�̎w���肪hit����
		&& tte->depth >= depth   // �u���\�ɓo�^����Ă���T���[���̂ق����[����
		&& ttValue != VALUE_NONE   // (VALUE_NONE���Ƃ���Ƒ��X���b�h����TTEntry���ǂ݂������O�ɔj�󂳂ꂽ�\��������)
		&& (ttValue >= beta ? (tte->bound & BOUND_LOWER)
			: (tte->bound & BOUND_UPPER))
		// ttValue�����E(�^�̕]���l�͂�����傫��)�������̓W���X�g�Ȓl�ŁA����ttValue >= beta�����Ȃ�beta cut�����
		// ttValue����E(�^�̕]���l�͂����菬����)�����Atte->depth()�̂ق���depth���[���Ƃ������Ƃ́A
		// ����̒T����肽������T���������ʂ̂͂��Ȃ̂ŁA������͎}���肪�Â��͂�������A���̒l��M������
		// ���̂܂܂��̒l��return���ėǂ��B
		)
	{
		// ���̎w����Ŏ}����������B������MOVE_NONE�ł��肤��B
		ss->currentMove = ttMove;

		// �u���\�̎w�����beta cut���N�����̂ł���΁A���̎w�����killer���ɓo�^����B
		if (ttValue >= beta && ttMove)
			update_stats(pos, ss, ttMove);

		return ttValue;
	}

	//�]���l�̌��ς���ƌ����ݒ肷��
	if (ttHit && tte->eval != VALUE_NONE)
	{
		ss->staticEval = eval = tte->eval;

		// ttValue(�T���l�j�̂ق������̋ǖʂ̕]���l�̌��ς���Ƃ��ēK�؂ł���Ȃ炻����̗p����B
		if (ttValue != VALUE_NONE && (tte->bound & (ttValue > eval ? BOUND_LOWER : BOUND_UPPER)))
			eval = ttValue;

	}
	else {
		//�u���\�ɂȂ��Ȃ�evaluate�����Ă��̋ǖʂ̕]���l�ƌ����ݒ肷��
		evalCount++;
		ss->staticEval = eval = evaluate(pos, ttMove);

		// �]���֐����Ăяo�����̂Œu���\�̃G���g���[�͂Ȃ��������Ƃ����A���͂Ƃ����ꂻ���ۑ����Ă����B
		tte->save(key, VALUE_NONE, BOUND_NONE, depth, ttMove, ss->staticEval, TT.generation());
	}

	//����eval��beta���傫���A�c��[�������Ȃ���΁A���O���艓����ŕK��beta cut�����͂�
	
	if (depth < 8 && eval >= beta)
	{
		return eval;
	}
	


	// -----------------------
	// 1�肸�w���������
	// -----------------------

	MovePicker mp(pos, ttMove, ss);

	// �]���l��2��O�̋ǖʂ���オ���čs���Ă���̂��̃t���O
	// �オ���čs���Ă���Ȃ�}������Â�����B
	bool improving = ss->staticEval >= (ss - 2)->staticEval;

	for (moveCount = 0; moveCount < moves; moveCount++)
	{
		// -----------------------
		//      1��i�߂�
		// -----------------------
		if (RootNode)
			move = rootMoves[moveCount].pv[0];
		else
			move = mp.next_move();


		if (move == PT_NULL) break;

		ss->moveCount++;

		// ����node��pv���N���A���Ă����B
		if (PvNode)
			(ss + 1)->pv = nullptr;

		// -----------------------
		//   1��i�߂�O�̎}����
		// -----------------------


		// -----------------------
		//      1��i�߂�
		// -----------------------

		// ���݂��̃X���b�h�ŒT�����Ă���w�����ۑ����Ă����B
		ss->currentMove = move;
		(ss + 1)->followAroundBB = ss->followAroundBB | AroundBB[move];

		Stateinfo si;
		pos.do_move(move, si);

		// ���O��Ƃ̋����Ő[�������炷
		int dx = ((int)(ss - 1)->currentMove / BOARD_SIZE) - ((int)move / BOARD_SIZE);
		int dy = ((int)(ss - 1)->currentMove % BOARD_SIZE) - ((int)move % BOARD_SIZE);
		Depth childDepth = depth - (Depth)sqrt((dx*dx) + (dy*dy));

#if 1
		// PV node��1�ڂ̎w����Ői�߂�node�́APV node�B�����Ȃ��΁Anon PV node�Ƃ��Ĉ����A
		// alpha�̒l��1�ł������邩�ǂ������������Ȃ̂ŊȒP�ȃ`�F�b�N�ōς܂���B

		// �܂��A�c��T���[�����Ȃ���ΐÎ~�T�����Ăяo���ĕ]���l��Ԃ��B
		// (search���ċA�I�ɌĂяo���āA���̐擪�Ń`�F�b�N����Ăяo���̃I�[�o�[�w�b�h�����Ȃ̂ł����ōs�Ȃ�)

		bool fullDepthSearch = (PvNode && moveCount == 0);

		if (!fullDepthSearch)
		{
			// nonPV�Ȃ炴������1/4�肮�炢�[��������Ă����̂ł�..(�{���͂����Ƃ����Ƃ��ׂ�)
			Depth R = depth / 4;

			value = (childDepth - R <= 0) ?
				eval :
				-search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, childDepth - R, true);

			// ��̒T���ɂ��alpha���X�V�����������A���������ȒT���Ȃ̂ŐM���ł��Ȃ��B�܂Ƃ��ȒT���Ō��؂��Ȃ���
			fullDepthSearch = value > alpha;
		}

		if (fullDepthSearch)
		{
			// ����node��PV�|�C���^�[�͂���node��pv�o�b�t�@���w���悤�ɂ��Ă����B
			pv[0] = PT_NULL;
			(ss + 1)->pv = pv;

			value = (childDepth <= 0) ?
				eval :
				-search<PV>(pos, ss + 1, -beta, -alpha, childDepth, false);
		}
#endif
#if 0
		// -----------------------
		// LMR(Late Move Reduction)
		// -----------------------

		// moveCount���傫�����̂Ȃǂ͒T���[�������炵�Ă������蒲�ׂ�B
		// alpha�l���X�V�������Ȃ�(fail high���N������)�Afull depth�ŒT�����Ȃ����B

		if (depth >= 4 && moveCount > 0)
		{
			// Reduction��
			Depth r = depth / 2;

			// cut node��Ahistory�̒l�������w����ɑ΂��Ă�reduction�ʂ𑝂₷�B
			if (!PvNode && cutNode)
				r += 2;

			//
			// �����ɂ��̑��̎}����A���������ׂ�
			//

			// moveCount > 1 ���Ȃ킿�A����node��2��ڈȍ~�Ȃ̂�search<NonPv>���Ăяo�����ׂ��B
			Depth d = std::max(childDepth - r, 1);
			value = -search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, d, true);

			// ��̒T���ɂ��alpha���X�V�����������A���������ȒT���Ȃ̂ŐM���ł��Ȃ��B�܂Ƃ��ȒT���Ō��؂��Ȃ���
			doFullDepthSearch = (value > alpha);

		}
		else {

			// non PV���APV�ł�2��ڈȍ~�ł����full depth search���s�Ȃ��B
			doFullDepthSearch = !PvNode || moveCount > 0;

		}

		// Full depth search
		// LMR��skip���ꂽ���ALMR�ɂ�����fail high���N�������Ȃ猳�̒T���[���ŒT������B
		if (doFullDepthSearch)
			value = (childDepth <= 0) ?
			eval :
			-search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, childDepth, !cutNode);

		// PV node�ɂ����ẮAfull depth search��fail high�����Ȃ�PV node�Ƃ���search���Ȃ����B
		// �������Avalue >= beta�Ȃ�A���m�Ȓl�����߂邱�Ƃɂ͂��܂�Ӗ����Ȃ��̂ŁA����͂�����beta cut���Ă��܂��B
		if (PvNode && (moveCount == 0 || (value > alpha && (RootNode || value < beta))))
		{
			// ����node��PV�|�C���^�[�͂���node��pv�o�b�t�@���w���悤�ɂ��Ă����B
			pv[0] = PT_NULL;
			(ss + 1)->pv = pv;

			// full depth�ŒT��
			value = (childDepth <= 0) ?
				eval :
				-search<PV>(pos, ss + 1, -beta, -alpha, childDepth, false);
		}

#endif

		// -----------------------
		//      1��߂�
		// -----------------------

		pos.undo_move(move, si);


		// -----------------------
		//    RootNode��p����
		// -----------------------

		if (RootNode)
		{
			//RootMove& rm = rootMoves.at(moveCount);
			RootMove& rm = *std::find(rootMoves.begin(), rootMoves.end(), move);


			if (moveCount == 0 || value > alpha)
			{
				// root node�ɂ�����PV�̎w����܂��́A���l���X�V�����ꍇ�A�X�R�A���Z�b�g���Ă����B
				// (iteration�̏I����sort����̂ł��̂Ƃ��Ɏw���肪����ւ��B)

				rm.score = value;
				rm.pv.resize(1); // PV�͕ω�����͂��Ȃ̂ł������񃊃Z�b�g

				// Root��PV���ς��̂͋H�Ȃ̂ł�����������Ƃ��炢�d���Ă����Ȃ��B
				// �V�����ς�����w����̌㑱��pv��RootMoves::pv�ɃR�s�[���Ă���B
				for (Point* m = (ss + 1)->pv; *m != PT_NULL; ++m)
					rm.pv.push_back(*m);

			}
			else {
				// root node�ɂ����ă��l���X�V���Ȃ������̂ł���΁A���̎w����̃X�R�A��-VALUE_INFINITE�ɂ��Ă����B
				// �������Ă����Ȃ���΁Astable_sort()���Ă���ɂ�������炸�A�O��̔����[���̂Ƃ��̒l�Ƃ�
				// �召��r���Ă��܂��w����̏��Ԃ�����ւ���Ă��܂����Ƃɂ��I�[�_�����O���\�̒ቺ�����肤��B
				rm.score = -VALUE_INFINITE;
			}
		}

		// -----------------------
		//  alpha�̍X�V��beta cut
		// -----------------------
		if (value > alpha)
		{
			alpha = value;
			bestMove = move;

			// ����������������beta cut
			if (alpha >= beta)
				break;
		}
#if 0
		// -----------------------
		//  alpha�l�̍X�V����
		// -----------------------

		if (value > bestValue)
		{
			bestValue = value;

			if (value > alpha)
			{
				bestMove = move;

				// fail high�̂Ƃ��ɂ�PV��update����B
				if (PvNode && !RootNode)
					update_pv(ss->pv, move, (ss + 1)->pv);

				// alpha�l���X�V�����̂ōX�V���Ă���
				if (PvNode && value < beta)
					alpha = value;
				else
				{
					// value >= beta �Ȃ� fail high(beta cut)
					// �܂��Anon PV�ł���Ȃ�T�����̕���0�Ȃ̂�alpha���X�V�������_�ŁAvalue >= beta�������āA
					// beta cut�ł���B
					break;
				}
			}
		}
#endif
	}
	if(bestMove)
		update_stats(pos, ss, bestMove);
	// -----------------------
	//  �u���\�ɕۑ�����
	// -----------------------
	tte->save(key, alpha,
		alpha >= beta ? BOUND_LOWER :
		PvNode && bestMove ? BOUND_EXACT : BOUND_UPPER,
		// beta�𒴂��Ă���Ƃ������Ƃ�beta cut�����킯�Ŏc��̎w����𒲂ׂĂ��Ȃ�����^�̒l�͂܂��傫���ƍl������B
		// ���Ȃ킿�A���̂Ƃ��l�͉��E�ƍl�����邩��ABOUND_LOWER�B
		// �����Ȃ��΁A(PvNode�Ȃ�)�}����͂��Ă��Ȃ��̂ŁA���ꂪ���m�Ȓl�ł���͂�������ABOUND_EXACT��Ԃ��B
		// �܂��APvNode�łȂ��Ȃ�A�}��������Ă���̂ŁA����͐��m�Ȓl�ł͂Ȃ�����ABOUND_UPPER�Ƃ��������ɂ���B
		depth, bestMove, ss->staticEval, TT.generation());


	return alpha;
}

Point Search::think()
{
	Stack stack[MAX_PLY + 4];
	Stack* ss = stack + 2;

	// aspiration search�̑��͈̔�(alpha,beta)
	// apritation search�ő��𓮂����傫��delta
	Value bestValue, alpha, beta, delta;

	// �����[����iteration���󂢂�����aspiration search���g��Ȃ��B
	// �T������ (-VALUE_INFINITE , +VALUE_INFINITE)�Ƃ���B
	bestValue = delta = alpha = -VALUE_INFINITE;
	beta = VALUE_INFINITE;

	// �擪5�����������Ă����Ώ\���B���̂��Ƃ�search�̐擪��ss+2������������B
	memset(stack, 0, (MAX_PLY + 4) * sizeof(Stack));


	TT.new_search();

	set_rootMoves();
	if((int)rootMoves.size() == 0)
		return PASS;

	rootDepth = (Depth)1;
	(ss - 1)->currentMove = (rootPos.game_play() == 0) ? PT_7G : rootPos.last_move();
	Depth max_ply = (rootPos.game_play() < 0) ? 20 : MAX_PLY;
	while ((rootDepth += (Depth)1) < max_ply)
	{
		// aspiration window search�̂��߂ɔ����[���̑O���iteration�̃X�R�A���R�s�[���Ă���
		for (RootMove& rm : rootMoves)
			rm.previousScore = rm.score;

		if (rootDepth >= (Depth)5)
		{
			// aspiration window�̕�
			// ���x�̗ǂ��]���֐��Ȃ�΂��̕�������������ƒT���������オ��̂����A
			// ���x�̈����]���֐����Ƃ��̕�����������������ƍĒT���������ĒT���������ቺ����B
			delta = Value(4);

			alpha = std::max(rootMoves[0].previousScore - delta, -VALUE_INFINITE);
			beta = std::min(rootMoves[0].previousScore + delta, VALUE_INFINITE);
		}
		
		while (true)
		{

			bestValue = search<Root>(rootPos, ss, alpha, beta, rootDepth, false);

			// ���ꂼ��̎w����ɑ΂���X�R�A�����O���I������̂ŕ��בւ������B
			std::stable_sort(rootMoves.begin(), rootMoves.end());

			if (bestValue <= alpha)
			{
				// fails low
				// beta��alpha�ɂ܂Ŋ񂹂Ă��܂��ƍ��x��fail high����\��������̂�
				// beta��alpha�̂ق��ɏ��������񂹂���x�ɗ��߂�B
				beta = (alpha + beta) / 2;
				alpha = std::max(bestValue - delta, -VALUE_INFINITE);
			}
			else if (bestValue >= beta)
			{
				// fails high
				// fails low�̂Ƃ��Ɠ����Ӗ��̏����B
				alpha = (alpha + beta) / 2;
				beta = std::min(bestValue + delta, VALUE_INFINITE);

			}
			else {
				// ����ȒT�����ʂȂ̂ł���ɂ�aspiration window search�͏I��
				break;
			}

			delta += Value(4);
		}
		// �ǂ݋؂��o�͂��Ă����B
		//std::cout << hex << rootPos.get_key() << endl;
		std::cout << dec << " evalCount = " << evalCount << " score = " << ((rootPos.side_to_move() == BLACK) ? bestValue : -bestValue);
		
		
		for (int i = 0; i < rootMoves[0].pv.size(); i++)
		{
			point_print(rootMoves[0].pv[i]);
			std::cout << " ";
		}
		std::cout << endl;

		if (evalCount > 1000) break;
	}
	evalCount = 0;
	return rootMoves[0].pv[0];
}