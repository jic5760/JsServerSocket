/**
 * @file	JsServerSocket/ServerSocket.h
 * @class	ServerSocket
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/30
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSSERVERSOCKET_SERVERCONTEXT_H__
#define __JSSERVERSOCKET_SERVERCONTEXT_H__

#include <list>
#include <map>

#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "../JsCPPUtils/Common.h"
#include "../JsCPPUtils/RandomWell512.h"
#include "../JsCPPUtils/JsThread.h"
#include "../JsCPPUtils/SmartPointer.h"
#include "../JsCPPUtils/Logger.h"

#include "ClientContext.h"

namespace JsServerSocket
{
	class ClientContext;
	class ServerContext {
	public:	
		typedef int(*StartWorkerPostHandler_t)(ServerContext *pserverctx, int threadidx, void **out_pthreaduserctx);
		typedef void(*StopWorkerHandler_t)(ServerContext *pserverctx, int threadidx, void *pthreaduserctx);

		typedef int(*Client_AcceptHandler_t)(ServerContext *pServerCtx, void *pthreaduserctx, int client_sock, struct sockaddr_in *client_paddr);
		typedef int(*Client_RecvHandler_t)(ServerContext *pServerCtx, void *pthreaduserctx, ClientContext *pClientCtx, int recv_len, char *recv_pbuf);
		typedef void(*Client_DelHandler_t)(ServerContext *pServerCtx, ClientContext *pClientCtx);

	private:
		class WorkerThreadInternalContext {
		public:
			ServerContext *pServerCtx;
			
			bool inited_userhandler;
			int threadidx;
			void *pthreaduserctx;
			
			char *precvbuf;

			WorkerThreadInternalContext(ServerContext *_pServerCtx, int _threadidx, void *_pthreaduserctx)
				: pServerCtx(_pServerCtx)
				, inited_userhandler(false)
				, threadidx(_threadidx)
				, pthreaduserctx(_pthreaduserctx)
				, precvbuf(NULL)
			{
			}
		};
		
		JsCPPUtils::Logger *m_pParentLogger;
		JsCPPUtils::Logger *m_plogger;

		int m_sock_domain;
		int m_sock_proto;
		int m_sock_type;
		
		bool m_bUseSSL;

		int m_sock_fd;
		int m_epoll_fd;
		
#ifdef USE_OPENSSL
		SSL_CTX *m_sslCtx;
#endif

		int m_conf_numOfMaxClients;
		long m_conf_recvdatabufsize;

		int          m_worker_numofthreads;
		std::list< JsCPPUtils::SmartPointer<JsCPPUtils::JsThread::ThreadContext> > m_worker_threads;
		int          m_worker_stateofstartthread;

		JsCPPUtils::Lockable      m_random_lock;
		JsCPPUtils::RandomWell512 m_random;

		std::map< int, JsCPPUtils::SmartPointer<ClientContext> > m_clients;
		JsCPPUtils::Lockable                                     m_clients_lock;

		void *userptr;

		StartWorkerPostHandler_t m_startworkerposthandler;
		StopWorkerHandler_t      m_stopworkerhandler;

		Client_AcceptHandler_t m_accepthandler;
		Client_RecvHandler_t   m_recvhandler;
		Client_DelHandler_t    m_delhandler;

		static void workerThreadProc_CleanUp(void *param);
		static int workerThreadProc(JsCPPUtils::JsThread::ThreadContext *pThreadCtx, int threadindex, void *threadparam);

	public:
		ServerContext(JsCPPUtils::Logger *logger);
		~ServerContext();
		int init(
			int sock_domain,
			int sock_type,
			int sock_proto,
			bool bUseSSL,
#ifdef USE_OPENSSL 
			const SSL_METHOD *ssl_method,
#else
			void *ssl_method,
#endif
			int numOfMaxClients,
			long recvdatabufsize,
			StartWorkerPostHandler_t startworkerposthandler,
			StopWorkerHandler_t stopworkerhandler,
			Client_AcceptHandler_t accepthandler,
			Client_RecvHandler_t recvhandler,
			Client_DelHandler_t delhandler);
		int close();
		int listen(const struct sockaddr *psockaddr, int sockaddrlen, int sizeOfListenQueue);
		int sslLoadCertificates(const char* szCertFile, const char* szKeyFile);
		int startWorkers(int numOfthreads);

		int clientAdd(int clientsock, struct sockaddr_in *client_paddr, JsCPPUtils::SmartPointer< ClientContext > *pout_spclientctx, void *userptr);
		int clientDel(JsCPPUtils::SmartPointer<ClientContext> spClientCtx);
		int clientDel(ClientContext *pClientCtx);
		int clientDel(int clientidx);
		int clientDel(std::map<int, JsCPPUtils::SmartPointer<ClientContext> >::iterator iter);

		JsCPPUtils::Logger *getLogger();

		int getConnections();
		bool getUseSSL();
	};

	/*
	extern int JsServerSocket_Client_Add(JsServerSocket_Context_t *pserverctx, int clientsock, struct sockaddr_in *pclientaddr, void *userptr, JsServerSocket_ClientCtx_t **pout_pclientctx);
	extern int JsServerSocket_Client_Del(JsServerSocket_ClientCtx_t *pclientctx);
	extern int JsServerSocket_Client_lockandcheck(JsServerSocket_ClientCtx_t *pclientctx, JsThread_SmartMutex_LocalCtx_t *plocklocalctx);
	extern int JsServerSocket_Client_unlock(JsThread_SmartMutex_LocalCtx_t *plocklocalctx);
	*/
}

#endif /* __JSSERVERSOCKET_SERVERCONTEXT_H__ */
