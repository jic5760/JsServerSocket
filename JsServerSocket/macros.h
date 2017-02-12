/**
 * @file	JsServerSocket/macros.h
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/30
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSSERVERSOCKET_MACROS_H__
#define __JSSERVERSOCKET_MACROS_H__

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((int)-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR ((int)-1)
#endif

#ifndef INVALID_FD
#define INVALID_FD ((int)-1)
#endif

#ifndef IS_SOCKET_ERROR
#define IS_SOCKET_ERROR(r) ((r) == SOCKET_ERROR)
#endif

#ifndef IS_BSDFUNC_ERROR
#define IS_BSDFUNC_ERROR(r) ((r) == -1)
#endif

#ifndef WIN32
#ifndef closesocket
#define closesocket(sock) close(sock)
#endif
#endif

#endif /* __JSSERVERSOCKET_MACROS_H__ */
