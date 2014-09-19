#ifndef __GuardClient_H__
#define __GuardClient_H__

#include <Windows.h>
#include "GuardData.h"
#include "WinTimer.h"

//用于被守护进程收到停止时处理
class ICallbackStop
{
public:
	virtual void OnStop() = 0;
};

class CGuardClient : ITimerCallback
{
public:
	CGuardClient();
	~CGuardClient();

	//启动前调用 打开创建共享内存文件 并获取访问共享内存的索引
	//pCallbackStop设置时表示 被守护进程重载ICallbackStop获得停止通知
	//pCallbackStop为NULL是 需要用Wait()等待守护服务发送停止命令
	//返回false表示失败 不应该在继续运行程序
	bool BeginStart(ICallbackStop* pCallbackStop = NULL);

	//被守护进程初始操作完成设置
	//初始化操作失败时 被守护进程应该停止自己 守护服务清楚响应的共享内存设置
	void StartError(bool bStartError = false);

	//等待 守护服务发送停止命令
	void Wait();

	//设置停止完成
	void SetStop();

	//发生异常时设置 返回异常内存转储文件路径
	//此时程序即将退出 资源也不会清除
	std::string Exception();

	//用于启动其他被守护的程序
	//pszAppName最好是psNTService.csv AppPath设置的进程名称(可能带路径和参数)
	//		因为一个程序可能因参数或者路径不同被启动多次
	//		只传入程序名称时会查找第一个匹配的被守护程序
	//		如果最终没有找到适当的被守护程序 返回false
	//启动失败返回false 被守护的程序已经被启动不操作并且返回true
	//启动当前进程是不操作的,当前进程已经运行
	bool Start(const char* pszAppName);

	//用于停止其他被守护的程序
	//pszAppName最好是psNTService.csv AppPath设置的进程名称(可能带路径和参数)
	//		因为一个程序可能因参数或者路径不同被启动多次
	//		只传入程序名称时会查找第一个匹配的被守护程序
	//		如果最终没有找到适当的被守护程序 返回false
	//停止失败返回false 被守护的程序已经停止不操作并且返回true
	//停止当前进程时注意处理停止通知,函数是同步执行的,否则等待超时当前进程被强杀
	bool Stop(const char* pszAppName);

private:
	//定时器执行函数
	virtual void OnTimer();
private:
	//定时器
	CWinTimer			m_Timer;
	//回调停止消息
	ICallbackStop*		m_pCallbackStop;
	//等待停止事件
	HANDLE				m_hStopEvent;
	//进程状态共享内存数据
	CGuardMappingData	m_GMData;
	//访问守护共享数据索引
	unsigned char		m_nIndex;
};

#endif