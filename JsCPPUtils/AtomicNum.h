/**
 * @file	AtomicNum.h
 * @class	AtomicNum
 * @brief	Thread-Safe한 숫자형을 구현한 클래스
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_ATOMICNUM_H__
#define __JSCPPUTILS_ATOMICNUM_H__

#include "LockableEx.h"

namespace JsCPPUtils
{

	template <typename T, bool _usebymanythread = false>
	class AtomicNum : public LockableEx
	{
	private:
		T m_value;

	public:
		AtomicNum() : 
			LockableEx(),
			m_value(0)
		{
		}

		AtomicNum(int initialvalue) : 
			LockableEx(),
			m_value(initialvalue)
		{
		}
	
		~AtomicNum()
		{
		}

		void set(T y)
		{
			LockableEx::lock();
			m_value = y;
			LockableEx::unlock(_usebymanythread);
		}
		
		void operator=(T y)
		{
			LockableEx::lock();
			m_value = y;
			LockableEx::unlock(_usebymanythread);
		}

		operator T()
		{
			T value;
			LockableEx::lock();
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return value;
		}

		T get()
		{
			T value;
			LockableEx::lock();
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return value;
		}

		T getset(T value)
		{
			T old;
			LockableEx::lock();
			old = m_value;
			m_value = value;
			LockableEx::unlock(_usebymanythread);
			return old;
		}

		T getifset(T value, T ifvalue)
		{
			T old;
			LockableEx::lock();
			old = m_value;
			if(old == ifvalue)
				m_value = value;
			LockableEx::unlock(_usebymanythread);
			return old;
		}

		T getifnset(T value, T ifnvalue)
		{
			T old;
			LockableEx::lock();
			old = m_value;
			if(old != ifnvalue)
				m_value = value;
			LockableEx::unlock(_usebymanythread);
			return old;
		}
		
		/*
#if defined(_MSC_VER)
		AtomicNum& operator--()
		{
			T value;
			LockableEx::lock();
			m_value -= 1;
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return *this;
		}

		T operator--(int)
		{
			T value;
			LockableEx::lock();
			value = m_value;
			m_value -= 1;
			LockableEx::unlock(_usebymanythread);
			return value;
		}

		AtomicNum& operator++()
		{
			T value;
			LockableEx::lock();
			m_value += 1;
			value = m_value;
			LockableEx::unlock(_usebymanythread);
			return *this;
		}

		T operator++(int)
		{
			T value;
			LockableEx::lock();
			value = m_value;
			m_value += 1;
			LockableEx::unlock(_usebymanythread);
			return value;
		}
#endif
		*/
		
		void operator+=(T y)
		{
			LockableEx::lock();
			m_value += y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator-=(T y)
		{
			LockableEx::lock();
			m_value -= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator*=(T y)
		{
			LockableEx::lock();
			m_value *= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator/=(T y)
		{
			LockableEx::lock();
			m_value /= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator%=(T y)
		{
			LockableEx::lock();
			m_value %= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator<<=(T y)
		{
			LockableEx::lock();
			m_value <<= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator>>=(T y)
		{
			LockableEx::lock();
			m_value >>= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator^=(T y)
		{
			LockableEx::lock();
			m_value ^= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator&=(T y)
		{
			LockableEx::lock();
			m_value &= y;
			LockableEx::unlock(_usebymanythread);
		}

		void operator|=(T y)
		{
			LockableEx::lock();
			m_value |= y;
			LockableEx::unlock(_usebymanythread);
		}

		bool operator==(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value == y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator!=(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value != y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator>(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value > y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator<(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value < y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator>=(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value >= y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}

		bool operator<=(T y)
		{
			bool retval;
			LockableEx::lock();
			retval = m_value <= y;
			LockableEx::unlock(_usebymanythread);
			return retval;
		}
	};

}

#endif
