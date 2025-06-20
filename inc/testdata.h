#pragma once

#include <vector>
#include <string>

struct TestDataStruct
{
	std::string RoomName = "";
	int PosX = 0;
	int PosY = 0;
	int GridType = 0;
	int RoomType = 0;
	int RoomZone = 0;
};

struct TestData
{
	std::vector<std::vector<TestDataStruct>> RoomData;
};