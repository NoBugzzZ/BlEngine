#ifndef __BLENGINECLIENT_H__
#define __BLENGINECLIENT_H__
#include<string>
#include<vector>
#include"BlEngine.h"

class BlEngineClient
{
public:
	BlEngineClient();
	int BlockEngine(std::string compressedDataFileName, int flag, std::string databaseFilename, std::string tablename);
	bool QueryCardID(std::string cardID);
	std::vector<int> _bListTypes;
private:
	BlEngine BasicBlEngine;
	BlEngine ExtraBlEngine;
};

#endif // !__BLENGINECLIENT_H__
