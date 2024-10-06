#include "TBitReader.hpp"
#include "BufferArray.hpp"


void TBitReader::ReadExponentialGolombCode(uint32_t& Data)
{
	int i = 0;
	
	while( true )
	{
		uint8 Bit;
		Read( Bit, 1 );
		
		if ( (Bit == 0) && (i < 32) )
			i++;
		else
			break;
	}
	Read( Data, i );
	Data += (1 << i) - 1;
}

void TBitReader::ReadExponentialGolombCodeSigned(sint32& Data)
{
	uint32 r;
	ReadExponentialGolombCode(r);
	if (r & 0x01)
	{
		Data = (r+1)/2;
	}
	else
	{
		Data = -size_cast<sint32>(r/2);
	}
}


template<int BYTECOUNT,typename STORAGE>
void TBitReader::ReadBytes(STORAGE& Data,size_t BitCount)
{
	//	gr: definitly correct
	Data = 0;
	
	BufferArray<uint8_t,BYTECOUNT> Bytes;
	int ComponentBitCount = size_cast<int>(BitCount);
	while ( ComponentBitCount > 0 )
	{
		Read( Bytes.PushBack(), std::min<size_t>(8,ComponentBitCount) );
		ComponentBitCount -= 8;
	}
	//	gr: should we check for mis-aligned bitcount?
	
	Data = 0;
	for ( int i=0;	i<Bytes.GetSize();	i++ )
	{
		int Shift = (i * 8);
		Data |= static_cast<STORAGE>(Bytes[i]) << Shift;
	}
	
	STORAGE DataBackwardTest = 0;
	for ( int i=0;	i<Bytes.GetSize();	i++ )
	{
		auto Shift = (Bytes.GetSize()-1-i) * 8;
		DataBackwardTest |= static_cast<STORAGE>(Bytes[i]) << Shift;
	}
	
	//	turns out THIS is the right way
	Data = DataBackwardTest;
}

bool TBitReader::ReadBit()
{
	uint8_t Byte = 0;
	Read( Byte, 1 );
	return Byte == 1;
}


void TBitReader::Read(uint32_t& Data,size_t BitCount)
{
	//	break up data
	if ( BitCount <= 8 )
	{
		uint8_t Data8;
		Read( Data8, BitCount );
		Data = Data8;
		return;
	}
	
	if ( BitCount <= 8 )
	{
		ReadBytes<1>( Data, BitCount );
		return;
	}
	
	if ( BitCount <= 16 )
	{
		ReadBytes<2>( Data, BitCount );
		return;
	}
	
	if ( BitCount <= 32 )
	{
		ReadBytes<4>( Data, BitCount );
		return;
	}
	
	std::stringstream Error;
	Error << __func__ << " not handling bit count > 32; " << BitCount;
	throw std::runtime_error( Error.str() );
}


void TBitReader::Read(uint64_t& Data,size_t BitCount)
{
	if ( BitCount <= 8 )
	{
		ReadBytes<1>( Data, BitCount );
		return;
	}
	
	if ( BitCount <= 16 )
	{
		ReadBytes<2>( Data, BitCount );
		return;
	}
	
	if ( BitCount <= 32 )
	{
		ReadBytes<4>( Data, BitCount );
		return;
	}
	
	if ( BitCount <= 64 )
	{
		ReadBytes<8>( Data, BitCount );
		return;
	}
	
	std::stringstream Error;
	Error << __func__ << " not handling bit count > 32; " << BitCount;
	throw std::runtime_error( Error.str() );
}

void TBitReader::Read(uint8_t& Data,size_t BitCount)
{
	if ( BitCount <= 0 )
		return;
	if ( BitCount > 8 )
		throw std::runtime_error("trying to read>8 bits to 8bit value");
	
	
	//	if we're on bit [7] and ask for 2 bits, we're going to overflow the byte
	//	and the code below doesn't handle that.
	//	quick hack, just read bit by bit
	//	todo: fix properly
	{
		auto CurrentBit = (mBitPos) % 8;
		auto EndBit = (mBitPos+BitCount-1) % 8;
		if ( EndBit < CurrentBit )
		{
			Data = 0;
			for ( auto b=0;	b<BitCount;	b++ )
			{
				uint8_t BitValue = ReadBit() ? (1<<b) : 0;
				Data |= BitValue;
			}
			return;
		}
	}
	
	
	//	current byte
	auto CurrentByte = mBitPos / 8;
	auto CurrentBit = mBitPos % 8;
	
	//	out of range
	if ( CurrentByte >= mData.size() )
		throw std::runtime_error("Reading byte out of range");
	
	//	move along
	mBitPos += BitCount;
	
	//	get byte
	Data = mData[CurrentByte];
	
	//	pick out certain bits
	//	gr: reverse endianess to what I thought...
	//Data >>= CurrentBit;
	Data >>= 8-CurrentBit-BitCount;
	Data &= (1<<BitCount)-1;
}


/*
template<int BYTECOUNT,typename STORAGE>
bool TBitReader::ReadBytes(STORAGE& Data,int BitCount)
{
	//	gr: definitly correct
	Data = 0;
	
	BufferArray<uint8,BYTECOUNT> Bytes;
	int ComponentBitCount = BitCount;
	while ( ComponentBitCount > 0 )
	{
		if ( !Read( Bytes.PushBack(), std::min(8,ComponentBitCount) ) )
			return false;
		ComponentBitCount -= 8;
	}
		
	Data = 0;
	for ( int i=0;	i<Bytes.GetSize();	i++ )
	{
		int Shift = (i * 8);
		Data |= static_cast<STORAGE>(Bytes[i]) << Shift;
	}

	STORAGE DataBackwardTest = 0;
	for ( int i=0;	i<Bytes.GetSize();	i++ )
	{
		auto Shift = (Bytes.GetSize()-1-i) * 8;
		DataBackwardTest |= static_cast<STORAGE>(Bytes[i]) << Shift;
	}

	//	turns out THIS is the right way
	Data = DataBackwardTest;

	return true;
}

bool TBitReader::Read(int& Data,int BitCount)
{
	//	break up data
	if ( BitCount <= 8 )
	{
		uint8 Data8;
		if ( !Read( Data8, BitCount ) )
			return false;
		Data = Data8;
		return true;
	}
	
	if ( BitCount <= 8 )
		return ReadBytes<1>( Data, BitCount );

	if ( BitCount <= 16 )
		return ReadBytes<2>( Data, BitCount );

	if ( BitCount <= 32 )
		return ReadBytes<4>( Data, BitCount );

	throw std::runtime_error("TBitReader_Lambda::Read(int) not handling > 32bit");
}


bool TBitReader::Read(uint64& Data,int BitCount)
{
	if ( BitCount <= 8 )
		return ReadBytes<1>( Data, BitCount );

	if ( BitCount <= 16 )
		return ReadBytes<2>( Data, BitCount );

	if ( BitCount <= 32 )
		return ReadBytes<4>( Data, BitCount );

	if ( BitCount <= 64 )
		return ReadBytes<8>( Data, BitCount );

	throw std::runtime_error("TBitReader_Lambda::Read(64) not handling > 64bit");
}

bool TBitReader::Read(uint8& Data,int BitCount)
{
	if ( BitCount <= 0 )
		return false;
	assert( BitCount <= 8 );

	//	current byte
	int CurrentByte = mBitPos / 8;
	int CurrentBit = mBitPos % 8;

	//	out of range
	if ( CurrentByte >= mData.GetSize() )
		return false;

	//	move along
	mBitPos += BitCount;

	//	get byte
	Data = mData[CurrentByte];

	//	pick out certain bits
	//	gr: reverse endianess to what I thought...
	//Data >>= CurrentBit;
	Data >>= 8-CurrentBit-BitCount;
	Data &= (1<<BitCount)-1;

	return true;
}
*/


template<int BYTECOUNT,typename STORAGE>
bool TBitReader_Lambda::ReadBytes(STORAGE& Data,int BitCount)
{
	//	gr: definitly correct
	Data = 0;
	
	BufferArray<uint8,BYTECOUNT> Bytes;
	int ComponentBitCount = BitCount;
	while ( ComponentBitCount > 0 )
	{
		if ( !Read( Bytes.PushBack(), std::min(8,ComponentBitCount) ) )
			return false;
		ComponentBitCount -= 8;
	}
		
	Data = 0;
	for ( int i=0;	i<Bytes.GetSize();	i++ )
	{
		int Shift = (i * 8);
		Data |= static_cast<STORAGE>(Bytes[i]) << Shift;
	}

	STORAGE DataBackwardTest = 0;
	for ( int i=0;	i<Bytes.GetSize();	i++ )
	{
		auto Shift = (Bytes.GetSize()-1-i) * 8;
		DataBackwardTest |= static_cast<STORAGE>(Bytes[i]) << Shift;
	}

	//	turns out THIS is the right way
	Data = DataBackwardTest;

	return true;
}

bool TBitReader_Lambda::Read(int& Data,int BitCount)
{
	//	break up data
	if ( BitCount <= 8 )
	{
		uint8 Data8;
		if ( !Read( Data8, BitCount ) )
			return false;
		Data = Data8;
		return true;
	}
	
	if ( BitCount <= 8 )
		return ReadBytes<1>( Data, BitCount );

	if ( BitCount <= 16 )
		return ReadBytes<2>( Data, BitCount );

	if ( BitCount <= 32 )
		return ReadBytes<4>( Data, BitCount );

	throw std::runtime_error("TBitReader_Lambda::Read not handling > 32bit");
}


bool TBitReader_Lambda::Read(uint64& Data,int BitCount)
{
	if ( BitCount <= 8 )
		return ReadBytes<1>( Data, BitCount );

	if ( BitCount <= 16 )
		return ReadBytes<2>( Data, BitCount );

	if ( BitCount <= 32 )
		return ReadBytes<4>( Data, BitCount );

	if ( BitCount <= 64 )
		return ReadBytes<8>( Data, BitCount );

	throw std::runtime_error("TBitReader_Lambda::Read not handling > 64bit");
}

bool TBitReader_Lambda::Read(uint8& Data,int BitCount)
{
	if ( BitCount <= 0 )
		return false;
	if ( BitCount > 8 )
		throw std::runtime_error("This function should not be requesting >8 bits");

	//	current byte
	int CurrentByte = mBitPos / 8;
	int CurrentBit = mBitPos % 8;

	//	out of range
	try
	{
		Data = mGetByte(CurrentByte);
	}
	catch(std::out_of_range& e)
	{
		return false;
	}

	//	move along
	mBitPos += BitCount;

	//	pick out certain bits
	//	gr: reverse endianess to what I thought...
	//Data >>= CurrentBit;
	Data >>= 8-CurrentBit-BitCount;
	Data &= (1<<BitCount)-1;

	return true;
}



void TBitWriter::Write(uint8 Data,int BitCount)
{
	//	gr: definitly correct
	assert( BitCount <= 8 && BitCount >= 0 );
	for ( int i=BitCount-1;	i>=0;	i-- )
	{
		int BitIndex = i;
		int Bit = Data & (1<<BitIndex);
		WriteBit( Bit ? 1 : 0 );
	}
}

template<int BYTES,typename STORAGE>
void TBitWriter::WriteBytes(STORAGE Data,int BitCount)
{
	//	gr: may not be correct
	BufferArray<uint8,BYTES> Bytes;
	for ( int i=0;	i<Bytes.MaxSize();	i++ )
	{
		int Shift = i*8;
		Bytes.PushBack( (Data>>Shift) & 0xff );
	}
	int BytesInUse = (BitCount+7) / 8;
	assert( Bytes.GetSize() >= BytesInUse );
	Bytes.SetSize( BytesInUse );

	for ( int i=0;	i<Bytes.GetSize();	i++ )
	{
		int ComponentBitCount = BitCount - (i*8);
		if ( ComponentBitCount <= 0 )
			continue;
		//	write in reverse
		auto ByteIndex = Bytes.GetSize()-1 - i;
		Write( Bytes[ByteIndex], std::min(ComponentBitCount,8) );
	}
}

void TBitWriter::Write(uint16 Data,int BitCount)
{
	WriteBytes<2>( Data, BitCount );
}

void TBitWriter::Write(uint32 Data,int BitCount)
{
	WriteBytes<4>( Data, BitCount );
}

void TBitWriter::Write(uint64 Data,int BitCount)
{
	WriteBytes<8>( Data, BitCount );
}


void TBitWriter::WriteBit(int Bit)
{
	//	current byte
	int CurrentByte = mBitPos / 8;
	int CurrentBit = mBitPos % 8;

	if ( CurrentByte >= mData.GetSize() )
		mData.PushBack(0);

	mBitPos++;

	int WriteBit = 7-CurrentBit;
	unsigned char AddBit = Bit ? (1<<WriteBit) : 0;
	
	//	bit already set??
	if ( mData[CurrentByte] & AddBit )
	{
		assert( false );
	}

	mData[CurrentByte] |= AddBit;
}
