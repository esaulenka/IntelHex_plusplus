#pragma once

#include <cstdint>
#include <optional>
#include <map>
#include <variant>
#include <vector>
#include <string>
#include <fstream>



class IntelHex
{
public:
	using Addr = uint32_t;
	using OptionalAddr = std::optional<Addr>;
	using BinArray = std::vector<uint8_t>;

	IntelHex()	{}

	IntelHex(const std::string &fileName)
	{	loadhex(fileName);	}

	IntelHex(std::istream & file)
	{	loadhex(file);	}

	IntelHex(std::initializer_list<std::pair<Addr, uint8_t> > init)
	{	for (auto & i : init)	buf[i.first] = i.second;	}

	void loadhex(std::istream &file);
	void loadhex(const std::string &fileName)
	{	std::ifstream f(fileName);	loadhex(f);	}

	void loadbin(std::istream &file, Addr offset=0);
	void loadbin(const std::string &fileName, Addr offset=0)
	{	std::ifstream f(fileName); loadbin(f, offset);	}

	void frombytes(const BinArray &bytes, Addr offset=0);

	// Return binary array
	BinArray tobinarray(OptionalAddr start = {}, OptionalAddr end = {}, OptionalAddr size = {}) const;

	// Convert to binary and write to file
	void tobinfile(std::ostream & file, OptionalAddr start = {}, OptionalAddr end = {}, OptionalAddr size = {}) const;
	void tobinfile(const std::string & fileName, OptionalAddr start = {}, OptionalAddr end = {}, OptionalAddr size = {}) const;

	// Returns all used addresses in sorted order
	std::vector<Addr> addresses() const;
	// Get minimal address of HEX content.
	OptionalAddr minaddr() const;
	// Get maximal address of HEX content.
	OptionalAddr maxaddr() const;

	uint8_t operator[](Addr addr) const
	{
		auto it = buf.find(addr);
		return (it != buf.end()) ? it->second : padding;
	}
//	uint8_t & operator[](Addr addr)
//	{	return buf[addr];	}

	void add(Addr addr, uint8_t data)
	{	buf.insert_or_assign(addr, data);	}

	void del(Addr addr)
	{	buf.erase(addr);	}

	size_t size() const
	{	return buf.size();	}

	// Write data to file in HEX format
	void write_hex_file(std::ostream & file, bool write_start_addr=true, uint32_t byte_count=16) const;
	void write_hex_file(const std::string & fileName, bool write_start_addr=true, uint32_t byte_count=16) const;

	enum class Overlap {
		error, ignore, replace
	};
	void merge(const IntelHex & other, Overlap overlap=Overlap::error);

	uint8_t padding = 0xFF;


	struct StartAddrSegmented {
		uint16_t CS;
		uint16_t IP;
	};
	struct StartAddrExtended {
		uint32_t EIP;
	};
	using StartAddr = std::optional< std::variant<StartAddrSegmented, StartAddrExtended> >;
	StartAddr start_addr;


	// Return a list of ordered tuple objects, representing contiguous occupied data addresses.
	// Each tuple has a length of two and follows the semantics of the range and xrange objects.
	// The second entry of the tuple is always an integer greater than the first entry.
	struct Segment {
		Addr begin;
		Addr end;
	};
	std::vector<Segment> segments();


private:

	std::map<Addr, uint8_t> buf;

	uint32_t offset = 0;

	bool decode_record(std::string s, uint32_t line=0);

	std::pair<OptionalAddr, OptionalAddr>
		get_start_end(OptionalAddr start = {}, OptionalAddr end = {}, OptionalAddr size = {}) const;


	static BinArray unhexlify(const std::string& input);
	static std::string hexlify(const BinArray& bin);

};



// some helpers
inline bool operator==(const IntelHex::StartAddr & a, const IntelHex::StartAddr & b)
{
	if (a.has_value() && b.has_value())
		return (a.value() == b.value());
	return (a.has_value() == b.has_value());
};
inline bool operator==(const IntelHex::StartAddrExtended & a, const IntelHex::StartAddrExtended & b)
{	return a.EIP == b.EIP;	};

inline bool operator==(const IntelHex::StartAddrSegmented & a, const IntelHex::StartAddrSegmented & b)
{	return (a.CS == b.CS) && (a.IP == b.IP);	};
