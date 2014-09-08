#ifndef __LOG_H_
#define __LOG_H_


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;

//Thread safe log class
class Log{

public:
	Log(std::string logPath);
	~Log();
	void LogDebug(std::wstring s,int code = 0);
	void LogDebug(std::string s,int code=0);
	void LogDebug(std::wstring s,std::wstring);
	void LogDebug(std::string,std::string);
	void LogError(std::string s,int code=0);
	void LogError(std::wstring s,int code=0);
	void EpicFail(std::string s, int code=0);
private:
	HANDLE outMutex;
	wfstream out;
	DWORD dwWaitResult;

};

#endif