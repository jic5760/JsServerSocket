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
		m_isUsable(true)
#ifdef USE_OPENSSL
		, m_ssl(NULL)
#endif
	{
		::memcpy(&m_addr, client_paddr, sizeof(struct sockaddr_in));
		m_last_recvedtime = JsCPPUtils::Common::getTickCount();
	}


	ClientContext::~ClientContext()
	{
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
		do {
			nrst = ::send(m_sockfd, pbuf, size, flags);
		}while(nrst < 0 && errno == EINTR);
		return nrst;
	}

	int ClientContext::recvfixedsize(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags, struct timeval *ptvtimeout)
	{
		int nrst;
		int processedLen = 0;
		fd_set readfds;
		do {
			FD_ZERO(&readfds);
			FD_SET(m_sockfd, &readfds);
			nrst = ::select(m_sockfd+1, &readfds, NULL, NULL, ptvtimeout);
			if(nrst == SOCKET_ERROR)
			{
				nrst = -errno;
			}else if(FD_ISSET(m_sockfd, &readfds))
			{
				nrst = ::recv(m_sockfd, &pbuf[processedLen], size - processedLen, flags);
				if(nrst < 0)
					nrst = -errno;
				else if(nrst > 0)
					processedLen += nrst;
			}
		}while(processedLen < size && ((nrst > 0) || ((nrst < 0) && (errno == EINTR))));
		if(nrst <= 0)
			return nrst;
		return 1;
	}

	int ClientContext::sendfixedsize(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags)
	{
		int nrst = 1;
		int processedLen = 0;
		do {
			nrst = ::send(m_sockfd, &pbuf[processedLen], size - processedLen, flags);
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
		::closesocket(m_sockfd);
		m_sockfd = INVALID_SOCKET;
		m_isUsable = false;
		return 1;
	}

}
