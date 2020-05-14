#include "legato.h"
#include "interfaces.h"
#include <sys/socket.h>
#include <netinet/in.h>
//
//#define BUFFER_SIZE 1024
//#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }
#define FWUPDATE_SERVER_PORT 5000


// [AssetDataPath]
//-------------------------------------------------------------------------------------------------
/**
 * Declare asset data path
 */
//-------------------------------------------------------------------------------------------------

/* variables */
// string - device name
#define DEVICE_NAME_VAR_RES   "/amr/deviceName"

// string - device value
#define DEVICE_ENERGY_VAR_RES "/amr/deviceEnergy"


//-------------------------------------------------------------------------------------------------
/**
 * AVC related variable and update timer
 */
//-------------------------------------------------------------------------------------------------
// reference timer for app session
le_timer_Ref_t sessionTimer;
//reference to AVC event handler
le_avdata_SessionStateHandlerRef_t  avcEventHandlerRef = NULL;
//reference to AVC Session handler
le_avdata_RequestSessionObjRef_t sessionRef = NULL;
//reference to temperature update timer
le_timer_Ref_t tempUpdateTimerRef = NULL;
//reference to push asset data timer
le_timer_Ref_t serverUpdateTimerRef = NULL;


//-------------------------------------------------------------------------------------------------
static char* DeviceNameVar;
static char* DeviceEnergyVar = "";

// [Update Energy status]
//-------------------------------------------------------------------------------------------------
/**
 * Push ack callback handler
 * This function is called whenever push has been performed successfully in AirVantage server
 */
//-------------------------------------------------------------------------------------------------

void UpdateEnergyStatus()
{
    //Energy
	LE_INFO("Update Energy is %s",  DeviceEnergyVar);
	le_result_t resultSetDeviceEnergy = le_avdata_SetString(DEVICE_ENERGY_VAR_RES, DeviceEnergyVar);
	if (LE_FAULT == resultSetDeviceEnergy)
	{
		LE_ERROR("Error in getting latest DEVICE_ENERGY_VAR_RES");
	}
}


//-------------------------------------------------------------------------------------------------
/**
 * Asset data push
 */
//-------------------------------------------------------------------------------------------------

// [PushCallbackHandler]
//-------------------------------------------------------------------------------------------------
/**
 * Push ack callback handler
 * This function is called whenever push has been performed successfully in AirVantage server
 */
//-------------------------------------------------------------------------------------------------
static void PushCallbackHandler
(
    le_avdata_PushStatus_t status,
    void* contextPtr
)
{
    switch (status)
    {
        case LE_AVDATA_PUSH_SUCCESS:
           // LE_INFO("Legato assetdata push successfully");
            break;
        case LE_AVDATA_PUSH_FAILED:
            LE_INFO("Legato assetdata push failed");
            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Push ack callback handler
 * This function is called every 10 seconds to push the data and update data in AirVantage server
 */
//-------------------------------------------------------------------------------------------------
void PushResources()
{
    // if session is still open, push the values
    if (NULL != avcEventHandlerRef)
    {

        le_result_t resultPushDeviceEnergy;
        resultPushDeviceEnergy = le_avdata_Push(DEVICE_ENERGY_VAR_RES, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushDeviceEnergy)
        {
            LE_ERROR("Error pushing DEVICE_ENERGY_VAR_RES");
        }


    }
}
// [PushCallbackHandler]

//-------------------------------------------------------------------------------------------------
/**
 * Function relevant to AirVantage server connection
 */
//-------------------------------------------------------------------------------------------------
static void sig_appTermination_cbh(int sigNum)
{
    LE_INFO("Close AVC session");
    le_avdata_ReleaseSession(sessionRef);
    if (NULL != avcEventHandlerRef)
    {
        //unregister the handler
        LE_INFO("Unregister the session handler");
        le_avdata_RemoveSessionStateHandler(avcEventHandlerRef);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Status handler for avcService updates
 */
//-------------------------------------------------------------------------------------------------
static void avcStatusHandler
(
    le_avdata_SessionState_t updateStatus,
    void* contextPtr
)
{
    switch (updateStatus)
    {
        case LE_AVDATA_SESSION_STARTED:
            LE_INFO("Legato session started successfully");
            break;
        case LE_AVDATA_SESSION_STOPPED:
            LE_INFO("Legato session stopped");
            sleep(5);
            LE_INFO("Try recovery");

            le_avdata_RequestSessionObjRef_t sessionRequestRef = le_avdata_RequestSession();
			if (NULL == sessionRequestRef)
			{
				LE_ERROR("AirVantage Connection Controller does not start.");
			}else{
				sessionRef=sessionRequestRef;
				LE_INFO("AirVantage Connection Controller started.");
			}

            break;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Monitor TCP Socket
 */
//-------------------------------------------------------------------------------------------------
static void SocketEventHandler
(
    int fd
)
{
    struct sockaddr_in clientAddress;

    int connFd;

    socklen_t addressLen = sizeof(clientAddress);

    connFd = accept(fd, (struct sockaddr *)&clientAddress, &addressLen);

    if (connFd == -1)
    {
        LE_ERROR("accept error: %m");
    }
    else
    {
    	char buf[128];
    	memset(buf,'\0',sizeof(buf));
    	read(connFd, buf, sizeof(buf));
    	LE_INFO("Got Data from Wifi");
    	LE_INFO(buf);

    	DeviceEnergyVar = "";
    	DeviceEnergyVar = malloc(sizeof(buf)+1);
    	snprintf(DeviceEnergyVar,sizeof(buf),"%s",buf);

    	UpdateEnergyStatus();
    	PushResources();

        close(connFd);

    }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function call the appropriate handler on event reception
 *
 */
//--------------------------------------------------------------------------------------------------
static void SocketListenerHandler
(
    int fd,
    short events
)
{
    if (events & POLLERR)
    {
        LE_ERROR("socket Error");
    }

    if (events & POLLIN)
    {
        SocketEventHandler(fd);
    }
}


    // [StartAVCSession]
COMPONENT_INIT
{
    LE_INFO("Start Legato AssetDataApp");

    le_sig_Block(SIGTERM);
    le_sig_SetEventHandler(SIGTERM, sig_appTermination_cbh);

    //Start AVC Session
    //Register AVC handler
    avcEventHandlerRef = le_avdata_AddSessionStateHandler(avcStatusHandler, NULL);
    //Request AVC session. Note: AVC handler must be registered prior starting a session
    le_avdata_RequestSessionObjRef_t sessionRequestRef = le_avdata_RequestSession();
    if (NULL == sessionRequestRef)
    {
        LE_ERROR("AirVantage Connection Controller does not start.");
    }else{
        sessionRef=sessionRequestRef;
        LE_INFO("AirVantage Connection Controller started.");
    }

    // [CreateTimer]
    // [CreateResources]
    LE_INFO("Create instances AssetData ");
    le_result_t resultCreateDeviceName;
    resultCreateDeviceName = le_avdata_CreateResource(DEVICE_NAME_VAR_RES,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateDeviceName)
    {
        LE_ERROR("Error in creating DEVICE_NAME_VAR_RES");
    }
    le_result_t resultCreateDeviceEnergy;
    resultCreateDeviceEnergy = le_avdata_CreateResource(DEVICE_ENERGY_VAR_RES,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateDeviceEnergy)
    {
        LE_ERROR("Error in creating DEVICE_ENERGY_VAR_RES");
    }


    // [CreateResources]
    // [AssignValues]
    //setting the variable initial value

    DeviceNameVar = "Room1";
    DeviceEnergyVar = "Room1";

    le_result_t resultSetRoomName = le_avdata_SetString(DEVICE_NAME_VAR_RES, DeviceNameVar);
    if (LE_FAULT == resultSetRoomName)
    {
        LE_ERROR("Error in setting ROOM_NAME_VAR_RES");
    }

    le_result_t resultSetEnergy = le_avdata_SetString(DEVICE_ENERGY_VAR_RES, DeviceEnergyVar);
    if (LE_FAULT == resultSetEnergy)
    {
        LE_ERROR("Error in set energy");
    }

    struct sockaddr_in myAddress;
    int ret, optVal = 1;
    int sockFd;

    // Create the socket
	sockFd = socket (AF_INET, SOCK_STREAM, 0);

	if (sockFd < 0)
	{
		LE_ERROR("creating socket failed: %m");
		return;
	}

	// set socket option
	// we use SO_REUSEADDR to be able to accept several clients without closing the socket
	ret = setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
	if (ret)
	{
		LE_ERROR("error setting socket option %m");
		close(sockFd);
		return;
	}

	memset(&myAddress, 0, sizeof(myAddress));

	myAddress.sin_port = htons(FWUPDATE_SERVER_PORT);
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind server - socket
	ret = bind(sockFd, (struct sockaddr_in *)&myAddress, sizeof(myAddress));
	if (ret)
	{
		LE_ERROR("bind failed %m");
		close(sockFd);
		return;
	}

	// Listen on the socket
	ret = listen(sockFd, 1);
	if (ret)
	{
		LE_ERROR("listen error: %m");
		close(sockFd);
		return;
	}

    le_fdMonitor_Create("energyUpdateStatus", sockFd, SocketListenerHandler, POLLIN);
}
