#include"stdafx.h"
#include"search.h"

using namespace std;

bool check = false;
int evalCount = 0;

// PV lineをコピーする。
// pv に move(1手) + childPv(複数手,末尾MOVE_NONE)をコピーする。
// 番兵として末尾はMOVE_NONEにすることになっている。
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
	//   killerのupdate

	// killer 2本しかないので[0]と違うならいまの[0]を[1]に降格させて[0]と差し替え
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
	// nodeの種類と変数の宣言
	// -----------------------

	// root nodeであるか
	const bool RootNode = NT == Root;

	// PV nodeであるか(root nodeはPV nodeに含まれる)
	const bool PvNode = NT == PV || NT == Root;

	//局面評価値
	Value eval;

	// このnodeからのPV line(読み筋)
	Point pv[MAX_PLY + 1];

	// この局面でdo_move()された合法手の数
	int moveCount = 0;

	// LMRのときにfail highが起きるなどしたので元の残り探索深さで探索することを示すフラグ
	//bool doFullDepthSearch;

	// この局面でのベストのスコア
	Value bestValue = -VALUE_INFINITE;

	// search()の戻り値を受ける一時変数
	Value value = bestValue;

	// この局面での最善手
	Point bestMove = PT_NULL;

	// 手の一時変数
	Point move;

	// 探索する手の数（幅）
	int moves = (RootNode) ? (int)rootMoves.size() : 100;

	// 何手目に試した手が最もよかったか
	int bestCount = 0;


	// -----------------------
	//  探索Stackの初期化
	// -----------------------

	// 1手先のskipEarlyPruningフラグの初期化。
	(ss + 1)->skipEarlyPruning = false;

	// 2手先のkillerの初期化。
	(ss + 2)->killers[0] = (ss + 2)->killers[1] = PT_NULL;

	// この局面で試した手の数
	ss->moveCount = 0;

	// rootからの手数
	ss->ply = (ss - 1)->ply + 1;

	// -----------------------
	//  置換表から探す
	// -----------------------

	auto key = pos.get_key();

	bool ttHit;    // 置換表がhitしたか
	TTEntry* tte = TT.probe(key, ttHit);

	// 置換表上のスコア
	// 置換表にhitしなければVALUE_NONE
	Value ttValue = ttHit ? tte->value : VALUE_NONE;

	// 置換表の指し手
	// 置換表にhitしなければPT_NULL
	Point ttMove = RootNode ? rootMoves[0].pv[0] : ttHit ? (Point)tte->move : PT_NULL;

	// RootNodeであるなら、指し手は現在注目している1手だけであるから、それが置換表にあったものとして指し手を進める。

	// 置換表の値による枝刈り
	if (!PvNode        // PV nodeでは置換表の指し手では枝刈りしない(PV nodeはごくわずかしかないので..)
		&& ttHit         // 置換表の指し手がhitして
		&& tte->depth >= depth   // 置換表に登録されている探索深さのほうが深くて
		&& ttValue != VALUE_NONE   // (VALUE_NONEだとすると他スレッドからTTEntryが読みだす直前に破壊された可能性がある)
		&& (ttValue >= beta ? (tte->bound & BOUND_LOWER)
			: (tte->bound & BOUND_UPPER))
		// ttValueが下界(真の評価値はこれより大きい)もしくはジャストな値で、かつttValue >= beta超えならbeta cutされる
		// ttValueが上界(真の評価値はこれより小さい)だが、tte->depth()のほうがdepthより深いということは、
		// 今回の探索よりたくさん探索した結果のはずなので、今回よりは枝刈りが甘いはずだから、その値を信頼して
		// このままこの値でreturnして良い。
		)
	{
		// この指し手で枝刈りをした。ただしMOVE_NONEでありうる。
		ss->currentMove = ttMove;

		// 置換表の指し手でbeta cutが起きたのであれば、この指し手をkiller等に登録する。
		if (ttValue >= beta && ttMove)
			update_stats(pos, ss, ttMove);

		return ttValue;
	}

	//評価値の見積もりと候補手を設定する
	if (ttHit && tte->eval != VALUE_NONE)
	{
		ss->staticEval = eval = tte->eval;

		// ttValue(探索値）のほうがこの局面の評価値の見積もりとして適切であるならそれを採用する。
		if (ttValue != VALUE_NONE && (tte->bound & (ttValue > eval ? BOUND_LOWER : BOUND_UPPER)))
			eval = ttValue;

	}
	else {
		//置換表にないならevaluateをしてこの局面の評価値と候補手を設定する
		evalCount++;
		ss->staticEval = eval = evaluate(pos, ttMove);

		// 評価関数を呼び出したので置換表のエントリーはなかったことだし、何はともあれそれを保存しておく。
		tte->save(key, VALUE_NONE, BOUND_NONE, depth, ttMove, ss->staticEval, TT.generation());
	}

	//もしevalがbetaより大きく、残り深さが少なければ、直前手より遠い手で必ずbeta cutされるはず
	
	if (depth < 8 && eval >= beta)
	{
		return eval;
	}
	


	// -----------------------
	// 1手ずつ指し手を試す
	// -----------------------

	MovePicker mp(pos, ttMove, ss);

	// 評価値が2手前の局面から上がって行っているのかのフラグ
	// 上がって行っているなら枝刈りを甘くする。
	bool improving = ss->staticEval >= (ss - 2)->staticEval;

	for (moveCount = 0; moveCount < moves; moveCount++)
	{
		// -----------------------
		//      1手進める
		// -----------------------
		if (RootNode)
			move = rootMoves[moveCount].pv[0];
		else
			move = mp.next_move();


		if (move == PT_NULL) break;

		ss->moveCount++;

		// 次のnodeのpvをクリアしておく。
		if (PvNode)
			(ss + 1)->pv = nullptr;

		// -----------------------
		//   1手進める前の枝刈り
		// -----------------------


		// -----------------------
		//      1手進める
		// -----------------------

		// 現在このスレッドで探索している指し手を保存しておく。
		ss->currentMove = move;
		(ss + 1)->followAroundBB = ss->followAroundBB | AroundBB[move];

		Stateinfo si;
		pos.do_move(move, si);

		// 直前手との距離で深さを減らす
		int dx = ((int)(ss - 1)->currentMove / BOARD_SIZE) - ((int)move / BOARD_SIZE);
		int dy = ((int)(ss - 1)->currentMove % BOARD_SIZE) - ((int)move % BOARD_SIZE);
		Depth childDepth = depth - (Depth)sqrt((dx*dx) + (dy*dy));

#if 1
		// PV nodeの1つ目の指し手で進めたnodeは、PV node。さもなくば、non PV nodeとして扱い、
		// alphaの値を1でも超えるかどうかだけが問題なので簡単なチェックで済ませる。

		// また、残り探索深さがなければ静止探索を呼び出して評価値を返す。
		// (searchを再帰的に呼び出して、その先頭でチェックする呼び出しのオーバーヘッドが嫌なのでここで行なう)

		bool fullDepthSearch = (PvNode && moveCount == 0);

		if (!fullDepthSearch)
		{
			// nonPVならざっくり1/4手ぐらい深さを削っていいのでは..(本当はもっとちゃんとやるべき)
			Depth R = depth / 4;

			value = (childDepth - R <= 0) ?
				eval :
				-search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, childDepth - R, true);

			// 上の探索によりalphaを更新しそうだが、いい加減な探索なので信頼できない。まともな探索で検証しなおす
			fullDepthSearch = value > alpha;
		}

		if (fullDepthSearch)
		{
			// 次のnodeのPVポインターはこのnodeのpvバッファを指すようにしておく。
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

		// moveCountが大きいものなどは探索深さを減らしてざっくり調べる。
		// alpha値を更新しそうなら(fail highが起きたら)、full depthで探索しなおす。

		if (depth >= 4 && moveCount > 0)
		{
			// Reduction量
			Depth r = depth / 2;

			// cut nodeや、historyの値が悪い指し手に対してはreduction量を増やす。
			if (!PvNode && cutNode)
				r += 2;

			//
			// ここにその他の枝刈り、何か入れるべき
			//

			// moveCount > 1 すなわち、このnodeの2手目以降なのでsearch<NonPv>が呼び出されるべき。
			Depth d = std::max(childDepth - r, 1);
			value = -search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, d, true);

			// 上の探索によりalphaを更新しそうだが、いい加減な探索なので信頼できない。まともな探索で検証しなおす
			doFullDepthSearch = (value > alpha);

		}
		else {

			// non PVか、PVでも2手目以降であればfull depth searchを行なう。
			doFullDepthSearch = !PvNode || moveCount > 0;

		}

		// Full depth search
		// LMRがskipされたか、LMRにおいてfail highを起こしたなら元の探索深さで探索する。
		if (doFullDepthSearch)
			value = (childDepth <= 0) ?
			eval :
			-search<NonPV>(pos, ss + 1, -(alpha + 1), -alpha, childDepth, !cutNode);

		// PV nodeにおいては、full depth searchがfail highしたならPV nodeとしてsearchしなおす。
		// ただし、value >= betaなら、正確な値を求めることにはあまり意味がないので、これはせずにbeta cutしてしまう。
		if (PvNode && (moveCount == 0 || (value > alpha && (RootNode || value < beta))))
		{
			// 次のnodeのPVポインターはこのnodeのpvバッファを指すようにしておく。
			pv[0] = PT_NULL;
			(ss + 1)->pv = pv;

			// full depthで探索
			value = (childDepth <= 0) ?
				eval :
				-search<PV>(pos, ss + 1, -beta, -alpha, childDepth, false);
		}

#endif

		// -----------------------
		//      1手戻す
		// -----------------------

		pos.undo_move(move, si);


		// -----------------------
		//    RootNode専用処理
		// -----------------------

		if (RootNode)
		{
			//RootMove& rm = rootMoves.at(moveCount);
			RootMove& rm = *std::find(rootMoves.begin(), rootMoves.end(), move);


			if (moveCount == 0 || value > alpha)
			{
				// root nodeにおいてPVの指し手または、α値を更新した場合、スコアをセットしておく。
				// (iterationの終わりでsortするのでそのときに指し手が入れ替わる。)

				rm.score = value;
				rm.pv.resize(1); // PVは変化するはずなのでいったんリセット

				// RootでPVが変わるのは稀なのでここがちょっとぐらい重くても問題ない。
				// 新しく変わった指し手の後続のpvをRootMoves::pvにコピーしてくる。
				for (Point* m = (ss + 1)->pv; *m != PT_NULL; ++m)
					rm.pv.push_back(*m);

			}
			else {
				// root nodeにおいてα値を更新しなかったのであれば、この指し手のスコアを-VALUE_INFINITEにしておく。
				// こうしておかなければ、stable_sort()しているにもかかわらず、前回の反復深化のときの値との
				// 大小比較してしまい指し手の順番が入れ替わってしまうことによるオーダリング性能の低下がありうる。
				rm.score = -VALUE_INFINITE;
			}
		}

		// -----------------------
		//  alphaの更新とbeta cut
		// -----------------------
		if (value > alpha)
		{
			alpha = value;
			bestMove = move;

			// αがβを上回ったらbeta cut
			if (alpha >= beta)
				break;
		}
#if 0
		// -----------------------
		//  alpha値の更新処理
		// -----------------------

		if (value > bestValue)
		{
			bestValue = value;

			if (value > alpha)
			{
				bestMove = move;

				// fail highのときにもPVをupdateする。
				if (PvNode && !RootNode)
					update_pv(ss->pv, move, (ss + 1)->pv);

				// alpha値を更新したので更新しておく
				if (PvNode && value < beta)
					alpha = value;
				else
				{
					// value >= beta なら fail high(beta cut)
					// また、non PVであるなら探索窓の幅が0なのでalphaを更新した時点で、value >= betaが言えて、
					// beta cutである。
					break;
				}
			}
		}
#endif
	}
	if(bestMove)
		update_stats(pos, ss, bestMove);
	// -----------------------
	//  置換表に保存する
	// -----------------------
	tte->save(key, alpha,
		alpha >= beta ? BOUND_LOWER :
		PvNode && bestMove ? BOUND_EXACT : BOUND_UPPER,
		// betaを超えているということはbeta cutされるわけで残りの指し手を調べていないから真の値はまだ大きいと考えられる。
		// すなわち、このとき値は下界と考えられるから、BOUND_LOWER。
		// さもなくば、(PvNodeなら)枝刈りはしていないので、これが正確な値であるはずだから、BOUND_EXACTを返す。
		// また、PvNodeでないなら、枝刈りをしているので、これは正確な値ではないから、BOUND_UPPERという扱いにする。
		depth, bestMove, ss->staticEval, TT.generation());


	return alpha;
}

Point Search::think()
{
	Stack stack[MAX_PLY + 4];
	Stack* ss = stack + 2;

	// aspiration searchの窓の範囲(alpha,beta)
	// apritation searchで窓を動かす大きさdelta
	Value bestValue, alpha, beta, delta;

	// 反復深化のiterationが浅いうちはaspiration searchを使わない。
	// 探索窓を (-VALUE_INFINITE , +VALUE_INFINITE)とする。
	bestValue = delta = alpha = -VALUE_INFINITE;
	beta = VALUE_INFINITE;

	// 先頭5つを初期化しておけば十分。そのあとはsearchの先頭でss+2を初期化する。
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
		// aspiration window searchのために反復深化の前回のiterationのスコアをコピーしておく
		for (RootMove& rm : rootMoves)
			rm.previousScore = rm.score;

		if (rootDepth >= (Depth)5)
		{
			// aspiration windowの幅
			// 精度の良い評価関数ならばこの幅を小さくすると探索効率が上がるのだが、
			// 精度の悪い評価関数だとこの幅を小さくしすぎると再探索が増えて探索効率が低下する。
			delta = Value(4);

			alpha = std::max(rootMoves[0].previousScore - delta, -VALUE_INFINITE);
			beta = std::min(rootMoves[0].previousScore + delta, VALUE_INFINITE);
		}
		
		while (true)
		{

			bestValue = search<Root>(rootPos, ss, alpha, beta, rootDepth, false);

			// それぞれの指し手に対するスコアリングが終わったので並べ替えおく。
			std::stable_sort(rootMoves.begin(), rootMoves.end());

			if (bestValue <= alpha)
			{
				// fails low
				// betaをalphaにまで寄せてしまうと今度はfail highする可能性があるので
				// betaをalphaのほうに少しだけ寄せる程度に留める。
				beta = (alpha + beta) / 2;
				alpha = std::max(bestValue - delta, -VALUE_INFINITE);
			}
			else if (bestValue >= beta)
			{
				// fails high
				// fails lowのときと同じ意味の処理。
				alpha = (alpha + beta) / 2;
				beta = std::min(bestValue + delta, VALUE_INFINITE);

			}
			else {
				// 正常な探索結果なのでこれにてaspiration window searchは終了
				break;
			}

			delta += Value(4);
		}
		// 読み筋を出力しておく。
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