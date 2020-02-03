#include"stdafx.h"
#include "position.h"

using namespace std;


//�����ǖʂ̏��ɃZ�b�g����
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

//���@�肩�ǂ����`�F�b�N
bool Position::legal(Point move) const
{
	
	// 0.move���Տ�̓_��
	if (PT_ZERO > move || move >= PT_NB)
		return false;

	// 0.���̓_����_�łȂ���ΑłĂȂ�
	if (PointBB[move] != BrankBB)
		return false;
	
	// 1.�ċz�_������΍��@��
	if (!(BreathBB[move] != BrankBB))
		return true;

	// 2.�R�E�Ȃ�񍇖@��  pop()���Ăяo���O��KoBB��andnot���ăR�E�̒�����Ԃ��̂Ȃ炱�̏����͕s�v
	if (PointBB[move] == KoBB) return false;
	
	//���ɒu��������BrankBB
	Bitboard ProBrankBB = BrankBB ^ PointBB[move];

	// 3.�����̘A������A�Ɍċz�_������Ȃ獇�@��
	//MyStone�Ƃ́A�u���ʒu�̌ċz�_�ɑ��݂��鎩���̐�
	Bitboard MyStone = StoneBB[side] & BreathBB[move];
	String NewString(move);
	//�����̘A������A�̌ċz�_����������
	while (auto pt = MyStone.pop())
	{
		Point root = root_num_c(pt);
		NewString.StringBreathBB |= string[root].StringBreathBB;
		//�A�������A�Ɍċz�_������Ȃ獇�@��
		if (!(NewString.StringBreathBB != ProBrankBB))
			return true;
		MyStone ^= string[root].StringBB;
	}

	// 4.����̘A���Ƃ��Ȃ獇�@��
	//YourStone�Ƃ́A�u���ʒu�̌ċz�_�ɑ��݂��鎩���̐�
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	while (auto pt = YourStone.pop())
	{
		Point root = root_num_c(pt);
		//��������̘A�̌ċz�_���Ȃ����������̂ō��@��
		if (string[root].StringBreathBB != ProBrankBB)
			return true;
		YourStone ^= string[root].StringBB;
	}

	return false;
}

//����
void Position::do_move(Point move)
{
	//�R�E�̉���
	KoBB = ZERO_BB;
	bool ko_flag = false;

	//�΂�u��
	BrankBB ^= PointBB[move];
	StoneBB[side] |= PointBB[move];

	//�A���쐬
	string[move] = String(move);

	//�����̘A������A�̌ċz�_���������āA�����̘A�̐e�i���o�[�����̐V�����A��
	Bitboard MyStone = StoneBB[side] & BreathBB[move];
	while (auto pt = MyStone.pop())
	{
		Point root = root_num(pt);
		string[move].StringBreathBB |= string[root].StringBreathBB;
		string[move].StringBB |= string[root].StringBB;
		string[root].parent_num = move;
		MyStone ^= string[root].StringBB;
	}
	//�����u�����΂̘A�̐ΐ���1�Ȃ�R�E���邢�͒���֎~�_�̉\������
	//�i�A��PointBB������Ȃ�1��������Ă��Ȃ����ƂɂȂ�j
	if (string[move].StringBB == PointBB[move])
		ko_flag = true;

	//����̘A�̂����Ƃ����̂����邩
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	while (auto pt = YourStone.pop())
	{
		Point root = root_num(pt);
		//��������̘A�̌ċz�_���Ȃ���������
		if (string[root].StringBreathBB != BrankBB)
		{
			StoneBB[~side] ^= string[root].StringBB;
			BrankBB |= string[root].StringBB;
			//�����Ƃ����΂̐���1�Ȃ�R�E���邢�͒���֎~�_
			//�i�Ƃ����A��PointBB������Ȃ�1��������Ă��Ȃ����ƂɂȂ�j
			if (ko_flag == true && string[root].StringBB == PointBB[pt])
				KoBB |= PointBB[pt];
		}
		YourStone ^= string[root].StringBB;
	}

	//���������Ď萔��+1��
	record[moves++] = move;
	
	//��Ԃ�ς���
	side = ~side;
}

//����
void Position::do_move(Point move, Stateinfo& si)
{
	si.key = key;

	//�R�E�̉���
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

	//�΂�u��
	BrankBB ^= PointBB[move];
	StoneBB[side] |= PointBB[move];
	key ^= STONEKEY[side][move];

	//�A���쐬
	string[move] = String(move);

	//�����̘A������A�̌ċz�_���������āA�����̘A�̐e�i���o�[�����̐V�����A��
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

	//����̘A�̂����Ƃ����̂����邩
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	Bitboard agehama = ZERO_BB;
	p = 0;
	while (auto pt = YourStone.pop())
	{
		Point root = root_num_c(pt);
		//��������̘A�̌ċz�_���Ȃ���������
		if (string[root].StringBreathBB != BrankBB)
		{
			StoneBB[~side] ^= string[root].StringBB;
			BrankBB |= string[root].StringBB;
			agehama |= string[root].StringBB;
			si.yourRoot[p++] = root;
		}
		YourStone ^= string[root].StringBB;
	}

	//�Ƃ����΁iagehama�j��Key����
	int agehamaNum = 0;
	Point ko_maybe = PT_NULL;
	while (Point pt = agehama.pop())
	{
		key ^= STONEKEY[~side][pt];
		ko_maybe = pt;
		agehamaNum++;
	}

	//�R�E�̔���
	//�Ƃ����΂���ŁA�u�����΂̘A���P�ƂŁA�u�����΂̌ċz�_���P�Ȃ�R�E
	if (agehamaNum == 1 &&
		string[move].StringBB == PointBB[move] &&
		(string[move].StringBreathBB & BrankBB) == PointBB[ko_maybe])
	{
		KoBB = PointBB[ko_maybe];
		key ^= KOKEY[ko_maybe];
	}

	//���������Ď萔��+1��
	record[moves++] = move;

	//��Ԃ�ς���
	side = ~side;
	key ^= SIDEKEY;

}


bool Position::legal_do_move(Point move)
{
	// 1.move���Տ�̓_��
	if (PT_ZERO > move || move >= PT_NB)
		return false;

	// 2.���̓_����_���R�E�łȂ���
	if (PointBB[move] != (BrankBB ^ KoBB))
		return false;

	//�Ƃ肠�����΂�u���Ă݂�
	BrankBB ^= PointBB[move];
	StoneBB[side] |= PointBB[move];

	// 3.����̘A�̂����Ƃ����̂����邩
	Bitboard KoMaybe = ZERO_BB;
	Bitboard YourStone = StoneBB[~side] & BreathBB[move];
	while (auto pt = YourStone.pop())
	{
		Point root = root_num(pt);
		//��������̘A�̌ċz�_���Ȃ���������
		if (string[root].StringBreathBB != BrankBB)
		{
			StoneBB[~side] ^= string[root].StringBB;
			BrankBB |= string[root].StringBB;
			//�����Ƃ����΂̐���1�Ȃ�R�E���邢�͒���֎~�_�ɂȂ�
			//�i�Ƃ����A��PointBB������Ȃ�1��������Ă��Ȃ����ƂɂȂ�j
			if (string[root].StringBB == PointBB[pt])
				KoMaybe |= PointBB[pt];
		}
		YourStone ^= string[root].StringBB;
	}

	// 4.�����̘A������A��V���ɍ��
	//MyStone�Ƃ́A�u���ʒu�̌ċz�_�ɑ��݂��鎩���̐�
	string[move] = String(move);
	Point st_num[4];
	int p = 0;
	Bitboard MyStone = StoneBB[side] & BreathBB[move];
	//�����̘A������A�̌ċz�_����������
	while (auto pt = MyStone.pop())
	{
		Point root = root_num(pt);
		string[move].StringBreathBB |= string[root].StringBreathBB;
		string[move].StringBB |= string[root].StringBB;
		MyStone ^= string[root].StringBB;
		st_num[p++] = root;
	}
	//�V���ȘA�Ɍċz�_�����݂��Ȃ�������񍇖@��
	if (string[move].StringBreathBB != BrankBB)
	{
		BrankBB |= PointBB[move];
		StoneBB[side] ^= PointBB[move];
		return false;
	}
	for (int i = 0; i < p; i++) string[st_num[i]].parent_num = move;

	//�R�E�̍X�V
	KoBB = ZERO_BB;
	if (string[move].StringBB == PointBB[move]) KoBB = KoMaybe;

	//���������Ď萔��+1��
	record[moves++] = move;

	//��Ԃ�ς���
	side = ~side;

	return true;
}

void Position::undo_move(Point move, Stateinfo& stateinfo)
{
	
	//�������������Ď萔��-1��
	record[--moves] = PT_NULL;

	//��Ԃ�ς���
	side = ~side;

	//Bitboard�����Ƃ�
	BrankBB |= PointBB[move];
	StoneBB[side] ^= PointBB[move];
	string[move] = ZERO_ST;

	//�����̘A�������A�̃��[�g�i���o�[�����Ƃ�
	for (int i = 0; i < 4; i++)
	{
		Point pt = stateinfo.myRoot[i];
		if (pt == PT_NULL) break;
		string[pt].parent_num = PT_NULL;
	}

	//���������̐΂����Ƃ�
	for (int i = 0; i < 4; i++)
	{
		Point pt = stateinfo.yourRoot[i];
		if (pt == PT_NULL) break;
		BrankBB ^= string[pt].StringBB;
		StoneBB[~side] |= string[pt].StringBB;
	}
	
	//�R�E�����Ƃ�
	KoBB = PointBB[stateinfo.ko];

	//key�����Ƃ�
	key = stateinfo.key;
}

//�Q�[���̌��ʂ��v�Z����(���҂�Ԃ�)
Color Position::result_game() const
{
	Bitboard B_area = StoneBB[0];
	Bitboard W_area = StoneBB[1];

	//�ڂ�w�n�Ɋ܂߂�
	B_area |= ((B_area << 1) | (B_area >> 1));
	W_area |= ((W_area << 1) | (W_area >> 1));
	B_area &= (ALL_BB ^ StoneBB[1]);
	W_area &= (ALL_BB ^ StoneBB[0]);

	//�w�n�̌v�Z
	if (B_area.pop_count() > W_area.pop_count() + KOMI)
		return BLACK;
	else
		return WHITE;
}

//�Q�[���̌��ʂ��v�Z����(���҂�Ԃ�)
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

		//��炵���Ƃ���i�l���������̐΂��ǁj�ȊO�̂Ƃ�����s���͗l�ɐ΂𖄂߂�
		//�l���������̐΂��ǂ̓_�͖��߂Ȃ�
		stone = StoneBB[c] | WALL_BB;
		bb = ALL_BB;
		bb &= stone << 1;
		bb &= stone >> 1;
		bb &= shiftN(stone);
		bb &= shiftS(stone);
		//���ꂽ�Ƃ���܂葊��̐w�n�͖��߂Ȃ�
		ichimatsu = ICHIMATSU[0] & take[~c];
		ichimatsu ^= bb;
		ichimatsu &= BrankBB;
		//�s���͗l�ɐ΂�u���Ă���
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
		//�ċz�_���P�̘A������΂��̓_�ɂ����Ȃ���Ύ��グ��
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

	//�ڂ�w�n�Ɋ܂߂�
	B = area[BLACK];
	W = area[WHITE];
	area[BLACK] |= ((area[BLACK] << 1) | (area[BLACK] >> 1));
	area[WHITE] |= ((area[WHITE] << 1) | (area[WHITE] >> 1));
	area[BLACK] &= (ALL_BB ^ W);
	area[WHITE] &= (ALL_BB ^ B);

	//�w�n�̌v�Z
	if (area[BLACK].pop_count() > area[WHITE].pop_count() + KOMI)
		return BLACK;
	else
		return WHITE;
}

//���҂̐΂̋����Ă���Ƃ��낪1��Bitboard��Ԃ�
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

		//���x�̂���Ȃ��ꍇ�͎���4�s�͂���Ȃ�����
		//�V�A�W�s�ڂ̋�؂���l����������
		//s1 = around.b[15];
		//s2 = around.b[16];
		//s3 = around.b[17];
		//s4 = around.b[18];

		ShiftN(bb, around);
		around |= bb;
		ShiftS(bb, around);
		around |= bb;

		//���x�̂���Ȃ��ꍇ�͎���4�s�͂���Ȃ�����
		//�V�A�W�s�ڂ̋�؂���l����������
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

	//���x�̂���Ȃ��ꍇ�͎���4�s�͂���Ȃ�����
	//�V�A�W�s�ڂ̋�؂���l����������
	//s1 = around.b[15];
	//s2 = around.b[16];
	//s3 = around.b[17];
	//s4 = around.b[18];

	ShiftN(bb, around);
	around |= bb;
	ShiftS(bb, around);
	around |= bb;

	//���x�̂���Ȃ��ꍇ�͎���4�s�͂���Ȃ�����
	//�V�A�W�s�ڂ̋�؂���l����������
	//around.b[15] |= s3;
	//around.b[16] |= s4;
	//around.b[17] |= s1;
	//around.b[18] |= s2;

	return around;
}

//�A�̃��[�g�ƂȂ�A��T��
Point Position::root_num(Point pt)
{
	Point root;
	Point parent = string[pt].parent_num;
	//�ċA�I��root�������o���Ɠ�����,����̂��߂�parent_num���X�V����
	if (parent == PT_NULL)
		root = pt;
	else {
		root = root_num(parent);
		string[pt].parent_num = root;
	}
	return root;
}

//�A�̃��[�g�ƂȂ�A��T��
Point Position::root_num_c(Point pt) const
{
	Point root;
	Point parent = string[pt].parent_num;
	//�ċA�I��root�������o��
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
	if (~side_to_move() == BLACK) cout << "�� ";
	else                          cout << "�� ";
	Point move = last_move();
	cout << "move = ";
	cout << hex << (1 + (move - 1) / 13) << " ";
	cout << hex << (1 + (move - 1) % 13) << dec << endl;
}

bool Position::is_eye(Point pt)
{
	//�ċz�_�����ׂĎ����̐΂��ǂłȂ���Ί�ł͂Ȃ�
	if ( !(BreathBB[pt] == StoneBB[side]) )
		return false;

	if (AroundBB[pt] == StoneBB[side])
		return true;
	
	//��������̔Ԃ��Ƃ����炻�̓_�ɒ���ł��邩�B�ł��Ȃ���Ί�
	bool eye;
	side = ~side;
	eye = ! legal(pt);
	side = ~side;

	return eye;
}


//���@��̓_��Bitboard
Bitboard Position::legal_bb()
{
	Bitboard legalBB = poss_bb();//����\�ȓ_��S���s�b�N�A�b�v����
	Bitboard bb = legalBB;
	while (Point pt = bb.pop())
	{
		if (!legal(pt) || is_eye(pt) == true) legalBB ^= PointBB[pt];
	}

	return legalBB;
}

//���O�肪�Ƃ�邩�@pt:�i�Ō�́j����̎�
Bitboard Position::move_catch(Point pt)
{
	String* st = get_string(pt);
	Bitboard bb = st->StringBreathBB & BrankBB;

	//�����A�̌ċz�_���P�Ȃ�΂Ƃ��
	if (bb.pop_count() == 1)
		return (bb == KoBB) ? ZERO_BB : bb;

	return ZERO_BB;
}

//�A�^�����ǂ����@pt:�Ō�̑���̎�
Bitboard Position::move_escape(Point pt)
{
	//�����钅��
	Bitboard escapeBB = ZERO_BB;

	//���O��ɐڂ��Ă��鎩���̐΂̘A�ɂ��Ē��ׂ�
	Bitboard myStone = BreathBB[pt] & StoneBB[side];
	while (Point move = myStone.pop())
	{
		String* st = get_string(move);
		Bitboard bb = st->StringBreathBB & BrankBB;

		//�����A�̌ċz�_���P�Ȃ�Γ�������
		if (bb.pop_count() == 1)
			escapeBB |= bb;
		myStone ^= st->StringBB;
	}
	return escapeBB;
}

//�V�`���E���ǂ����B�ȈՓI�Ȃ��́B�čl�̗]�n����B
//�V�`���E��������Ȃ��ꍇ�A�Ƃ�鎟�̎肪�Ԃ�B�V�`���E�łȂ��ꍇ�APT_NULL���Ԃ�
Bitboard Position::move_shichou(Point pt)
{
	//���O��̘A�̌ċz�_�Ƃ��̐�
	Bitboard breath = BrankBB & get_string(pt)->StringBreathBB;
	int breath_num = breath.pop_count();

	//�ċz�_�̐����P�̂Ƃ�
	if (breath_num == 1)
	{
		return (breath == KoBB) ? ZERO_BB : breath;
	}
	//�ċz�_�̐����Q�̂Ƃ�
	else if (breath_num == 2)
	{
		//shichouBB:���邩������Ȃ�����
		Bitboard shichouBB = ZERO_BB;
		Bitboard st_b = breath;
		for (int i = 0; i < 2; i++)
		{
			Point move = st_b.pop();
			if (!legal(move)) continue;
			//�ċA�I�ɃV�`���E���ǂ�����T������
			if (shichou_search(breath, BrankBB, move)) shichouBB |= PointBB[move];
		}
		return shichouBB;
	}
	//����ȏ�̌ċz�_������Ƃ��̓V�`���E�ł͂Ȃ�
	return ZERO_BB;
}

//true:���邩�� false:���Ȃ�
bool shichou_search(Bitboard breath, Bitboard brank, Point pt)
{
	//pt�ɐ΂�u���i��낤�Ƃ��鑤�j
	breath ^= PointBB[pt];
	brank  ^= PointBB[pt];

	//������
	//��͎c���̌ċz�_�����Ȃ�
	Point escape = breath.pop();
	brank ^= PointBB[escape];
	breath |= BreathBB[escape] & brank;

	int breath_num = breath.pop_count();
	if (breath_num < 2) 
		return true; //�V���Ȍċz�_��2���Ȃ���ΕK������
	if (breath_num == 2)
	{
		Bitboard st_b = breath;
		for (int i = 0; i < 2; i++)
		{
			Point move = st_b.pop();
			if (shichou_search(breath, brank, move)) return true;
		}
	}
	//�ċz�_��3�ȏ゠��Ύ��Ȃ�
	return false;	
}
