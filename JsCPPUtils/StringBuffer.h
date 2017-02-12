/**
 * @file	StringBuffer.h
 * @class	StringBuffer
 * @author	Jichan (jic5760@naver.com / ablog.jc-lab.net)
 * @date	2016/09/29
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <string>

#include "Common.h"
#include "MemoryBuffer.h"

namespace JsCPPUtils
{

	class StringBuffer
	{
	private:
		size_t m_bufsize;
		size_t m_bufpos;
		char *m_pbuf;
		int   m_blocksize;
		
		bool putData(const char *data, size_t size);

	public:
		StringBuffer();
		StringBuffer(const char *szText);
		StringBuffer(const char *szText, int len);
		StringBuffer(const std::string& strText);
		StringBuffer(int blocksize);
		~StringBuffer();
		
		void clear();
		void append(const char *szAppend, int len);
		void append(const StringBuffer& sbAppend);
		void append(const char *szAppend);
		void append(const std::string& strAppend);
		void append(bool bvalue);
		bool append(uint32_t unvalue, int radix);
		bool append(int32_t nvalue, int radix);
		bool append(uint64_t unvalue, int radix);
		bool append(int64_t nvalue, int radix);
		bool append(uint32_t unvalue);
		bool append(int32_t nvalue);
		bool append(uint64_t unvalue);
		bool append(int64_t nvalue);
		void appendHex(unsigned char x, bool uppercase);
		void appendHex(unsigned char x);
		void appendHexBytes(unsigned char *pdata, int length, bool uppercase);
		void appendHexBytes(unsigned char *pdata, int length);

		void operator=(const StringBuffer& szAppend);
		void operator=(const std::string& strAppend);
		void operator=(const char *szAppend);
		void operator=(bool value);
		void operator=(uint32_t value);
		void operator=(int32_t value);
		void operator=(uint64_t value);
		void operator=(int64_t value);
		void operator+=(const StringBuffer& sbAppend);
		void operator+=(const std::string& strAppend);
		void operator+=(const char *szAppend);
		void operator+=(bool value);
		void operator+=(uint32_t value);
		void operator+=(int32_t value);
		void operator+=(uint64_t value);
		void operator+=(int64_t value);
		const StringBuffer& operator+(const StringBuffer& sbAppend);
		const StringBuffer& operator+(const std::string& strAppend);
		const StringBuffer& operator+(const char *szAppend);
		const StringBuffer& operator+(bool value);
		const StringBuffer& operator+(uint32_t value);
		const StringBuffer& operator+(int32_t value);
		const StringBuffer& operator+(uint64_t value);
		const StringBuffer& operator+(int64_t value);
		
		bool operator==(const StringBuffer &ref) const;
		bool operator==(const char *szText) const;
		
		std::string toString() const;
		int length() const;
		const char *c_str() const;
		
		void replaceToUpper();
		void replaceToLower();
		const char *trim2_cstr();

		char *getBuffer();
		size_t getBufferSize();
		char *getCurPosPtr();
	};

}
