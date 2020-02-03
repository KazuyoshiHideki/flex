// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#include <immintrin.h>
#include <intrin.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdarg.h>
#include <vector>
#include <thread>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#include "flex.h"
#include "key.h"
#include "bitboard.h"
#include "position.h"
#include "playout.h"
#include "gtp.h"
#include "time.h"
#include "node.h"
#include "search.h"
#include "move_picker.h"
#include "tt.h"
