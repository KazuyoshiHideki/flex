#pragma once
#include"stdafx.h"

void prt(const char *fmt, ...);
void send_gtp(const char *fmt, ...);
void gtp_loop();

void start_game();
Point move_scan(Position& pos);

extern Color think_side;