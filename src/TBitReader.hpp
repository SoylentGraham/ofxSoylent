#pragma once

#include <span>
#include "Array.hpp"

//	my most modern DataReader is in
//	https://github.com/NewChromantics/PopMp4/blob/main/Source/Mp4Parser.hpp
//	merge these all together so the data reader is also a bit reader
class TBitReader
{
public:
	TBitReader(std::span<uint8_t> Data) :
		mData	( Data )
	{
	}

	bool			ReadBit();
	void			Read(uint32_t& Data,size_t BitCount);
	void			Read(uint64_t& Data,size_t BitCount);
	void			Read(uint8_t& Data,size_t BitCount);
	uint32_t		Read(size_t BitCount)	{	uint32_t Value;	Read(Value,BitCount);	return Value;	}
	size_t			BitPosition() const					{	return mBitPos;	}
	
	template<int BYTECOUNT,typename STORAGE>
	void			ReadBytes(STORAGE& Data,size_t BitCount);

	void			ReadExponentialGolombCode(uint32_t& Data);
	void			ReadExponentialGolombCodeSigned(int32_t& Data);
	
private:
	std::span<uint8_t>	mData;
	
	//	current bit-to-read/write-pos (the tail).
	//	This is absolute, so we get the current byte from this value
	//	it also means this class is limited to (32/64bit max / 8) byte-sized data
	size_t				mBitPos = 0;
};



//	gr: replace the other reader with this. Also, put in a better file
class TBitReader_Lambda
{
public:
	TBitReader_Lambda(std::function<uint8_t(size_t)> GetNthByte) :
		mGetByte	( GetNthByte )
	{
	}
	
	bool						Read(int& Data,int BitCount);
	bool						Read(uint64_t& Data,int BitCount);
	bool						Read(uint8_t& Data,int BitCount);
	int							Read(int BitCount)					{	int Data;	return Read( Data, BitCount) ? Data : -1;	}
	size_t						BitPosition() const					{	return mBitPos;	}
	size_t						BytesRead() const
	{
		auto RoundUpBits = (8 - (mBitPos % 8)) % 8;
		return (mBitPos+RoundUpBits) / 8;
	}

	template<int BYTES,typename STORAGE>
	bool						ReadBytes(STORAGE& Data,int BitCount);

private:
	std::function<uint8_t(size_t)>	mGetByte;
	unsigned int				mBitPos = 0;	//	current bit-to-read/write-pos (the tail)
};

class TBitWriter
{
public:
	TBitWriter(ArrayBridge<char>& Data) :
		mData	( Data ),
		mBitPos	( 0 )
	{
	}
	
	unsigned int				BitPosition() const					{	return mBitPos;	}
	
	void						Write(uint8 Data,int BitCount);
	void						Write(uint16 Data,int BitCount);
	void						Write(uint32 Data,int BitCount);
	void						Write(uint64 Data,int BitCount);
	void						WriteBit(int Bit);

	template<int BYTES,typename STORAGE>
	void						WriteBytes(STORAGE Data,int BitCount);

private:
	ArrayBridge<char>&	mData;
	unsigned int		mBitPos;	//	current bit-to-read/write-pos (the tail)
};
