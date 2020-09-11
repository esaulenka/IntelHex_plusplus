#include <sstream>
#include "../intelhex.h"
#include "catch.hpp"

using namespace std;

using StartAddr = IntelHex::StartAddr;
using HexData = std::map<IntelHex::Addr, uint8_t>;

static const string hex_rectype3 = R"(:0400000312345678E5
:0100000001FE
:00000001FF
)";
static const HexData data_rectype3 { {0, 1} };
static const StartAddr start_addr_rectype3 { IntelHex::StartAddrSegmented{0x1234, 0x5678} };


static const string hex_rectype5 = R"(:0400000512345678E3
:0100000002FD
:00000001FF
)";
static const HexData data_rectype5 { {0, 2} };
static const StartAddr start_addr_rectype5 { IntelHex::StartAddrExtended{0x12345678} };


TEST_CASE("TestIntelHexStartingAddressRecords")
{
	SECTION("test_read")
	{
		auto test_read = [](const string & hexstr, const HexData & data, const StartAddr & start_addr)
		{
			istringstream sio(hexstr);
			IntelHex ih(sio);

			for (auto d : data)
				REQUIRE(ih[d.first] == d.second);

			REQUIRE(start_addr == ih.start_addr);
		};

		test_read(hex_rectype3, data_rectype3, start_addr_rectype3);
		test_read(hex_rectype5, data_rectype5, start_addr_rectype5);

	}

	SECTION("test_write")
	{
		auto test_write = [](const string &hexstr, const HexData &data, const StartAddr &start_addr, bool write_start_addr=true)
		{
			// prepare
			IntelHex ih;
			for (auto & d : data)
				ih.add(d.first, d.second);
			ih.start_addr = start_addr;

			// write
			ostringstream sio;
			ih.write_hex_file(sio, write_start_addr);
			auto s = sio.str();

			REQUIRE(hexstr == s);
		};

		auto test_dont_write = [&](const string &hexstr, const HexData &data, const StartAddr &start_addr)
		{
			// skip first line with address record
			auto rest = hexstr.find("\n") + 1;
			auto expected = hexstr.substr(rest);
			test_write(expected, data, start_addr, false);
		};

		test_write(hex_rectype3, data_rectype3, start_addr_rectype3);

		test_dont_write(hex_rectype3, data_rectype3, start_addr_rectype3);

		test_write(hex_rectype5, data_rectype5, start_addr_rectype5);

		test_dont_write(hex_rectype5, data_rectype5, start_addr_rectype5);

	}

}

