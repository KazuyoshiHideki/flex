#include"stdafx.h"
#include "position.h"

using namespace std;


//初期局面の情報にセットする
void Position::set_start_board()
{
	BrankBB = ALL_BB;
	StoneBB[BLACK] = ZERO_BB;
	StoneBB[WHITE] = ZERO_BB;
	KoBB = ZERO_BB;
	side = BLACK;
	moves = 0;
	key = 0;
}

//合法手かどうかチェック
bool Position::legal(Point move) const
{
	
	// 0.moveが盤上の点か
	if (PT_ZERO > move || move >= PT_NB)
		return false;

	// 0.その点が空点でなければ打てない
	if (PointBB[move] != BrankBB)
		return false;
	
	// 1.呼吸点があれば合法手
	if (!(BreathBB[move] != BrankBB))
		return true;

	// 2.コウなら非合法手  pop()を呼び出す前にKoBBとandnotしてコウの着手をつぶすのならこの処理は不要
	if (PointBB[move] == KoBB) return false;
	
	//仮に置いた時のBrankBB
	Bitboard ProBrankBB = BrankBB ^ PointBB[move];

	// 3.自分の連結する連に呼吸点があるなら合法手
	//MyStoneとは、置く位置の呼吸点に存在する自分の石
	Bitboard MyStone = StoneBB[side] & BreathBB[move];
	String NewString(move);
	//自分の連結する連の呼吸点を合成する
	while (auto pt = MyStone.pop())
	{
		Point root = root_num_c(pt);
		NewString.StringBreathBB |= string[root].StringBreathBB;
		//連結した連に呼吸点があるなら合法手
		if (!(NewString.StringBreathBB != ProBrankBB))
			return true;
		MyStone ^= string[root].StringBB;
	}

	// 4.相手の連をとれるなら合法手
	//YourStoneとは、置く位置の呼吸点に存在する自分の石
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	while (auto pt = YourStone.pop())
	{
		Point root = root_num_c(pt);
		//もし相手の連の呼吸点がなかったら取れるので合法手
		if (string[root].StringBreathBB != ProBrankBB)
			return true;
		YourStone ^= string[root].StringBB;
	}

	return false;
}

//着手
void Position::do_move(Point move)
{
	//コウの解消
	KoBB = ZERO_BB;
	bool ko_flag = false;

	//石を置く
	BrankBB ^= PointBB[move];
	StoneBB[side] |= PointBB[move];

	//連を作成
	string[move] = String(move);

	//自分の連結する連の呼吸点を合成して、それらの連の親ナンバーをこの新しい連に
	Bitboard MyStone = StoneBB[side] & BreathBB[move];
	while (auto pt = MyStone.pop())
	{
		Point root = root_num(pt);
		string[move].StringBreathBB |= string[root].StringBreathBB;
		string[move].StringBB |= string[root].StringBB;
		string[root].parent_num = move;
		MyStone ^= string[root].StringBB;
	}
	//もし置いた石の連の石数が1つならコウあるいは着手禁止点の可能性あり
	//（連とPointBBが同一なら1つしか取っていないことになる）
	if (string[move].StringBB == PointBB[move])
		ko_flag = true;

	//相手の連のうちとれるものがあるか
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	while (auto pt = YourStone.pop())
	{
		Point root = root_num(pt);
		//もし相手の連の呼吸点がなかったら取る
		if (string[root].StringBreathBB != BrankBB)
		{
			StoneBB[~side] ^= string[root].StringBB;
			BrankBB |= string[root].StringBB;
			//もしとった石の数が1つならコウあるいは着手禁止点
			//（とった連とPointBBが同一なら1つしか取っていないことになる）
			if (ko_flag == true && string[root].StringBB == PointBB[pt])
				KoBB |= PointBB[pt];
		}
		YourStone ^= string[root].StringBB;
	}

	//棋譜をつけて手数を+1に
	record[moves++] = move;
	
	//手番を変える
	side = ~side;
}

//着手
void Position::do_move(Point move, Stateinfo& si)
{
	si.key = key;

	//コウの解消
	si.ko = PT_NULL;
	if (KoBB != ZERO_BB)
	{
		si.ko = KoBB.pop_const();
		key ^= KOKEY[si.ko];
	}
	KoBB = ZERO_BB;

	if (move == PASS)
	{
		side = ~side;
		key ^= SIDEKEY;
		return;
	}

	//石を置く
	BrankBB ^= PointBB[move];
	StoneBB[side] |= PointBB[move];
	key ^= STONEKEY[side][move];

	//連を作成
	string[move] = String(move);

	//自分の連結する連の呼吸点を合成して、それらの連の親ナンバーをこの新しい連に
	Bitboard MyStone = StoneBB[side] & BreathBB[move];
	int p = 0;
	while (auto pt = MyStone.pop())
	{
		Point root = root_num_c(pt);
		string[move].StringBreathBB |= string[root].StringBreathBB;
		string[move].StringBB |= string[root].StringBB;
		string[root].parent_num = move;
		si.myRoot[p++] = root;
		MyStone ^= string[root].StringBB;
	}

	//相手の連のうちとれるものがあるか
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	Bitboard agehama = ZERO_BB;
	p = 0;
	while (auto pt = YourStone.pop())
	{
		Point root = root_num_c(pt);
		//もし相手の連の呼吸点がなかったら取る
		if (string[root].StringBreathBB != BrankBB)
		{
			StoneBB[~side] ^= string[root].StringBB;
			BrankBB |= string[root].StringBB;
			agehama |= string[root].StringBB;
			si.yourRoot[p++] = root;
		}
		YourStone ^= string[root].StringBB;
	}

	//とった石（agehama）のKey処理
	int agehamaNum = 0;
	Point ko_maybe = PT_NULL;
	while (Point pt = agehama.pop())
	{
		key ^= STONEKEY[~side][pt];
		ko_maybe = pt;
		agehamaNum++;
	}

	//コウの判定
	//とった石が一つで、置いた石の連が単独で、置いた石の呼吸点が１つならコウ
	if (agehamaNum == 1 &&
		string[move].StringBB == PointBB[move] &&
		(string[move].StringBreathBB & BrankBB) == PointBB[ko_maybe])
	{
		KoBB = PointBB[ko_maybe];
		key ^= KOKEY[ko_maybe];
	}

	//棋譜をつけて手数を+1に
	record[moves++] = move;

	//手番を変える
	side = ~side;
	key ^= SIDEKEY;

}


bool Position::legal_do_move(Point move)
{
	// 1.moveが盤上の点か
	if (PT_ZERO > move || move >= PT_NB)
		return false;

	// 2.その点が空点かコウでないか
	if (PointBB[move] != (BrankBB ^ KoBB))
		return false;

	//とりあえず石を置いてみる
	BrankBB ^= PointBB[move];
	StoneBB[side] |= PointBB[move];

	// 3.相手の連のうちとれるものがあるか
	Bitboard KoMaybe = ZERO_BB;
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	while (auto pt = YourStone.pop())
	{
		Point root = root_num(pt);
		//もし相手の連の呼吸点がなかったら取る
		if (string[root].StringBreathBB != BrankBB)
		{
			StoneBB[~side] ^= string[root].StringBB;
			BrankBB |= string[root].StringBB;
			//もしとった石の数が1つならコウあるいは着手禁止点になる
			//（とった連とPointBBが同一なら1つしか取っていないことになる）
			if (string[root].StringBB == PointBB[pt])
				KoMaybe |= PointBB[pt];
		}
		YourStone ^= string[root].StringBB;
	}

	// 4.自分の連結する連を新たに作る
	//MyStoneとは、置く位置の呼吸点に存在する自分の石
	string[move] = String(move);
	Point st_num[4];
	int p = 0;
	Bitboard MyStone = StoneBB[side] & BreathBB[move];
	//自分の連結する連の呼吸点を合成する
	while (auto pt = MyStone.pop())
	{
		Point root = root_num(pt);
		string[move].StringBreathBB |= string[root].StringBreathBB;
		string[move].StringBB |= string[root].StringBB;
		MyStone ^= string[root].StringBB;
		st_num[p++] = root;
	}
	//新たな連に呼吸点が存在しなかったら非合法手
	if (string[move].StringBreathBB != BrankBB)
	{
		BrankBB |= PointBB[move];
		StoneBB[side] ^= PointBB[move];
		return false;
	}
	for (int i = 0; i < p; i++) string[st_num[i]].parent_num = move;

	//コウの更新
	KoBB = ZERO_BB;
	if (string[move].StringBB == PointBB[move]) KoBB = KoMaybe;

	//棋譜をつけて手数を+1に
	record[moves++] = move;

	//手番を変える
	side = ~side;

	return true;
}

void Position::undo_move(Point move, Stateinfo& stateinfo)
{
	
	//棋譜を消去して手数を-1に
	record[--moves] = PT_NULL;

	//手番を変える
	side = ~side;

	//Bitboardをもとに
	BrankBB |= PointBB[move];
	StoneBB[side] ^= PointBB[move];
	string[move] = ZERO_ST;

	//自分の連結した連のルートナンバーをもとに
	for (int i = 0; i < 4; i++)
	{
		Point pt = stateinfo.myRoot[i];
		if (pt == PT_NULL) break;
		string[pt].parent_num = PT_NULL;
	}

	//取った相手の石をもとに
	for (int i = 0; i < 4; i++)
	{
		Point pt = stateinfo.yourRoot[i];
		if (pt == PT_NULL) break;
		BrankBB ^= string[pt].StringBB;
		StoneBB[~side] |= string[pt].StringBB;
	}
	
	//コウをもとに
	KoBB = PointBB[stateinfo.ko];

	//keyをもとに
	key = stateinfo.key;
}

//ゲームの結果を計算する(勝者を返す)
Color Position::result_game() const
{
	Bitboard B_area = StoneBB[0];
	Bitboard W_area = StoneBB[1];

	//目を陣地に含める
	B_area |= ((B_area << 1) | (B_area >> 1));
	W_area |= ((W_area << 1) | (W_area >> 1));
	B_area &= (ALL_BB ^ StoneBB[1]);
	W_area &= (ALL_BB ^ StoneBB[0]);

	//陣地の計算
	if (B_area.pop_count() > W_area.pop_count() + KOMI)
		return BLACK;
	else
		return WHITE;
}

//ゲームの結果を計算する(勝者を返す)
Color Position::result_game_maybe(Bitboard* take)
{
	Bitboard area[2];
	Bitboard ichimatsu;
	Bitboard bb;
	Bitboard stone;
	Bitboard B, W;
	
	for (Color c = COLOR_ZERO; c < COLOR_NB; c++)
	{
		side = c;

		//眼らしきところ（四方が自分の石か壁）以外のところを市松模様に石を埋める
		//四方が自分の石か壁の点は埋めない
		stone = StoneBB[c] | WALL_BB;
		bb = ALL_BB;
		bb &= stone << 1;
		bb &= stone >> 1;
		bb &= shiftN(stone);
		bb &= shiftS(stone);
		//取られたところつまり相手の陣地は埋めない
		ichimatsu = ICHIMATSU[0] & take[~c];
		ichimatsu ^= bb;
		ichimatsu &= BrankBB;
		//市松模様に石を置いていく
		while (auto move = ichimatsu.pop())
		{
			BrankBB ^= PointBB[move];
			StoneBB[c] |= PointBB[move];
			Bitboard MyStone = StoneBB[c] & BreathBB[move];
			string[move] = String(move);
			while (auto pt = MyStone.pop())
			{
				Point root = root_num(pt);
				string[move].StringBreathBB |= string[root].StringBreathBB;
				string[move].StringBB |= string[root].StringBB;
				string[root].parent_num = move;
				MyStone ^= string[root].StringBB;
			}
		}
	}

	area[BLACK] = StoneBB[BLACK];
	area[WHITE] = StoneBB[WHITE];

	for (Color c = COLOR_ZERO; c < COLOR_NB; c++)
	{
		//呼吸点が１の連があればその点におけなければ取り上げる
		stone = StoneBB[c];
		while (auto pt = stone.pop())
		{
			bb = string[root_num(pt)].StringBreathBB & BrankBB;
			int pops = bb.pop_count();
			while (pops == 1)
			{
				pt = bb.pop();
				BrankBB ^= PointBB[pt];
				string[pt] = String(pt);
				Bitboard MyStone = StoneBB[c] & BreathBB[pt];
				while (auto pt1 = MyStone.pop())
				{
					Point root = root_num(pt1);
					string[pt].StringBreathBB |= string[root].StringBreathBB;
					string[pt].StringBB |= string[root].StringBB;
					string[root].parent_num = pt;
					MyStone ^= string[root].StringBB;
				}
				bb = string[pt].StringBreathBB & BrankBB;
				pops = bb.pop_count();
			}
			if (pops == 0)
			{
				area[c] ^= string[pt].StringBB;
				area[~c] |= string[pt].StringBB;
			}
			stone ^= string[pt].StringBB;
		}
	}

	//目を陣地に含める
	B = area[BLACK];
	W = area[WHITE];
	area[BLACK] |= ((area[BLACK] << 1) | (area[BLACK] >> 1));
	area[WHITE] |= ((area[WHITE] << 1) | (area[WHITE] >> 1));
	area[BLACK] &= (ALL_BB ^ W);
	area[WHITE] &= (ALL_BB ^ B);

	//陣地の計算
	if (area[BLACK].pop_count() > area[WHITE].pop_count() + KOMI)
		return BLACK;
	else
		return WHITE;
}

//両者の石の競っているところが1のBitboardを返す
Bitboard Position::stick() const
{
	Bitboard stickBB = ALL_BB;

	for (Color c = COLOR_ZERO; c < COLOR_ALL; c++)
	{
		Bitboard around;
		Bitboard bb;
		//uint8_t s1, s2, s3, s4;
		
		around = StoneBB[c];
		ShiftE(bb, around);
		around |= bb;
		ShiftW(bb, around);
		around |= bb;

		//精度のいらない場合は次の4行はいらないかも
		//７、８行目の区切りを考慮したもの
		//s1 = around.b[15];
		//s2 = around.b[16];
		//s3 = around.b[17];
		//s4 = around.b[18];

		ShiftN(bb, around);
		around |= bb;
		ShiftS(bb, around);
		around |= bb;

		//精度のいらない場合は次の4行はいらないかも
		//７、８行目の区切りを考慮したもの
		//around.b[15] |= s3;
		//around.b[16] |= s4;
		//around.b[17] |= s1;
		//around.b[18] |= s2;

		stickBB &= around;
	}
	return stickBB;
}

Bitboard Position::breath(Color c) const
{
	Bitboard stone = StoneBB[c];
	Bitboard breath = ZERO_BB;
	Bitboard bb;

	ShiftE(bb, stone);
	breath |= bb;
	ShiftW(bb, stone);
	breath |= bb;
	ShiftN(bb, stone);
	breath |= bb;
	ShiftS(bb, stone);
	breath |= bb;

	return breath ^ stone;
}

Bitboard Position::around(Color c) const
{
	Bitboard around;
	Bitboard bb;
	//uint8_t s1, s2, s3, s4;
	around = StoneBB[c];
	ShiftE(bb, around);
	around |= bb;
	ShiftW(bb, around);
	around |= bb;

	//精度のいらない場合は次の4行はいらないかも
	//７、８行目の区切りを考慮したもの
	//s1 = around.b[15];
	//s2 = around.b[16];
	//s3 = around.b[17];
	//s4 = around.b[18];

	ShiftN(bb, around);
	around |= bb;
	ShiftS(bb, around);
	around |= bb;

	//精度のいらない場合は次の4行はいらないかも
	//７、８行目の区切りを考慮したもの
	//around.b[15] |= s3;
	//around.b[16] |= s4;
	//around.b[17] |= s1;
	//around.b[18] |= s2;

	return around;
}

//連のルートとなる連を探す
Point Position::root_num(Point pt)
{
	Point root;
	Point parent = string[pt].parent_num;
	//再帰的にrootを見つけ出すと同時に,次回のためにparent_numを更新する
	if (parent == PT_NULL)
		root = pt;
	else {
		root = root_num(parent);
		string[pt].parent_num = root;
	}
	return root;
}

//連のルートとなる連を探す
Point Position::root_num_c(Point pt) const
{
	Point root;
	Point parent = string[pt].parent_num;
	//再帰的にrootを見つけ出す
	if (parent == PT_NULL)
		root = pt;
	else {
		root = root_num_c(parent);
	}
	return root;
}

void Position::print() const
{
	std::cout << " ";
	for (int f = 0; f < BOARD_SIZE; f++) {
		std::cout << " " << std::hex << (f+1);
	}
	std::cout << endl;
	for (int r = 0; r < BOARD_SIZE; r++){
		std::cout << std::hex << (r+1);
		for (int f = 0; f < BOARD_SIZE; f++) {
			int pt = r * 13 + f + 1;
			if (PointBB[pt] == StoneBB[BLACK]) std::cout << " X";
			if (PointBB[pt] == StoneBB[WHITE]) std::cout << " 0";
			if (PointBB[pt] == BrankBB)        std::cout << " +";
		}
		std::cout << endl;
	}
	std::cout << endl;
	//std::cout << BrankBB;
	//std::cout << StoneBB[0];
	//std::cout << StoneBB[1];
}

void Position::print_move() const
{
	if (~side_to_move() == BLACK) cout << "黒 ";
	else                          cout << "白 ";
	Point move = last_move();
	cout << "move = ";
	cout << hex << (1 + (move - 1) / 13) << " ";
	cout << hex << (1 + (move - 1) % 13) << dec << endl;
}

bool Position::is_eye(Point pt)
{
	//呼吸点がすべて自分の石か壁でなければ眼ではない
	if ( !(BreathBB[pt] == StoneBB[side]) )
		return false;

	if (AroundBB[pt] == StoneBB[side])
		return true;
	
	//もし相手の番だとしたらその点に着手できるか。できなければ眼
	bool eye;
	side = ~side;
	eye = ! legal(pt);
	side = ~side;

	return eye;
}


//合法手の点のBitboard
Bitboard Position::legal_bb()
{
	Bitboard legalBB = poss_bb();//着手可能な点を全部ピックアップする
	Bitboard bb = legalBB;
	while (Point pt = bb.pop())
	{
		if (!legal(pt) || is_eye(pt) == true) legalBB ^= PointBB[pt];
	}

	return legalBB;
}

//直前手がとれるか　pt:（最後の）相手の手
Bitboard Position::move_catch(Point pt)
{
	String* st = get_string(pt);
	Bitboard bb = st->StringBreathBB & BrankBB;

	//もし連の呼吸点が１ならばとれる
	if (bb.pop_count() == 1)
		return (bb == KoBB) ? ZERO_BB : bb;

	return ZERO_BB;
}

//アタリかどうか　pt:最後の相手の手
Bitboard Position::move_escape(Point pt)
{
	//逃げる着手
	Bitboard escapeBB = ZERO_BB;

	//直前手に接している自分の石の連について調べる
	Bitboard myStone = BreathBB[pt] & StoneBB[side];
	while (Point move = myStone.pop())
	{
		String* st = get_string(move);
		Bitboard bb = st->StringBreathBB & BrankBB;

		//もし連の呼吸点が１ならば逃げる候補
		if (bb.pop_count() == 1)
			escapeBB |= bb;
		myStone ^= st->StringBB;
	}
	return escapeBB;
}

//シチョウかどうか。簡易的なもの。再考の余地あり。
//シチョウかもしれない場合、とれる次の手が返る。シチョウでない場合、PT_NULLが返る
Bitboard Position::move_shichou(Point pt)
{
	//直前手の連の呼吸点とその数
	Bitboard breath = BrankBB & get_string(pt)->StringBreathBB;
	int breath_num = breath.pop_count();

	//呼吸点の数が１のとき
	if (breath_num == 1)
	{
		return (breath == KoBB) ? ZERO_BB : breath;
	}
	//呼吸点の数が２のとき
	else if (breath_num == 2)
	{
		//shichouBB:取れるかもしれない着手
		Bitboard shichouBB = ZERO_BB;
		Bitboard st_b = breath;
		for (int i = 0; i < 2; i++)
		{
			Point move = st_b.pop();
			if (!legal(move)) continue;
			//再帰的にシチョウかどうかを探索する
			if (shichou_search(breath, BrankBB, move)) shichouBB |= PointBB[move];
		}
		return shichouBB;
	}
	//それ以上の呼吸点があるときはシチョウではない
	return ZERO_BB;
}

//true:取れるかも false:取れない
bool shichou_search(Bitboard breath, Bitboard brank, Point pt)
{
	//ptに石を置く（取ろうとする側）
	breath ^= PointBB[pt];
	brank  ^= PointBB[pt];

	//逃げる
	//手は残り一つの呼吸点しかない
	Point escape = breath.pop();
	brank ^= PointBB[escape];
	breath |= BreathBB[escape] & brank;

	int breath_num = breath.pop_count();
	if (breath_num < 2) 
		return true; //新たな呼吸点が2つもなければ必ず取れる
	if (breath_num == 2)
	{
		Bitboard st_b = breath;
		for (int i = 0; i < 2; i++)
		{
			Point move = st_b.pop();
			if (shichou_search(breath, brank, move)) return true;
		}
	}
	//呼吸点が3つ以上あれば取れない
	return false;	
}
