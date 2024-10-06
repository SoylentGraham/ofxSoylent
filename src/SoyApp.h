#pragma once

#include "SoyThread.h"
#include <functional>


class SoyApp
{
public:
	SoyApp()					{}
	virtual ~SoyApp()			{}

	virtual bool	Init()		{	return true;	}
	virtual bool	Update()	{	return true;	}
	virtual void	Exit()		{}
};

namespace Soy
{
	namespace Platform
	{
		class TConsoleApp;
	}
};

class Soy::Platform::TConsoleApp
{
public:
	void				Exit();
	void				WaitForExit();
private:
#if defined(TARGET_WINDOWS)
	static BOOL WINAPI	ConsoleHandler(DWORD dwType);
#endif
	
private:
	SoyWorkerDummy		mWorker;
};



