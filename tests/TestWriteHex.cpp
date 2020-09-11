#include <sstream>
#include "../intelhex.h"
#include "catch.hpp"
#include "TestData.h"

using namespace std;


TEST_CASE("TestWriteHexFileByteCount")
{
	istringstream f(hex8);
	IntelHex ih(f);
	stringstream sio;

	SECTION("test_write_hex_file_bad_byte_count")
	{
		REQUIRE_THROWS(ih.write_hex_file(sio, true, 0));
		REQUIRE_THROWS(ih.write_hex_file(sio, true, 256));
	}

	SECTION("test_write_hex_file_byte_count_1")
	{
		IntelHex ih1;
		for (size_t i = 0; i < 4; i++)
			ih1.add(i, ih[i]);
		ih1.write_hex_file(sio, true, 1);
		auto s = sio.str();

		// check that we have all data records with data length == 1
		string expected =
		":0100000002FD\n"
		":0100010005F9\n"
		":01000200A25B\n"
		":01000300E517\n"
		":00000001FF\n";
		REQUIRE(s == expected);

		// read back and check content
		istringstream fin(s);
		IntelHex ih2(fin);
		REQUIRE(ih1.tobinarray() == ih2.tobinarray());
	}

	SECTION("test_write_hex_file_byte_count_13")
	{
		ih.write_hex_file(sio, true, 13);
		auto s = sio.str();

		// control written hex first line to check that byte count is 13
		REQUIRE(s.rfind(":0D0000000205A2E576246AF8E6057622786E\n", 0) == 0);

		// read back and check content
		istringstream fin(s);
		IntelHex ih2(fin);
		REQUIRE(ih.tobinarray() == ih2.tobinarray());
	}

	SECTION("test_write_hex_file_byte_count_255")
	{
		ih.write_hex_file(sio, true, 255);
		auto s = sio.str();

		// control written hex first line to check that byte count is 255
		string expected =
		 ":FF0000000205A2E576246AF8E60576227867300702786AE475F0011204AD02"
		  "04552000EB7F2ED2008018EF540F2490D43440D4FF30040BEF24BFB41A0050"
		  "032461FFE57760021577057AE57A7002057930070D7867E475F0011204ADEF"
		  "02049B02057B7403D2078003E4C207F5768B678A688969E4F577F579F57AE5"
		  "7760077F2012003E80F57578FFC201C200C202C203C205C206C20812000CFF"
		  "700D3007057F0012004FAF7AAE7922B4255FC2D5C20412000CFF24D0B40A00"
		  "501A75F00A787730D50508B6FF0106C6A426F620D5047002D20380D924CFB4"
		  "1A00EF5004C2E5D20402024FD20180C6D20080C0D20280BCD2D580BAD20580"
		  "B47F2012003E20020774010E\n";
		REQUIRE(s.rfind(expected, 0) == 0);

		istringstream fin(s);
		IntelHex ih2(fin);
		REQUIRE(ih.tobinarray() == ih2.tobinarray());
	}

}
