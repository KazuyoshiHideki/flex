#pragma once
#include"stdafx.h"

enum NodeType { Root, PV, NonPV };

enum Bound : uint8_t {
	BOUND_NONE,  // 探索していない(DEPTH_NONE)ときに、候補手か、静的評価スコアだけを置換表に格納したいときに用いる。
	BOUND_UPPER, // 上界(真の評価値はこれより小さい) nonPVで評価値があまり信用ならない状態であることを表現する。
	BOUND_LOWER, // 下界(真の評価値はこれより大きい)
	BOUND_EXACT = BOUND_UPPER | BOUND_LOWER // 真の評価値と一致している。PV node。
};

struct LimitsType
{
	// コンストラクタで明示的にゼロクリア(MSVCがPODでないと破壊することがあるため)
	LimitsType() { memset(this, 0, offsetof(LimitsType, startTime)); }

	// 残り時間(ms換算で)
	int time[COLOR_NB];

	// 秒読み(ms換算で)
	int byoyomi[COLOR_NB];

	// movetime : 思考時間固定(0以外が指定してあるなら) : 単位は[ms]
	// infinite : 思考時間無制限かどうかのフラグ。非0なら無制限。
	// ponder   : ponder中であるかのフラグ
	int movetime, infinite, ponder;

	// 今回のgoコマンドでの探索ノード数
	int64_t nodes;

	// goコマンドで探索を開始した時刻。
	Time startTime;
};

extern LimitsType Limits;

// root(探索開始局面)での指し手として使われる。それぞれのroot moveに対して、
// その指し手で進めたときのscore(評価値)とPVを持っている。(PVはfail lowしたときには信用できない)
// scoreはnon-pvの指し手では-VALUE_INFINITEで初期化される。
struct RootMove
{
	// sortするときに必要。std::stable_sort()で降順になって欲しいので比較の不等号を逆にしておく。
	bool operator<(const RootMove& m) const { return score > m.score; }

	// std::count(),std::find()などで指し手と比較するときに必要。
	bool operator==(const Point& m) const { return pv[0] == m; }

	// pv[0]には、このコンストラクタの引数で渡されたmを設定する。
	explicit RootMove(Point m) : pv(1, m) {}

	// this->pvに読み筋(PV)が保存されているとして、
	// これを置換表に保存する。
	void insert_pv_in_tt(Position& pos);

	// 今回の(反復深化の)iterationでの探索結果のスコア
	Value score = -VALUE_INFINITE;

	// 前回の(反復深化の)iterationでの探索結果のスコア
	// 次のiteration時の探索窓の範囲を決めるときに使う。
	Value previousScore = -VALUE_INFINITE;

	std::vector<Point> pv;
};

// -----------------------
//  探索のときに使うStack
// -----------------------

struct Stack {
	Bitboard followAroundBB;

	Point* pv;              // PVへのポインター。RootMovesのvector<Move> pvを指している。
	int ply;               // rootからの手数
	Point currentMove;      // そのスレッドの探索においてこの局面で現在選択されている指し手
	Point excludedMove;     // singular extension判定のときに置換表の指し手をそのnodeで除外して探索したいのでその除外する指し手
	Point killers[2];       // killer move
	Value staticEval;      // 評価関数を呼び出して得た値。NULL MOVEのときに親nodeでの評価値が欲しいので保存しておく。
	bool skipEarlyPruning; // 指し手生成前に行なう枝刈りを省略するか。(NULL MOVEの直後など)
	int moveCount;         // このnodeでdo_move()した生成した何手目の指し手か。(1ならおそらく置換表の指し手だろう)
};

struct Search
{
	Search(Position& pos) { rootPos = pos; rootMoves.clear(); maxPly = 0; rootDepth = (Depth)0; }

	void set_rootMoves();

	//
	template <NodeType NT>
	Value search(Position& pos, Stack* ss, Value alpha, Value beta, Depth depth, bool cutNode);

	//探索の初めに呼ばれる
	Point think();

	// 探索開始局面
	Position rootPos;

	// 探索開始局面で思考対象とする指し手の集合。
	std::vector<RootMove> rootMoves;

	// rootから最大、何手目まで探索したか(選択深さの最大)
	int maxPly;

	// 反復深化の深さ
	Depth rootDepth;
};

namespace Searchs
{
	// 探索部の初期化。
	void init();

	// 探索部のclear。
	// 置換表のクリアなど時間のかかる探索の初期化処理をここでやる。isreadyに対して呼び出される。
	void clear();
}