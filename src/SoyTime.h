#pragma once


//#include <iomanip>
#include "SoyTypes.h"
#if defined(TARGET_OSX)||defined(TARGET_IOS)||defined(TARGET_PS4)
#include <sys/time.h>
#endif
#include <chrono>


class SoyTime;

namespace Soy
{
	namespace Platform
	{
#if defined(__OBJC__)
		std::chrono::milliseconds	GetTime(CMTime Time);
		CMTime						GetTime(std::chrono::milliseconds Time);
		std::chrono::milliseconds	GetTime(CFTimeInterval Time);
#endif
	}
}



//	deprecate all this for std::chrono
class SoyTime
{
public:
	explicit SoyTime(bool InitToNow=false) : 
		mTime	( InitToNow ? Now().GetTime() : 0 )
	{
	}
	__deprecated_prefix explicit SoyTime(uint64_t Time) __deprecated :
		mTime	( Time )
	{
	}
	SoyTime(const std::chrono::milliseconds& ms) :
		mTime	( ms.count() )
	{
	}
	SoyTime(const SoyTime& Time) :
		mTime	( Time.GetTime() )
	{
	}
	explicit SoyTime(const std::string& String) :
		mTime	( 0 )
	{
		FromString( String );
	}

	bool			FromString(const std::string& String);
	std::string		ToString() const __deprecated;

	uint64_t			GetTime() const							{	return mTime;	}
	bool			IsValid() const							{	return mTime!=0;	}
	static SoyTime	Now();				//	time since jan 1 1970, this will inevitably be a 32bit number
	static SoyTime	UpTime();			//	smaller clock, will loop after ~40 days
	ssize_t			GetDiff(const SoyTime& that) const
	{
		ssize_t a = size_cast<ssize_t>( this->GetTime() );
		ssize_t b = size_cast<ssize_t>( that.GetTime() );
		return a - b;
	}
	uint64_t			GetNanoSeconds() const					{	return mTime * 1000000;	}
	void			SetNanoSeconds(uint64_t NanoSecs)			{	mTime = NanoSecs / 1000000;	}
	void			SetMicroSeconds(uint64_t MicroSecs)		{	mTime = MicroSecs / 1000;	}
	uint64_t			GetMicroSeconds() const					{	return mTime * 1000;	}
	std::chrono::milliseconds	GetMilliSeconds() const		{	return std::chrono::milliseconds(mTime);	}
	float			GetSecondsf() const						{	return mTime / 1000.f;	}

	inline bool		operator==(const SoyTime& Time) const	{	return mTime == Time.mTime;	}
	inline bool		operator!=(const SoyTime& Time) const	{	return mTime != Time.mTime;	}
	inline bool		operator<(const SoyTime& Time) const	{	return mTime < Time.mTime;	}
	inline bool		operator<=(const SoyTime& Time) const	{	return mTime <= Time.mTime;	}
	inline bool		operator>(const SoyTime& Time) const	{	return mTime > Time.mTime;	}
	inline bool		operator>=(const SoyTime& Time) const	{	return mTime >= Time.mTime;	}
	inline SoyTime&	operator+=(const uint64_t& Step) 			{	mTime += Step;	return *this;	}
	inline SoyTime&	operator+=(const SoyTime& Step)			{	mTime += Step.GetTime();	return *this;	}
	inline SoyTime&	operator-=(const uint64_t& Step) 			{	mTime -= Step;	return *this;	}
	inline SoyTime&	operator-=(const SoyTime& Step)			{	mTime -= Step.GetTime();	return *this;	}
	inline SoyTime	operator+(const SoyTime& B) const		{	return SoyTime( std::chrono::milliseconds(mTime + B.mTime) );	}
	inline SoyTime	operator-(const SoyTime& B) const		{	return SoyTime( std::chrono::milliseconds(mTime - B.mTime) );	}

public:	//	gr: temporarily public during android/ios merge
	uint64_t	mTime;
};
DECLARE_TYPE_NAME( SoyTime );

//	<iomanip> crashes the x86 emulator on windows 11 arm and setfill() is from that
//	gr: time has gone on long enough that now we can just print the number
//		any code still using SoyTime should migrate to std::chrono
inline std::ostream& operator<< (std::ostream &out,const SoyTime &in)
{
	out << in.GetTime();
	return out;
}

/*
inline std::istream& operator>> (std::istream &in,SoyTime &out)
{
	std::string TimeStr;
	in >> TimeStr;
	
	if ( in.fail() )
		out = SoyTime();
	else
		out.FromString( TimeStr );

	return in;
}

*/
