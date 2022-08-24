#pragma once

#include "MessageHandler.h"

namespace msgs
{
	/*
	Format:

		0 = type(int32_t)
		1 = x pos (int32_t)
		2 = y pos (int32_t)
		3 = radius (int32_t)
	*/
	Message msg_fetchWorldState(const Message& msg);
}
