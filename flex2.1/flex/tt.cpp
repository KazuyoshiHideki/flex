#include "stdafx.h"
#include "tt.h"

inline int MSB64(uint64_t v) { unsigned long index; _BitScanReverse64(&index, v); return index; }

TranspositionTable TT; // �u���\��global�Ɋm�ہB

// �u���\�̃T�C�Y���m�ۂ��Ȃ����B
void TranspositionTable::resize(size_t mbSize) {

	size_t newClusterCount = size_t(1) << MSB64((mbSize * 1024 * 1024) / sizeof(Cluster));

	// �����T�C�Y�Ȃ�m�ۂ��Ȃ����K�v�͂Ȃ��B
	if (newClusterCount == clusterCount)
		return;

	clusterCount = newClusterCount;

	free(mem);

	// table��CacheLineSize��align���ꂽ�������ɔz�u�������̂ŁACacheLineSize-1�����]���Ɋm�ۂ���B
	mem = calloc(clusterCount * sizeof(Cluster) + CacheLineSize - 1, 1);

	if (!mem)
	{
		std::cerr << "Failed to allocate " << mbSize
			<< "MB for transposition table." << std::endl;
		exit(EXIT_FAILURE);
	}

	table = (Cluster*)((uintptr_t(mem) + CacheLineSize - 1) & ~(CacheLineSize - 1));
}


TTEntry* TranspositionTable::probe(const Key key, bool& found) const {

	// �ŏ���TT_ENTRY�̃A�h���X(���̃A�h���X����TT_ENTRY��ClusterSize�������A�Ȃ��Ă���)
	// key�̉���bit���������g���āA���̃A�h���X�����߂�̂ŁA�����Ɖ���bit�͂����炩�͈�v���Ă��邱�ƂɂȂ�B
	TTEntry* const tte = &table[(size_t)(key)& (clusterCount - 1)].entry[0];

	// �N���X�^�[�̂Ȃ�����Akey�����v����TT_ENTRY��T��
	for (int i = 0; i < ClusterSize; ++i)
	{
		// return�������
		// 1. key�����v���Ă���entry���������B(found==true�ɂ��Ă���TT_ENTRY�̃A�h���X��Ԃ�)
		// 2. ��̃G���g���[��������(�����܂ł�key�����v���Ă��Ȃ��̂ŁAfound==false�ɂ��ĐV�KTT_ENTRY�̃A�h���X�Ƃ��ĕԂ�)
		if (!tte[i].key32)
			return found = false, &tte[i];

		if (tte[i].key32 == (key >> 32))
		{
			tte[i].set_generation(generation8); // Refresh
			return found = true, &tte[i];
		}
	}

	// �󂫃G���g���[���A�T���Ă���key���i�[����Ă���entry����������Ȃ������B
	// �N���X�^�[���̂ǂꂩ���ׂ��K�v������B

	TTEntry* replace = tte;
	for (int i = 1; i < ClusterSize; ++i)

		// �E�[���T���̌��ʂł�����̂قǉ��l������̂Ŏc���Ă��������Bdepth8 �~ �d��1.0
		// �Egeneration�����܂̒T��generation�ɋ߂����̂قǉ��l������̂Ŏc���Ă��������Bgeration�~�d�� 8.0
		// �ȏ�Ɋ�ăX�R�A�����O����B
		// �ȏ�̍��v����ԏ�����TTEntry���g���B
		// generation��256�ɂȂ�ƃI�[�o�[�t���[����0�ɂȂ�̂ł�������܂������ł��Ȃ���΂Ȃ�Ȃ��B
		// a,b��8bit�ł���Ƃ� ( 256 + a - b ) & 0xff�@�̂悤�ɂ���΁A�I�[�o�[�t���[���l�����������Z���o����B
		// ���̃e�N�j�b�N��p����B
		if (replace->depth - ((256 + generation8 - replace->generation) & 0xff) * 8
			>   tte[i].depth - ((256 + generation8 - tte[i].generation) & 0xff) * 8)
			replace = &tte[i];

	return found = false, replace;
}

int TranspositionTable::hashfull() const
{
	// ���ׂẴG���g���[�ɃA�N�Z�X����Ǝ��Ԃ����ɂ����邽�߁A�擪����1000�G���g���[����
	// �T���v�����O���Ďg�p����Ă���G���g���[����Ԃ��B
	int cnt = 0;
	for (int i = 0; i < 1000 / ClusterSize; ++i)
	{
		const auto tte = &table[i].entry[0];
		for (int j = 0; j < ClusterSize; ++j)
			if ((tte[j].generation == generation8))
				++cnt;
	}
	return cnt;
}