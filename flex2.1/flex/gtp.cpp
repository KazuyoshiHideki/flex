#include "stdafx.h"
#include "gtp.h"

#define _CRT_SECURE_NO_WARNINGS
#define STR_MAX 256
#define TOKEN_MAX 3

using namespace std;

void prt(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	//{ FILE *fp = fopen("out.txt","a"); if ( fp ) { vfprt( fp, fmt, ap ); fclose(fp); } }
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
void send_gtp(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}
void gtp_loop()
{
	// �T���J�n�ǖ�(root)���i�[����Position�N���X
	Position pos;
	char sa[TOKEN_MAX][STR_MAX];

	string cmd, token;
	while (getline(cin, cmd))
	{
		istringstream is(cmd);

		token = "";
		is >> skipws >> token;

		break; //����͉�

		if (token == "boardsize") {
			//    int new_board_size = atoi( sa[1] );
			send_gtp("= \n\n");
		}
		else if (token == "clear_board") {
			pos = Position();
			send_gtp("= \n\n");
		}
		else if (token == "quit") {
			break;
		}
		else if (token == "protocol_version") {
			send_gtp("= 2\n\n");
		}
		else if (token == "name") {
			send_gtp("= your_program_name\n\n");
		}
		else if (token == "version") {
			send_gtp("= 0.0.1\n\n");
		}
		else if (token == "list_commands") {
			send_gtp("= boardsize\nclear_board\nquit\nprotocol_version\nundo\n"
				"name\nversion\nlist_commands\nkomi\ngenmove\nplay\n\n");
		}
		else if (token == "komi") {
			//thinkType.komi = atof(sa[1]);
			send_gtp("= \n\n");
		}
		else if (token == "undo") {
			//undo();
			send_gtp("= \n\n");
		}
		else if (token == "genmove") {
			if (tolower(sa[1][0]) == 'b' && pos.side_to_move() == WHITE) pos.flip_color();

			//z = play_computer_move(color, SEARCH_UCT);
			//send_gtp("= %s\n\n", get_char_z((int)move));
		}
		else if (token == "play") {  // "play b c4", "play w d17"
			if (tolower(sa[1][0]) == 'b' && pos.side_to_move() == WHITE) pos.flip_color();
			char ax, x, y, z;
			ax = tolower(sa[2][0]);
			x = ax - 'a' + 1;
			if (ax >= 'i') x--;
			y = atoi(&sa[2][1]);
			//z = get_z(x, B_SIZE - y + 1);
			if (tolower(sa[2][0]) == 'p') z = 0;  // pass
			//add_moves(z, color, 0);
			send_gtp("= \n\n");
		}
		else {
			send_gtp("? unknown_command\n\n");
		}
		/*
		if (token == "quit" || token == "stop")
		{
			Search::Signals.stop = true;
			Threads.main()->notify_one(); // main thread�Ɏ󗝂�����

			if (token == "quit") break;
		}

		// �^����ꂽ�ǖʂɂ��Ďv�l����R�}���h
		else if (token == "go") go_cmd(pos, is);

		// (�v�l�ȂǂɎg�����߂�)�J�n�ǖ�(root)��ݒ肷��
		else if (token == "position") position_cmd(pos, is);

		// �N���������Ȃ肱�ꂪ���ł���̂ő��U�������Ȃ��ƃ^�C���A�E�g�ɂȂ�B
		else if (token == "usi")
			sync_cout << "id name " << engine_info() << Options << "usiok" << sync_endl;

		// �I�v�V������ݒ肷��
		else if (token == "setoption") setoption_cmd(is);

		// �v�l�G���W���̏������o�������̊m�F
		else if (token == "isready") is_ready_cmd();

		// ���[�U�[�ɂ������p�R�}���h�Buser.cpp��user()���Ăяo�����B
		else if (token == "user") user_test(pos, is);

		// ���݂̋ǖʂ�\������B(�f�o�b�O�p)
		else if (token == "d") cout << pos << endl;

		// �w���萶���Ղ�̋ǖʂ��Z�b�g����B
		else if (token == "matsuri") pos.set("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w GR5pnsg 1");

		// "position sfen"�̗��B
		else if (token == "sfen") position_cmd(pos, is);

		// ���O�t�@�C���̏����o����on
		else if (token == "log") start_logger(true);

		// ���݂̋ǖʂɂ��ĕ]���֐����Ăяo���āA���̒l��Ԃ��B
		else if (token == "eval") cout << "eval = " << Eval::eval(pos) << endl;
		else if (token == "evalstat") Eval::print_eval_stat(pos);

		// ���̋ǖʂł̎w��������ׂďo��
		else if (token == "moves") {
			for (auto m : MoveList<LEGAL_ALL>(pos))
				cout << m.move << ' ';
			cout << endl;
		}

		// ���̋ǖʂ��l��ł��邩�̔���
		else if (token == "mated") cout << pos.is_mated() << endl;

		// ���̋ǖʂ�hash key�̒l���o��
		else if (token == "key") cout << hex << pos.state()->key() << dec << endl;
		*/
	}
}

void start_game()
{

	int player[COLOR_ALL];
	const int max_moves = MAX_MOVES;
	int wins[COLOR_ALL] = { 0, 0 };
	Color winner;
	Color newType = BLACK;
	int loops = 1;

	cout << "�V�����΋ǊJ�n" << endl;
	cout << "���ԁ@�l�F0�@�R���s���[�^�F1" << endl;
	cin >> player[BLACK];
	cout << "���ԁ@�l�F0�@�R���s���[�^�F1" << endl;
	cin >> player[WHITE];
	
	for (int loop = 0; loop < loops; loop++)
	{
		Position pos;
		Point move;
		
		for (int moves = 0; moves < max_moves; moves++)
		{
			
			if (player[pos.side_to_move()] == 0)
				move = move_scan(pos);
			else {
				think_side = (pos.side_to_move() == newType) ? NEW : OLD;
				move = Search(pos).think();
			}
			if (move == PASS && pos.last_move() == PASS) { winner = pos.result_game(); break; }
			if (move == RESIGN) { winner = ~pos.side_to_move(); break; }
			
			Stateinfo si;
			pos.do_move(move,si);
			pos.print_move();
			pos.print();
		}
		pos.print();
		wins[winner ^ newType] ++;
		std::cout << "newType = " << newType << " winner = " << winner << endl;
		std::cout << dec << "��" << wins[OLD] << "���@�V" << wins[NEW] << "��" << endl;
		newType = ~newType;
	}

	
	std::cout << "�I��";
}

Point move_scan(Position& pos)
{
	int rank, file;
	Point move;

	do {
		cout << "�s ";
		cin >> std::hex >> rank;
		cout << "�� ";
		cin >> std::hex >> file;

		move = (Point)((rank - 1) * 13 + file);
	} while (move != PASS && move != RESIGN && !pos.legal(move));

	return move;
}
