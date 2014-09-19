#ifndef __LoadCSVFile_H__
#define __LoadCSVFile_H__

#include <string>
#include <fstream>
#include <vector>
#include <map>

//加载CSV配置文件
//然后保存为二维数组 并提供多种访问方式
//会处理比如遇到"等特殊字符
//要求以后每一行表示数据相等 行最后,可有可以没有
class CLoadCSVFile
{
public:
	//pszCSVFile csv文件路径
	//nColumnRow 标识列名称的行 忽略大小写
	CLoadCSVFile(const char* pszCSVFile = NULL, int nColumnRow = 0)
	{
		if (pszCSVFile == NULL)
		{
			return;
		}
		m_strCSVFile = pszCSVFile;
		m_nColumnRow = nColumnRow;
		LoadCSVFile();
	}
	~CLoadCSVFile()
	{
		m_vecData.clear();
		m_mapColumnName.clear();
	}

	//加载csv文件
	//pszCSVFile csv文件路径
	//nColumnRow 标识列名称的行 忽略大小写
	void Load(const char* pszCSVFile, int nColumnRow = 0)
	{
		if (pszCSVFile == NULL)
		{
			return;
		}
		m_strCSVFile = pszCSVFile;
		m_nColumnRow = nColumnRow;
		LoadCSVFile();
	}

	//行数量
	int GetRowCount()
	{
		return m_vecData.size();
	}
	//列数量
	int GetColumnCount()
	{
		if (m_vecData.size() > 0)
		{
			return m_vecData[0].size();
		}
		return 0;
	}
	//获取标识列名称
	const char* GetColumnName(int nColumn)
	{
		return GetText(m_nColumnRow, nColumn);
	}
	//根据行列 返回字符串
	const char* GetText(int nRow, int nColumn)
	{
		if (nRow >= 0 && nColumn >= 0 
			&& nRow < m_vecData.size() 
			&& nColumn < m_vecData[0].size())
		{
			return m_vecData[nRow][nColumn].c_str();
		}
		return "";
	}
	//根据行列 返回字符串
	const char* GetText(int nRow, const char* pszColumnName)
	{
		if (pszColumnName == NULL)
		{
			return "";
		}
		std::string strTemp = pszColumnName;
		ToLower(strTemp);
		std::map<std::string, int>::iterator iter;
		iter = m_mapColumnName.find(strTemp);
		if (iter == m_mapColumnName.end())
		{
			return "";
		}
		int nColumn = iter->second;
		return GetText(nRow, nColumn);
	}
	//根据行列 返回整形
	int GetInt(int nRow, int nColumn)
	{
		return atoi(GetText(nRow, nColumn));
	}
	//根据行列 返回整形
	int GetInt(int nRow, const char* pszColumnName)
	{
		return atoi(GetText(nRow, pszColumnName));
	}
	//根据行列 返回浮点
	double GetDouble(int nRow, int nColumn)
	{
		return atof(GetText(nRow, nColumn));
	}
	//根据行列 返回浮点
	double GetDouble(int nRow, const char* pszColumnName)
	{
		return atof(GetText(nRow, pszColumnName));
	}
public:
	//转换为小写
	static void ToLower(std::string& s)
	{
		for (size_t i=0; i<s.size(); ++i)
		{
			if ( s[i]>='A' && s[i]<='Z')
				s[i] += 'a'- 'A'; 
		}
	}
	//转换为大写
	static void ToUper(std::string& s)
	{
		for ( size_t i=0; i<s.size(); ++i)
		{
			if ( s[i]>'a' && s[i]<'z')
				s[i] -= 'a'-'A';
		}
	}
	static void GetCSVLineItem(std::vector<std::string>& itemVector,const std::string& sLine,const std::string sSeparator = ",")
	{
		if(sLine.size()<=0)
		{
			//最后一列可能为空
			itemVector.push_back(sLine);
			return ;
		}
		std::string::size_type iIndex = sLine.find(sSeparator);
		if(iIndex == std::string::npos)
		{
			std::string sTemp = sLine.substr(0,sLine.find('\r'));
			std::string::size_type nIndex1 = sTemp.find_first_not_of(' ');
			sTemp.erase(0, nIndex1);
			nIndex1 = sTemp.find_last_not_of(' ');
			sTemp.erase(nIndex1+1);
			itemVector.push_back(sTemp);
			return ;
		}

		std::string strLeft = sLine.substr(0,iIndex);
		//处理导入特殊字符
		int nTemp = 0;
		while (strLeft[0] == '\"')
		{
			bool bLast = false;
			//先看看是否读取下一行
			//如果最后一个不是"
			if(iIndex > 1 && strLeft[iIndex-1]!='\"')
			{
				bLast = true;
			}
			else
			{
				int nCount = 1;
				nTemp = 0;
				while((nTemp=strLeft.find('\"', nTemp+1)) != std::string::npos)
				{
					nCount++;
				}
				//如果"个数不是偶数
				if (nCount%2)
				{
					bLast = true;
				}
			}
			if (bLast)
			{
				nTemp = sLine.find(sSeparator, iIndex+1);
				//找不到下一个, 默认处理吧
				if(iIndex == sLine.npos)
				{
					break;
				}
				iIndex = nTemp;
				strLeft = sLine.substr(0,iIndex);
			}
			else
			{
				strLeft.erase(iIndex-1,1);
				strLeft.erase(0,1);

				int nCountTemp = strLeft.length();
				for (int n = 0; n <nCountTemp; n++)
				{
					//连续的两个"" 去掉一个
					if (strLeft[n]=='\"' && strLeft[n+1]=='\"')
					{
						strLeft.erase(n,1);
						nCountTemp--;
					}
					//遇到1F 1F 插入 0A 0D
					else if (strLeft[n]==0x1F && strLeft[n+1]==0x1F)
					{
						strLeft[n] = 0x0D;
						strLeft[n+1] = 0x0A;
					}
				}
				break;
			}
		}
		//去掉空格
		std::string::size_type nIndex = strLeft.find_first_not_of(' ');
		strLeft.erase(0, nIndex);
		nIndex = strLeft.find_last_not_of(' ');
		strLeft.erase(nIndex+1);

		itemVector.push_back(strLeft);
		std::string strRight = sLine.substr(iIndex+sSeparator.length(),sLine.size()-iIndex-1);
		GetCSVLineItem(itemVector,strRight,sSeparator);
	}
protected:
	void LoadCSVFile()
	{
		//清除
		m_vecData.clear();
		m_mapColumnName.clear();

		std::ifstream CSVFile(m_strCSVFile.c_str());
		if (!CSVFile || !CSVFile.is_open())
		{
			return;
		}
		//读取每一行
		std::vector<std::string> itemVector;
		std::string sLine;

		int nIndex = 0;
		while (!CSVFile.eof())
		{
			itemVector.clear();
			std::getline(CSVFile, sLine);
			if (sLine.empty())
			{
				break;
			}
			GetCSVLineItem(itemVector, sLine);
			//如果不是第一行 判断是否等于第一行数量
			if (nIndex > 0 && itemVector.size() != m_vecData[0].size())
			{
				continue;
			}
			//标识行
			if (nIndex == m_nColumnRow)
			{
				for (int n = 0; n < itemVector.size(); n++)
				{
					std::string strTemp = itemVector[n];
					ToLower(strTemp);
					m_mapColumnName[strTemp] = n;
				}
			}
			m_vecData.push_back(itemVector);
			nIndex++;
		}
		CSVFile.close();
	}
protected:
	//CSV文件路径
	std::string m_strCSVFile;
	//表示列名称的行
	int m_nColumnRow;

	//保存的二维字符串
	std::vector<std::vector<std::string> > m_vecData;
	//列名称映射
	std::map<std::string, int> m_mapColumnName;
};

#endif
