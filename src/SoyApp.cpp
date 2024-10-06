#include "SoyApp.h"


std::function<void(bool)> gOnConsoleStop;


#if defined(TARGET_WINDOWS)
BOOL WINAPI Soy::Platform::TConsoleApp::ConsoleHandler(DWORD dwType)
{
	bool Dummy=false;
	switch(dwType) 
	{
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			if ( gOnConsoleStop )
				gOnConsoleStop(Dummy);

			//Returning would make the process exit immediately!
			//We just make the handler sleep until the main thread exits,
			//or until the maximum execution time for this handler is reached.
			Sleep(10000);
		return TRUE;

	default:
		return FALSE;
	}
}
#endif

void Soy::Platform::TConsoleApp::Exit()
{
	mWorker.Stop();
}

void Soy::Platform::TConsoleApp::WaitForExit()
{
	//	setup handler
#if defined(TARGET_WINDOWS)
	SetConsoleCtrlHandler( ConsoleHandler, true );
#endif
	auto& Worker = mWorker;
	auto OnConsoleStop = [&Worker](bool)
	{
		Worker.Stop();
	};
	gOnConsoleStop = OnConsoleStop;
	
	//	runs until something tells it to exit
	mWorker.Start();
}
