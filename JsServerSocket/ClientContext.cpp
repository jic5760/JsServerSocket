/**
 * @file	JsServerSocket/ClientContext.cpp
 * @class	ClientContext
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/30
 * @brief	ClientContext
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include "ClientContext.h"
#include "ServerContext.h"
#include "macros.h"

#include <iostream>

namespace JsServerSocket {

	ClientContext::ClientContext(ServerContext *pServerCtx, int index, int clientsock, struct sockaddr_in *client_paddr, void *userptr) : 
		m_pServerCtx(pServerCtx),
		m_index(index),
		m_sockfd(clientsock),
		m_userptr(userptr),
		m_isUsable(true),
		m_sslstate(0)
#ifdef USE_OPENSSL
		, m_ssl(NULL)
#endif
	{
		m_freed = false;
		::memcpy(&m_addr, client_paddr, sizeof(struct sockaddr_in));
		m_last_recvedtime = JsCPPUtils::Common::getTickCount();
	}

	ClientContext::~ClientContext()
	{
		m_freed = true;
	}

	int ClientContext::getIndex()
	{
		return m_index;
	}

	bool ClientContext::isUsable()
	{
		return m_isUsable;
	}

	int ClientContext::lockandcheck()
	{
		int nrst;

		nrst = lock();
		if(nrst <= 0)
		{
			return nrst;
		}

		if(!m_isUsable)
		{
			nrst = unlock();
			if (nrst <= 0)
			{
				return nrst;
			}
			return 0;
		}

		return 1;
	}

	int ClientContext::recv(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags)
	{
		int nrst;
		do {
			nrst = ::recv(m_sockfd, pbuf, size, flags);
		}while(nrst < 0 && errno == EINTR);
		return nrst;
	}

	int ClientContext::send(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags)
	{
		int nrst;
		int neno = 0;
		
		if (m_sslstate == 1)
			return 0;
		
		do
		{
			if (m_sslstate == 2)
			{
				int sslerr = 0;
#ifdef USE_OPENSSL
				nrst = SSL_write(m_ssl, pbuf, size);
				if (nrst < 0)
				{
					neno = errno;
					sslerr = SSL_get_error(m_ssl, nrst);
					switch (sslerr)
					{
					case SSL_ERROR_WANT_READ:
					case SSL_ERROR_WANT_WRITE:
						neno = EAGAIN;
						break;
					}
				}
#else
				nrst = 0;
#endif
			} else {
				nrst = ::send(m_sockfd, pbuf, size, flags);
			}
		} while ((nrst < 0) && (neno == EINTR));
		errno = neno;
		return nrst;
	}

	int ClientContext::recvfixedsize(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags, struct timeval *ptvtimeout)
	{
		int nrst;
		int neno = 0;
		int processedLen = 0;
		struct pollfd tmppollfd;
		do {
			int procpass = 0;
			int readableBytes = 0;
			
			//SSL_pending
			if (m_sslstate == 2)
			{
				readableBytes = SSL_pending(m_ssl);
				if (readableBytes > 0)
				{
					nrst = ::SSL_read(m_ssl, &pbuf[processedLen], readableBytes);
					if (nrst > 0)
					{
						processedLen += nrst;
						if (processedLen >= size)
							break;
					}
				}
			}
			
			memset(&tmppollfd, 0, sizeof(tmppollfd));
			tmppollfd.fd = m_sockfd;
			tmppollfd.events = POLLIN;
			nrst = ::poll(&tmppollfd, 1, (ptvtimeout == NULL) ? -1 : (ptvtimeout->tv_sec * 1000 + ptvtimeout->tv_usec / 1000));
			if (nrst < 0)
			{
				nrst = -errno;
			}
			else if (nrst > 0)
			{
				if (m_sslstate == 2)
				{
					int sslerr;
					nrst = ::SSL_read(m_ssl, &pbuf[processedLen], size - processedLen);
					if (nrst < 0)
					{
						neno = errno;
						sslerr = SSL_get_error(m_ssl, nrst);
						switch (sslerr)
						{
						case SSL_ERROR_SSL:
							ERR_print_errors_fp(stderr);
							procpass = -1;
							break;
						case SSL_ERROR_WANT_READ:
						case SSL_ERROR_WANT_WRITE:
							neno = EAGAIN;
							procpass = 1;
							break;
						case SSL_ERROR_SYSCALL:
							if (neno == EINTR)
								break;
							procpass = -1;
							break;
						default:
							procpass = -1;
						}
					}
					else if (nrst > 0)
					{
						processedLen += nrst;
					}
					else
						break;
				}
				else
				{
					nrst = ::recv(m_sockfd, &pbuf[processedLen], size - processedLen, flags);
					if (nrst < 0)
					{
						neno = errno;
						nrst = -errno;
					}
					else if (nrst > 0)
						processedLen += nrst;
					else
						break;
					
				}
			}
			else
			{
				// timeout
				break;
			}
		}while(processedLen < size && ((nrst > 0) || ((nrst < 0) && (neno == EINTR))));
		if(nrst <= 0)
			return nrst;
		return 1;
	}

	int ClientContext::sendfixedsize(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags)
	{
		int nrst = 1;
		int processedLen = 0;
		do {
			nrst = this->send(&pbuf[processedLen], size - processedLen, flags);
			if(nrst > 0)
				processedLen += nrst;
		}while(processedLen < size && ((nrst > 0) || ((nrst < 0) && (errno == EINTR))));
		if(nrst <= 0)
			return nrst;
		return 1;
	}

	int ClientContext::close()
	{	
#ifdef USE_OPENSSL
		if (m_pServerCtx->getUseSSL())
		{
			::SSL_free(m_ssl);
			m_ssl = NULL;
		}
#endif
		::shutdown(m_sockfd, SHUT_RDWR);
		::closesocket(m_sockfd);
		m_sockfd = INVALID_SOCKET;
		m_isUsable = false;
		return 1;
	}
	
	void ClientContext::setUserPtr(void *userptr)
	{
		m_userptr = userptr;
	}
	
	void *ClientContext::getUserPtr()
	{
		return m_userptr;
	}
	
#ifdef USE_OPENSSL
	SSL *ClientContext::getSSL()
	{
		return m_ssl;
	}
#endif

}
