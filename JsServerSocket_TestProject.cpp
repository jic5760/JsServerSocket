#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>

#include <openssl/ssl.h>
#include <openssl/ssl3.h>

#include <signal.h>

#include "JsServerSocket/ServerContext.h"

using namespace std;

volatile int a = 1;

int StartWorkerPostHandler(JsServerSocket::ServerContext *pserverctx, int threadidx, void **out_pthreaduserctx)
{
	printf("StartWorkerPostHandler: %d\n", threadidx);
	return 1;
}

void StopWorkerHandler(JsServerSocket::ServerContext *pserverctx, int threadidx, void *pthreaduserctx)
{
	printf("StopWorkerHandler: %d\n", threadidx);
}

int Client_AcceptHandler(JsServerSocket::ServerContext *pServerCtx, void *pthreaduserctx, int client_sock, struct sockaddr_in *client_paddr)
{
	int procrst;
	JsCPPUtils::SmartPointer<JsServerSocket::ClientContext> spClientCtx;
	
	procrst = pServerCtx->clientAdd(client_sock, client_paddr, &spClientCtx, NULL);
	
	//printf("AcceptHandler: %d: %d: %d\n", client_sock, procrst, (spClientCtx != NULL) ? spClientCtx->getIndex() : -1);
	
	return 1;
}

int Client_RecvHandler(JsServerSocket::ServerContext *pServerCtx, void *pthreaduserctx, JsServerSocket::ClientContext *pClientCtx, int recv_len, char *recv_pbuf)
{
	int32_t datasize;
	char databuf[4100];
	
	if (recv_len != 4)
		return 0;
	
	memcpy(&datasize, recv_pbuf, 4);
	memcpy(databuf, &datasize, 4);
	
	pClientCtx->recvfixedsize(&databuf[4], datasize - 4, 0, NULL);
	pClientCtx->sendfixedsize(databuf, datasize, 0);
	
	//printf("RecvHandler: %d: %d\n", pClientCtx->getIndex(), recv_len);
	return 1;
}

void Client_DelHandler(JsServerSocket::ServerContext *pServerCtx, JsServerSocket::ClientContext *pClientCtx)
{
	printf("DelHandler: %d\n", pClientCtx->getIndex());
}


int main(int argc, char *argv [])
{
	JsServerSocket::ServerContext serverCtx(NULL);

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family      = AF_INET; 
	server_addr.sin_port        = htons(12345); 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	struct rlimit aa;
	int n;
	
	getrlimit(RLIMIT_NOFILE, &aa);
	aa.rlim_cur = aa.rlim_max;
	n = setrlimit(RLIMIT_NOFILE, &aa);
	printf("rlimit : %d\n", n);
	
	signal(SIGPIPE, SIG_IGN);
	
	SSL_library_init();
	
	//serverCtx.init(AF_INET, SOCK_STREAM, IPPROTO_TCP, true, TLSv1_2_server_method(), 128, 4, StartWorkerPostHandler, StopWorkerHandler, Client_AcceptHandler, Client_RecvHandler, Client_DelHandler);
	//serverCtx.sslLoadCertificates("/tmp/cert.pem", "/tmp/key.pem");
	serverCtx.init(AF_INET, SOCK_STREAM, IPPROTO_TCP, false, NULL, 128, 4, StartWorkerPostHandler, StopWorkerHandler, Client_AcceptHandler, Client_RecvHandler, Client_DelHandler);
	
	serverCtx.listen((sockaddr*)&server_addr, sizeof(server_addr), 128);

	serverCtx.startWorkers(2);

	while (a)
	{
		usleep(1000000);
	}

	serverCtx.close();

	return 0;
}