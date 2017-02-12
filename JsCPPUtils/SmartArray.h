/**
 * @file	SmartArray.h
 * @class	SmartArray
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/01
 * @brief	SmartArray
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <stdlib.h>
#include <string.h>

namespace JsCPPUtils
{
	template <typename T>
	class SmartArray
	{
	private:
		T *m_ptr;
		int m_size;
		int m_length;

	public:
		SmartArray() : 
		  m_ptr(NULL), m_size(0), m_length(0)
		{
			
		}

		bool alloc(int size)
		{
			free();
			m_size = size;
			m_length = 0;
			m_ptr = (T*)::malloc(sizeof(T) * size);
			if(m_ptr == NULL)
			{
				std::bad_alloc e;
				throw e;
				return false; // or
			}

			return true;
		}

		bool realloc(int size)
		{
			T* newptr = (T*)::realloc(m_ptr, sizeof(T) * size);
			if(newptr == NULL)
			{
				std::bad_alloc e;
				throw e;
				return false; // or
			}
			m_ptr = newptr;
			m_size = size;
			return true;
		}

		bool free()
		{
			if(m_ptr != NULL)
			{
				::free(m_ptr);
				m_ptr = NULL;
				m_size = 0;
				m_length = 0;
				return true;
			}else{
				return false;
			}
		}

		operator T*() const
		{
			return m_ptr;
		}

		T* getPtr()
		{
			return m_ptr;
		}

		int getSize()
		{
			return m_length;
		}

		int getLength()
		{
			return m_length;
		}

		void setLength(int len)
		{
			m_length = len;
		}
	};

}