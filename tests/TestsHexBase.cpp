#include <sstream>
#include "../intelhex.h"
#include "catch.hpp"
#include "TestData.h"

using namespace std;




static const string hex_simple =
R"(:10000000000083120313072055301820042883169C
:10001000031340309900181598168312031318160D
:1000200098170800831203138C1E14281A0808005E
:0C003000831203130C1E1A28990008000C
:00000001FF
)";

static const string hex_bug_lp_341051 =
R"(:020FEC00E4E738
:040FF00022E122E1F7
:00000001FF
)";



TEST_CASE("test_init_from_file")
{
	istringstream stream(hex8);
	IntelHex ih(stream);
	for (size_t i = 0; i < size(bin8); i++)
	{
		REQUIRE(ih[i] == bin8[i]);
	}
}

TEST_CASE("test_tobinarray_empty")
{
	IntelHex ih;
	ih.padding = 0xFF;   // set-up explicit padding value

	auto arr1 = ih.tobinarray();
	REQUIRE(arr1.empty());

	auto arr2 = ih.tobinarray(0);
	REQUIRE(arr2.empty());

	// tobinarray(end = 2)
	auto arr3 = ih.tobinarray({}, 2);
	REQUIRE(arr3.empty());

	// tobinarray(start = 0, end = 2)
	auto arr4 = ih.tobinarray(0, 2);
	REQUIRE(arr4.size() == 3);
	REQUIRE((arr4[0] == 0xFF && arr4[1] == 0xFF && arr4[2] == 0xFF));
}


TEST_CASE("test_tobinarray_with_size")
{
	istringstream stream(hex8);
	IntelHex ih(stream);

	IntelHex::BinArray arr1{2, 5, 162, 229, 118, 36, 106, 248};
	REQUIRE(arr1 == ih.tobinarray({}, {}, 8));		// from addr 0

	IntelHex::BinArray arr2{120, 103, 48, 7, 2, 120, 106, 228};
	REQUIRE(arr2 == ih.tobinarray(12, {}, 8));	// start=12 size=8

	IntelHex::BinArray arr3{2, 5, 162, 229, 118, 36, 106, 248};
	REQUIRE(arr3 == ih.tobinarray({}, 7, 8));	// addr: 0..7, 8 bytes

	IntelHex::BinArray arr4{120, 103, 48, 7, 2, 120, 106, 228};
	REQUIRE(arr4 == ih.tobinarray({}, 19, 8));	// addr: 12..19, 8 bytes

	REQUIRE_THROWS_AS(ih.tobinarray(0, 7, 8), out_of_range);

	REQUIRE_THROWS_AS(ih.tobinarray({}, 3, 8), out_of_range);

	REQUIRE_THROWS_AS(ih.tobinarray({}, {}, 0), range_error);
}

TEST_CASE("test_tobinfile")
{
	istringstream stream(hex8);
	IntelHex ih(stream);

	ostringstream sio;
	ih.tobinfile(sio);
	auto s1 = sio.str();

	string s2 {(char*)bin8, size(bin8)};
	REQUIRE(s1 == s2);

//	# new API: .tofile universal method
//	ih.tofile(sio, format='bin')

}

TEST_CASE("test_write_hexfile")
{
	istringstream stream(hex_simple);
	IntelHex ih(stream);

	ostringstream sio;
	ih.write_hex_file(sio);
	auto s = sio.str();
	REQUIRE(s == hex_simple);

//	# new API: .tofile universal method
//	ih.tofile(sio, format='hex')
}

TEST_CASE("test_write_hex_bug_341051")
{
	istringstream stream(hex_bug_lp_341051);
	IntelHex ih(stream);
	ostringstream sio;
	ih.write_hex_file(sio);
	auto s = sio.str();
	REQUIRE(s == hex_bug_lp_341051);
}

TEST_CASE("test_write_hex_first_extended_linear_address")
{
	IntelHex ih;
	ih.add(0x20000, 0x01);
	ostringstream sio;
	ih.write_hex_file(sio);
	auto s = sio.str();

	// should be
//	r = [Record.extended_linear_address(2),
//		 Record.data(0x0000, [0x01]),
//		 Record.eof()]
//	h = '\n'.join(r) + '\n'
//	// compare
//	self.assertEqual(h, s)
}


TEST_CASE("test_dict_interface")
{
	IntelHex ih;
	REQUIRE(ih[0] == 0xFF);

	ih.add(0, 0x01);
	REQUIRE(ih[0] == 0x01);

	ih.del(0);
	REQUIRE(ih.tobinarray().empty());
}


TEST_CASE("test_len")
{
	IntelHex ih;
	REQUIRE(ih.size() == 0);

	ih.add(2, 0x01);
	REQUIRE(ih.size() == 1);

	ih.add(1000, 0x02);
	REQUIRE(ih.size() == 2);
}


TEST_CASE("test__getitem__")
{
	IntelHex ih;
	// simple cases
	REQUIRE(0xFF == ih[0]);
	ih.add(0, 0x01);
	REQUIRE(0x01 == ih[0]);

	// big address
	REQUIRE(0xFF == ih[(1ull << 32) - 1]);

	// wrong addr type/value for indexing operations
//	def getitem(index):
//		return ih[index]
//	self.assertRaisesMsg(TypeError,
//		'Address should be >= 0.',
//		getitem, -1)
//	self.assertRaisesMsg(TypeError,
//		"Address has unsupported type: %s" % type('foo'),
//		getitem, 'foo')

	// new object with some data
	ih = IntelHex();
	ih.add( 0, 1);
	ih.add( 1, 2);
	ih.add( 2, 3);
	ih.add(10, 4);


	// other slice operations
//	self.assertEqual({}, ih[3:8].todict())
//	self.assertEqual({0:1, 1:2}, ih[0:2].todict())
//	self.assertEqual({0:1, 1:2}, ih[:2].todict())
//	self.assertEqual({2:3, 10:4}, ih[2:].todict())
//	self.assertEqual({0:1, 2:3, 10:4}, ih[::2].todict())

}


TEST_CASE("test_addresses")
{
	// empty object
	IntelHex ih;
	REQUIRE(ih.addresses().empty());
	REQUIRE(! ih.minaddr().has_value());
	REQUIRE(! ih.maxaddr().has_value());

	// normal object
	ih = IntelHex({{1,2}, {7,8}, {10,0}});
	vector<IntelHex::Addr> addr {1,7,10};
	REQUIRE(addr == ih.addresses());
	REQUIRE(1 == ih.minaddr());
	REQUIRE(10 == ih.maxaddr());
}


TEST_CASE("test_segments")
{
	// test that address segments are correctly summarized
	IntelHex ih;
	auto sg = ih.segments();
	REQUIRE(sg.size() == 0);

	ih.add(0x100, 0);
	sg = ih.segments();
	REQUIRE(sg.size() == 1);
	REQUIRE(sg[0].begin < sg[0].end);
	REQUIRE(sg[0].begin == 0x100);
	REQUIRE(sg[0].end == 0x101);

	ih.add(0x101, 1);
	sg = ih.segments();
	REQUIRE(sg.size() == 1);
	REQUIRE(sg[0].begin < sg[0].end);
	REQUIRE(sg[0].begin == 0x100);
	REQUIRE(sg[0].end == 0x102);

	ih.add(0x200, 2);
	ih.add(0x201, 3);
	ih.add(0x202, 4);
	sg = ih.segments();
	REQUIRE(sg.size() == 2);
	REQUIRE(sg[0].begin < sg[0].end);
	REQUIRE(sg[1].begin < sg[1].end);
	REQUIRE(sg[0].begin == 0x100);
	REQUIRE(sg[0].end == 0x102);
	REQUIRE(sg[1].begin == 0x200);
	REQUIRE(sg[1].end == 0x203);

}
