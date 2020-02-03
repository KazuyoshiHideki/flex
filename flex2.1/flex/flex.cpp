// flex.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//
#include "stdafx.h"

using namespace std;

void point_print(Point pt)
{
	std::cout << "point = ";
	std::cout << hex << (1 + (pt - 1) / 13) << " ";
	std::cout << hex << (1 + (pt - 1) % 13) << dec;// << endl;
}

int main()
{
	Bitboards::init();
	Ucts::init();
	Keys::init();

	start_game();
	//gtp_loop();
	int a;
	cin >> a;
	for (int i = 0; i < 1000; i++) {
		Position pos;
		Playout think(pos);
		think.playout();
	}
	
	int shuryo;
	cout << "�I��";
	std::cin >> shuryo;
    return 0;
}

