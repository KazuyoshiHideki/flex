#pragma once
#include"stdafx.h"

//全部で16byteに
struct TTEntry {

	friend struct TranspositionTable;

	// hash keyの上位32bit。下位32bitはこのエントリー(のアドレス)に一致しているということは
	// そこそこ合致しているという前提のコード
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

		// このエントリーの現在の内容のほうが価値があるなら上書きしない。
		// 1. hash keyが違うということはTT::probeでここを使うと決めたわけだから、このEntryは無条件に潰して良い
		// 2. hash keyが同じだとしても今回の情報のほうが残り探索depthが深い(新しい情報にも価値があるので
		// 　少しの深さのマイナスなら許容)
		// 3. BOUND_EXACT(これはPVnodeで探索した結果で、とても価値のある情報なので無条件で書き込む)
		// 1. or 2. or 3.
		if ((k >> 32) != key32
			|| (d > depth - 2) // ここ、2と4とどちらがいいか。あとで比較する。
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

// --- 置換表本体
// TT_ENTRYをClusterSize個並べて、クラスターをつくる。
// このクラスターのTT_ENTRYは同じhash keyに対する保存場所である。(保存場所が被ったときに後続のTT_ENTRYを使う)
// このクラスターが、clusterCount個だけ確保されている。
struct TranspositionTable {

	// 置換表のなかから与えられたkeyに対応するentryを探す。
	// 見つかったならfound == trueにしてそのTT_ENTRY*を返す。
	// 見つからなかったらfound == falseで、このとき置換表に書き戻すときに使うと良いTT_ENTRY*を返す。
	TTEntry* probe(const Key key, bool& found) const;

	// 置換表のサイズを変更する。mbSize == 確保するメモリサイズ。MB単位。
	void resize(size_t mbSize);

	// 置換表のエントリーの全クリア
	void clear() { memset(table, 0, clusterCount*sizeof(Cluster)); }

	// 新しい探索ごとにこの関数を呼び出す。(generationを加算する。)
	void new_search() { generation8++; }

	// 世代を返す。これはTTEntry.save()のときに使う。
	uint8_t generation() const { return generation8; }

	// 置換表の使用率を1000分率で返す。(USIプロトコルで統計情報として出力するのに使う)
	int hashfull() const;

	TranspositionTable() { mem = nullptr; clusterCount = 0; generation8 = 0; resize(2500);/*デバッグ時ようにデフォルトで16MB確保*/ }
	~TranspositionTable() { free(mem); }

private:
	// TTEntryはこのサイズでalignされたメモリに配置する。(される)
	static const int CacheLineSize = 64;

	// 1クラスターにおけるTTEntryの数
	static const int ClusterSize = 4;

	struct Cluster {
		TTEntry entry[ClusterSize];
	};
	static_assert(sizeof(Cluster) == CacheLineSize, "Cluster size incorrect");

	// この置換表が保持しているクラスター数。2の累乗。
	size_t clusterCount;

	// 確保されているクラスターの先頭(alignされている)
	Cluster* table;

	// 確保されたメモリの先頭(alignされていない)
	void* mem;

	uint8_t generation8; // TT_ENTRYのset_gen()で書き込む
};


extern TranspositionTable TT;