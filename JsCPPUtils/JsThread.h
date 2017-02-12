/**
 * @file	JsThread.h
 * @class	JsThread
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/11/03
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <iostream>

#include <stdlib.h>
#include <pthread.h>

#include "SmartPointer.h"
#include "AtomicNum.h"

namespace JsCPPUtils
{
	class JsThread
	{
	public:
		class ThreadContext;

		typedef int (*StartRoutine_t)(ThreadContext *pThreadCtx, int param_idx, void *param_ptr);

		enum RunningStatus
		{
			RS_INITALIZING = 0,
			RS_INITALIZED = 1,
			RS_STARTED = 2,
			RS_STOPPED = 3
		};
		
		class ThreadContext {
		friend class JsThread;
		private:
			int m_index;
			pthread_t m_pthread;
			pthread_mutex_t m_run_mutex;
			
			StartRoutine_t m_startroutine;
			void *m_param;
			
			JsCPPUtils::AtomicNum<int> m_runningstatus;

			ThreadContext(StartRoutine_t startroutine, int index, void *param)
				: m_startroutine(startroutine)
				, m_index(index)
				, m_param(param)
				, m_runningstatus(0)
			{
			}
		public:
			int reqStop();
			int _inthread_isRun();
			RunningStatus getRunningStatus();
		};
		
		template<class T>
		class MessageHandler
		{
		private:
			pthread_mutex_t m_mutex;
			pthread_cond_t m_cond;
			JsCPPUtils::SmartPointer<T> m_spmsg;
			volatile int m_status;
			
		public:
			MessageHandler()
			{
				pthread_mutex_init(&m_mutex, NULL);
				pthread_cond_init(&m_cond, NULL);
				m_status = 0;
			}
	
			int post(JsCPPUtils::SmartPointer<T> spmsg)
			{
				pthread_mutex_lock(&m_mutex);
				m_spmsg = spmsg;
				m_status = 1;
				pthread_cond_signal(&m_cond);
				pthread_mutex_unlock(&m_mutex);
				return 1;
			}
	
			int postCancel()
			{
				pthread_mutex_lock(&m_mutex);
				m_status = 2;
				pthread_cond_signal(&m_cond);
				pthread_mutex_unlock(&m_mutex);
				return 1;
			}
	
			int wait(JsCPPUtils::SmartPointer<T> *pspmsg)
			{
				int retval = 0;
		
				pthread_mutex_lock(&m_mutex);
				while (m_status == 0)
					pthread_cond_wait(&m_cond, &m_mutex);
				if (m_status == 2)
					retval = 0;
				else
					retval = 1;
		
				if (pspmsg != NULL)
					(*pspmsg) = m_spmsg;
				
				m_status = 0;
		
				pthread_mutex_unlock(&m_mutex);
		
				return retval;
			}
			
		};

	private:
		static void *threadProc(void *param);

	public:
		static int start(JsCPPUtils::SmartPointer<ThreadContext> *pspThreadCtx, StartRoutine_t startroutine, int param_idx, void *param_ptr);
		static int reqStop(ThreadContext *pThreadCtx);
	};
}
