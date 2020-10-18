#include"BlEngine.h"
#include<iostream>
#include<fstream>
#include<ctime>
#include"spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
using namespace std;

BlEngine::BlEngine()
{
}

bool BlEngine::ConnectDatabase(string databaseFilename) {
	bool isSuccess = false;
	const char* p = _strdup(databaseFilename.c_str());
	int result = sqlite3_open_v2(p, &_extraDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
	if (result==SQLITE_OK) {
		spdlog::info("成功链接数据库: {}", databaseFilename);
		isSuccess = true;
	}
	else {
		spdlog::error("无法链接数据库: {}", databaseFilename);
		isSuccess = false;
	}
	return isSuccess;
}

void BlEngine::getVersion(string tablename,int* lVersion,int* Version) {
	sqlite3_stmt* stmt = NULL;
	string sql = "";
	sql = "select lVersion,Version from " + tablename;
	const char* p = _strdup(sql.c_str());
	int result = sqlite3_prepare_v2(_extraDB, p, -1, &stmt, NULL);
	if (result == SQLITE_OK) {
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			*lVersion = sqlite3_column_int(stmt, 0);
			*Version = sqlite3_column_int(stmt, 1);
		}
	}
	else {
		*lVersion = -1;
		*Version = -1;
	}
}

unsigned char BlEngine::compressData(string str, bool* isCorrect)
{
	int num = 0;
	int temp = 0;
	for (int i = 0; i < 2; i++)
	{
		if (((str[i] - '0') >= 0) && ((str[i] - '0') <= 9))
		{
			temp = str[i] - '0';
			num = (num << 4) | (temp & 0xf);
		}
		else if (((str[i] - 'a') >= 0) && ((str[i] - 'a') <= 5))
		{
			temp = 10 + (str[i] - 'a');
			num = (num << 4) | (temp & 0xf);
		}
		else if (((str[i] - 'A') >= 0) && ((str[i] - 'A') <= 5))
		{
			temp = 10 + (str[i] - 'A');
			num = (num << 4) | (temp & 0xf);
		}
		else
		{
			*isCorrect = false;
		}
	}
	unsigned char tempByte = (unsigned char)(num & 0xff);
	return tempByte;
}


int BlEngine::BlockEngine(string compressedDataFileName, int flag, string databaseFilename, string tablename)
{
	if (flag == 1) {
		if (!ConnectDatabase(databaseFilename)) {
			return 0;//返回0表示无法打开增量数据库
		}
		int* lVersion = new int;
		int* Version = new int;
		*lVersion = -10;
		*Version = -10;
		getVersion(tablename, lVersion, Version);
		if (*lVersion == -1 || *Version == -1) {
			return 0;//返回0表示无法打开增量数据库查询version
		}
		if (*lVersion > *Version) {
			return -1;//返回-1表示增量参数中主版本号小于全量版本号
		}
		delete lVersion;
		delete Version;
	}
	
	int isSuccess = 1;
	ifstream inFile(compressedDataFileName, ios::binary);
	if (!inFile){
		isSuccess = -2;//-2表示该文件不存在
		return isSuccess;
	}
	char index;
	CardNetInfo cardNetInfo;
	string str = "";
	int sequence = 0;
	//读取dat文件中索引
	while (inFile.read((char*)&index, sizeof(char)))
	{
		if (index == ':')
		{
			cardNetInfo.cardnet = str;
			cardNetInfo.count = 0;
			_cardNetInfos.push_back(cardNetInfo);
			str = "";
			continue;
		}
		else if (index == '#')
		{
			_cardNetInfos[sequence].count = stoi(str);
			sequence++;
			str = "";
			continue;
		}
		else if (index == '\n')
		{
			break;
		}
		str = str + index;
	}
	inFile.close();
	int category = _cardNetInfos.size();

	ifstream inFile2(compressedDataFileName, ios::binary);
	unsigned char dataStream;
	bool isBinary = false;
	//读取压缩数据
	while (inFile2.read((char*)&dataStream, sizeof(char)))
	{
		//cout << tempB << endl;
		if (isBinary == false)
		{
			if (dataStream == '\n')
			{
				isBinary = true;
				continue;
			}
			continue;
		}
		_loadedData.push_back(dataStream);
	}
	inFile.close();
	int loadedDataLength = _loadedData.size();
	if (loadedDataLength == 0) {
		isSuccess = 0;
	}
	else if(loadedDataLength>0){
		isSuccess = 1;
	}
	return isSuccess;
}

bool BlEngine::QueryCardID(string cardID, int flag, bool* formatIsCorrect, vector<int>* CardNets, vector<int>* BListTypes, vector<int>* statuses)
{
	bool queryResult = false;
	vector<string> strings;
	bool* isCorrect = new bool;
	*isCorrect = true;
	ByteArray byteArray;
	ByteArray tempByteArray;
	int category = _cardNetInfos.size();
	strings.push_back(cardID);
	int lengthPerData = 0;
	if (flag == 0) {
		lengthPerData = 9;
	}
	else if (flag == 1) {
		lengthPerData = 10;
	}
	for (int i = 0; i < strings.size(); i++) {
		//float start = clock();
		int pre = 0;
		int post = 0;
		int pos = 0;
		int lbound = 0;
		int rbound = -1;
		bool flag = true;
		bool isExist = false;
		int count = 0;
		string str = strings[i];
		if (str.size() < 16)
		{
			for (int j = str.size(); j < 16; j++)
			{
				str = str + "0";
			}
		}
		else if (str.size() > 16)
		{
			*formatIsCorrect = false;
			//spdlog::error("数据有误: {}", str);
			continue;
		}
		for (int j = 0; j < 15; j = j + 2)
		{
			byteArray.array[count++] = compressData(str.substr(j, 2), isCorrect);
		}
		if (*isCorrect == false)
		{
			*formatIsCorrect = false;
			//spdlog::error("数据有误: {}",strings[i]);
			*isCorrect = true;
			continue;
		}
		for (int j = 0; j < category; j++)
		{

			lbound = pre = rbound + 1;
			rbound = post = lbound + _cardNetInfos[j].count - 1;

			while (post >= pre)
			{
				//cout << j << endl;
				pos = (post + pre) / 2;
				pos = pos * lengthPerData;
				for (int k = 0; k < 8; k++) {
					tempByteArray.array[k] = _loadedData[pos + k];
				}
				if (byteArray > tempByteArray) {
					pre = (pos / lengthPerData) + 1;
				}
				else if (byteArray < tempByteArray) {
					post = (pos / lengthPerData) - 1;
				}
				else {
					CardNets->push_back(stoi(_cardNetInfos[j].cardnet));
					BListTypes->push_back((_loadedData[pos + 8] & 0xff));
					if (flag == 1) {
						statuses->push_back((_loadedData[pos + 9] & 0xff));
					}
					queryResult = true;
					isExist = true;
					//spdlog::info(" {} - {} - {} ", strings[i], _cardNetInfos[j].cardnet, (_loadedData[pos + 8] & 0xff));

					flag = true;
					int num = pos;
					while (flag == true) {
						pos = pos - lengthPerData;

						if ((pos / lengthPerData) < lbound) {
							flag = false;
							break;
						}
						for (int k = 0; k < 8; k++) {
							tempByteArray.array[k] = _loadedData[pos + k];
						}
						if (byteArray == tempByteArray) {
							CardNets->push_back(stoi(_cardNetInfos[j].cardnet));
							BListTypes->push_back((_loadedData[pos + 8] & 0xff));
							if (flag == 1) {
								statuses->push_back((_loadedData[pos + 9] & 0xff));
							}
						}
						else {
							flag = false;
							break;
						}
					}

					flag = true;
					pos = num;
					while (flag == true) {
						pos = pos + lengthPerData;

						if ((pos / lengthPerData) > rbound) {
							flag = false;
							break;
						}
						for (int k = 0; k < 8; k++) {
							tempByteArray.array[k] = _loadedData[pos + k];
						}

						if (byteArray == tempByteArray) {
							CardNets->push_back(stoi(_cardNetInfos[j].cardnet));
							BListTypes->push_back((_loadedData[pos + 8] & 0xff));
							if (flag == 1) {
								statuses->push_back((_loadedData[pos + 9] & 0xff));
							}
						}
						else {
							flag = false;
							break;
						}
					}

					break;
				}

			}

		}
	}
	return queryResult;
}