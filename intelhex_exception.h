#pragma once

#include <stdexcept>
#include <sstream>


class IntelHexException : std::exception {
protected:
	std::stringstream ss;
public:
	const char* what() const noexcept override { return ss.str().c_str(); }
};




class HexRecordError : IntelHexException {
public:
	HexRecordError(uint32_t line)
	{	ss << "Hex file contains invalid record at line " << line;	}
};

class RecordLengthError : IntelHexException {
public:
	RecordLengthError(uint32_t line)
	{	ss << "Record at line " << line << " has invalid length";	}
};

class RecordTypeError : IntelHexException {
public:
	RecordTypeError(uint32_t line)
	{	ss << "Record at line " << line << " has invalid record type";	}
};

class RecordChecksumError : IntelHexException {
public:
	RecordChecksumError(uint32_t line)
	{	ss << "Record at line "  << line << " has invalid checksum";	}
};

class AddressOverlapError : IntelHexException {
public:
	AddressOverlapError(const std::string & str)
	{	ss << str;	}
	AddressOverlapError(uint32_t address, uint32_t line)
	{	ss << "Hex file has data overlap at address 0x" << std::hex << address << std::dec
		   << " on line " << line;	}
};

class EOFRecordError : IntelHexException {
public:
	EOFRecordError(uint32_t line)
	{	ss << "File has invalid End-of-File record at line " << line;	}
};

class ExtendedSegmentAddressRecordError : IntelHexException {
public:
	ExtendedSegmentAddressRecordError(uint32_t line)
	{	ss << "Invalid Extended Segment Address Record at line " << line;	}
};

class ExtendedLinearAddressRecordError : IntelHexException {
public:
	ExtendedLinearAddressRecordError(uint32_t line)
	{	ss << "Invalid Extended Linear Address Record at line " << line;	}
};

class StartSegmentAddressRecordError : IntelHexException {
public:
	StartSegmentAddressRecordError(uint32_t line)
	{	ss << "Invalid Start Segment Address Record at line " << line;	}
};

class StartLinearAddressRecordError : IntelHexException {
public:
	StartLinearAddressRecordError(uint32_t line)
	{	ss << "Invalid Start Linear Address Record at line " << line;	}
};

class DuplicateStartAddressRecordError : IntelHexException {
public:
	DuplicateStartAddressRecordError(uint32_t line)
	{	ss << "Start Address Record appears twice at line " << line;	}
};

class InvalidStartAddressValueError : IntelHexException {
public:
	InvalidStartAddressValueError()
	{	ss << "Invalid start address value";	}
};

class EmptyIntelHexError : IntelHexException {
public:
	EmptyIntelHexError()
	{	ss << "Requested operation cannot be executed with empty object";	}
};
