/**
 * @file	Lockable.h
 * @class	Lockable
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/14
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_LOCKABLE_H__
#define __JSCPPUTILS_LOCKABLE_H__

#include "Common.h"

#if defined(JSCUTILS_OS_WINDOWS)
#include <windows.h>
#elif defined(JSCUTILS_OS_LINUX)
#include <pthread.h>
#endif

namespace JsCPPUtils
{

	class Lockable
	{
	private:
#if defined(JSCUTILS_OS_WINDOWS)
		HANDLE m_hMutex;
#elif defined(JSCUTILS_OS_LINUX)
		pthread_mutex_t m_mutex;
#endif

	public:
		Lockable();
		~Lockable();
		int lock();
		int unlock();
	};

}

#endif
