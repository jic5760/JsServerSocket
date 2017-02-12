/**
 * @file	LockableEx.h
 * @class	LockableEx
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/14
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_LOCKABLEEX_H__
#define __JSCPPUTILS_LOCKABLEEX_H__

#include "Lockable.h"
#include "TSSimpleMap.h"

namespace JsCPPUtils
{
	class LockableEx
	{
	private:
		Lockable m_lock;
#if defined(JSCUTILS_OS_WINDOWS)
		TSSimpleMap<DWORD, int> m_tinfo;
#elif defined(JSCUTILS_OS_LINUX)
		TSSimpleMap<pthread_t, int> m_tinfo;
#endif

	public:
		LockableEx();
		~LockableEx();
		int lock();
		int unlock();

	};
}

#endif
