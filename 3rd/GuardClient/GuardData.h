#ifndef __GuardData_H__
#define __GuardData_H__

#include <string>

#pragma pack(push, GuardData_H)
#pragma pack(1)

//程序路径最大长度
#define _MAX_PATH_LEN_ 256
//最大管理进程数量
#define _MAX_PROCESS_COUNT_ 256

//全局打开共享内存文件名称
const char	g_pszGuardMappingName[] = "psGuardMMapFile";
//心跳时间 毫秒
const int	g_nHeartThrobTime = 1000;


//被守护程序状态
typedef enum __EnumGuardAppStatus
{
	GuardAppStatus_Stop = 0,		//停止 默认状态
	GuardAppStatus_Starting,		//正在启动
	GuardAppStatus_StartError,		//启动失败
	GuardAppStatus_Run,				//正在运行
	GuardAppStatus_HeartThrobError,	//心跳失败
	GuardAppStatus_Exception,		//异常
	GuardAppStatus_Stopping			//正在停止
}EnumGuardAppStatus;


//被守护进程使用数据结构
typedef struct __GuardClientData
{
	//是否启用
	bool			bUse;

	//守护服务进程状态 由被守护进程自己维护
	//守护服务可能设置停止和心跳失败状态
	unsigned char	nStatus;
	//守护服务停止命令默认0 1表示通知停止
	unsigned char	bSendStop;

	//心跳值 守护服务每次递增1
	unsigned char	nHeartThrob;
	//被守护应答 值是nHeartThrob取反
	unsigned char	nHeartThrobResponsion;

	//被守护进程ID 由被守护进程设置
	unsigned long	nPid;
	//被守护进程路径 由被守护进程设置
	//验证守护进程是否存在
	//守护服务使用看是否有相同的进程存在
	char			pszFullName[_MAX_PATH_LEN_];
}GuardClientData;

//共享内存数据结构
//守护服务申请共享内存
typedef struct __GuardMappingData
{
	//守护服务进程ID
	unsigned long	nPid;
	//守护服务绝对路径
	char			pszFullName[_MAX_PATH_LEN_];
	//被守护进程使用数据结构数组
	GuardClientData guardClientData[_MAX_PROCESS_COUNT_];
} GuardMappingData;

#pragma pack(pop, GuardData_H)

//封装Windows下进程锁
class CProcessMutex
{
public:
	CProcessMutex(const char* pszMutex = NULL):m_hMutex(NULL)
	{
		//全局名称
		std::string strMutex = "Global\\";
		if (pszMutex != NULL)
		{
			std::string strTemp = pszMutex;
			for (int n = 0; n < strTemp.length(); n++)
			{
				if (strTemp[n] == '\\')
				{
					strTemp[n] = '-';
				}
			}
			strMutex += strTemp;
			strMutex += "_Mutex";
		}
		//创建互斥对象 已经有时就打开
		SECURITY_ATTRIBUTES sa = {0};
		//注意访问控制 不同会话访问需要设置
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH); 
		if (!InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
		{
			return;
		}
		if (!SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE))
		{
			return;
		}
		
		//创建不带控制的互斥对象
		m_hMutex = CreateMutexA(&sa, FALSE, pszMutex?strMutex.c_str():NULL);
	}
	~CProcessMutex()
	{
		CloseHandle(m_hMutex);
	}

	int acquire()
	{
		switch (WaitForSingleObject(m_hMutex, INFINITE))
		{
		case WAIT_OBJECT_0:
			return 0;
		default:
			return -1;
		}
	}
	//成功之后已经得到了锁
	int tryacquire()
	{
		switch (WaitForSingleObject(m_hMutex, 0))
		{
		case WAIT_OBJECT_0:
			return 0;
		default:
			return -1;
		}
	}
	int release()
	{
		return !ReleaseMutex(m_hMutex);
	}
private:
	HANDLE m_hMutex;
};

//封装Windows下共享内存访问
template<typename T>
class TMapData
{
public:
	TMapData():m_pT(NULL)
	{
		//
	}

	BOOL Create(const char* pszMap)
	{
		//ASSERT(pszMap != NULL);
		//全局名称
		std::string strMap = "Global\\";
		strMap += pszMap;
		BOOL bCreate = FALSE;
		//尝试打开
		m_hMMap = OpenFileMappingA(FILE_MAP_WRITE|FILE_MAP_READ, FALSE, strMap.c_str());
		if (m_hMMap == NULL)
		{
			SECURITY_ATTRIBUTES sa = {0};
			//注意访问控制 不同会话访问需要设置
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH); 
			if (!InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
			{
				return FALSE;
			}
			if (!SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE))
			{
				return FALSE;
			}
			m_hMMap = CreateFileMappingA(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 
				0, sizeof(T), strMap.c_str());
			LocalFree(sa.lpSecurityDescriptor);
			if (m_hMMap == NULL || m_hMMap == INVALID_HANDLE_VALUE)
			{
				return FALSE;
			}
			bCreate = TRUE;
		}
		m_pT = (T*)MapViewOfFile(m_hMMap, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
		if (bCreate)
		{
			memset(m_pT, 0, sizeof(T));
		}
		return TRUE;
	}

	~TMapData()
	{
		//关闭共享内存
		if (m_pT != NULL)
		{
			UnmapViewOfFile(m_pT);
		}
		if (m_hMMap != NULL)
		{
			CloseHandle(m_hMMap);
		}
	}

	T* GetData()
	{
		return m_pT;
	}
public:
	//共享内存文件对象
	HANDLE	m_hMMap;
	//共享数据
	T* m_pT;
};

class CGuardMappingData
{
public:
	CGuardMappingData():m_Mutex(g_pszGuardMappingName)
	{
		Lock();
		m_MapData.Create(g_pszGuardMappingName);
		UnLock();
	}
	~CGuardMappingData()
	{
		//
	}
	GuardMappingData* GetData()
	{
		return m_MapData.GetData();
	}
	bool Lock()
	{
		return m_Mutex.acquire()==0;
	}
	bool UnLock()
	{
		return m_Mutex.release()==0;
	}
private:
	TMapData<GuardMappingData> m_MapData;
	CProcessMutex m_Mutex;
};

#endif