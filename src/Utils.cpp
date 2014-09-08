#include "Utils.h"

using namespace std;

Log logFile(LOGPATH);

buffer readFile(char* path){
	
	char* temp = NULL;
	buffer buf = {0};
	ifstream myFile(path,ios::in|ios::binary);
	if(myFile) { //make sure file opened ok.
		myFile.seekg(0,myFile.end); //move to end of file
		int len = (int)myFile.tellg(); //get length of file
		myFile.seekg(0,myFile.beg); //move back to begining
		temp = new char[len]; //define array to the right size for file
		memset(temp,0,len);
		myFile.read(temp,len); //read in entire file
		if(!myFile) {//error while reading
			logFile.LogError("Error while reading");
		}
		buf.data = temp;
		buf.len = len;
	}
	else{ // error opening
		logFile.LogError("Error opening file");
	}
	
	return buf;
}


bool connectToShare(WCHAR* remotePath, WCHAR* username,WCHAR* password) {
		DWORD rc;
		NETRESOURCE nr = {0};
		nr.dwType = RESOURCETYPE_ANY;
		nr.lpLocalName = NULL;
		nr.lpRemoteName = (LPWSTR)(LPCWSTR)remotePath;
		nr.lpProvider = NULL;

		//Check if we are already connected tot he share.
		HANDLE hEnum = NULL;
		if(NO_ERROR == WNetOpenEnum(RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &hEnum))
		{
			bool bConnected = false;
			BYTE buf[65536] = {0};
			DWORD count = (DWORD)-1;
			DWORD bufSize = sizeof(buf);
			rc = WNetEnumResource(hEnum, &count, buf, &bufSize); //returns 259 or ERROR_NO_MORE_ITEMS when it finds nothing
			for(DWORD i = 0; i < count; i++)
			{
				NETRESOURCE* pNR = (NETRESOURCE*)buf;
				if(0 == _wcsicmp(pNR[i].lpRemoteName, remotePath))
				{
					WNetCloseEnum(hEnum);
					return true;
				}					
			}
		}
	
		WNetCloseEnum(hEnum);

		if(NO_ERROR!=WNetAddConnection2(&nr,password,username,0)){
			logFile.LogError("Error trying to connect to Share",GetLastError());
			return false; //Need to error handle
		}

		return true;
} 



WCHAR* charToWchar(const char* input){
	std::string a(input);
	std::wstring b(a.begin(),a.end());
	WCHAR* wstr = new WCHAR[b.length()+1];
	wcscpy_s(wstr,b.length()+1,b.c_str());
	return wstr;
}

std::vector<std::wstring> &split(std::wstring &s, WCHAR delim, std::vector<std::wstring> &elems) {
    std::wstringstream ss(s);
    std::wstring item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::wstring> split(std::wstring &s, WCHAR delim) {
    std::vector<std::wstring> elems;
    split(s, delim, elems);
    return elems;
}

std::vector<std::string> &split(std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

bool compare(char* a, char*b){

	std::string str1(a);
	std::string str2(b);
	if(str1.compare(str2) != 0)
		return false;

	return true;
}

bool compare(WCHAR* a, WCHAR*b){

	std::wstring str1(a);
	std::wstring str2(b);
	if(str1.compare(str2) != 0)
		return false;

	return true;
}

double GetRandom(double Min, double Max)
{
		
    return ((double(rand()) / double(RAND_MAX)) * (Max - Min)) + Min;    
}

std::vector<networkInfo> getInterfaceInfo() {
	std::vector<networkInfo> interfaces;
	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);
	
	networkInfo info ={0};
	char* mac_addr = (char*)calloc(MAC_SIZE,sizeof(char));
	info.mac = (char*)calloc(MAC_SIZE,sizeof(char));
	info.ip = (char*)calloc(16,sizeof(char));
	info.mask = (char*)calloc(16,sizeof(char));

	AdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
	if (AdapterInfo == NULL) {
		logFile.LogError("Error allocating memory needed to call GetAdaptersinfo");
		return interfaces;
	}

	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
		AdapterInfo = (IP_ADAPTER_INFO *) malloc(dwBufLen);
		if (AdapterInfo == NULL) {
			logFile.LogError("Error allocating memory needed to call GetAdaptersinfo");
			return interfaces;
		 }
	 }

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
		do { //For each adapter 
			sprintf_s(mac_addr,MAC_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X",
			pAdapterInfo->Address[0], pAdapterInfo->Address[1],
			pAdapterInfo->Address[2], pAdapterInfo->Address[3],
			pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
			memcpy(info.ip,pAdapterInfo->IpAddressList.IpAddress.String,strlen(pAdapterInfo->IpAddressList.IpAddress.String));
			memcpy(info.mask,pAdapterInfo->IpAddressList.IpMask.String,strlen(pAdapterInfo->IpAddressList.IpMask.String));
			info.mac = mac_addr;
			
			interfaces.push_back(info); //Add info onto vector

			//prepare for next interface
			pAdapterInfo = pAdapterInfo->Next;   
			mac_addr = (char*)calloc(17,sizeof(char));
			info.mac = (char*)calloc(17,sizeof(char));
			info.ip = (char*)calloc(16,sizeof(char));
			info.mask = (char*)calloc(16,sizeof(char));

		}while(pAdapterInfo);                        
	}
	free(AdapterInfo);
	return interfaces;
}

int str2int(std::string str){
	int x;
	istringstream(str) >> x;
	return x;
}