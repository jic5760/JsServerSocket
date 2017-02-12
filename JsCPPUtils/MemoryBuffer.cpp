/**
 * @file	MemoryBuffer.cpp
 * @class	MemoryBuffer
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/09/29
 * @brief	MemoryBuffer
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "MemoryBuffer.h"

#include <string.h>

namespace JsCPPUtils
{

	MemoryBuffer::MemoryBuffer()
	{
		m_blocksize = 4096;
		m_bufsize = 0;
		m_bufpos = 0;
		m_pbuf = NULL;
	}

	MemoryBuffer::MemoryBuffer(int blocksize)
	{
		m_blocksize = blocksize;
		m_bufsize = 0;
		m_bufpos = 0;
		m_pbuf = NULL;
	}

	MemoryBuffer::~MemoryBuffer()
	{
		if(m_pbuf != NULL)
		{
			free(m_pbuf);
			m_pbuf = NULL;
		}
		m_bufsize = 0;
		m_bufpos = 0;
	}

	bool MemoryBuffer::increaseBuffer(int increaseSize)
	{
		size_t needsize = m_bufpos + increaseSize;
		if(needsize > m_bufsize)
		{
			char *pnewptr;
			size_t newsize = needsize;
			size_t realIncreaseSize;
			if(newsize % m_blocksize)
				newsize += m_blocksize - (newsize % m_blocksize);
			realIncreaseSize = needsize - m_bufpos;
			pnewptr = (char*)realloc(m_pbuf, newsize);
			if(pnewptr == NULL)
				return false; // Memory allocate failed!!
			m_bufsize = newsize;
			memset(&pnewptr[m_bufpos], 0, realIncreaseSize);
			m_pbuf = pnewptr;
		}
		return true;
	}
	
	bool MemoryBuffer::increasePos(int increaseSize)
	{
		size_t needsize = m_bufpos + increaseSize;
		if(needsize > m_bufsize)
			return false;
		m_bufpos = needsize;
		return true;
	}

	bool MemoryBuffer::putData(const char *data, size_t size)
	{
		size_t needsize = m_bufpos + size;
		if(needsize > m_bufsize)
		{
			char *pnewptr;
			size_t newsize = needsize;
			if(newsize % m_blocksize)
				newsize += m_blocksize - (newsize % m_blocksize);
			pnewptr = (char*)realloc(m_pbuf, newsize);
			if(pnewptr == NULL)
				return false; // Memory allocate failed!!
			m_bufsize = newsize;
			m_pbuf = pnewptr;
		}

		memcpy(&m_pbuf[m_bufpos], data, size);
		m_bufpos += size;

		return true;
	}

	char *MemoryBuffer::getBuffer()
	{
		return m_pbuf;
	}

	size_t MemoryBuffer::getLength()
	{
		return m_bufpos;
	}

	size_t MemoryBuffer::getBufferSize()
	{
		return m_bufsize;
	}
	
	char *MemoryBuffer::getCurPosPtr()
	{
		return &m_pbuf[m_bufpos];
	}

	void MemoryBuffer::clear()
	{
		m_bufpos = 0;
	}

}
