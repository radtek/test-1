#ifndef __PS_APISERVEREX_H__
#define __PS_APISERVEREX_H__

#include "psAPICommon.h"

/*
	本头文件内部专用，不对外发布，提供能设置客户端类型和名称,超时设置
*/

#ifdef __cplusplus
extern "C"
{
#endif

	/* 
	<summary>
	[API] 连接实时数据库(增加设置客户端类型和名称,超时设置)
	</summary>
	<param name="pszServer">
	需连接的服务器计算机地址
	支持IP地址和DNS名称，本地据库名称为"127.0.0.1"或 "localhost"
	如果需要指定服务器端口号,在地址后面加冒号后跟端口号 如192.168.0.1:8889
	如果没有指定服务器端口号,使用8889默认端口
	</param>
	<param name="pszUserName">
	用户名称 大小写敏感，支持中文
	</param>
	<param name="pszPassword">
	用户密码 大小写敏感，空格敏感
	</param>
	<param name="pszAppName">
	客户端程序名称，可以设置为空串和PSNULL，没多大作用，提供给事件日志记录的时候使用
	</param>
	<param name="nAppType">
	PS_APPTYPE_ENUM应用程序类型，不同类型的应用程序所具有的权限可能不一样
	</param>
	<param name="nConnectTimeout">
	连接超时时间，时间单位为秒 ,如果设为0，则默认时间为10秒
	</param>
	<param name="nExcuteTimeout">
	执行超时时间，时间单位为秒 ,如果设为0，则默认时间为10秒
	</param>
	<param name="bUseProxy">
	是否使用代理，保留为FALSE, 暂时没作用 
	</param>
	<param name="phServer">
	如果成功，则返回数据库连接句柄
	</param>
	<returns>
	成功时返回PSRET_OK，失败时返回相应的错误代码
	</returns>
	<remarks>
		相比psAPI_Server_Connect设置客户端类型和名称,超时设置
		使用psAPI_Server_Connect客户端类型为PS_APPTYPE_USERCUSTOM,
			客户端名称为空串,使用3秒的网络链接超时和3秒服务器执行本函数超时
	</remarks> */
	PSDLL PSAPIStatus PSAPI psAPI_Server_ConnectEx(
		PSIN PSSTR pszServer,
		PSIN PSSTR pszUserName,
		PSIN PSSTR pszPassword,
		PSIN PSSTR pszAppName,
		PSIN PSUINT32 nAppType,
		PSIN PSUINT16 nConnectTimeout,
		PSIN PSUINT16 nExcuteTimeout,
		PSIN PSBOOL bUseProxy,
		PSOUT PSHANDLE *phServer
		);

#ifdef  __cplusplus
}
#endif

#endif	/* __PS_APISERVEREX_H__ */