#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

enum RoomType
{
	Room0 = 0,
	Room1 = 1,
	Room2 = 2,
	Room2C = 3,
	Room3 = 4,
	Room4 = 5
};

enum ERoomZone
{
	None,
	LCZ = 1,
	HCZ = 2,
	EZ = 3,
};

constexpr int MapWidth = 18;
constexpr int MapHeight = 18;
constexpr int ZoneAmount = 3;

/** Entry within the map array */
struct RoomArrayEntry
{
	std::string RoomName = "";
	int PosX = 0;
	int PosY = 0;
	int GridType = 0;
	RoomType RoomType = RoomType::Room0;
	/**
	* @note On grid coords containing a room, this is a number from 0 -> 2, on grid coords with a room it's 0 -> 3. 0 Being no zone, 3 being EZ
	* @todo Can we alter this once we have level gen completely working? Maybe change it to GridZone and RoomZone
	*/
	int RoomZone = 0;
	float RoomRotation = 0.f;
};

struct RoomData
{
	std::string RoomName = "";
	bool bDisableOverlapCheck = "";
	RoomType Shape;
};

class Generator
{
public:
	Generator(bool _DebugPrint = false);
	~Generator();

	int GenerateSeed(const std::string& SeedStr);
	void GenerateMap(const std::string& SeedStr);

	RoomArrayEntry& GetDataAtCoordinate(int X, int Y);
private:
	int GetMapZone(int Y);
	void OutputMap();

	/** Equivalent to MapTemp, but using a struct for everything */
	std::array<std::array<RoomArrayEntry, MapWidth + 1>, MapWidth + 1> MapArray{};

	/** Array safe getters */
	void SetGridType(int X, int Y, int Value = 0);
	int GetGridType(int X, int Y);

	void SetRoomType(int X, int Y, RoomType Value = RoomType::Room1);
	RoomType GetRoomType(int X, int Y);

	void SetZone(int X, int Y, int Value);
	int GetZone(int X, int Y);

	/** Copy of CreateRoom but doesn't spawn the room, but assigns it to the grid */
	bool AssignRoomToCoordinate(ERoomZone RoomZone, RoomType RoomType, int X, int Y, const std::string& Name = "");

	float GetDesiredRoomAngle(RoomType RoomType, int X, int Y);

	bool DebugPrint = false;

	/** @todo Maybe make this a map? The first index is the roomtype*/
	std::vector<std::vector<std::string>> PredefinedRooms;
	bool SetRoom(std::string RoomName, RoomType RoomType, int Pos, int MinPos, int MaxPos);
};