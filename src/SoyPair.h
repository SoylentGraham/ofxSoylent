#pragma once


template<typename FIRST,typename SECOND>
class SoyPair
{
public:
	SoyPair(const FIRST& First=FIRST(),const SECOND& Second=SECOND()) :
		mFirst	( First ),
		mSecond	( Second )
	{
	}
	
	inline bool		operator==(const FIRST& Match) const		{	return mFirst == Match;	}

public:
	FIRST		mFirst;
	SECOND		mSecond;
};



template<class STRING,typename FIRST,typename SECOND>
inline STRING& operator<<(STRING& str,const SoyPair<FIRST,SECOND>& Pair)
{
	str << '[' << Pair.mFirst << '.' << Pair.mSecond << ']';
	return str;
}


