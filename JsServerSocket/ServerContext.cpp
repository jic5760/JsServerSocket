/**
 * @file	JsServerSocket/ServerSocket.cpp
 * @class	ServerSocket
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/30
 * @brief	ServerSocket
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

#include <time.h>

#include "ServerContext.h"
#include "ClientContext.h"
#include "macros.h"

namespace JsServerSocket
{

	ServerContext::ServerContext(JsCPPUtils::Logger *plogger) :
		m_pParentLogger(plogger),
		m_plogger(NULL),
		m_sock_fd(INVALID_SOCKET),
		m_epoll_fd(INVALID_FD),
		m_sock_domain(0),
		m_sock_type(0),
		m_sock_proto(0),
		m_conf_numOfMaxClients(0),
		m_conf_recvdatabufsize(0),
		m_worker_numofthreads(0),
		m_startworkerposthandler(NULL),
		m_stopworkerhandler(NULL),
		m_accepthandler(NULL),
		m_recvhandler(NULL),
		m_delhandler(NULL)
#ifdef USE_OPENSSL
		,m_sslCtx(NULL)
#endif
	{
		if(m_pParentLogger != NULL)
		{
			m_plogger = new JsCPPUtils::Logger(m_pParentLogger, "JsServerSocket->ServerContext");
		}
	}

	ServerContext::~ServerContext()
	{
		if(m_plogger != NULL)
		{
			delete m_plogger;
			m_plogger = NULL;
		}
	}

	int ServerContext::init(
		int sock_domain, int sock_type, int sock_proto,
		bool bUseSSL,
#ifdef USE_OPENSSL 
		const SSL_METHOD *ssl_method,
#else
		void *ssl_method,
#endif
		int numOfMaxClients, long recvdatabufsize,
		StartWorkerPostHandler_t startworkerposthandler,
		StopWorkerHandler_t stopworkerhandler, Client_AcceptHandler_t accepthandler,
		Client_RecvHandler_t recvhandler, Client_DelHandler_t delhandler
		)
	{
		int retval = 0;
		
		m_sock_domain = sock_domain;
		m_sock_type = sock_type;
		m_sock_proto = sock_proto;
		m_bUseSSL = bUseSSL;
		
		m_conf_numOfMaxClients = numOfMaxClients;
		m_conf_recvdatabufsize = recvdatabufsize;

		m_startworkerposthandler = startworkerposthandler;
		m_stopworkerhandler = stopworkerhandler;

		m_accepthandler = accepthandler;
		m_recvhandler = recvhandler;
		m_delhandler = delhandler;

		do {
			m_sock_fd = socket(m_sock_domain, m_sock_type, m_sock_proto);
			if(m_sock_fd == INVALID_SOCKET)
			{
				retval = -errno;
				break;
			}

			m_epoll_fd = epoll_create(128);
			if(m_sock_fd == INVALID_FD)
			{
				retval = -errno;
				break;
			}
			
#ifdef USE_OPENSSL
			if (m_bUseSSL)
			{
				m_sslCtx = SSL_CTX_new(ssl_method);
				if (m_sslCtx == NULL)
				{
					ERR_print_errors_fp(stderr);
					retval = -1;
					break;
				}
			}
#endif

			retval = 1;
		}while(0);

		if(retval != 1)
		{
			close();
		}

		return retval;
	}

	int ServerContext::close()
	{
		for(std::list< JsCPPUtils::SmartPointer< JsCPPUtils::JsThread::ThreadContext > >::iterator iter = m_worker_threads.begin(); iter != m_worker_threads.end(); )
		{
			(*iter)->reqStop();
			iter = m_worker_threads.erase(iter);
		}

		m_clients_lock.lock();
		for(std::map< int, JsCPPUtils::SmartPointer<ClientContext> >::iterator iter = m_clients.begin(); iter != m_clients.end(); )
		{
			iter->second->close();
			m_clients.erase(iter++);
		}
		m_clients_lock.unlock();

		if(m_sock_fd != INVALID_SOCKET)
		{
			::close(m_sock_fd);
			m_sock_fd = INVALID_SOCKET;
		}
		if(m_epoll_fd != INVALID_SOCKET)
		{
			::close(m_epoll_fd);
			m_epoll_fd = INVALID_SOCKET;
		}
		
#ifdef USE_OPENSSL
		if (m_bUseSSL)
		{
			SSL_CTX_free(m_sslCtx);
			m_sslCtx = NULL;
		}
#endif

		return 1;
	}

	int ServerContext::sslLoadCertificates(char* szCertFile, char* szKeyFile)
	{
#ifdef USE_OPENSSL
	    /* set the local certificate from CertFile */
		if (SSL_CTX_use_certificate_file(m_sslCtx, szCertFile, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			return -1;
		}
		/* set the private key from KeyFile (may be the same as CertFile) */
		if (SSL_CTX_use_PrivateKey_file(m_sslCtx, szKeyFile, SSL_FILETYPE_PEM) <= 0)
		{
			ERR_print_errors_fp(stderr);
			return -1;
		}
		/* verify private key */
		if (!SSL_CTX_check_private_key(m_sslCtx))
		{
			fprintf(stderr, "Private key does not match the public certificate\n");
			return -1;
		}
		return 1;
#else
		return -1;
#endif
	}
	
	int ServerContext::listen(const struct sockaddr *psockaddr, int sockaddrlen, int sizeOfListenQueue)
	{
		int retval = 0;
		int nrst;
		struct epoll_event tmpepevent;

		int nvalue;

		nvalue = 1;
		nrst = setsockopt(m_sock_fd, SOL_SOCKET, SO_REUSEADDR, &nvalue, sizeof(nvalue));

		do
		{
			nrst = bind(m_sock_fd, psockaddr, sockaddrlen);
			if (IS_SOCKET_ERROR(nrst))
			{
				retval = -errno;
				break;
			}

			nrst = ::listen(m_sock_fd, sizeOfListenQueue);
			if (IS_SOCKET_ERROR(nrst))
			{
				retval = -errno;
				break;
			}

			memset(&tmpepevent, 0, sizeof(tmpepevent));
			tmpepevent.events = EPOLLIN | EPOLLONESHOT;
			tmpepevent.data.ptr = NULL;

			nrst = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_sock_fd, &tmpepevent);
			if (IS_BSDFUNC_ERROR(nrst))
			{
				retval = -errno;
				break;
			}

			retval = 1;
		} while (0);

		if (retval <= 1)
		{
			// Error handling
		}

		return retval;
	}

	int ServerContext::startWorkers(int numOfthreads)
	{
		int i;
		int nrst;
		size_t n;

		uint64_t ns = 0;

		if((numOfthreads < 0) || (numOfthreads > 64))
			return 0;

		m_worker_numofthreads = numOfthreads;

		for(i=0; i<numOfthreads; i++)
		{
			JsCPPUtils::SmartPointer< JsCPPUtils::JsThread::ThreadContext > spThreadCtx;
			nrst = JsCPPUtils::JsThread::start(&spThreadCtx, workerThreadProc, i, this);
			if(nrst <= 0)
			{
				// ERROR
			}else{
				m_worker_threads.push_back(spThreadCtx);
			}
		}

		return 1;
	}

	void ServerContext::workerThreadProc_CleanUp(void *param)
	{
		WorkerThreadInternalContext *pmyctx = (WorkerThreadInternalContext*)param;
		if(pmyctx->precvbuf != NULL)
		{
			free(pmyctx->precvbuf);
			pmyctx->precvbuf = NULL;
		}
		if(pmyctx->pServerCtx->m_stopworkerhandler != NULL && pmyctx->inited_userhandler)
		{
			pmyctx->pServerCtx->m_stopworkerhandler(pmyctx->pServerCtx, pmyctx->threadidx, pmyctx->pthreaduserctx);
			pmyctx->inited_userhandler = false;
		}
	}

	int ServerContext::workerThreadProc(JsCPPUtils::JsThread::ThreadContext *pThreadCtx, int threadindex, void *threadparam)
	{
		ServerContext *pServerCtx = (ServerContext*)threadparam;
		WorkerThreadInternalContext myctx(pServerCtx, threadindex, threadparam);

		int threadrunrst;
		int nrst;
		int neno;

		int epnum;
		int epi;
		struct epoll_event tmpepevent;
		struct epoll_event epevents[16];

		int procrst = 0;

		int ecnt;

		int clientsock;
		struct sockaddr_in clientaddr;
		socklen_t clientaddrsize;

		int recvlen;
		ClientContext *pclientctx;

		if(pServerCtx->m_startworkerposthandler != NULL)
		{
			if((nrst = pServerCtx->m_startworkerposthandler(pServerCtx, threadindex, &myctx.pthreaduserctx)) <= 0)
			{
				goto EXIT_STARTERR1;
			}
			myctx.inited_userhandler = true;
		}

		pthread_cleanup_push(workerThreadProc_CleanUp, &myctx);

		myctx.precvbuf = (char*)malloc(pServerCtx->m_conf_recvdatabufsize);

		if(myctx.precvbuf == NULL)
		{
			goto EXIT_STARTERR2;
		}

		while(likely((threadrunrst = pThreadCtx->_inthread_isRun()) == 1))
		{
			epnum = epoll_wait(pServerCtx->m_epoll_fd, epevents, 16, 100);
			if (epnum == 0)
			{

			}
			else if (epnum < 0)
			{
				// Error
				neno = errno;
				if (neno != EINTR)
				{
					if(pServerCtx->m_plogger != NULL)
						pServerCtx->m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] epoll_wait failed: %d", neno);
					break;
				}
			}
			else
			{
				for (epi = 0; epi < epnum; epi++)
				{
					if (epevents[epi].data.ptr == NULL)
					{
						ecnt = 5;
						do
						{
							clientaddrsize = sizeof(clientaddr);
							clientsock = accept(pServerCtx->m_sock_fd, (struct sockaddr *)&clientaddr, &clientaddrsize);
							neno = errno;
							ecnt--;
						} while ((ecnt > 0) && ((clientsock == INVALID_SOCKET) && (neno == EINTR)));

						if (unlikely(clientsock == INVALID_SOCKET))
						{
							// Error
							neno = errno;
							if(pServerCtx->m_plogger != NULL)
								pServerCtx->m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] client accept failed: %d", neno);
							procrst = -1;
						}
						else
						{
							if (likely(pServerCtx->m_accepthandler != NULL))
							{
								procrst = pServerCtx->m_accepthandler(pServerCtx, myctx.pthreaduserctx, clientsock, &clientaddr);	
							}
							else
							{
								procrst = pServerCtx->clientAdd(clientsock, &clientaddr, NULL, NULL);
							}
							if (procrst <= 0)
							{
								::closesocket(clientsock);
							}
						}

						memset(&tmpepevent, 0, sizeof(tmpepevent));
						tmpepevent.events = EPOLLIN | EPOLLONESHOT;
						tmpepevent.data.ptr = NULL;

						if (unlikely(epoll_ctl(pServerCtx->m_epoll_fd, EPOLL_CTL_MOD, pServerCtx->m_sock_fd, &tmpepevent) < 0))
						{
							// Error
							neno = errno;
						}
					}
					else
					{
						pclientctx = (ClientContext*)(epevents[epi].data.ptr);
						if (pServerCtx->m_bUseSSL)
						{
#ifdef USE_OPENSSL
							recvlen = SSL_read(pclientctx->m_ssl, myctx.precvbuf, pServerCtx->m_conf_recvdatabufsize);
#else
							recvlen = -1;
#endif
						}
						else
						{
							ecnt = 5;
							do
							{
								recvlen = recv(pclientctx->m_sockfd, myctx.precvbuf, pServerCtx->m_conf_recvdatabufsize, 0);
								neno = errno;
								ecnt--;
							} while ((ecnt > 0) && ((IS_BSDFUNC_ERROR(recvlen)) && (neno == EINTR)));
						}
						if (likely((nrst = pclientctx->lockandcheck()) == 1))
						{
							if (recvlen <= 0)
							{
								if(pServerCtx->m_plogger != NULL)
									pServerCtx->m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_INFO, "[server_workerthreadproc] Client[%d] recvlen=0", pclientctx->m_index);
							}
							else
							{
								pclientctx->m_last_recvedtime = JsCPPUtils::Common::getTickCount();
								if(likely(pServerCtx->m_recvhandler != NULL))
									procrst = pServerCtx->m_recvhandler(pServerCtx, myctx.pthreaduserctx, pclientctx, recvlen, myctx.precvbuf);
								else
									procrst = 0;
								if (procrst <= 0)
								{
									if(pServerCtx->m_plogger != NULL)
										pServerCtx->m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_INFO, "[server_workerthreadproc] Client[%d] recvproc=%d", pclientctx->m_index, procrst);
								}else{
									memset(&tmpepevent, 0, sizeof(tmpepevent));
									tmpepevent.events = EPOLLIN | EPOLLONESHOT;
									tmpepevent.data.ptr = pclientctx;

									if (unlikely(epoll_ctl(pServerCtx->m_epoll_fd, EPOLL_CTL_MOD, pclientctx->m_sockfd, &tmpepevent) < 0))
									{
										// Error
										neno = -errno;
										if(pServerCtx->m_plogger != NULL)
											pServerCtx->m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] server socket epoll_ctl_mod failed: %d", neno);
										procrst = neno;
									}
								}
							}
							if (procrst <= 0)
							{
								pServerCtx->clientDel(pclientctx);
							}
							pclientctx->unlock();
						}
					}
				}
			}
			usleep(1);
		}

	EXIT_STARTERR2:
		pthread_cleanup_pop(1);

	EXIT_STARTERR1:
		return 0;
	}

	int ServerContext::getConnections()
	{
		int value;
		m_clients_lock.lock();
		value = m_clients.size();
		m_clients_lock.unlock();
		return value;
	}
	
	bool ServerContext::getUseSSL()
	{
		return m_bUseSSL;
	}

	JsCPPUtils::Logger *ServerContext::getLogger()
	{
		return m_pParentLogger;
	}

	int ServerContext::clientAdd(int clientsock, struct sockaddr_in *client_paddr, JsCPPUtils::SmartPointer< ClientContext > *pout_spclientctx, void *userptr)
	{
		int retval = 0;

		int nrst;
		int neno;

		struct epoll_event tmpepevent;

		int i;

		int clientidx = -1;
		ClientContext *pclientctx = NULL;
		JsCPPUtils::SmartPointer< ClientContext > spclientctx;

		int nval;
		struct timeval timeout_tv;

		memset(&timeout_tv, 0, sizeof(timeout_tv));
		timeout_tv.tv_sec = 10;
		timeout_tv.tv_usec = 0;

		if ((nrst = setsockopt (clientsock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout_tv, sizeof(timeout_tv))) < 0)
		{
			neno = -errno;
			if(m_plogger != NULL)
				m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] client socket setsockopt(SO_RCVTIMEO) failed: %d", neno);
		}

		if ((nrst = setsockopt(clientsock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout_tv, sizeof(timeout_tv))) < 0)
		{
			neno = -errno;
			if(m_plogger != NULL)
				m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] client socket setsockopt(SO_SNDTIMEO) failed: %d", neno);
		}

		nval = 1;
		if ((nrst = setsockopt(clientsock, SOL_SOCKET, SO_KEEPALIVE, (char *)&nval, sizeof(nval))) < 0)
		{
			neno = -errno;
			if(m_plogger != NULL)
				m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] client socket setsockopt(TCP_USER_TIMEOUT) failed: %d", neno);
		}

		nval = 600;
		if ((nrst = setsockopt(clientsock, SOL_TCP, TCP_KEEPIDLE, (char *)&nval, sizeof(nval))) < 0)
		{
			neno = -errno;
			if(m_plogger != NULL)
				m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] client socket setsockopt(TCP_USER_TIMEOUT) failed: %d", neno);
		}

		nval = 6;
		if ((nrst = setsockopt(clientsock, SOL_TCP, TCP_KEEPCNT, (char *)&nval, sizeof(nval))) < 0)
		{
			neno = -errno;
			if(m_plogger != NULL)
				m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] client socket setsockopt(TCP_USER_TIMEOUT) failed: %d", neno);
		}

		nval = 5;
		if ((nrst = setsockopt(clientsock, SOL_TCP, TCP_KEEPINTVL, (char *)&nval, sizeof(nval))) < 0)
		{
			neno = -errno;
			if(m_plogger != NULL)
				m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] client socket setsockopt(TCP_USER_TIMEOUT) failed: %d", neno);
		}

		do
		{
			int failcnt = 0;
			int powval = 10;
			
			m_clients_lock.lock();
			m_random_lock.lock();
			do {
				std::map<int, JsCPPUtils::SmartPointer<ClientContext> >::iterator iter;

				clientidx = m_random.nextInt();
				if(clientidx < 0)
					clientidx *= -1;
				if(likely(failcnt < 8))
				{
					clientidx = clientidx % powval;
					powval *= 10;
				}
				iter = m_clients.find(clientidx);
				if(unlikely(iter != m_clients.end()))
				{
					clientidx = -1;
					failcnt++;
				}
			}while(clientidx == -1 && failcnt < 128);
			m_random_lock.unlock();
			if(failcnt < 128)
			{
				pclientctx = new ClientContext(this, clientidx, clientsock, client_paddr, userptr);
				if(likely(pclientctx != NULL))
				{
					m_clients[clientidx] = pclientctx;
				}
				else
				{
					neno = -errno;
					retval = neno;
					if(m_plogger != NULL)
						m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[workerthreadproc] Memory allocation failed(new ClientContext): %d", neno);
				}
			}
			m_clients_lock.unlock();
			if(failcnt >= 128 || pclientctx == NULL)
			{
				break;
			}
			
#ifdef USE_OPENSSL
			if (m_bUseSSL)
			{
				pclientctx->m_ssl = SSL_new(m_sslCtx);
				if (unlikely(pclientctx->m_ssl == NULL))
				{
					ERR_print_errors_fp(stderr);
					retval = -1;
					break;
				}
				else
				{
					SSL_set_fd(pclientctx->m_ssl, clientsock);
					if ((nrst = SSL_accept(pclientctx->m_ssl)) <= 0)
					{
						ERR_print_errors_fp(stderr);
						retval = -1000;
						break;
					}
				}
			}
#endif

			memset(&tmpepevent, 0, sizeof(tmpepevent));
			tmpepevent.events = EPOLLIN | EPOLLONESHOT;
			tmpepevent.data.ptr = pclientctx;

			if ((nrst = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, clientsock, &tmpepevent)) < 0)
			{
				// Error
				neno = -errno;
				retval = neno;
				if (m_plogger != NULL)
					m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[server_workerthreadproc] server socket epoll_ctl_mod failed: %d", neno);
				break;
			}

			retval = 1;
		} while (0);

		if (retval <= 0)
		{
			if (clientidx != -1)
			{
				std::map<int, JsCPPUtils::SmartPointer<ClientContext> >::iterator iter = m_clients.find(clientidx);
				if (likely(iter != m_clients.end()))
				{
					m_clients_lock.lock();
					m_clients[clientidx]->close();
					m_clients.erase(iter);
					m_clients_lock.unlock();
				}
			}
		}
		else
		{
			if (pout_spclientctx)
				*pout_spclientctx = m_clients[clientidx];
		}

		return retval;
	}

	int ServerContext::clientDel(ClientContext *pClientCtx)
	{
		int retval = 0;
		
		m_clients_lock.lock();
		for(std::map<int, JsCPPUtils::SmartPointer<ClientContext> >::iterator iter = m_clients.begin(); iter != m_clients.end(); iter++)
		{
			retval = clientDel(iter);
			break;
		}
		m_clients_lock.unlock();

		return retval;
	}

	int ServerContext::clientDel(int clientidx)
	{
		int retval = 0;

		std::map<int, JsCPPUtils::SmartPointer<ClientContext> >::iterator iter;

		m_clients_lock.lock();
		iter = m_clients.find(clientidx);
		if(iter != m_clients.end())
		{
			retval = clientDel(iter);
		}
		m_clients_lock.unlock();

		return retval;
	}
	
	int ServerContext::clientDel(std::map<int, JsCPPUtils::SmartPointer<ClientContext> >::iterator iter)
	{
		int retval = 0;

		int nrst;
		int neno;

		ClientContext *pclientctx = iter->second.getPtr();
		struct epoll_event tmpepevent;
		
		memset(&tmpepevent, 0, sizeof(tmpepevent));

		tmpepevent.events = EPOLLIN;
		tmpepevent.data.ptr = pclientctx;
		if ((nrst = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, pclientctx->m_sockfd, &tmpepevent)) < 0)
		{
			neno = -errno;
			retval = neno;
			if(m_plogger != NULL)
				m_plogger->printf(JsCPPUtils::Logger::LOGTYPE_ERR, "[clientDel] server socket epoll_ctl_del failed: %d", neno);
		}
		pclientctx->close();

		m_clients.erase(iter);

		return retval;
	}
}
