#include "Log.h"

Log::Log(std::string logPath){
	outMutex = CreateMutex(NULL,FALSE,NULL);
	out.open(logPath,ios::out);
	if(outMutex == NULL){
		printf("CreateMutex failed. Code: %d",GetLastError());

	}
}

Log::~Log(){

	CloseHandle(outMutex);
	out.close();
}
void Log::LogError(std::string s,int code){
	std::wstring wsTmp(s.begin(),s.end());
	dwWaitResult = WaitForSingleObject(outMutex,INFINITE);
	switch(dwWaitResult){
		//Thread got ownership
	case WAIT_OBJECT_0:
		out<<wsTmp.c_str()<<" Error: "<<code<<endl; 
		//Done reading, release mutex
		if(!ReleaseMutex(outMutex))
			printf("Error Releaseing mutex: %d",GetLastError());
		break;
	case WAIT_ABANDONED:
		printf("Abandoned Mutex!!");
		exit(-1);
		break; //useless
	default:
		printf("Unknown Mutex State!!");
		exit(-1);
	}
	
}

void Log::LogError(std::wstring s,int code){
	
		out<<s.c_str()<<L" Error:"<<code<<endl; 
		//Done reading, release mutex
		if(!ReleaseMutex(outMutex))
			printf("Error Releaseing mutex: %d",GetLastError());
		break;
	case WAIT_ABANDONED:
		printf("Abandoned Mutex!!");
		exit(-1);
		break; //useless
	default:
		printf("Unknown Mutex State!!");
		exit(-1);
	}
}

void Log::EpicFail(std::string s, int code){
	std::wstring wsTmp(s.begin(),s.end());

	dwWaitResult = WaitForSingleObject(outMutex,INFINITE);
	switch(dwWaitResult){
		//Thread got ownership
	case WAIT_OBJECT_0:
		out<<wsTmp.c_str()<<" Error: "<<code<<endl;
		out<<L"EXITING"<<endl;
		exit(-1);
		//Done reading, release mutex
		if(!ReleaseMutex(outMutex))
			printf("Error Releaseing mutex: %d",GetLastError());
		break;
	case WAIT_ABANDONED:
		printf("Abandoned Mutex!!");
		exit(-1);
		break; //useless
	default:
		printf("Unknown Mutex State!!");
		exit(-1);
	}

}

void Log::LogDebug(std::wstring s,int code) {

	dwWaitResult = WaitForSingleObject(outMutex,INFINITE);
	switch(dwWaitResult){
		//Thread got ownership
	case WAIT_OBJECT_0:
		if(code ==0)
			out<<s.c_str()<<endl;
		else
			out<<s.c_str()<<L"Error: "<<code<<endl;
		//Done reading, release mutex
		if(!ReleaseMutex(outMutex))
			printf("Error Releaseing mutex: %d",GetLastError());
		break;
	case WAIT_ABANDONED:
		printf("Abandoned Mutex!!");
		exit(-1);
		break; //useless
	default:
		printf("Unknown Mutex State!!");
		exit(-1);
	}
} 
void Log::LogDebug(std::string s,int code) {
	std::wstring wsTmp(s.begin(),s.end());

	dwWaitResult = WaitForSingleObject(outMutex,INFINITE);
	switch(dwWaitResult){
		//Thread got ownership
	case WAIT_OBJECT_0:
		if(code ==0)
			out<<wsTmp.c_str()<<endl;
		else
			out<<wsTmp.c_str()<<L"Error: "<<code<<endl;
		//Done reading, release mutex
		if(!ReleaseMutex(outMutex))
			printf("Error Releaseing mutex: %d",GetLastError());
		break;
	case WAIT_ABANDONED:
		printf("Abandoned Mutex!!");
		exit(-1);
		break; //useless
	default:
		printf("Unknown Mutex State!!");
		exit(-1);
	}
}
void Log::LogDebug(std::wstring s,std::wstring w){
	

	dwWaitResult = WaitForSingleObject(outMutex,INFINITE);
	switch(dwWaitResult){
		//Thread got ownership
	case WAIT_OBJECT_0:
		out<<s.c_str()<<L""<<w.c_str()<<endl;
		//Done reading, release mutex
		if(!ReleaseMutex(outMutex))
			printf("Error Releaseing mutex: %d",GetLastError());
		break;
	case WAIT_ABANDONED:
		printf("Abandoned Mutex!!");
		exit(-1);
		break; //useless
	default:
		printf("Unknown Mutex State!!");
		exit(-1);
	}
}
void Log::LogDebug(std::string s,std::string w){
	std::wstring wsTmp(s.begin(),s.end());
	std::wstring wsTmp2(w.begin(),w.end());

	dwWaitResult = WaitForSingleObject(outMutex,INFINITE);
	switch(dwWaitResult){
		//Thread got ownership
	case WAIT_OBJECT_0:
		out<<wsTmp.c_str()<<" "<<wsTmp2.c_str()<<endl;
		//Done reading, release mutex
		if(!ReleaseMutex(outMutex))
			printf("Error Releaseing mutex: %d",GetLastError());
		break;
	case WAIT_ABANDONED:
		printf("Abandoned Mutex!!");
		exit(-1);
		break; //useless
	default:
		printf("Unknown Mutex State!!");
		exit(-1);
	}


	
}