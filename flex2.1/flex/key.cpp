#include"stdafx.h"
#include "key.h"

Key STONEKEY[COLOR_ALL][PT_NB];
Key KOKEY[PT_NB];
Key SIDEKEY;

void Keys::init()
{
	SIDEKEY = rand64();
	for (Point p = PT_NULL; p < PT_NB; p++)
	{
		KOKEY[p] = rand64();
		for (Color c = COLOR_ZERO; c < COLOR_NB; c++)
			STONEKEY[c][p] = rand64();
	}	
}