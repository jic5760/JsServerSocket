/**
 * @file	TSSimpleMap.h
 * @class	TSSimpleMap
 * @author	Jichan (jic5760@naver.com / ablog.jc-lab.net)
 * @date	2016/09/30
 * @brief	Thread-Safe Simple Map
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_TSSIMPLEMAP_H__
#define __JSCPPUTILS_TSSIMPLEMAP_H__

#include <map>

#include "Lockable.h"

namespace JsCPPUtils
{
	template <typename TKEY, typename TVALUE>
	class TSSimpleMap : private Lockable
	{
	private:
		std::map<TKEY, TVALUE > m_map;

	public:
		TSSimpleMap() : 
			JsCPPUtils::Lockable()
		{
		}
		~TSSimpleMap()
		{
		}

		/*
		TVALUE& operator[] (const TKEY& key) {
			lock();
			TVALUE& valueref = m_map[key];
			unlock();
			return valueref;
		}
		*/

		TVALUE get(const TKEY& key) {
			TVALUE value;
			lock();
			value = m_map[key];
			unlock();
			return value;
		}

		void set(const TKEY& key, const TVALUE& value) {
			lock();
			m_map[key] = value;
			unlock();
		}
	};

}

#endif
