/**
 * @file	JsThread.cpp
 * @class	JsThread
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/11/03
 * @brief	JsThread
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <errno.h>

#include "JsThread.h"

namespace JsCPPUtils
{
	void *JsThread::threadProc(void *param)
	{
		JsCPPUtils::SmartPointer<ThreadContext> spThreadCtx;
		int retval;
		spThreadCtx.attach((JsCPPUtils::SmartPointer<ThreadContext>*)param);
		
		spThreadCtx->m_runningstatus = 2;
		retval = spThreadCtx->m_startroutine(spThreadCtx.getPtr(), spThreadCtx->m_index, spThreadCtx->m_param);
		spThreadCtx->m_runningstatus = 3;
		
		return (void*)retval;
	}

	int JsThread::start(JsCPPUtils::SmartPointer<ThreadContext> *pspThreadCtx, StartRoutine_t startroutine, int param_idx, void *param_ptr)
	{
		int retval = 0;
		int nrst;
		int step = 0;
	
		ThreadContext *pThreadCtx;
		JsCPPUtils::SmartPointer< ThreadContext > spThreadCtx;
		
		pThreadCtx = new ThreadContext(startroutine, param_idx, param_ptr);
		if(pThreadCtx == NULL)
		{
			return -errno;
		}

		spThreadCtx = pThreadCtx;

		do
		{
			nrst = pthread_mutex_init(&pThreadCtx->m_run_mutex, NULL);
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			step = 1;
			nrst = pthread_mutex_lock(&pThreadCtx->m_run_mutex);
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			
			pThreadCtx->m_runningstatus = 1;
			
			step = 2;
			nrst = pthread_create(&pThreadCtx->m_pthread, NULL, threadProc, spThreadCtx.detach());
			if (nrst != 0)
			{
				retval = -nrst;
				break;
			}
			step = 3;
		
			retval = 1;
		} while (0);
	
		if (retval <= 0)
		{
			if (step >= 2)
			{
				pthread_mutex_unlock(&pThreadCtx->m_run_mutex);
			}
			if (step >= 1)
			{
				pthread_mutex_destroy(&pThreadCtx->m_run_mutex);
			}
		}
		else
		{
			if (pspThreadCtx != NULL)
				*pspThreadCtx = spThreadCtx;
		}
	
		return retval;
	}

	int JsThread::reqStop(ThreadContext *pThreadCtx)
	{
		return pThreadCtx->reqStop();
	}

	int JsThread::ThreadContext::reqStop()
	{
		int nrst = pthread_mutex_unlock(&m_run_mutex);
		pthread_cancel(m_pthread);
		if(nrst != 0)
			return -errno;
		return 1;
	}

	int JsThread::ThreadContext::_inthread_isRun()
	{
		int nrst = pthread_mutex_trylock(&m_run_mutex);
		switch(nrst)
		{
		case 0: // unlocked
			pthread_mutex_unlock(&m_run_mutex);
			return 0; // quit
		case EBUSY: // Already locked
			return 1; // run
		}
		return -nrst; // error / quit
	}
	
	JsThread::RunningStatus JsThread::ThreadContext::getRunningStatus()
	{
		return (RunningStatus)m_runningstatus.get();
	}
	
	
}
