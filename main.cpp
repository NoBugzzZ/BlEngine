#include"BlEngineClient.h"
#include<iostream>
#include"spdlog/spdlog.h"
using namespace std;

int main()
{
	BlEngineClient blEngineClient;
	blEngineClient.BlockEngine("C:/Users/tz/source/repos/BasicCompressedData.txt", 0, "", "");
	blEngineClient.BlockEngine("C:/Users/tz/source/repos/ExtraCompressedData.txt", 1, "C:/Users/tz/source/repos/10716135_BIncList.db", "tbl_VersionInfo");
	int a;
	cout << "查询条数: ";
	cin >> a;
	vector<string> strs;
	string str;
	for (int i = 0; i < a; i++) 
	{
		cin >> str;
		strs.push_back(str);
	}
	for (int i = 0; i < strs.size(); i++)
	{
		if (!blEngineClient.QueryCardID(strs[i])) {
			spdlog::error("不存在该CardID: {}", strs[i]);
		}
	}
	return 0;
}