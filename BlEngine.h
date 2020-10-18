#ifndef __BLENGINE_H__
#define __BLENGINE_H__
#include<vector>
#include<string>
#include"sqlite3.h"

class BlEngine
{
public:
	BlEngine();
	int BlockEngine(std::string compressedDataFileName, int flag, std::string databaseFilename, std::string tablename);
	bool QueryCardID(std::string cardID, int flag, bool* formatIsCorrect,std::vector<int>* CardNets, std::vector<int>* BListTypes,std::vector<int>* statuses);
private:
	struct CardNetInfo {
		std::string cardnet;
		int count;
	};
	struct ByteArray
	{
		unsigned char array[8];
		inline friend bool operator < (const struct ByteArray& ls, const struct ByteArray& rs)
		{
			bool flag = false;
			for (int i = 0; i < 8; i++) {
				if (ls.array[i] < rs.array[i]) {
					flag = true;
					break;
				}
				else if (ls.array[i] > rs.array[i])
					break;
			}
			return flag;
		}
		inline friend bool operator > (const struct ByteArray& ls, const struct ByteArray& rs)
		{
			bool flag = false;
			for (int i = 0; i < 8; i++) {
				if (ls.array[i] > rs.array[i]) {
					flag = true;
					break;
				}
				else if (ls.array[i] < rs.array[i])
					break;
			}
			return flag;
		}
		inline friend bool operator == (const struct ByteArray& ls, const struct ByteArray& rs)
		{
			bool flag = true;
			for (int i = 0; i < 8; i++) {
				if (ls.array[i] < rs.array[i]) {
					flag = false;
					break;
				}
				else if (ls.array[i] > rs.array[i]) {
					flag = false;
					break;
				}
			}
			return flag;
		}

	};

	bool ConnectDatabase(std::string databaseFilename);
	void getVersion(std::string tablename, int* IVersion, int* Version);
	unsigned char compressData(std::string str, bool* isCorrect);	//Ñ¹ËõÊý¾Ý
	std::vector<unsigned char> _loadedData;
	std::vector<CardNetInfo> _cardNetInfos;
	sqlite3* _extraDB;
};

#endif // !__BLENGINE_H__

