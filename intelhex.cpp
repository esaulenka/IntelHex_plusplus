#include "intelhex.h"
#include "intelhex_exception.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

//// Used for internal needs only
//class _EndOfFile : exception {
//};





// Decode one record of HEX file.
// @param  s       line with HEX record.
// @param  line    line number (for error messages).
// @return false   if EOF record encountered.
bool IntelHex::decode_record(std::string s, uint32_t line)
{
	if (s.back() == '\n') s.pop_back();
	if (s.back() == '\r') s.pop_back();

	if (s.empty()) return true;


	if (s[0] != ':')
		throw HexRecordError(line);

	std::vector<uint8_t> bin;
	try {
		bin = unhexlify(s.substr(1));
	}
	catch(...) {
		// this might be raised by unhexlify when odd hexascii digits
		throw  HexRecordError(line);
	}
	uint32_t length = bin.size();
	if (length < 5)
		throw HexRecordError(line);

	const uint8_t record_length = bin[0];
	if (length != (5u + record_length))
		throw RecordLengthError(line);

	Addr addr = bin[1]*256 + bin[2];

	const uint8_t record_type = bin[3];
	if (record_type > 5)
		throw RecordTypeError(line);

	uint8_t crc = 0;
	for (auto d : bin) crc += d;
	if (crc != 0)
		throw RecordChecksumError(line);

	if (record_type == 0)
	{
		// data record
		addr += offset;
		for (uint32_t i = 4; i < 4u + record_length; i++)
		{
			if (buf.find(addr) != buf.end())
				throw AddressOverlapError(addr, line);
			buf[addr] = bin[i];
			addr += 1;	// FIXME: addr should be wrapped
						// BUT after 02 record (at 64K boundary)
						// and after 04 record (at 4G boundary)
		}
	}
	else if (record_type == 1)
	{
		// end of file record
		if (record_length != 0)
			throw EOFRecordError(line);
		return false;	// EOF
	}
	else if (record_type == 2)
	{
		// Extended 8086 Segment Record
		if (record_length != 2 || addr != 0)
			throw ExtendedSegmentAddressRecordError(line);
		offset = (bin[4]*256 + bin[5]) * 16;
	}
	else if (record_type == 4)
	{
		// Extended Linear Address Record
		if (record_length != 2 || addr != 0)
			throw ExtendedLinearAddressRecordError(line);
		offset = (bin[4]*256 + bin[5]) * 65536;
	}
	else if (record_type == 3)
	{
		// Start Segment Address Record
		if (record_length != 4 || addr != 0)
			throw StartSegmentAddressRecordError(line);
		if (start_addr.has_value())
			throw DuplicateStartAddressRecordError(line);
		StartAddrSegmented addr;
		addr.CS = uint16_t(bin[4]*256 + bin[5]);
		addr.IP = uint16_t(bin[6]*256 + bin[7]);
		start_addr = addr;
	}
	else if (record_type == 5)
	{
		// Start Linear Address Record
		if (record_length != 4 || addr != 0)
			throw StartLinearAddressRecordError(line);
		if (start_addr.has_value())
			throw DuplicateStartAddressRecordError(line);
		StartAddrExtended addr = {
			uint32_t(bin[4]*0x1000000 +
					 bin[5]*0x10000 +
					 bin[6]*0x100 +
					 bin[7]) };
		start_addr = addr;
	}
	return true;
}

void IntelHex::loadhex(istream &file)
{
	offset = 0;
	uint32_t line = 0;

	for (string s; getline(file, s); )
	{
		line++;
//		try {
			decode_record(s, line);
//		}
//		catch (_EndOfFile) {
//			// pass
//		}
	}
}

void IntelHex::loadbin(std::istream &file, Addr offset)
{
	BinArray data((std::istreambuf_iterator<char>(file)),
				   std::istreambuf_iterator<char>());
	frombytes(data, offset);
}


void IntelHex::frombytes(const BinArray &bytes, Addr offset)
{
	for (auto b : bytes)
	{
		buf[offset] = b;
		offset++;
	}
}


std::pair<IntelHex::OptionalAddr, IntelHex::OptionalAddr> IntelHex::get_start_end
	(OptionalAddr start, OptionalAddr end, OptionalAddr size) const
{
	if (! start.has_value() && ! end.has_value() && buf.empty())
		throw EmptyIntelHexError();

	if (size.has_value())
	{
		if (start.has_value() && end.has_value())
			throw out_of_range("tobinarray: you can't use start,end and size"
							 " arguments in the same time");
		if (!start.has_value() && !end.has_value())
			start = minaddr();
		if (start.has_value())
			end = start.value() + size.value() - 1;
		else
		{
			start = end.value() - size.value() + 1;
			if (end.value() + 1u < size.value())
				throw out_of_range("tobinarray: invalid size (%d) "
								 "for given end address (%d)");
		}
	}
	else
	{
		if (!start.has_value())
			start = minaddr();
		if (!end.has_value())
			end = maxaddr();

		if (start.value_or(0) > end.value_or(0))
			std::swap(start, end);
	}
	return { start, end };
}

IntelHex::BinArray IntelHex::tobinarray(OptionalAddr start, OptionalAddr end, OptionalAddr size) const
{
	BinArray bin;
	if (buf.empty() && !start.has_value() && !end.has_value())
		return bin;
	if (size.has_value() && size.value() <= 0)
		throw range_error("tobinarray: wrong value for size");

	std::tie(start, end)  = get_start_end(start, end, size);
	if (start.has_value() && end.has_value())
		for (Addr i = start.value(); i <= end.value(); i++)
		{
			bin.push_back(operator[](i));
		}
	return bin;
}


void IntelHex::tobinfile(ostream & file, OptionalAddr start, OptionalAddr end, OptionalAddr size) const
{
	auto arr = tobinarray(start, end, size);
	file.write((const char*) arr.data(), arr.size());
}
void IntelHex::tobinfile(const string &fileName, OptionalAddr start, OptionalAddr end, OptionalAddr size) const
{
	ofstream file(fileName);
	tobinfile(file, start, end, size);
}

std::vector<IntelHex::Addr> IntelHex::addresses() const
{
	vector<Addr> keys;
	for (auto kv : buf)
		keys.push_back(kv.first);
	sort(keys.begin(), keys.end());
	return keys;
}

IntelHex::OptionalAddr IntelHex::minaddr() const
{
//	std::vector<uint32_t> keys;
//	for (auto kv : buf)
//		keys.push_back(kv.first);
//	return * min_element(keys.begin(), keys.end());
	if (buf.empty()) return {};
	auto cmp = [](auto a, auto b)
	{	return a.first < b.first; };
	return  min_element(buf.begin(), buf.end(), cmp)->first;
}

IntelHex::OptionalAddr IntelHex::maxaddr() const
{
//	std::vector<uint32_t> keys;
//	for (auto kv : buf)
//		keys.push_back(kv.first);
//	return * max_element(keys.begin(), keys.end());
	if (buf.empty()) return {};
	auto cmp = [](auto a, auto b)
	{	return a.first < b.first; };
	return  max_element(buf.begin(), buf.end(), cmp)->first;
}

void IntelHex::write_hex_file(const std::string &fileName, bool write_start_addr, uint32_t byte_count) const
{
	ofstream file(fileName);
	write_hex_file(file, write_start_addr, byte_count);
}
void IntelHex::write_hex_file(std::ostream &file, bool write_start_addr, uint32_t byte_count) const
{
	if (byte_count > 255 || byte_count < 1)
		throw length_error("wrong byte_count value");

	auto make_chksum = [](BinArray & buf)
	{
		uint8_t chksum = 0;
		for (size_t i = 0; i < buf.size() - 1; i++)
			chksum += buf[i];
		buf[buf.size() - 1] = -chksum;
	};

	// start address record if any
	if (write_start_addr && start_addr.has_value())
	{
		BinArray bin(9);
		if (holds_alternative<StartAddrSegmented>(start_addr.value()))
		{
			// Start Segment Address Record
			bin[0] = 4;		// reclen
			bin[1] = 0;		// offset msb
			bin[2] = 0;		// offset lsb
			bin[3] = 3;		// rectyp
			auto addr = get<StartAddrSegmented>(start_addr.value());
			bin[4] = addr.CS >> 8;
			bin[5] = addr.CS;
			bin[6] = addr.IP >> 8;
			bin[7] = addr.IP;
			make_chksum(bin);

			file << ":" << hexlify(bin) << endl;
		}
		else
		if (holds_alternative<StartAddrExtended>(start_addr.value()))
		{
			// Start Linear Address Record
			bin[0] = 4;		// reclen
			bin[1] = 0;		// offset msb
			bin[2] = 0;		// offset lsb
			bin[3] = 5;		// rectyp
			auto addr = get<StartAddrExtended>(start_addr.value());
			bin[4] = (addr.EIP >> 24) & 0xFF;
			bin[5] = (addr.EIP >> 16) & 0xFF;
			bin[6] = (addr.EIP >>  8) & 0xFF;
			bin[7] = (addr.EIP >>  0) & 0xFF;
			make_chksum(bin);

			file << ":" << hexlify(bin) << endl;
		}
	}

	// data
	if (! buf.empty())
	{
		const auto addresses = this->addresses();
		const uint32_t addr_len = addresses.size();
		const Addr maxaddr = * this->maxaddr();
		const bool need_offset_record = (maxaddr > 65535);

		Addr high_ofs = 0;
		uint32_t cur_ix = 0;

		for (auto cur_addr = *addresses.begin(); cur_addr <= maxaddr; )
		{
			if (need_offset_record)
			{
				BinArray bin(7);
				bin[0] = 2;		// reclen
				bin[1] = 0;		// offset msb
				bin[2] = 0;		// offset lsb
				bin[3] = 4;		// rectyp
				high_ofs = cur_addr >> 16;
				bin[4] = high_ofs >> 8;	// msb of high_ofs
				bin[5] = high_ofs;		// lsb of high_ofs
				make_chksum(bin);

				file << ":" << hexlify(bin) << endl;
			}
			while(true)
			{
				// produce one record
				const uint16_t low_addr = cur_addr & 0xFFFF;
				// chain_len off by 1
				size_t chain_len =
						min(byte_count-1,
							min(uint32_t(0xFFFF - low_addr), maxaddr - cur_addr));

				// search continuous chain
				const auto stop_addr = cur_addr + chain_len;
				if (chain_len)
				{
					auto ix = upper_bound(
						addresses.begin() + min(uint32_t(cur_ix+chain_len+1), cur_ix),
						addresses.end(),
						stop_addr);
					if (ix != addresses.end())
						chain_len = *ix - cur_ix;     // real chain_len
					else
						chain_len = addr_len - cur_ix;

					// there could be small holes in the chain
					// but we will catch them by try-except later
					// so for big continuous files we will work
					// at maximum possible speed
				}
				else
					chain_len = 1;	// real chain_len


				BinArray bin(5 + chain_len, 0x00);

				bin[1] = low_addr >> 8;	// msb of low_addr
				bin[2] = low_addr;		// lsb of low_addr
				bin[3] = 0;				// rectype
				size_t i = 0;
				try {    // if there is small holes we'll catch them
					for ( ; i < chain_len; i++)
						bin[4 + i] = buf.at(cur_addr + i);
				}
				catch (const out_of_range &)
				{
					// we catch a hole so we should shrink the chain
					chain_len = i;
					bin.resize(5 + i);
				}

				bin[0] = chain_len;
				make_chksum(bin);

				file << ":" << hexlify(bin) << endl;


				// adjust cur_addr/cur_ix
				cur_ix += chain_len;
				if (cur_ix < addr_len)
					cur_addr = addresses[cur_ix];
				else
				{
					cur_addr = maxaddr + 1;
					break;
				}
				Addr high_addr = cur_addr >> 16;
				if (high_addr > high_ofs)
					break;
			}
		}
	}

	// end-of-file record
	file << ":00000001FF" << endl;

}


void IntelHex::merge(const IntelHex &other, Overlap overlap)
{
	if (&other == this)
		throw logic_error("Can't merge itself");

	// merge data
	for (auto i : other.buf)
	{
		if (buf.count(i.first))
		{
			if (overlap == Overlap::error)
			{
				stringstream ss;
				ss << "Data overlapped at address 0x" << hex << i.first;
				throw AddressOverlapError(ss.str());
			}
			else if (overlap == Overlap::ignore)
				continue;
		}
		buf[i.first] = i.second;
	}

	// merge start_addr
	if (! (start_addr == other.start_addr))
	{
		if (! start_addr.has_value())		// set start addr from other
			start_addr = other.start_addr;
		else if (! other.start_addr.has_value())  // keep existing start addr
			; // do nothing
		else						// conflict
		{
			if (overlap == Overlap::error)
				throw AddressOverlapError("Starting addresses are different");
			else if (overlap == Overlap::replace)
				start_addr = other.start_addr;
		}
	}
}

vector<IntelHex::Segment> IntelHex::segments()
{
	vector<Segment> seg;
	const auto addr = addresses();

	if (addr.empty())
		return seg;
	else if (addr.size() == 1)
	{
		seg.push_back({addr[0], addr[0]+1});
		return seg;
	}

//	adjacent_differences = [(b - a) for (a, b) in zip(addresses[:-1], addresses[1:])]
//	breaks = [i for (i, x) in enumerate(adjacent_differences) if x > 1]
//	endings = [addresses[b] for b in breaks]
//	endings.append(addresses[-1])
//	beginings = [addresses[b+1] for b in breaks]
//	beginings.insert(0, addresses[0])
//	return [(a, b+1) for (a, b) in zip(beginings, endings)]

	Addr start = addr[0];
	// search break
	for (size_t i = 0; i < addr.size() - 1; i++)
		if (addr[i] + 1 != addr[i + 1])
		{
			seg.push_back({start, addr[i]+1});
			start = addr[i + 1];
		}

	// last segment
	seg.push_back({start, addr[addr.size()-1]+1});


	return seg;
}


IntelHex::BinArray IntelHex::unhexlify(const string &inp)
{
	if (inp.length() % 2)
		throw length_error("Hex string: non-even length");

	BinArray res;
	for (size_t i = 0; i < inp.length() - 1; i += 2)
	{
		const auto hi = inp[i];
		const auto lo = inp[i + 1];
		res.push_back(
			(hi >= 'A' ? hi - 'A' + 10 : hi - '0') * 16 +
			(lo >= 'A' ? lo - 'A' + 10 : lo - '0'));
	}
	return res;
}

std::string IntelHex::hexlify(const BinArray &bin)
{
	string str;
	for (const auto b : bin)
	{
		auto hex = [](uint8_t b) -> char {
			return (b < 10) ? (b + '0') : (b - 10 + 'A');
		};
		str += hex(b >> 4);
		str += hex(b & 0x0F);
	}
	return str;

}
