#include <gtest/gtest.h>
#include <glaze/glaze.hpp>

#include "config.h"
#include "testdata.h"
#include "generator.h"

// ADD BACK MyMap, DONTBLINK, d9341, JORGE, dirtymetal

TEST(MapGeneration, MyMap)
{
    std::vector<std::vector<TestDataStruct>> Data;
    auto ec = glz::read_file_json(Data, RESOURCES_ROOT_PATH "/mapdump_MyMap.json", std::string{});

    Generator Gen(false);
    Gen.GenerateMap("MyMap");

    for (int X = 0; X <= MapWidth; X++)
    {
        for (int Y = 0; Y <= MapHeight; Y++)
        {
            TestDataStruct& TestData = Data[X][Y];
            RoomArrayEntry& RoomEntry = Gen.GetDataAtCoordinate(X, Y);

            EXPECT_EQ(RoomEntry.RoomName, TestData.RoomName);
            EXPECT_EQ(RoomEntry.PosX, TestData.PosX) << "Invalid room type on " << RoomEntry.PosX << ", " << RoomEntry.PosY << ". Test Data Room: " << TestData.RoomName;
            EXPECT_EQ(RoomEntry.PosY, TestData.PosY) << "Invalid room type on " << RoomEntry.PosX << ", " << RoomEntry.PosY << ". Test Data Room: " << TestData.RoomName;
            EXPECT_EQ(RoomEntry.GridType, TestData.GridType) << "Invalid room type on " << RoomEntry.PosX << ", " << RoomEntry.PosY << ". Test Data Room: " << TestData.RoomName;
            EXPECT_EQ(RoomEntry.RoomType, TestData.RoomType) << "Invalid room type on " << RoomEntry.PosX << ", " << RoomEntry.PosY << ". Test Data Room: " << TestData.RoomName;
            EXPECT_EQ(RoomEntry.RoomZone, TestData.RoomZone) << "Invalid room type on " << RoomEntry.PosX << ", " << RoomEntry.PosY << ". Test Data Room: " << TestData.RoomName;
        }
    }
}