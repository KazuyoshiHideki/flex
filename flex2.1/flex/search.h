#pragma once
#include"stdafx.h"

enum NodeType { Root, PV, NonPV };

enum Bound : uint8_t {
	BOUND_NONE,  // �T�����Ă��Ȃ�(DEPTH_NONE)�Ƃ��ɁA���肩�A�ÓI�]���X�R�A������u���\�Ɋi�[�������Ƃ��ɗp����B
	BOUND_UPPER, // ��E(�^�̕]���l�͂����菬����) nonPV�ŕ]���l�����܂�M�p�Ȃ�Ȃ���Ԃł��邱�Ƃ�\������B
	BOUND_LOWER, // ���E(�^�̕]���l�͂�����傫��)
	BOUND_EXACT = BOUND_UPPER | BOUND_LOWER // �^�̕]���l�ƈ�v���Ă���BPV node�B
};

struct LimitsType
{
	// �R���X�g���N�^�Ŗ����I�Ƀ[���N���A(MSVC��POD�łȂ��Ɣj�󂷂邱�Ƃ����邽��)
	LimitsType() { memset(this, 0, offsetof(LimitsType, startTime)); }

	// �c�莞��(ms���Z��)
	int time[COLOR_NB];

	// �b�ǂ�(ms���Z��)
	int byoyomi[COLOR_NB];

	// movetime : �v�l���ԌŒ�(0�ȊO���w�肵�Ă���Ȃ�) : �P�ʂ�[ms]
	// infinite : �v�l���Ԗ��������ǂ����̃t���O�B��0�Ȃ疳�����B
	// ponder   : ponder���ł��邩�̃t���O
	int movetime, infinite, ponder;

	// �����go�R�}���h�ł̒T���m�[�h��
	int64_t nodes;

	// go�R�}���h�ŒT�����J�n���������B
	Time startTime;
};

extern LimitsType Limits;

// root(�T���J�n�ǖ�)�ł̎w����Ƃ��Ďg����B���ꂼ���root move�ɑ΂��āA
// ���̎w����Ői�߂��Ƃ���score(�]���l)��PV�������Ă���B(PV��fail low�����Ƃ��ɂ͐M�p�ł��Ȃ�)
// score��non-pv�̎w����ł�-VALUE_INFINITE�ŏ����������B
struct RootMove
{
	// sort����Ƃ��ɕK�v�Bstd::stable_sort()�ō~���ɂȂ��ė~�����̂Ŕ�r�̕s�������t�ɂ��Ă����B
	bool operator<(const RootMove& m) const { return score > m.score; }

	// std::count(),std::find()�ȂǂŎw����Ɣ�r����Ƃ��ɕK�v�B
	bool operator==(const Point& m) const { return pv[0] == m; }

	// pv[0]�ɂ́A���̃R���X�g���N�^�̈����œn���ꂽm��ݒ肷��B
	explicit RootMove(Point m) : pv(1, m) {}

	// this->pv�ɓǂ݋�(PV)���ۑ�����Ă���Ƃ��āA
	// �����u���\�ɕۑ�����B
	void insert_pv_in_tt(Position& pos);

	// �����(�����[����)iteration�ł̒T�����ʂ̃X�R�A
	Value score = -VALUE_INFINITE;

	// �O���(�����[����)iteration�ł̒T�����ʂ̃X�R�A
	// ����iteration���̒T�����͈̔͂����߂�Ƃ��Ɏg���B
	Value previousScore = -VALUE_INFINITE;

	std::vector<Point> pv;
};

// -----------------------
//  �T���̂Ƃ��Ɏg��Stack
// -----------------------

struct Stack {
	Bitboard followAroundBB;

	Point* pv;              // PV�ւ̃|�C���^�[�BRootMoves��vector<Move> pv���w���Ă���B
	int ply;               // root����̎萔
	Point currentMove;      // ���̃X���b�h�̒T���ɂ����Ă��̋ǖʂŌ��ݑI������Ă���w����
	Point excludedMove;     // singular extension����̂Ƃ��ɒu���\�̎w���������node�ŏ��O���ĒT���������̂ł��̏��O����w����
	Point killers[2];       // killer move
	Value staticEval;      // �]���֐����Ăяo���ē����l�BNULL MOVE�̂Ƃ��ɐenode�ł̕]���l���~�����̂ŕۑ����Ă����B
	bool skipEarlyPruning; // �w���萶���O�ɍs�Ȃ��}������ȗ����邩�B(NULL MOVE�̒���Ȃ�)
	int moveCount;         // ����node��do_move()����������������ڂ̎w���肩�B(1�Ȃ炨���炭�u���\�̎w���肾�낤)
};

struct Search
{
	Search(Position& pos) { rootPos = pos; rootMoves.clear(); maxPly = 0; rootDepth = (Depth)0; }

	void set_rootMoves();

	//
	template <NodeType NT>
	Value search(Position& pos, Stack* ss, Value alpha, Value beta, Depth depth, bool cutNode);

	//�T���̏��߂ɌĂ΂��
	Point think();

	// �T���J�n�ǖ�
	Position rootPos;

	// �T���J�n�ǖʂŎv�l�ΏۂƂ���w����̏W���B
	std::vector<RootMove> rootMoves;

	// root����ő�A����ڂ܂ŒT��������(�I��[���̍ő�)
	int maxPly;

	// �����[���̐[��
	Depth rootDepth;
};

namespace Searchs
{
	// �T�����̏������B
	void init();

	// �T������clear�B
	// �u���\�̃N���A�Ȃǎ��Ԃ̂�����T���̏����������������ł��Bisready�ɑ΂��ČĂяo�����B
	void clear();
}