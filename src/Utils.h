#ifndef __UTILS_H_
#define __UTILS_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <time.h>
#include <random>
#include <Windows.h>
#include <IPHlpApi.h>
#include"Log.h"

#pragma comment(lib , "iphlpapi.lib") //For iphlpapi

#define INT2STR( x ) dynamic_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x ) ).str()
#define MAC_SIZE 18
#define LOGPATH "C:\\WandererError.txt"

typedef struct buffer {
	 char* data;
	 int len;
} buf;

typedef struct networkInfo {
	char* ip;
	char* mask;
	char* mac;
} networkInfo;



buffer readFile(char* path);
bool connectToShare(WCHAR* path, WCHAR* username,WCHAR* password);
std::vector<std::wstring> &split(std::wstring &s, WCHAR delim, std::vector<std::wstring> &elems);
std::vector<std::string> &split(std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::wstring> split(std::wstring &s, WCHAR delim);
std::vector<std::string> split(std::string &s, char delim);
WCHAR* charToWchar(const char* input);
bool initializeLog();
bool compare(char*,char*);
bool compare(WCHAR* a, WCHAR*b);
double GetRandom(double Min, double Max);
std::vector<networkInfo> getInterfaceInfo();
int str2int(std::string);
#endif