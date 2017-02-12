/**
 * @file	Common.cpp
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/09/27
 * @brief	JsCUtils Common file
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <time.h>

#include "Common.h"

#ifdef JSCUTILS_OS_WINDOWS
#include <Windows.h>
#endif

namespace JsCPPUtils
{
	int64_t Common::getTickCount()
	{
#if defined(JSCUTILS_OS_WINDOWS)
		return GetTickCount64();
#elif defined(JSCUTILS_OS_LINUX)
		struct timespec ts = {0};
		int64_t ticks = 0;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		ticks  = ((int64_t)(ts.tv_nsec / 1000000));
		ticks += ((int64_t)(ts.tv_sec)) * 1000;
		return ticks;
#endif
	}
}