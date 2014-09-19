#ifndef __GuardCSVData_H__
#define __GuardCSVData_H__

#include <WinBase.h>
#include "LoadCSVFile.h"
#include "GuardData.h"
#include "DirectoryPath.h"

//把CSV文件放在共享内存里
//因为服务管理器与守护服务不同步时非常麻烦

//服务管理器与守护服务共享内存名称
const char	g_pszTrayMappingName[] = "psTrayMMapFile";
//刷新控制操作时间 毫秒
const int	g_nManageProcessTime = 100;

#pragma pack(push, ManageProcessMMapData_H)
#pragma pack(1)

//控制每个进程数据结构
typedef struct __ManageProcessData
{
	//服务管理器控制启动
	bool			bStart;
	//服务管理器控制停止
	bool			bStop;
	//服务器控制重启动
	bool			bRestart;
	//强杀被守护进程
	bool			bKill;

	//进程索引 在GuardMappingData里的位置
	//注意是从1开始的，因为默认值是0表示没有设置
	int				nIndex;
	//守护进程设置进程状态 nIndex为0时有效
	//停止 正在启动 启动失败 异常 
	unsigned char	nStatus;

	//启动超时(S)
	int				nStartTimeOut;
	//启动失败时重启次数
	int				nRestartTimes;
	//重启间隔时间(S)
	int				nIntervalTime;
	//发生异常重启次数
	int				nExceptionTimers;
	//心跳超时(S)
	int				nHeartThrob;
	//启动失败时重启次数
	int				nStopTimeOut;

	//守护服务启动时一起启动
	bool			bAutoStart;
	//尝试以当前桌面用户启动(带界面程序启动)
	bool			bDesktopUser;

	//进程版本号
	char			pszVersion[16];
	//进程说明
	char			pszProcessDesc[256];
	//进程名称
	char			pszProcessName[_MAX_PATH_LEN_];
	//进程绝对路径
	char			pszProcessFullName[_MAX_PATH_LEN_];
}ManageProcessData;

//服务管理器发送给服务控制数据结构
typedef struct __ManageProcessMMapData
{
	//表示服务管理器控制进程的数量
	unsigned char		nCount;
	//是否可以控制启动 停止
	bool				bManage;
	ManageProcessData	processData[_MAX_PROCESS_COUNT_];
}ManageProcessMMapData;

#pragma pack(pop, ManageProcessMMapData_H)

//封装Windows下共享内存访问
class CManageProcessMMapData
{
public:
	CManageProcessMMapData():m_Mutex(g_pszTrayMappingName)
	{
		m_Mutex.acquire();
		m_MapData.Create(g_pszTrayMappingName);
		m_Mutex.release();
		m_pMMapData = m_MapData.GetData();
	}

	~CManageProcessMMapData()
	{
	}

	void Clear()
	{
		if (m_pMMapData != NULL)
		{
			memset(m_pMMapData, 0, sizeof(ManageProcessMMapData));
		}
	}

	void LoadCSVFile(const char* pszFile = "psNTService.csv")
	{
		if (m_pMMapData == NULL)
		{
			return ;
		}
		memset(m_pMMapData, 0, sizeof(ManageProcessMMapData));
		//从CSV文件里读取
		CLoadCSVFile CSVFile(pszFile);
		if (CSVFile.GetColumnCount() != 11)
		{
			return ;
		}
		//第一行 字段标题
		//第二行 说明
		int nIndex = 0;
		m_pMMapData->nCount = CSVFile.GetRowCount() - 2;
		for (int n = 2; n < CSVFile.GetRowCount(); n++)
		{
			std::string strFullName = CDirectoryPath::GetProcessFullName(CSVFile.GetText(n, "AppPath"));
			//发现了相同的
			if (Find(strFullName.c_str()) != -1)
			{
				//忽略
				continue;
			}
			strncpy(m_pMMapData->processData[nIndex].pszProcessName, CSVFile.GetText(n, "AppPath"), _MAX_PATH_LEN_-1);
			strncpy(m_pMMapData->processData[nIndex].pszProcessFullName, CDirectoryPath::GetProcessFullName(CSVFile.GetText(n, "AppPath")).c_str(), _MAX_PATH_LEN_-1);
			strncpy(m_pMMapData->processData[nIndex].pszProcessDesc, CSVFile.GetText(n, "AppDesc"), 255);
			strncpy(m_pMMapData->processData[nIndex].pszVersion, CSVFile.GetText(n, "AppVersion"), 15);
			m_pMMapData->processData[nIndex].nStartTimeOut = CSVFile.GetInt(n, "StartTimeOut");
			m_pMMapData->processData[nIndex].nRestartTimes = CSVFile.GetInt(n, "RestartTimes");
			m_pMMapData->processData[nIndex].nIntervalTime = CSVFile.GetInt(n, "IntervalTime");
			m_pMMapData->processData[nIndex].nExceptionTimers = CSVFile.GetInt(n, "ExceptionTimers");
			m_pMMapData->processData[nIndex].nHeartThrob = CSVFile.GetInt(n, "HeartThrob");
			m_pMMapData->processData[nIndex].nStopTimeOut = CSVFile.GetInt(n, "StopTimeOut");
			m_pMMapData->processData[nIndex].bAutoStart = CSVFile.GetInt(n, "AutoStart");
			m_pMMapData->processData[nIndex].bDesktopUser = CSVFile.GetInt(n, "DesktopUser");
			nIndex++;
		}
		m_pMMapData->nCount = nIndex;
	}
	
	//根据进程绝对路径 忽略大小写 失败返回-1
	//nIndex 查找范围
	int Find(const char* pszProcessName, int nIndex = _MAX_PROCESS_COUNT_)
	{
		//先尝试查找长名
		for (int n = 0; n < nIndex; n++)
		{
			if (stricmp(pszProcessName, 
					m_pMMapData->processData[n].pszProcessFullName) == 0)
			{
				return n;
			}
		}
		//再尝试相对路径
		for (int n = 0; n < nIndex; n++)
		{
			if (stricmp(pszProcessName, 
					m_pMMapData->processData[n].pszProcessName) == 0)
			{
				return n;
			}
		}
		//再尝试程序名称
		for (int n = 0; n < nIndex; n++)
		{
			if (stricmp(pszProcessName, 
					CDirectoryPath::GetProcessName(m_pMMapData->processData[n].pszProcessFullName).c_str()) == 0)
			{
				return n;
			}
		}
		return -1;
	}

	ManageProcessMMapData* GetData()
	{
		return m_MapData.GetData();
	}
private:
	TMapData<ManageProcessMMapData> m_MapData;
	CProcessMutex m_Mutex;
	//共享内存数据
	ManageProcessMMapData* m_pMMapData;
};

#endif