#include "stdafx.h"
#include "tt.h"

inline int MSB64(uint64_t v) { unsigned long index; _BitScanReverse64(&index, v); return index; }

TranspositionTable TT; // 置換表をglobalに確保。

// 置換表のサイズを確保しなおす。
void TranspositionTable::resize(size_t mbSize) {

	size_t newClusterCount = size_t(1) << MSB64((mbSize * 1024 * 1024) / sizeof(Cluster));

	// 同じサイズなら確保しなおす必要はない。
	if (newClusterCount == clusterCount)
		return;

	clusterCount = newClusterCount;

	free(mem);

	// tableはCacheLineSizeでalignされたメモリに配置したいので、CacheLineSize-1だけ余分に確保する。
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

	// 最初のTT_ENTRYのアドレス(このアドレスからTT_ENTRYがClusterSize分だけ連なっている)
	// keyの下位bitをいくつか使って、このアドレスを求めるので、自ずと下位bitはいくらかは一致していることになる。
	TTEntry* const tte = &table[(size_t)(key)& (clusterCount - 1)].entry[0];

	// クラスターのなかから、keyが合致するTT_ENTRYを探す
	for (int i = 0; i < ClusterSize; ++i)
	{
		// returnする条件
		// 1. keyが合致しているentryを見つけた。(found==trueにしてそのTT_ENTRYのアドレスを返す)
		// 2. 空のエントリーを見つけた(そこまではkeyが合致していないので、found==falseにして新規TT_ENTRYのアドレスとして返す)
		if (!tte[i].key32)
			return found = false, &tte[i];

		if (tte[i].key32 == (key >> 32))
		{
			tte[i].set_generation(generation8); // Refresh
			return found = true, &tte[i];
		}
	}

	// 空きエントリーも、探していたkeyが格納されているentryが見当たらなかった。
	// クラスター内のどれか一つを潰す必要がある。

	TTEntry* replace = tte;
	for (int i = 1; i < ClusterSize; ++i)

		// ・深い探索の結果であるものほど価値があるので残しておきたい。depth8 × 重み1.0
		// ・generationがいまの探索generationに近いものほど価値があるので残しておきたい。geration×重み 8.0
		// 以上に基いてスコアリングする。
		// 以上の合計が一番小さいTTEntryを使う。
		// generationは256になるとオーバーフローして0になるのでそれをうまく処理できなければならない。
		// a,bが8bitであるとき ( 256 + a - b ) & 0xff　のようにすれば、オーバーフローを考慮した引き算が出来る。
		// このテクニックを用いる。
		if (replace->depth - ((256 + generation8 - replace->generation) & 0xff) * 8
			>   tte[i].depth - ((256 + generation8 - tte[i].generation) & 0xff) * 8)
			replace = &tte[i];

	return found = false, replace;
}

int TranspositionTable::hashfull() const
{
	// すべてのエントリーにアクセスすると時間が非常にかかるため、先頭から1000エントリーだけ
	// サンプリングして使用されているエントリー数を返す。
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