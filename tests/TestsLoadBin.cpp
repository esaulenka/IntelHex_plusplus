#include <sstream>
#include "../intelhex.h"
#include "catch.hpp"

using namespace std;


TEST_CASE("TestLoadBin")
{
	// setup
	string bytes("0123456789");
	CHECK(bytes.size() == 10);
	istringstream f(bytes);

	SECTION("test_loadbin")
	{
		IntelHex ih;
		ih.loadbin(f);

		REQUIRE(0 == ih.minaddr());
		REQUIRE(9 == ih.maxaddr());
		auto arr = ih.tobinarray();
		string str((char*)arr.data(), arr.size());
		REQUIRE(bytes == str);
	}

	SECTION("test_loadbin_w_offset")
	{
		IntelHex ih;
		ih.loadbin(f, 100);

		REQUIRE(100 == ih.minaddr());
		REQUIRE(109 == ih.maxaddr());
		auto arr = ih.tobinarray();
		string str((char*)arr.data(), arr.size());
		REQUIRE(bytes == str);
	}

}
