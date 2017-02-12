/**
 * @file	JsServerSocket/ClientContext.cpp
 * @class	ClientContext
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/30
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSSERVERSOCKET_CLIENTCONTEXT_H__
#define __JSSERVERSOCKET_CLIENTCONTEXT_H__

#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "../JsCPPUtils/Common.h"
#include "../JsCPPUtils/LockableEx.h"

namespace JsServerSocket
{
	class ServerContext;
	class ClientContext : public JsCPPUtils::LockableEx
	{
	friend class ServerContext;

	public:
		bool m_isUsable;

		ServerContext *m_pServerCtx;
		int m_index;
		int m_sockfd;
		struct sockaddr_in m_addr;
		void *m_userptr;

		int64_t m_last_recvedtime;
		
#ifdef USE_OPENSSL
		SSL *m_ssl;
#endif

	public:
		ClientContext(ServerContext *pServerContext, int index, int clientsock, struct sockaddr_in *client_paddr, void *userptr);
		~ClientContext();

		int getIndex();
		bool isUsable();
		int lockandcheck();
		
		int recv(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags);
		int send(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags);
		int recvfixedsize(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags, struct timeval *ptvtimeout);
		int sendfixedsize(char *pbuf, int size, JSCUTILS_TYPE_FLAG flags);
		int close();
	};
}

#endif /* __JSSERVERSOCKET_CLIENTCONTEXT_H__ */
