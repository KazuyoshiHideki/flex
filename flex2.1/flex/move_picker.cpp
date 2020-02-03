#include "stdafx.h"
#include "move_picker.h"


Point MovePicker::next_move()
{

	Point move;
	while (true)
	{
		while (stageBB == ZERO_BB)
		{
			stageBB = pos.poss_bb() ^ doneBB;
			switch (++stage)
			{
			case MAIN_SEARCH_START:
				stageBB &= PointBB[ttMove];
			case CAPTURES:
				stageBB &= pos.move_catch(pos.last_move());
				break;
			case ESCAPES:
				stageBB &= pos.move_escape(pos.last_move());
				break;
			case KILLERS:
				stageBB &= PointBB[ss->killers[0]] | PointBB[ss->killers[1]];
				break;
			case LAST_AROUND:
				stageBB &= AroundBB[pos.last_move()];
				break;
			case GOOD_FAR:
			{
				int i = 0;
				stageBB ^= ss->followAroundBB;
				while (true)
				{
					if (PointBB[rootMoves->at(i).pv[0]] == stageBB && pos.legal(rootMoves->at(i).pv[0]))
					{
						stageBB = PointBB[rootMoves->at(i).pv[0]];
						break;
					}
					i++;
				}
				break;
			}
			case FOLLOW_AROUND:
				stageBB &= ss->followAroundBB;
				break;
			case BAD_FAR:
				stageBB &= UNMASK234_BB;
				break;
			case OTHERS:
				stageBB &= ALL_BB;
				break;
			case STOP:
				return PT_NULL;
			default:
				return PT_NULL;
			}
			
		}
		move = stageBB.pop();
		doneBB |= PointBB[move];
		if (pos.legal(move))
			break;
	}

	return move;
}