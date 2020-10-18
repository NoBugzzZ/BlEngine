#include"BlEngineClient.h"
#include<iostream>
#include"spdlog/spdlog.h"
using namespace std;

BlEngineClient::BlEngineClient()
{
}

int BlEngineClient::BlockEngine(string compressedDataFileName, int flag, string databaseFilename, string tablename)
{
	int code = -10;
	if (flag == 0) 
	{
		code=BasicBlEngine.BlockEngine(compressedDataFileName, flag, databaseFilename, tablename);
		if (code == -2) {
			spdlog::error("{} 文件不存在 [code:{}]", compressedDataFileName, code);
		}
		else if (code == 1) {
			spdlog::info("读取全量文件成功 [code:{}]", code);
		}
	}
	else if (flag == 1)
	{
		code = ExtraBlEngine.BlockEngine(compressedDataFileName, flag, databaseFilename, tablename);
		if (code == -2) {
			spdlog::error("{} 文件不存在 [code:{}]", compressedDataFileName, code);
		}
		else if (code == 1) {
			spdlog::info("读取增量文件成功 [code:{}]", code);
		}
		else if (code == 0) {
			spdlog::error("无法打开增量数据库或表 [code:{}]", code);
		}
		else if (code == -1) {
			spdlog::error("增量参数中主版本号小于全量版本号,无法加载增量文件 [code:{}]", code);
		}
	}
	return code;
}

bool BlEngineClient::QueryCardID(string cardID)
{
	_bListTypes.clear();
	vector<int>* CardNets=new vector<int>;
	vector<int>* BListTypes=new vector<int>;
	vector<int>* statuses = new vector<int>;
	bool* formatIsCorrect = new bool;
	*formatIsCorrect = true;
	if (BasicBlEngine.QueryCardID(cardID,0,formatIsCorrect,CardNets,BListTypes, statuses))
	{
		_bListTypes.resize(BListTypes->size());
		for (int i = 0; i < BListTypes->size(); i++) {
			_bListTypes[i] = (*BListTypes)[i];
		}
		for (int i = 0; i < CardNets->size(); i++) {
			spdlog::info("{} {} {}", cardID, (*CardNets)[i], (*BListTypes)[i]);
		}
		return true;
	}
	if (*formatIsCorrect == false) 
	{
		return false;
	}
	if (ExtraBlEngine.QueryCardID(cardID,1, formatIsCorrect, CardNets, BListTypes,statuses))
	{
		_bListTypes.resize(BListTypes->size());
		for (int i = 0; i < BListTypes->size(); i++) {
			_bListTypes[i] = (*BListTypes)[i];
		}
		for (int i = 0; i < CardNets->size(); i++) {
			spdlog::info("{} {} {} {}", cardID, (*CardNets)[i], (*BListTypes)[i], (*statuses)[i]);
		}
		return true;
	}
	return false;
}
