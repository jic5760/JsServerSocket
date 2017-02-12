/**
 * @file	StringBuffer.h
 * @class	StringBuffer
 * @author	Jichan (jic5760@naver.com / ablog.jc-lab.net)
 * @date	2016/09/29
 * @brief	StringBuffer
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "StringBuffer.h"

#include <string.h>

namespace JsCPPUtils
{
	StringBuffer::StringBuffer() : 
		m_blocksize(128),
		m_bufsize(0),
		m_bufpos(0),
		m_pbuf(NULL)
	{
	}

	StringBuffer::StringBuffer(int blocksize) : 
		m_blocksize(blocksize),
		m_bufsize(0),
		m_bufpos(0),
		m_pbuf(NULL)
	{
	}

		
	StringBuffer::StringBuffer(const char *szText) : 
		m_blocksize(128),
		m_bufsize(0),
		m_bufpos(0),
		m_pbuf(NULL)
	{
		append(szText);
	}

	StringBuffer::StringBuffer(const char *szText, int len): 
		m_blocksize(128),
		m_bufsize(0),
		m_bufpos(0),
		m_pbuf(NULL)
	{
		append(szText, len);
	}

	StringBuffer::StringBuffer(const std::string& strText): 
		m_blocksize(128),
		m_bufsize(0),
		m_bufpos(0),
		m_pbuf(NULL)
	{
		append(strText.c_str(), strText.length());
	}


	StringBuffer::~StringBuffer()
	{
		if(m_pbuf != NULL)
		{
			free(m_pbuf);
			m_pbuf = NULL;
		}
		m_bufsize = 0;
		m_bufpos = 0;

	}

	bool StringBuffer::putData(const char *data, size_t size)
	{
		size_t needsize = m_bufpos + size + 1;
		if(needsize > m_bufsize)
		{
			char *pnewptr;
			size_t newsize = needsize;
			if(newsize % m_blocksize)
				newsize += m_blocksize - (newsize % m_blocksize);
			pnewptr = (char*)realloc(m_pbuf, newsize);
			if(pnewptr == NULL)
			{
				return false;
			}
			m_bufsize = newsize;
			m_pbuf = pnewptr;
		}

		memcpy(&m_pbuf[m_bufpos], data, size);
		m_bufpos += size;
		m_pbuf[m_bufpos] = '\0';

		return true;
	}

	void StringBuffer::clear()
	{
		m_bufpos = 0;
	}
	
	void StringBuffer::append(const char *szAppend, int len)
	{
		if(len < 0)
			len = strlen(szAppend);
		putData(szAppend, len);
	}

	void StringBuffer::append(const StringBuffer& sbAppend)
	{
		putData(sbAppend.m_pbuf, sbAppend.m_bufpos);
	}

	void StringBuffer::append(const char *szAppend)
	{
		putData(szAppend, strlen(szAppend));
	}

	void StringBuffer::append(const std::string& strAppend)
	{
		putData(strAppend.c_str(), strAppend.length());
	}
	
	void StringBuffer::append(bool bvalue)
	{
		static const char cszTrue[] = "true";
		static const char cszFalse[] = "false";
		if(bvalue)
		{
			putData(cszTrue, 4);
		}else{
			putData(cszFalse, 5);
		}
	}
	
	void StringBuffer::appendHex(unsigned char x, bool uppercase)
	{
		static const char cszHex_lower[] = "0123456789abcdef";
		static const char cszHex_upper[] = "0123456789ABCDEF";
		const char *cszHex;
		char tmpbuf[2];
		if(uppercase)
			cszHex = cszHex_upper;
		else
			cszHex = cszHex_lower;
		tmpbuf[0] = cszHex[(x >> 4) & 0x0F];
		tmpbuf[1] = cszHex[(x     ) & 0x0F];
		putData(tmpbuf, 2);
	}
	
	void StringBuffer::appendHex(unsigned char x)
	{
		appendHex(x, false);
	}
	
	bool StringBuffer::append(uint32_t unvalue, int radix)
	{
		static const char cszHex_lower[] = "0123456789abcdef";

		char tmpbuf[32];
		char *ppos = &tmpbuf[sizeof(tmpbuf)];
		int n = 0;

		if((radix <= 1) || (radix > 16))
			return false;

		if(unvalue == 0)
		{
			*--ppos = '0';
			n = 1;
		}else{
			while(unvalue != 0)
			{
				int x = unvalue % radix;
				*--ppos = cszHex_lower[x];
				n++;
				unvalue /= radix;
			}
		}

		putData(ppos, n);

		return true;
	}

	bool StringBuffer::append(int32_t nvalue, int radix)
	{
		static const char cszHex_lower[] = "0123456789abcdef";

		char tmpbuf[32];
		char *ppos = &tmpbuf[sizeof(tmpbuf)];
		int n = 0;

		if((radix <= 1) || (radix > 16))
			return false;

		if(nvalue == 0)
		{
			*--ppos = '0';
			n = 1;
		}else{
			bool isNegNum = (nvalue < 0);
			while(nvalue != 0)
			{
				int x = nvalue % radix;
				if(x<0) x *= -1;
				*--ppos = cszHex_lower[x];
				n++;
				nvalue /= radix;
			}
			if(isNegNum)
			{
				*--ppos = '-';
				n++;
			}
		}

		putData(ppos, n);

		return true;
	}
	
	bool StringBuffer::append(uint64_t unvalue, int radix)
	{
		static const char cszHex_lower[] = "0123456789abcdef";

		char tmpbuf[32];
		char *ppos = &tmpbuf[sizeof(tmpbuf)];
		int n = 0;

		if((radix <= 1) || (radix > 16))
			return false;

		if(unvalue == 0)
		{
			*--ppos = '0';
			n = 1;
		}else{
			while(unvalue != 0)
			{
				int x = unvalue % radix;
				*--ppos = cszHex_lower[x];
				n++;
				unvalue /= radix;
			}
		}

		putData(ppos, n);

		return true;
	}

	bool StringBuffer::append(int64_t nvalue, int radix)
	{
		static const char cszHex_lower[] = "0123456789abcdef";

		char tmpbuf[32];
		char *ppos = &tmpbuf[sizeof(tmpbuf)];
		int n = 0;

		if((radix <= 1) || (radix > 16))
			return false;

		if(nvalue == 0)
		{
			*--ppos = '0';
			n = 1;
		}else{
			bool isNegNum = (nvalue < 0);
			while(nvalue != 0)
			{
				int x = nvalue % radix;
				if(x<0) x *= -1;
				*--ppos = cszHex_lower[x];
				n++;
				nvalue /= radix;
			}
			if(isNegNum)
			{
				*--ppos = '-';
				n++;
			}
		}

		putData(ppos, n);

		return true;
	}

	bool StringBuffer::append(uint32_t unvalue)
	{
		return append(unvalue, 10);
	}
	bool StringBuffer::append(int32_t nvalue)
	{
		return append(nvalue, 10);
	}
	bool StringBuffer::append(uint64_t unvalue)
	{
		return append(unvalue, 10);
	}
	bool StringBuffer::append(int64_t nvalue)
	{
		return append(nvalue, 10);
	}
	
	void StringBuffer::appendHexBytes(unsigned char *pdata, int length, bool uppercase)
	{
		static const char cszHex_lower[] = "0123456789abcdef";
		static const char cszHex_upper[] = "0123456789ABCDEF";
		const char *cszHex;
		int i;
		char tmpbuf[2];
		if(uppercase)
			cszHex = cszHex_upper;
		else
			cszHex = cszHex_lower;
		for(i=0; i<length; i++)
		{
			unsigned char x = pdata[i];
			tmpbuf[0] = cszHex[(x >> 4) & 0x0F];
			tmpbuf[1] = cszHex[(x     ) & 0x0F];
			putData(tmpbuf, 2);
		}
	}
	
	void StringBuffer::appendHexBytes(unsigned char *pdata, int length)
	{
		appendHexBytes(pdata, length, false);
	}
	
	void StringBuffer::operator=(const StringBuffer& sbAppend)
	{
		clear();
		putData(sbAppend.m_pbuf, sbAppend.m_bufpos);
	}
	void StringBuffer::operator=(const std::string& strAppend)
	{
		clear();
		putData(strAppend.c_str(), strAppend.length());
	}
	void StringBuffer::operator=(const char *szAppend)
	{
		clear();
		putData(szAppend, strlen(szAppend));
	}
	void StringBuffer::operator=(bool value)
	{
		clear();
		append(value);
	}
	void StringBuffer::operator=(uint32_t value)
	{
		clear();
		append(value, 10);
	}
	void StringBuffer::operator=(int32_t value)
	{
		clear();
		append(value, 10);
	}
	void StringBuffer::operator=(uint64_t value)
	{
		clear();
		append(value, 10);
	}
	void StringBuffer::operator=(int64_t value)
	{
		clear();
		append(value, 10);
	}

	void StringBuffer::operator+=(const StringBuffer& sbAppend)
	{
		append(sbAppend);
	}
	void StringBuffer::operator+=(const std::string& strAppend)
	{
		append(strAppend);
	}
	void StringBuffer::operator+=(const char *szAppend)
	{
		append(szAppend);
	}
	void StringBuffer::operator+=(bool value)
	{
		append(value);
	}
	void StringBuffer::operator+=(uint32_t value)
	{
		append(value, 10);
	}
	void StringBuffer::operator+=(int32_t value)
	{
		append(value, 10);
	}
	void StringBuffer::operator+=(uint64_t value)
	{
		append(value, 10);
	}
	void StringBuffer::operator+=(int64_t value)
	{
		append(value, 10);
	}

	const StringBuffer& StringBuffer::operator+(const StringBuffer& sbAppend)
	{
		append(sbAppend);
		return *this;
	}
	const StringBuffer& StringBuffer::operator+(const std::string& strAppend)
	{
		append(strAppend);
		return *this;
	}
	const StringBuffer& StringBuffer::operator+(const char *szAppend)
	{
		append(szAppend);
		return *this;
	}
	const StringBuffer& StringBuffer::operator+(bool value)
	{
		append(value);
		return *this;
	}
	const StringBuffer& StringBuffer::operator+(uint32_t value)
	{
		append(value, 10);
		return *this;
	}
	const StringBuffer& StringBuffer::operator+(int32_t value)
	{
		append(value, 10);
		return *this;
	}
	const StringBuffer& StringBuffer::operator+(uint64_t value)
	{
		append(value, 10);
		return *this;
	}
	const StringBuffer& StringBuffer::operator+(int64_t value)
	{
		append(value, 10);
		return *this;
	}
	
	bool StringBuffer::operator==(const StringBuffer &ref) const
	{
		if(m_bufpos != ref.m_bufpos)
			return false;
		return (::memcmp(m_pbuf, ref.m_pbuf, m_bufpos) == 0);
	}
	
	bool StringBuffer::operator==(const char *szText) const
	{
		int len = strlen(szText);
		if(m_bufpos != len)
			return false;
		return (::strncmp(m_pbuf, szText, len) == 0);
	}

	std::string StringBuffer::toString() const
	{
		return std::string(m_pbuf, m_bufpos);
	}
	
	int StringBuffer::length() const
	{
		return m_bufpos;
	}

	const char *StringBuffer::c_str() const
	{
		return m_pbuf;
	}
	
	void StringBuffer::replaceToUpper()
	{
		size_t i;

		if(m_pbuf == NULL)
			return;

		for(i=0; i<m_bufpos; i++)
		{
			m_pbuf[i] = toupper(m_pbuf[i]);
		}
	}

	void StringBuffer::replaceToLower()
	{
		size_t i;

		if(m_pbuf == NULL)
			return;
	
		for(i=0; i<m_bufpos; i++)
		{
			m_pbuf[i] = tolower(m_pbuf[i]);
		}
	}

	const char *StringBuffer::trim2_cstr()
	{
		char *startptr = m_pbuf;
		char *lastptr;

		if(m_pbuf == NULL)
			return NULL;

		if(m_bufpos <= 0)
		{
			m_pbuf[0] = '\0';
			return m_pbuf;
		}

		while((*startptr == ' ') || (*startptr == '\t') || (*startptr == '\r') || (*startptr == '\n'))
			startptr++;

		lastptr = &startptr[m_bufpos-1];
	
		while((*lastptr == ' ') || (*lastptr == '\t') || (*lastptr == '\r') || (*lastptr == '\n'))
			*lastptr-- = '\0';

		return startptr;
	}

	char *StringBuffer::getBuffer()
	{
		return m_pbuf;
	}

	size_t StringBuffer::getBufferSize()
	{
		return m_bufsize;
	}
	
	char *StringBuffer::getCurPosPtr()
	{
		return &m_pbuf[m_bufpos];
	}
}
