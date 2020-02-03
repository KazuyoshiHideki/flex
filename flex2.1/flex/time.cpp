#include"stdafx.h"
#include"time.h"

// ms’PˆÊ‚ÅŒ»İ‚ğ•Ô‚·
inline Time now() {
	return std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::steady_clock::now().time_since_epoch()).count();
}


uint64_t rand64()
{
	static uint64_t s = (uint64_t)now();
	s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
	return s * 2685821657736338717LL;
}

