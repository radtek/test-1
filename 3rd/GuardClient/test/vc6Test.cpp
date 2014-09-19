#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "../GuardClient.h"
#include "../DirectoryPath.h"

#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")

CGuardClient* g_pGuardClient = NULL;
//发生异常时回调函数
LONG WINAPI pSpaceUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	std::string strException;
	if (g_pGuardClient != NULL)
	{
		strException = g_pGuardClient->Exception();
	}
	else
	{
		strException = CDirectoryPath::GetAppFileName();
		strException += ".dmp";
	}
	printf("%s\n", strException.c_str());
	HANDLE lhDumpFile = CreateFileA(strException.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL ,NULL);
	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = ExceptionInfo;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
	CloseHandle(lhDumpFile);
	return EXCEPTION_EXECUTE_HANDLER;
}


//主线程ID
DWORD	g_dwMainThreadID = 0;

//用于处理收到停止时处理
class CCallbackStop : ICallbackStop
{
private:
	virtual void OnStop()
	{
		printf("CCallbackStop::OnStop()\n");
		if (g_dwMainThreadID != 0)
		{
			::PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
		}
	}
};
int main(int argc, char* argv[])
{
	//注册异常时处理函数
	SetUnhandledExceptionFilter(pSpaceUnhandledExceptionFilter);

	CCallbackStop callbackStop;
	CGuardClient guardClient;
	g_pGuardClient = &guardClient;

	g_dwMainThreadID = GetCurrentThreadId();
	//设置当前路径
	CDirectoryPath::SetCurDirectory();
	//初始化
	if (!guardClient.BeginStart((ICallbackStop*)&callbackStop))
	{
		return 0;
	}
	//设置启动完成 是否启动失败
	guardClient.StartError();

	//循环等待线程退出
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		printf("message:%d wParam:%d lParam:%d\n", msg.message, msg.wParam, msg.lParam);
	}

	//停止完成
	guardClient.SetStop();
	return 0;
}
