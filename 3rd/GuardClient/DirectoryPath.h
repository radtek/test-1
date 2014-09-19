#ifndef __DirectoryPath_H__
#define __DirectoryPath_H__

#include <WinBase.h>
#include <io.h>
#include <string>
#include <vector>

//跨平台获取程序所在目录 程序名称 和设置程序使用当前路径
class CDirectoryPath
{
public:
	//获取程序所在目录 失败返回空串
	//带目录符合
	static std::string GetAppDirectory(std::string strPath = "")
	{
		if (strPath.size() == 0)
		{
			strPath = GetAppFileName();
		}
		int nPos = strPath.rfind(GetSeparator());
		if (nPos != std::string::npos)
		{
			strPath = strPath.substr(0, nPos+1);
		}
		else
		{
			strPath = "";
		}
		return strPath;
	}

	//设置程序当前目录
	//strPath为空时表示设置为程序所在目录
	static bool SetCurDirectory(std::string strPath = "")
	{
		if (strPath.size() == 0)
		{
			strPath = GetAppDirectory();
		}
		if (strPath.size() > 0 
			&& strPath[strPath.size()-1] == GetSeparator())
		{
			strPath = strPath.substr(0, strPath.size()-1);
		}
		if (strPath.size() != 0)
		{
			return SetCurrentDirectoryA(strPath.c_str());
		}
		return false;
	}
	//获取平台路径分割符
	static char GetSeparator()
	{
		char chSeparator = '\\';
		return chSeparator;
	}
	//获取程序绝对路径
	static std::string GetAppFileName()
	{
		std::string strAppFileName;
		char pszFullPath[MAX_PATH] = {0};
		::GetModuleFileNameA(NULL, pszFullPath, MAX_PATH);
		strAppFileName = pszFullPath;
		return strAppFileName;
	}

	//检测目录是否存在 结尾可以带\/
	static bool PathIsExist(std::string strPath)
	{
		//这里相反 路径后面加分隔符是能检测存在的
		//这样可以验证是否是目录
		if (strPath.size() > 0 
			&& strPath[strPath.size()-1] != GetSeparator())
		{
			strPath += GetSeparator();
		}
		return (access(strPath.c_str(), 0) == 0);
	}
	//创建目录 结尾可以带\/
	//可能创建目录的父目录不存在 那么也创建其父目录
	static bool CreatePath(std::string strPath)
	{
		if (strPath.size() > 0 
			&& strPath[strPath.size()-1] == GetSeparator())
		{
			strPath = strPath.substr(0, strPath.size()-1);
		}
		std::vector<std::string> vecPath;
		//从目录查找 直到遇到存在的父目录
		while(!PathIsExist(strPath))
		{
			int nPos = strPath.rfind(GetSeparator());
			if (nPos == std::string::npos)
			{
				return false;
			}
			else
			{
				vecPath.push_back(strPath.substr(nPos));
				strPath = strPath.substr(0, nPos);
			}
		}
		for (int n = 0; n < vecPath.size(); n++)
		{
			strPath += vecPath[vecPath.size()-1-n];
			if (!CreateDirectoryA(strPath.c_str(), NULL))
			{
				return false;
			}
		}
		return true;
	}

	//根据当前路径 获取绝对路径
	static std::string GetProcessFullName(const char* pszProcessName)
	{
		std::string strFullPath;
		char pszFullPath[MAX_PATH] = {0};
		char* lpPart[MAX_PATH] = {NULL};
		DWORD dwRetval = GetFullPathNameA(pszProcessName, MAX_PATH, pszFullPath, lpPart);
		strFullPath = pszFullPath;
		int nPos = strFullPath.find(".exe ");
		if (nPos != std::string::npos)
		{
			strFullPath = strFullPath.substr(0, nPos + 4);
		}
		return strFullPath;
	}

	//获取程序名称
	static std::string GetProcessName(std::string strPath = "")
	{
		if (strPath.size() == 0)
		{
			strPath = GetAppFileName();
		}
		else
		{
			strPath = GetProcessFullName(strPath.c_str());
		}
		int nPos = strPath.rfind('\\');
		if (nPos == std::string::npos)
		{
			nPos = strPath.rfind('/');
		}
		if (nPos != std::string::npos)
		{
			strPath = strPath.substr(nPos + 1);
			int nPos = strPath.find(".exe ");
			if (nPos != std::string::npos)
			{
				strPath = strPath.substr(0, nPos + 4);
			}
		}
		return strPath;
	}
};

#endif