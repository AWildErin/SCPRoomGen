#include <gtest/gtest.h>

#include "generator.h"

#define MANUAL_TEST 0

int main(int argc, char** argv)
{
#if MANUAL_TEST
	Generator Gen(true);
	Gen.GenerateMap("d9341");
#else
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif
}
