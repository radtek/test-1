#include <windows.h>
#include <WinBase.h>
#include <assert.h>
#include <time.h>
#include "DirectoryPath.h"
#include "GuardClient.h"
#include "GuardCSVData.h"

CGuardClient::CGuardClient()
{
	m_pCallbackStop = NULL;
	m_nIndex = 0xFF;
}

CGuardClient::~CGuardClient()
{
	m_Timer.Stop();
}
//初始化操作
bool CGuardClient::BeginStart(ICallbackStop* pCallbackStop)
{
	m_pCallbackStop = pCallbackStop;
	//看看打开共享内存没
	if (m_GMData.GetData() == 0)
	{
		printf("打开共享内存失败\n");
		return false;
	}
	//设置运行标准
	m_GMData.Lock();
	//找到没有使用进程设置的并设置
	for (int n = 0; n < _MAX_PROCESS_COUNT_; n++)
	{
		GuardClientData* pGCData = &(m_GMData.GetData()->guardClientData[n]);
		if (!pGCData->bUse)
		{
			pGCData->nStatus = GuardAppStatus_Starting;
			pGCData->bSendStop = false;
			pGCData->nPid = GetCurrentProcessId();
			strncpy(pGCData->pszFullName, CDirectoryPath::GetAppFileName().c_str(), _MAX_PATH_LEN_-1);
			pGCData->bUse = true;
			m_nIndex = n;
			m_GMData.UnLock();
			return true;
		}
	}
	m_GMData.UnLock();
	printf("寻找没有使用进程设置失败\n");
	return false;
}

//设置启动完成
void CGuardClient::StartError(bool bStartError)
{
	if (m_GMData.GetData() == 0 || m_nIndex == 0xFF)
	{
		return;
	}
	if (bStartError)
	{
		m_GMData.GetData()->guardClientData[m_nIndex].nStatus = GuardAppStatus_StartError;
		return;
	}
	//创建心跳定时器
	m_Timer.Start(g_nHeartThrobTime, this);
	//创建等待停止事件
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_GMData.GetData()->guardClientData[m_nIndex].nStatus = GuardAppStatus_Run;
}

//等待 守护服务发送停止命令
void CGuardClient::Wait()
{
	if (m_GMData.GetData() == NULL || m_nIndex == 0xFF)
	{
		return ;
	}
	if (m_pCallbackStop == NULL)
	{
		WaitForSingleObject(m_hStopEvent, INFINITE);
		CloseHandle(m_hStopEvent);
	}
}

//设置停止完成
void CGuardClient::SetStop()
{
	if (m_GMData.GetData() == NULL || m_nIndex == 0xFF)
	{
		return;
	}
	//停止定时器
	m_Timer.Stop();
	//清除设置
	m_GMData.GetData()->guardClientData[m_nIndex].nStatus = GuardAppStatus_Stop;
	m_GMData.GetData()->guardClientData[m_nIndex].bUse = false;
	m_GMData.GetData()->guardClientData[m_nIndex].nPid = 0;
	m_GMData.GetData()->guardClientData[m_nIndex].pszFullName[0] = 0;
	m_nIndex = 0xFF;
}

//发生异常时设置 返回异常内存转储文件路径
//此时程序即将退出 资源也不会清除
std::string CGuardClient::Exception()
{
	char pszDumpPath[MAX_PATH] = {0};
	time_t tmt = time(NULL);
	struct tm* ptm = localtime(&tmt);
	sprintf(pszDumpPath, 
		"%s-Except%.4d-%.2d-%.2d-%.2d-%.2d-%.2d.dmp",
		CDirectoryPath::GetAppFileName().c_str(),
		ptm->tm_year+1900, 
		ptm->tm_mon + 1, 
		ptm->tm_mday, 
		ptm->tm_hour, 
		ptm->tm_min, 
		ptm->tm_sec);

	if (m_GMData.GetData() != NULL && m_nIndex != 0xFF)
	{
		m_GMData.GetData()->guardClientData[m_nIndex].nStatus = GuardAppStatus_Exception;
	}
	return pszDumpPath;
}

//定时器
void CGuardClient::OnTimer()
{
//#ifdef _DEBUG
//	time_t tmt = time(NULL);
//	struct tm* ptm = localtime(&tmt);
//	printf("\t%.4d-%.2d-%.2d-%.2d-%.2d-%.2d",
//		ptm->tm_year+1900, 
//		ptm->tm_mon + 1, 
//		ptm->tm_mday, 
//		ptm->tm_hour, 
//		ptm->tm_min, 
//		ptm->tm_sec);
//#endif
	if (m_GMData.GetData() == NULL || m_nIndex == 0xFF)
	{
		return ;
	}
	if (m_GMData.GetData()->guardClientData[m_nIndex].bSendStop)
	{
		printf("进程收到停止信号\n");
		m_GMData.GetData()->guardClientData[m_nIndex].bSendStop = false;
		m_GMData.GetData()->guardClientData[m_nIndex].nStatus = GuardAppStatus_Stopping;
		if (m_pCallbackStop)
		{
			m_pCallbackStop->OnStop();
		}
		else
		{
			SetEvent(m_hStopEvent);
		}
		return ;
	}
	unsigned char nHeartThrob = m_GMData.GetData()->guardClientData[m_nIndex].nHeartThrob;
	m_GMData.GetData()->guardClientData[m_nIndex].nHeartThrobResponsion = ~nHeartThrob;
	return ;
}

bool CGuardClient::Start(const char* pszAppName)
{
	assert(pszAppName != NULL);
	if (m_GMData.GetData() == NULL || m_nIndex == 0xFF)
	{
		return false;
	}
	CManageProcessMMapData procData;
	if (procData.GetData() == NULL 
		|| !procData.GetData()->bManage
		|| procData.GetData()->nCount == 0)
	{
		printf("CGuardClient::Start 无法启动进程\n");
		return false;
	}
	
	int nIndex = procData.Find(pszAppName, procData.GetData()->nCount);
	if (nIndex == -1)
	{
		printf("CGuardClient::Start 获取进程%s索引失败\n", pszAppName);
		return false;
	}
	//发送启动信号
	procData.GetData()->processData[nIndex].bStart = true;
	int nStartTime = 0;
	do 
	{
		Sleep(1000);
		nStartTime++;
		int nMDataIndex = procData.GetData()->processData[nIndex].nIndex;
		if (nMDataIndex == 0)
		{
			continue;
		}
		if (m_GMData.GetData()->guardClientData[nMDataIndex -1].nStatus == GuardAppStatus_Run)
		{
			return true;
		}
	} while (nStartTime < procData.GetData()->processData[nIndex].nStartTimeOut);
	return false;
}

bool CGuardClient::Stop(const char* pszAppName)
{
	assert(pszAppName != NULL);
	if (m_GMData.GetData() == NULL || m_nIndex == 0xFF)
	{
		return false;
	}
	CManageProcessMMapData procData;
	if (procData.GetData() == NULL 
		|| !procData.GetData()->bManage
		|| procData.GetData()->nCount == 0)
	{
		printf("CGuardClient::Start 无法停止进程\n");
		return false;
	}

	int nIndex = procData.Find(pszAppName, procData.GetData()->nCount);
	if (nIndex == -1)
	{
		printf("CGuardClient::Start 获取进程%s索引失败\n", pszAppName);
		return false;
	}
	//发送启动信号
	procData.GetData()->processData[nIndex].bStop = true;
	int nStartTime = 0;
	do 
	{
		Sleep(1000);
		nStartTime++;
		int nMDataIndex = procData.GetData()->processData[nIndex].nIndex;
		if (nMDataIndex == 0)
		{
			return true;
		}
		if (m_GMData.GetData()->guardClientData[nMDataIndex -1].nStatus == GuardAppStatus_Stop)
		{
			return true;
		}
	} while (nStartTime < procData.GetData()->processData[nIndex].nStopTimeOut);
	return false;
}