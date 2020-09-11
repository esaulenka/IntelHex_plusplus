#include <sstream>
#include "../intelhex.h"
#include "catch.hpp"

using namespace std;

TEST_CASE("test_merge_empty")
{
	IntelHex ih1, ih2;
	ih1.merge(ih2);
	REQUIRE(ih1.tobinarray().empty());
}


TEST_CASE("test_merge_simple")
{
	IntelHex ih1{ {0,1}, {1,2}, {2,3}, };
	IntelHex ih2{ {3,4}, {4,5}, {5,6}, };
	ih1.merge(ih2);
	REQUIRE(ih1.size() == 6);
	REQUIRE((ih1[0] == 1 &&
			ih1[1] == 2 &&
			ih1[2] == 3 &&
			ih1[3] == 4 &&
			ih1[4] == 5 &&
			ih1[5] == 6));
}

TEST_CASE("test_merge_wrong_args")
{
	IntelHex ih1;

	// can not merge itself
	REQUIRE_THROWS(ih1.merge(ih1));
}

TEST_CASE("test_merge_overlap")
{
	IntelHex ih1{ {0,1}, };
	IntelHex ih2{ {0,2}, };
	SECTION("overlap error") {
		REQUIRE_THROWS(ih1.merge(ih2, IntelHex::Overlap::error));
	}

	SECTION("overlap ignore") {
		ih1.merge(ih2, IntelHex::Overlap::ignore);
		REQUIRE(ih1.size() == 1);
		REQUIRE(ih1[0] == 1);
	}

	SECTION("overlap replace") {
		ih1.merge(ih2, IntelHex::Overlap::replace);
		REQUIRE(ih1.size() == 1);
		REQUIRE(ih1[0] == 2);
	}
}

TEST_CASE("test_merge_start_addr")
{
	IntelHex::StartAddrExtended start_addr{ 0x12345678 };

	SECTION("this, none") {
		IntelHex ih1;
		ih1.start_addr = start_addr;
		IntelHex ih2;
		ih1.merge(ih2);
		REQUIRE(ih1.start_addr == start_addr);
	}

	SECTION("None, other") {
		IntelHex ih1;
		IntelHex ih2;
		ih2.start_addr = start_addr;
		ih1.merge(ih2);
		REQUIRE(ih1.start_addr == start_addr);
	}

	SECTION("this == other: no conflict") {
		IntelHex ih1;
		IntelHex ih2;
		ih1.start_addr = start_addr;
		ih2.start_addr = start_addr;
		ih1.merge(ih2);
		REQUIRE(ih1.start_addr == start_addr);
	}

	IntelHex::StartAddrExtended start_addr2{ 0x87654321 };

	SECTION("this != other: conflict; overlap=error") {
		IntelHex ih1;
		IntelHex ih2;
		ih1.start_addr = start_addr;
		ih2.start_addr = start_addr2;

		REQUIRE_THROWS(ih1.merge(ih2, IntelHex::Overlap::error));
	}

	SECTION("this != other: conflict; overlap=ignore") {
		IntelHex ih1;
		IntelHex ih2;
		ih1.start_addr = start_addr;
		ih2.start_addr = start_addr2;

		ih1.merge(ih2, IntelHex::Overlap::ignore);
		REQUIRE(ih1.start_addr == start_addr);
	}

	SECTION("this != other: conflict; overlap=replace") {
		IntelHex ih1;
		IntelHex ih2;
		ih1.start_addr = start_addr;
		ih2.start_addr = start_addr2;

		ih1.merge(ih2, IntelHex::Overlap::replace);
		REQUIRE(ih1.start_addr == start_addr2);
	}
}

