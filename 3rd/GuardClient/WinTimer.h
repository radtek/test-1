#ifndef __WinTimer_H__
#define __WinTimer_H__


#include <Windows.h>
#include <WinBase.h>
#include <process.h>

class ITimerCallback
{
public:
virtual void OnTimer() = 0;
};

class CWinTimer
{
public:
	CWinTimer() : m_hThread(NULL)
	{
		m_pTimerCallback = NULL;
		m_nTimer = 0;
		m_bEndThread = false;
	}
	~CWinTimer()
	{
		if (m_hThread != NULL)
		{
			m_bEndThread = true;
			WaitForSingleObject(m_hThread, INFINITE);
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}

	//nTimer 周期 毫秒
	//pTimerCallback 时钟回调
	bool Start(int nTimer, ITimerCallback* pTimerCallback)
	{
		//创建定时器执行线程
		unsigned  int threadID;
		if (nTimer == 0 || pTimerCallback == NULL)
		{
			return false;
		}
		Stop();
		m_nTimer = nTimer;
		m_bEndThread = false;
		m_pTimerCallback = pTimerCallback;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadCallback, this, 0, &threadID);
		return (m_hThread != 0);
	}
	bool Stop()
	{
		if (m_hThread != NULL)
		{
			m_bEndThread = true;
			WaitForSingleObject(m_hThread, INFINITE);
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		return true;
	}
private:
	static unsigned __stdcall ThreadCallback(void* pVoid)
	{
		CWinTimer* pWinTimer = (CWinTimer*)pVoid;
		DWORD dwLastTime = 0;
		while(!pWinTimer->m_bEndThread)
		{
			DWORD dwCurTime = GetTickCount();
			if (dwLastTime == 0)
			{
				//第一次
				dwLastTime = dwCurTime;
			}
			else if (dwLastTime > dwCurTime)
			{
				//过了49.7天
				dwLastTime = dwCurTime;
			}
			else if (dwLastTime + pWinTimer->m_nTimer <= dwCurTime)
			{
				//执行
				pWinTimer->m_pTimerCallback->OnTimer();
				dwLastTime = dwCurTime;
			}
			Sleep(1);
		}
		_endthreadex(0);
		return 0;
	}
private:
	//时钟周期 毫秒
	int				m_nTimer;
	//是否停止生成曲线线程
	volatile bool	m_bEndThread;
	//线程句柄
	HANDLE			m_hThread;
	//时钟回调
	ITimerCallback*	m_pTimerCallback;
};

#endif