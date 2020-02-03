#pragma once
#include"stdafx.h"

//�S����16byte��
struct TTEntry {

	friend struct TranspositionTable;

	// hash key�̏��32bit�B����32bit�͂��̃G���g���[(�̃A�h���X)�Ɉ�v���Ă���Ƃ������Ƃ�
	// �����������v���Ă���Ƃ����O��̃R�[�h
	uint32_t key32; // 4byte
	
	Depth depth; // 4byte

	Point move; // 2byte

	Value eval; // 2byte

	Value value; // 2byte

	uint8_t generation; // 1byte

	Bound bound; // 1byte

	void set_generation(uint8_t g) { generation = g; }

	void save(Key k, Value v, Bound b, Depth d, Point m, Value ev, uint8_t gen)
	{
		if (m != PT_NULL || (k >> 32) != key32)
			move = m;

		// ���̃G���g���[�̌��݂̓��e�̂ق������l������Ȃ�㏑�����Ȃ��B
		// 1. hash key���Ⴄ�Ƃ������Ƃ�TT::probe�ł������g���ƌ��߂��킯������A����Entry�͖������ɒׂ��ėǂ�
		// 2. hash key���������Ƃ��Ă�����̏��̂ق����c��T��depth���[��(�V�������ɂ����l������̂�
		// �@�����̐[���̃}�C�i�X�Ȃ狖�e)
		// 3. BOUND_EXACT(�����PVnode�ŒT���������ʂŁA�ƂĂ����l�̂�����Ȃ̂Ŗ������ŏ�������)
		// 1. or 2. or 3.
		if ((k >> 32) != key32
			|| (d > depth - 2) // �����A2��4�Ƃǂ��炪�������B���ƂŔ�r����B
			|| b == BOUND_EXACT
			)
		{
			key32 = (uint32_t)(k >> 32);
			value = v;
			eval = ev;
			generation = gen;
			bound = b;
			depth = d;
		}
	}
};

// --- �u���\�{��
// TT_ENTRY��ClusterSize���ׂāA�N���X�^�[������B
// ���̃N���X�^�[��TT_ENTRY�͓���hash key�ɑ΂���ۑ��ꏊ�ł���B(�ۑ��ꏊ��������Ƃ��Ɍ㑱��TT_ENTRY���g��)
// ���̃N���X�^�[���AclusterCount�����m�ۂ���Ă���B
struct TranspositionTable {

	// �u���\�̂Ȃ�����^����ꂽkey�ɑΉ�����entry��T���B
	// ���������Ȃ�found == true�ɂ��Ă���TT_ENTRY*��Ԃ��B
	// ������Ȃ�������found == false�ŁA���̂Ƃ��u���\�ɏ����߂��Ƃ��Ɏg���Ɨǂ�TT_ENTRY*��Ԃ��B
	TTEntry* probe(const Key key, bool& found) const;

	// �u���\�̃T�C�Y��ύX����BmbSize == �m�ۂ��郁�����T�C�Y�BMB�P�ʁB
	void resize(size_t mbSize);

	// �u���\�̃G���g���[�̑S�N���A
	void clear() { memset(table, 0, clusterCount*sizeof(Cluster)); }

	// �V�����T�����Ƃɂ��̊֐����Ăяo���B(generation�����Z����B)
	void new_search() { generation8++; }

	// �����Ԃ��B�����TTEntry.save()�̂Ƃ��Ɏg���B
	uint8_t generation() const { return generation8; }

	// �u���\�̎g�p����1000�����ŕԂ��B(USI�v���g�R���œ��v���Ƃ��ďo�͂���̂Ɏg��)
	int hashfull() const;

	TranspositionTable() { mem = nullptr; clusterCount = 0; generation8 = 0; resize(2500);/*�f�o�b�O���悤�Ƀf�t�H���g��16MB�m��*/ }
	~TranspositionTable() { free(mem); }

private:
	// TTEntry�͂��̃T�C�Y��align���ꂽ�������ɔz�u����B(�����)
	static const int CacheLineSize = 64;

	// 1�N���X�^�[�ɂ�����TTEntry�̐�
	static const int ClusterSize = 4;

	struct Cluster {
		TTEntry entry[ClusterSize];
	};
	static_assert(sizeof(Cluster) == CacheLineSize, "Cluster size incorrect");

	// ���̒u���\���ێ����Ă���N���X�^�[���B2�̗ݏ�B
	size_t clusterCount;

	// �m�ۂ���Ă���N���X�^�[�̐擪(align����Ă���)
	Cluster* table;

	// �m�ۂ��ꂽ�������̐擪(align����Ă��Ȃ�)
	void* mem;

	uint8_t generation8; // TT_ENTRY��set_gen()�ŏ�������
};


extern TranspositionTable TT;