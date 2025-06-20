#include "generator.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <random>

/** Remaps */
namespace FMath
{
	template<typename T>
	inline T Min(const T& a, const T& b)
	{
		return std::min(a, b);
	}

	template<typename T>
	inline T Max(const T& a, const T& b)
	{
		return std::max(a, b);
	}

	template<typename T>
	inline T Abs(const T& value)
	{
		return std::abs(value);
	}

	inline int Floor(int value)
	{
		return std::floor(value);
	}

	inline float Floor(float value)
	{
		return std::floor(value);
	}

	inline double Floor(double value)
	{
		return std::floor(value);
	}

	inline long double Floor(long double value)
	{
		return std::floor(value);
	}
}

#define LogDebug(format, ...) if (DebugPrint) { std::printf("[DEBUG]" format "\n", ##__VA_ARGS__); }
#define LogWarning(format, ...) std::printf("[WARNING]" format "\n", ##__VA_ARGS__);
#define LogError(format, ...) std::printf("[ERROR]" format "\n", ##__VA_ARGS__);

static constexpr int RND_A = 48271;
static constexpr int RND_M = 2147483647;
static constexpr int RND_Q = 44488;
static constexpr int RND_R = 3399;
static int RndState;

static inline float Rnd()
{
	RndState = RND_A * (RndState % RND_Q) - RND_R * (RndState / RND_Q);
	if (RndState < 0)
		RndState += RND_M;
	return (RndState & 65535) / 65536.0f + (.5f / 65536.0f);
}

int BlitzRand(int From, int To = 1)
{
	if (To < From) std::swap(From, To);
	return int(Rnd() * (To - From + 1)) + From;
}

void BlitzSeedRand(int Seed)
{
	Seed &= 0x7fffffff;
	RndState = Seed ? Seed : 1;
}

Generator::Generator(bool _DebugPrint /*= false*/)
{
	DebugPrint = _DebugPrint;
}

Generator::~Generator()
{
}

int Generator::GenerateSeed(const std::string& SeedStr)
{
	int SeedNum = 0;
	int Shift = 0;

	for (const char Char : SeedStr)
	{
		SeedNum ^= Char << Shift;
		Shift = (Shift + 1) % 24;
	}

	return SeedNum;
}

static constexpr int ZONE_AMOUNT = 3;
static constexpr int CHECKPOINT = 255;

/**
* !NOTES!
*
* BlitzRand will affect later calls, so if you call BlitzRand before the first while loop, generation will be skewed.
* : in BlitzBasic is a statement separator
*
* !TODO!
* Use Room Enum over raw 1,2,3,4,5 values
*/

void Generator::GenerateMap(const std::string& SeedStr)
{
	RoomType test = RoomType(5);

	int Seed = GenerateSeed(SeedStr);
	BlitzSeedRand(Seed);

	int X = 0, Y = 0;
	int X2 = 0, Y2 = 0;
	int Temp = 0, TempHeight = 0;

	if (DebugPrint) std::printf("Generating map with seed %s (%d)\n", SeedStr.data(), Seed);

	X = FMath::Floor(MapWidth / 2);
	Y = MapHeight - 2;

	// Default the grid coords
	// Note, this doesn't exist in CB. Originally CB did everything based off a single int on a huge grid
	// That isn't really expandable and doesn't work well for us in Unreal, so note that anything that used MapTemp (i.e. just the room number)
	// would be assessible using MapArray[X][Y].GridType. Also note, that SCP:CB is a bit weird, so the RoomZone will get incremented by 1.
	// Unfortunately we can't change that without altering a bunch of code, so it shall stay forever.
	for (int locX = 0; locX < MapWidth + 1; locX++)
	{
		for (int locY = 0; locY < MapHeight + 1; locY++)
		{
			MapArray[locX][locY].PosX = locX;
			MapArray[locX][locY].PosY = locY;
			MapArray[locX][locY].RoomZone = GetMapZone(locY);
		}
	}

	for (int i = Y; i < MapHeight; i++)
	{
		SetGridType(X, i, 1);
	}

	// Generate initial layout
	do
	{
		// Random number between 10 and 15
		int Width = BlitzRand(10, 15);
		if (X > (MapWidth * 0.6f))
		{
			Width = -Width;
		}
		else if (X > (MapWidth * 0.4f))
		{
			X = X - Width / 2;
		}

		// Make sure the hallway doesn't go outside the array
		if ((X + Width) > (MapWidth - 3))
		{
			Width = MapWidth - 3 - X;
		}
		else if ((X + Width) < 2)
		{
			Width = -X + 2;
		}

		X = FMath::Min(X, X + Width);
		Width = FMath::Abs(Width);

		for (int i = X; i <= (X + Width); i++)
		{
			int xIndex = FMath::Min(i, MapWidth);
			SetGridType(xIndex, Y, 1);
		}

		int Height = BlitzRand(3, 4);
		if ((Y - Height) < 1)
		{
			Height = Y - 1;
		}

		int yHallways = BlitzRand(4, 5);

		int val1 = GetMapZone(Y - Height);
		int val2 = GetMapZone(Y - Height + 1);
		if (GetMapZone(Y - Height) != GetMapZone(Y - Height + 1))
		{
			Height = Height - 1;
		}

		for (int i = 1; i <= yHallways; i++)
		{
			int test = FMath::Min(BlitzRand(X, X + Width - 1), MapWidth - 2);
			X2 = FMath::Max(test, 2);
			while (GetGridType(X2, Y - 1) || GetGridType(X2 - 1, Y - 1) || GetGridType(X2 + 1, Y - 1))
			{
				X2 = X2 + 1;
			}

			if (X2 < (X + Width))
			{
				if (i == 1)
				{
					TempHeight = Height;
					if (BlitzRand(2, 1) == 1)
					{
						X2 = X;
					}
					else
					{
						X2 = X + Width;
					}
				}
				else
				{
					TempHeight = BlitzRand(1, Height);
				}

				for (Y2 = (Y - TempHeight); Y2 <= Y; Y2++)
				{
					if (GetMapZone(Y2) != GetMapZone(Y2 + 1))
					{
						SetGridType(X2, Y2, CHECKPOINT);
					}
					else
					{
						SetGridType(X2, Y2, 1);
					}
				}

				if (TempHeight == Height)
				{
					Temp = X2;
				}
			}
		}

		X = Temp;
		Y = Y - Height;
	} while (!(Y < 2));

	/** @UE_PORT_TODO Use a map instead*/
	int Room1Amount[3]{};
	int Room2Amount[3]{};
	int Room2CAmount[3]{};
	int Room3Amount[3]{};
	int Room4Amount[3]{};

	// Correctly set room type depending on adjacent rooms
	int Zone = 0;
	for (Y = 1; Y < MapHeight; Y++)
	{
		Zone = GetMapZone(Y);

		for (X = 1; X < MapWidth; X++)
		{
			if (GetGridType(X, Y) > 0)
			{
				Temp = FMath::Min(GetGridType(X + 1, Y), 1) + FMath::Min(GetGridType(X - 1, Y), 1);
				Temp = Temp + FMath::Min(GetGridType(X, Y + 1), 1) + FMath::Min(GetGridType(X, Y - 1), 1);

				if (GetGridType(X, Y) < CHECKPOINT)
				{
					SetGridType(X, Y, Temp);
				}
				// Assume it to be a checkpoint
				else
				{
					/** @todo We set the checkpoint room type to be room2, but maybe we might want different ones in the future? */
					SetRoomType(X, Y, RoomType::Room2);
				}

				switch (GetGridType(X, Y))
				{
				case 1:
					Room1Amount[Zone] = Room1Amount[Zone] + 1;
					SetRoomType(X, Y, RoomType::Room1);
					break;
				case 2:
				{
					if (FMath::Min(GetGridType(X + 1, Y), 1) + FMath::Min(GetGridType(X - 1, Y), 1) == 2)
					{
						Room2Amount[Zone] = Room2Amount[Zone] + 1;
						SetRoomType(X, Y, RoomType::Room2);
					}
					else if (FMath::Min(GetGridType(X, Y + 1), 1) + FMath::Min(GetGridType(X, Y - 1), 1) == 2)
					{
						Room2Amount[Zone] = Room2Amount[Zone] + 1;
						SetRoomType(X, Y, RoomType::Room2);
					}
					else
					{
						Room2CAmount[Zone] = Room2CAmount[Zone] + 1;
						SetRoomType(X, Y, RoomType::Room2C);
					}
					break;
				}
				case 3:
					Room3Amount[Zone] = Room3Amount[Zone] + 1;
					SetRoomType(X, Y, RoomType::Room3);
					break;

				case 4:
					Room4Amount[Zone] = Room4Amount[Zone] + 1;
					SetRoomType(X, Y, RoomType::Room4);
					break;
				}
			}
		}
	}

	// Force more Room1s (if needed)
	for (int i = 0; i <= 2; i++)
	{
		Temp = -Room1Amount[i] + 5;
		if (Temp > 0)
		{
			for (Y = (MapHeight / ZONE_AMOUNT) * (2 - i) + 1; Y <= ((MapHeight / ZONE_AMOUNT) * ((2 - i) + 1.0)) - 2; Y++)
			{
				for (X = 2; X <= MapWidth - 2; X++)
				{
					if (GetGridType(X, Y) == 0)
					{
						if ((FMath::Min(GetGridType(X + 1, Y), 1) + FMath::Min(GetGridType(X - 1, Y), 1) + FMath::Min(GetGridType(X, Y + 1), 1) +
							FMath::Min(GetGridType(X, Y - 1), 1)) == 1)
						{
							if (GetGridType(X + 1, Y))
							{
								X2 = X + 1;
								Y2 = Y;
							}
							else if (GetGridType(X - 1, Y))
							{
								X2 = X - 1;
								Y2 = Y;
							}
							else if (GetGridType(X, Y + 1))
							{
								X2 = X;
								Y2 = Y + 1;
							}
							else if (GetGridType(X, Y - 1))
							{
								X2 = X;
								Y2 = Y - 1;
							}

							bool bPlaced = false;

							if (GetGridType(X2, Y2) > 1 && GetGridType(X2, Y2) < 4)
							{
								switch (GetGridType(X2, Y2))
								{
								case 2:
								{
									if (FMath::Min(GetGridType(X2 + 1, Y2), 1) + FMath::Min(GetGridType(X2 - 1, Y2), 1) == 2)
									{
										Room2Amount[i] = Room2Amount[i] - 1;
										Room3Amount[i] = Room3Amount[i] + 1;
										bPlaced = true;
									}
									else if (FMath::Min(GetGridType(X2, Y2 + 1), 1) + FMath::Min(GetGridType(X2, Y2 - 1), 1) == 2)
									{
										Room2Amount[i] = Room2Amount[i] - 1;
										Room3Amount[i] = Room3Amount[i] + 1;
										bPlaced = true;
									}
									break;
								}
								case 3:
								{
									Room3Amount[i] = Room3Amount[i] - 1;
									Room4Amount[i] = Room4Amount[i] + 1;
									bPlaced = true;
								}
								}

								if (bPlaced)
								{
									int GridType = GetGridType(X2, Y2);
									SetGridType(X2, Y2, GridType + 1);
									SetRoomType(X2, Y2, (RoomType)(GridType + 2));

									SetGridType(X, Y, 1);
									SetRoomType(X, Y, RoomType::Room1);
									Room1Amount[i] = Room1Amount[i] + 1;

									Temp = Temp - 1;
								}
							}
						}
					}

					if (Temp == 0)
						break;
				}
				if (Temp == 0)
					break;
			}
		}
	}

	// Force more Room4s and Room2Cs
	for (int i = 0; i <= 2; i++)
	{
		int Temp2 = 0;
		switch (i)
		{
		case 2:
			Zone = 2;
			Temp2 = MapHeight / 3;
			break;
		case 1:
			Zone = MapHeight / 3 + 1;
			Temp2 = MapHeight * (2.0 / 3.0) - 1;
			break;
		case 0:
			Zone = MapHeight * (2.0 / 3.0) + 1;
			Temp2 = MapHeight - 2;
			break;
		}

		// We want atleast 1 ROOM4
		if (Room4Amount[i] < 1)
		{
			LogDebug("Forcing a ROOM4 into zone %d", i);
			Temp = 0;

			for (Y = Zone; Y <= Temp2; Y++)
			{
				for (X = 2; X <= MapWidth - 2; X++)
				{
					if (GetGridType(X, Y) == 3)
					{
						if (!(GetGridType(X + 1, Y) || GetGridType(X + 1, Y + 1) || GetGridType(X + 1, Y - 1) || GetGridType(X + 2, Y)))
						{
							SetGridType(X + 1, Y, 1);
							SetRoomType(X+1, Y, RoomType::Room1);
							Temp = 1;
						}
						else if (!(GetGridType(X - 1, Y) || GetGridType(X - 1, Y + 1) || GetGridType(X - 1, Y - 1) || GetGridType(X - 2, Y)))
						{
							SetGridType(X - 1, Y, 1);
							SetRoomType(X - 1, Y, RoomType::Room1);
							Temp = 1;
						}
						else if (!(GetGridType(X, Y + 1) || GetGridType(X + 1, Y + 1) || GetGridType(X - 1, Y + 1) || GetGridType(X, Y + 2)))
						{
							SetGridType(X, Y + 1, 1);
							SetRoomType(X, Y + 1, RoomType::Room1);
							Temp = 1;
						}
						else if (!(GetGridType(X, Y - 1) || GetGridType(X + 1, Y - 1) || GetGridType(X - 1, Y - 1) || GetGridType(X, Y - 2)))
						{
							SetGridType(X, Y - 1, 1);
							SetRoomType(X, Y - 1, RoomType::Room1);
							Temp = 1;
						}

						if (Temp == 1)
						{
							SetGridType(X, Y, 4); // Turn this into a Room4
							SetRoomType(X, Y, RoomType::Room4);
							LogDebug("\tROOM4 forced into slot (%d, %d)", X, Y);
							Room4Amount[i] = Room4Amount[i] + 1;
							Room3Amount[i] = Room3Amount[i] - 1;
							Room1Amount[i] = Room1Amount[i] + 1;
						}
					}

					if (Temp == 1)
						break;
				}
				if (Temp == 1)
					break;
			}

			if (Temp == 0)
			{
				LogDebug("Couldn't place ROOM4 in Zone %d", i);
			}
		}

		if (Room2CAmount[i] < 1)
		{
			LogDebug("Forcing a ROOM2C into %d", Zone);

			Temp = 0;

			Zone = Zone + 1;
			Temp2 = Temp2 - 1;

			for (Y = Zone; Y <= Temp2; Y++)
			{
				for (X = 3; X <= MapWidth - 3; X++)
				{
					if (GetGridType(X, Y) == 1)
					{
						if (GetGridType(X - 1, Y) > 0)
						{
							if ((GetGridType(X, Y - 1) + GetGridType(X, Y + 1) + GetGridType(X + 2, Y)) == 0)
							{
								if ((GetGridType(X, Y - 1) + GetGridType(X + 2, Y - 1) + GetGridType(X + 1, Y - 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetRoomType(X, Y, RoomType::Room2C);
									SetGridType(X + 1, Y, 2);
									SetRoomType(X + 1, Y, RoomType::Room2C);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X + 1, Y);
									SetGridType(X + 1, Y - 1, 1);
									SetRoomType(X + 1, Y - 1, RoomType::Room1);
									Temp = 1;
								}
								else if ((GetGridType(X + 1, Y + 2) + GetGridType(X + 2, Y + 1) + GetGridType(X + 1, Y + 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetRoomType(X, Y, RoomType::Room2C);
									SetGridType(X + 1, Y, 2);
									SetRoomType(X + 1, Y, RoomType::Room2C);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X + 1, Y);
									SetGridType(X + 1, Y + 1, 1);
									SetRoomType(X + 1, Y + 1, RoomType::Room1);
									Temp = 1;
								}
							}
						}
						else if (GetGridType(X + 1, Y) > 0)
						{
							if ((GetGridType(X, Y - 1) + GetGridType(X, Y + 1) + GetGridType(X - 2, Y)) == 0)
							{
								if ((GetGridType(X - 1, Y - 2) + GetGridType(X - 2, Y - 1) + GetGridType(X - 1, Y - 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetGridType(X - 1, Y, 2);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X - 1, Y);
									SetGridType(X - 1, Y - 1, 1);
									Temp = 1;
								}
								else if ((GetGridType(X - 1, Y + 2) + GetGridType(X - 2, Y + 1) + GetGridType(X - 1, Y + 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetRoomType(X, Y, RoomType::Room2C);
									SetGridType(X - 1, Y, 2);
									SetRoomType(X - 1, Y, RoomType::Room2C);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X - 1, Y);
									SetGridType(X - 1, Y + 1, 1);
									SetRoomType(X + 1, Y + 1, RoomType::Room1);
									Temp = 1;
								}
							}
						}
						else if (GetGridType(X, Y - 1) > 0)
						{
							if ((GetGridType(X - 1, Y) + GetGridType(X + 1, Y) + GetGridType(X, Y + 2)) == 0)
							{
								if ((GetGridType(X - 2, Y + 1) + GetGridType(X - 1, Y + 2) + GetGridType(X - 1, Y + 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetRoomType(X, Y, RoomType::Room2C);
									SetGridType(X, Y + 1, 2);
									SetRoomType(X, Y + 1, RoomType::Room2C);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X, Y + 1);
									SetGridType(X - 1, Y + 1, 1);
									SetRoomType(X - 1, Y + 1, RoomType::Room1);
									Temp = 1;
								}
								else if ((GetGridType(X + 2, Y + 1) + GetGridType(X + 1, Y + 2) + GetGridType(X + 1, Y + 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetRoomType(X, Y, RoomType::Room2C);
									SetGridType(X, Y + 1, 2);
									SetRoomType(X, Y + 1, RoomType::Room2C);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X, Y + 1);
									SetGridType(X + 1, Y + 1, 1);
									SetRoomType(X + 1, Y + 1, RoomType::Room1);
									Temp = 1;
								}
							}
						}
						else if (GetGridType(X, Y + 1) > 0)
						{
							if ((GetGridType(X - 1, Y) + GetGridType(X + 1, Y) + GetGridType(X, Y - 2)) == 0)
							{
								if ((GetGridType(X - 2, Y - 1) + GetGridType(X - 1, Y - 2) + GetGridType(X - 1, Y - 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetRoomType(X, Y, RoomType::Room2C);
									SetGridType(X, Y - 1, 2);
									SetRoomType(X, Y - 1, RoomType::Room2C);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X, Y - 1);
									SetGridType(X - 1, Y - 1, 1);
									SetRoomType(X - 1, Y - 1, RoomType::Room1);
									Temp = 1;
								}
								else if ((GetGridType(X + 2, Y - 1) + GetGridType(X + 1, Y - 2) + GetGridType(X + 1, Y - 1)) == 0)
								{
									SetGridType(X, Y, 2);
									SetRoomType(X, Y, RoomType::Room2C);
									SetGridType(X, Y - 1, 2);
									SetRoomType(X, Y - 1, RoomType::Room2C);
									LogDebug("\tROOM2C Forced into slot (%d), (%d)", X, Y - 1);
									SetGridType(X + 1, Y - 1, 1);
									SetRoomType(X + 1, Y - 1, RoomType::Room1);
									Temp = 1;
								}
							}
						}

						if (Temp == 1)
						{
							Room2CAmount[i] = Room2CAmount[i] + 1;
							Room2Amount[i] = Room2Amount[i] + 1;
						}
					}
					if (Temp == 1)
					{
						break;
					}
				}
				if (Temp == 1)
				{
					break;
				}
			}
			if (Temp == 0)
			{
				LogDebug("Couldn't place ROOM2C into zone %d", Zone);
			}
		}
	}

	// Specify some hardcoded rooms
	int MaxRooms = 55 * MapWidth / 20;
	MaxRooms = FMath::Max(MaxRooms, Room1Amount[0] + Room1Amount[1] + Room1Amount[2] + 1);
	MaxRooms = FMath::Max(MaxRooms, Room2Amount[0] + Room2Amount[1] + Room2Amount[2] + 1);
	MaxRooms = FMath::Max(MaxRooms, Room2CAmount[0] + Room2CAmount[1] + Room2CAmount[2] + 1);
	MaxRooms = FMath::Max(MaxRooms, Room3Amount[0] + Room3Amount[1] + Room3Amount[2] + 1);
	MaxRooms = FMath::Max(MaxRooms, Room4Amount[0] + Room4Amount[1] + Room4Amount[2] + 1);

	/** @todo ROOM4 + 1, use the enum instead */
	PredefinedRooms = std::vector<std::vector<std::string>>(5 + 1, std::vector<std::string>(MaxRooms));

	/** LIGHT CONTAINMENT ZONE */

	int MinPos = 1;
	int MaxPos = Room1Amount[0] - 1;

	/** @UE_PORT_TODO Fix room names */
	PredefinedRooms[RoomType::Room1][0] = "start";
	SetRoom("roompj", RoomType::Room1, FMath::Floor(0.1 * float(Room1Amount[0])), MinPos, MaxPos);
	SetRoom("914", RoomType::Room1, FMath::Floor(0.3 * float(Room1Amount[0])), MinPos, MaxPos);
	SetRoom("room1archive", RoomType::Room1, FMath::Floor(0.5 * float(Room1Amount[0])), MinPos, MaxPos);
	SetRoom("room205", RoomType::Room1, FMath::Floor(0.6 * float(Room1Amount[0])), MinPos, MaxPos);

	PredefinedRooms[RoomType::Room2C][0] = "lockroom";

	MinPos = 1;
	MaxPos = Room2Amount[0] - 1;

	PredefinedRooms[RoomType::Room2][0] = "room2closets";
	SetRoom("room2testroom2", RoomType::Room2, FMath::Floor(0.1 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room2scps", RoomType::Room2, FMath::Floor(0.2 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room2storage", RoomType::Room2, FMath::Floor(0.3 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room2gw_b", RoomType::Room2, FMath::Floor(0.4 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room2sl", RoomType::Room2, FMath::Floor(0.5 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room012", RoomType::Room2, FMath::Floor(0.55 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room2scps2", RoomType::Room2, FMath::Floor(0.6 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room1123", RoomType::Room2, FMath::Floor(0.7 * (float)Room2Amount[0]), MinPos, MaxPos);
	SetRoom("room2elevator", RoomType::Room2, FMath::Floor(0.85 * (float)Room2Amount[0]), MinPos, MaxPos);

	/** HEAVY CONTAINMENT ZONE */

	MinPos = Room1Amount[0];
	MaxPos = Room1Amount[0] + Room1Amount[1] - 1;

	SetRoom("room079", RoomType::Room1, Room1Amount[0] + FMath::Floor(0.15 * (float)Room1Amount[1]), MinPos, MaxPos);
	SetRoom("room106", RoomType::Room1, Room1Amount[0] + FMath::Floor(0.3 * (float)Room1Amount[1]), MinPos, MaxPos);
	SetRoom("008", RoomType::Room1, Room1Amount[0] + FMath::Floor(0.4 * (float)Room1Amount[1]), MinPos, MaxPos);
	SetRoom("room035", RoomType::Room1, Room1Amount[0] + FMath::Floor(0.5 * (float)Room1Amount[1]), MinPos, MaxPos);
	SetRoom("coffin", RoomType::Room1, Room1Amount[0] + FMath::Floor(0.7 * (float)Room1Amount[1]), MinPos, MaxPos);

	MinPos = Room2Amount[0];
	MaxPos = Room2Amount[0] + Room2Amount[1] - 1;

	PredefinedRooms[RoomType::Room2][Room2Amount[0] + FMath::Floor(0.1 * (float)Room2Amount[1])] = "room2nuke";
	SetRoom("room2tunnel", RoomType::Room2, Room2Amount[0] + FMath::Floor(0.25 * (float)Room2Amount[1]), MinPos, MaxPos);
	SetRoom("room049", RoomType::Room2, Room2Amount[0] + FMath::Floor(0.4 * (float)Room2Amount[1]), MinPos, MaxPos);
	SetRoom("room2shaft", RoomType::Room2, Room2Amount[0] + FMath::Floor(0.6 * (float)Room2Amount[1]), MinPos, MaxPos);
	SetRoom("testroom", RoomType::Room2, Room2Amount[0] + FMath::Floor(0.7 * (float)Room2Amount[1]), MinPos, MaxPos);
	SetRoom("room2servers", RoomType::Room2, Room2Amount[0] + FMath::Floor(0.9 * Room2Amount[1]), MinPos, MaxPos);

	PredefinedRooms[RoomType::Room3][Room3Amount[0] + FMath::Floor(0.3 * (float)Room3Amount[1])] = "room513";
	PredefinedRooms[RoomType::Room3][Room3Amount[0] + FMath::Floor(0.6 * (float)Room3Amount[1])] = "room966";

	PredefinedRooms[RoomType::Room2C][Room2CAmount[0] + FMath::Floor(0.5 * (float)Room2CAmount[1])] = "room2cpit";

	/** ENTRANCE ZONE */

	PredefinedRooms[RoomType::Room1][Room1Amount[0] + Room1Amount[1] + Room1Amount[2] - 2] = "exit1";
	PredefinedRooms[RoomType::Room1][Room1Amount[0] + Room1Amount[1] + Room1Amount[2] - 1] = "gateaentrance";
	PredefinedRooms[RoomType::Room1][Room1Amount[0] + Room1Amount[1]] = "room1lifts";

	MinPos = Room2Amount[0] + Room2Amount[1];
	MaxPos = Room2Amount[0] + Room2Amount[1] + Room2Amount[2] - 1;

	PredefinedRooms[RoomType::Room2][MinPos + FMath::Floor(0.1 * (float)Room2Amount[2])] = "room2poffices";
	SetRoom("room2cafeteria", RoomType::Room2, MinPos + FMath::Floor(0.2 * (float)Room2Amount[2]), MinPos, MaxPos);
	SetRoom("room2sroom", RoomType::Room2, MinPos + FMath::Floor(0.3 * (float)Room2Amount[2]), MinPos, MaxPos);
	SetRoom("room2servers2", RoomType::Room2, MinPos + FMath::Floor(0.4 * Room2Amount[2]), MinPos, MaxPos);
	SetRoom("room2offices", RoomType::Room2, MinPos + FMath::Floor(0.45 * Room2Amount[2]), MinPos, MaxPos);
	SetRoom("room2offices4", RoomType::Room2, MinPos + FMath::Floor(0.5 * Room2Amount[2]), MinPos, MaxPos);
	SetRoom("room860", RoomType::Room2, MinPos + FMath::Floor(0.6 * Room2Amount[2]), MinPos, MaxPos);
	SetRoom("medibay", RoomType::Room2, MinPos + FMath::Floor(0.7 * (float)Room2Amount[2]), MinPos, MaxPos);
	SetRoom("room2poffices2", RoomType::Room2, MinPos + FMath::Floor(0.8 * Room2Amount[2]), MinPos, MaxPos);
	SetRoom("room2offices2", RoomType::Room2, MinPos + FMath::Floor(0.9 * (float)Room2Amount[2]), MinPos, MaxPos);

	PredefinedRooms[RoomType::Room2C][Room2CAmount[0] + Room2CAmount[1]] = "room2ccont";
	PredefinedRooms[RoomType::Room2C][Room2CAmount[0] + Room2CAmount[1] + 1] = "lockroom2";

	PredefinedRooms[RoomType::Room3][Room3Amount[0] + Room3Amount[1] + FMath::Floor(0.3 * (float)Room3Amount[2])] = "room3servers";
	PredefinedRooms[RoomType::Room3][Room3Amount[0] + Room3Amount[1] + FMath::Floor(0.7 * (float)Room3Amount[2])] = "room3servers2";
	//PredefinedRooms[RoomType::Room3][Room3Amount[0] + Room3Amount[1]] = "room3gw";
	PredefinedRooms[RoomType::Room3][Room3Amount[0] + Room3Amount[1] + FMath::Floor(0.5 * (float)Room3Amount[2])] = "room3offices";

	for (int Y = MapHeight - 1; Y >= 1; Y--)
	{
		ERoomZone Zone = ERoomZone::LCZ;
		if (Y < MapHeight / 3 + 1)
		{
			Zone = ERoomZone::EZ;
		}
		else if (Y < MapHeight * (2.0 / 3.0))
		{
			Zone = ERoomZone::HCZ;
		}

		for (int X = 1; X <= MapWidth - 2; X++)
		{
			int GridType = GetGridType(X, Y);
			RoomType RoomType = GetRoomType(X, Y);

			if (GridType == 0)
			{
				continue;
			}

			// Spawn a checkpoint
			if (GridType == CHECKPOINT)
			{
				if (Zone == ERoomZone::LCZ)
				{
					AssignRoomToCoordinate(Zone, RoomType, X, Y, "checkpoint1");
				}
				else if (Zone == ERoomZone::EZ)
				{
					AssignRoomToCoordinate(Zone, RoomType, X, Y, "checkpoint2");
				}

				// We forcefully made a room for this coordinate, continue to the next coordinate
				continue;
			}

			Temp = FMath::Min(GetGridType(X + 1, Y), 1) + FMath::Min(GetGridType(X - 1, Y), 1) + FMath::Min(GetGridType(X, Y + 1), 1) +
				FMath::Min(GetGridType(X, Y - 1), 1);

			switch (Temp)
			{
			case 1:
				AssignRoomToCoordinate(Zone, RoomType, X, Y);
				break;
			case 2:
				AssignRoomToCoordinate(Zone, RoomType, X, Y);
				break;
			case 3:
				AssignRoomToCoordinate(Zone, RoomType, X, Y);
				break;
			case 4:
				AssignRoomToCoordinate(Zone, RoomType, X, Y);
				break;
			}
		}
	}

	// Assign some rooms at some specific coordinates
	/** @todo some rooms need to be below the map. These rooms are not really on the grid, but I gave them grid coords by dividing their X Y by 8 so some rooms may intersect */
	AssignRoomToCoordinate(ERoomZone::None, RoomType::Room1, (MapWidth - 1), 1, "gatea");
	AssignRoomToCoordinate(ERoomZone::None, RoomType::Room1, (MapWidth - 1), (MapHeight - 1), "pocketdimension");

	/** @todo add intro check and re-enable this. Will also require updating the test data to include this */
	//AssignRoomToCoordinate(ERoomZone::None, RoomType::Room1, 1, (MapHeight - 1), "173");

	AssignRoomToCoordinate(ERoomZone::None, RoomType::Room1, 1, 0, "dimension1499");

	if (DebugPrint)
	{
		OutputMap();
		//__debugbreak();
	}
}


RoomArrayEntry& Generator::GetDataAtCoordinate(int X, int Y)
{
	return MapArray[X][Y];
}

int Generator::GetMapZone(int Y)
{
	float Val1 = (MapWidth - Y);
	int Val2 = floor(Val1 / MapWidth * ZONE_AMOUNT);
	float FinalVal = FMath::Min(Val2, ZONE_AMOUNT - 1);

	return FinalVal;
}

void Generator::OutputMap()
{
	for (auto& row : MapArray)
	{
		for (auto& cell : row)
		{
			std::printf(" %3d", cell.GridType);
		}
		std::printf("\n");
	}
}

void Generator::SetGridType(int X, int Y, int Value /*= 0*/)
{
	if (X <= MapArray.size() && Y <= MapArray[X].size())
	{
		MapArray[X][Y].GridType = Value;
	}
}

int Generator::GetGridType(int X, int Y)
{
	if (X >= MapArray.size())
	{
		return 0;
	}
	else if (Y >= MapArray[X].size())
	{
		return 0;
	}

	return MapArray[X][Y].GridType;
}

void Generator::SetRoomType(int X, int Y, RoomType Value)
{
	if (X <= MapArray.size() && Y <= MapArray[X].size())
	{
		MapArray[X][Y].RoomType = Value;
	}
}

RoomType Generator::GetRoomType(int X, int Y)
{
	if (X >= MapArray.size())
	{
		return RoomType::Room1;
	}
	else if (Y >= MapArray[X].size())
	{
		return RoomType::Room1;
	}

	return MapArray[X][Y].RoomType;
}


void Generator::SetZone(int X, int Y, int Value)
{
	if (X <= MapArray.size() && Y <= MapArray[X].size())
	{
		MapArray[X][Y].RoomZone = Value;
	}
}

int Generator::GetZone(int X, int Y)
{
	if (X >= MapArray.size())
	{
		return ERoomZone::LCZ;
	}
	else if (Y >= MapArray[X].size())
	{
		return ERoomZone::LCZ;
	}

	return MapArray[X][Y].RoomZone;
}

bool Generator::AssignRoomToCoordinate(ERoomZone RoomZone, RoomType RoomType, int X, int Y, const std::string& Name)
{
	RoomArrayEntry& Data = GetDataAtCoordinate(X, Y);
	Data.RoomRotation = GetDesiredRoomAngle(RoomType, X, Y);
	Data.RoomType = RoomType;
	Data.RoomZone = RoomZone;
	
	if (Name != "")
	{
		Data.RoomName = Name;
	}


	return false;
}

float Generator::GetDesiredRoomAngle(RoomType RoomType, int X, int Y)
{
	// Get the angle for the current room
	float Angle = 0.f;

	// FourWay is purposely missing as we don't need to rotate them
	switch (RoomType)
	{
		// @todo 90 and 270 are flipped here, why?
	case RoomType::Room1:
	{
		if (GetGridType(X, Y + 1) > 0)
		{
			Angle = 180.f;
		}
		else if (GetGridType(X - 1, Y) > 0)
		{
			Angle = 90.f;
		}
		else if (GetGridType(X + 1, Y) > 0)
		{
			Angle = 270.f;
		}
		else
		{
			Angle = 0.;
		}
		break;
	}
	case RoomType::Room2:
	{
		if (GetGridType(X - 1, Y) > 0 && GetGridType(X + 1, Y) > 0)
		{
			if (BlitzRand(2) == 1)
			{
				Angle = 270.f;
			}
			else
			{
				Angle = 90.f;
			}
		}
		else if (GetGridType(X, Y - 1) > 0 && GetGridType(X, Y + 1) > 0)
		{
			if (BlitzRand(2) == 1)
			{
				Angle = 180.f;
			}
			else
			{
				Angle = 0.f;
			}
		}
		break;
	}
	case RoomType::Room2C:
	{
		if (GetGridType(X - 1, Y) > 0 && GetGridType(X, Y + 1) > 0)
		{
			Angle = 180.f;
		}
		else if (GetGridType(X + 1, Y) > 0 && GetGridType(X, Y + 1) > 0)
		{
			Angle = 270.f;
		}
		else if (GetGridType(X - 1, Y) > 0 && GetGridType(X, Y - 1) > 0)
		{
			Angle = 90.f;
		}
		break;
	}
	case RoomType::Room3:
	{
		if (!GetGridType(X, Y - 1))
		{
			Angle = 180.f;
		}
		else if (!GetGridType(X - 1, Y))
		{
			Angle = 270.f;
		}
		else if (!GetGridType(X + 1, Y))
		{
			Angle = 90.f;
		}

		break;
	}
	}

	return Angle;
}

bool Generator::SetRoom(std::string RoomName, RoomType RoomType, int Pos, int MinPos, int MaxPos)
{
	if (MaxPos < MinPos)
	{
		LogDebug("Can't place %s", RoomName.c_str());
		return false;
	}

	bool bCanPlace = true;
	bool bLooped = false;
	while (PredefinedRooms[RoomType][Pos] != "")
	{
		LogDebug("Found %s", PredefinedRooms[RoomType][Pos].c_str());

		Pos++;
		if (Pos > MaxPos)
		{
			if (!bLooped)
			{
				Pos = MinPos + 1;
				bLooped = false;
			}
			else
			{
				bCanPlace = false;
				break;
			}
		}
	}

	if (bCanPlace)
	{
		LogDebug("Adding %s to predefined rooms at %d", RoomName.c_str(), Pos);
		PredefinedRooms[RoomType][Pos] = RoomName;
		return true;
	}
	else
	{
		LogWarning("Couldn't place %s", RoomName.c_str());
		return false;
	}
}
