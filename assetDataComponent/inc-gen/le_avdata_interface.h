

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */

/**
 * @page c_le_avdata AirVantage Data API
 *
 * @ref le_avdata_interface.h "API Reference" <br>
 * @ref avData "How To Manage Data"
 *
 * This API provides a data service to allow Apps to manage App-specific data on the AirVantage
 * server.
 *
 * @section le_avdata_binding IPC Interfaces Binding
 *
 * All the functions of this API are provided by the @b avcService platform service.
 *
 * Code sample for binding to avcService:
 * @verbatim
 bindings:
 {
    clientExe.clientComponent.le_avdata -> avcService.le_avdata
 }
 @endverbatim
 *
 *
 * @section le_avdata_overview AirVantage Data Overview
 *
 * Data is setup as @b Assets &mdash;  a collection of fields that can be managed by the AirVantage
 * server.
 *
 * An @b asset @b field is a single data point taken from a sensor that can be managed by the
 * AirVantage server.
 *
 * A field can be:
 * - @b variable allowing the AirVantage server to read the value, and can be read/writtten to by an App.
 * - @b setting allowing the AirVantage server to read/write the value, and can be read/written to by an App.
 * - @b command allowing the AirVantage server to invoke a function in the App.
 *
 * Fields are referred to by paths and need to follow these rules:
 *
 * - A parent path @b can't contain a field, parent path or child path that has the same name.
 * - Paths must be separated by a slash ("/") or a dot (".").
 * - When using a ("/") the path must start with a ("/") (e.g., /a/b/c)
 * - When using a (".") the path must start with the first field, (e.g., a.b.c)
 *
 * @note There is no leading "." when using "." delimiter.
 *
 * Variable and setting fields also have types. The available field types are:
 * - string
 * - integer
 * - float
 * - boolean
 *
 * Variable and setting fields have no default values. When they are first created with the
 * le_avdata_CreateResource() , all values are "null". They can also be set to "null" values with
 * the le_avdata_SetNull() .
 *
 * Fields do not have a fixed data type, so any of the SetInt/Float/Bool/String can
 * be called for a certain field to change its value and its type. However, A
 * GetInt/Float/Bool/String API must be called on a field with the matching type. In other words,
 * a Get API does not perform type-casting.
 *
 * @note If a user enters a value 0 for float data type, an error will be returned (LE_NOT_FOUND),
 * as the system infers 0 as an integer value and the data type doesn't match.  0.0 needs to be set
 * for the float data type to be zero.
 *
 * @section le_avdata_field Field Values and Activity
 *
 * Set functions are available to set field values (including "null"). Get functions are
 * available to get field values.
 *
 * An App can register a handler so that it can be called when an activity occurs on a field.
 * This is optional for variable and setting fields, but is @b required for command fields.
 * Registered handlers are called only when activities from AV server occurs. Client activities do
 * not trigger handlers.
 *
 * A handler registered with a command field is invoked with an optional argument list sent from the
 * AirVantage server. The APIs GetInt/Float/Bool/StringArg and le_avdata_GetStringArgLength() are
 * available to extract the arguments in the handler definition. AV server does not send argument
 * lists for handlers registered with variable and setting fields.
 *
 * A handler registered with a command field must call the le_avdata_ReplyExecResult() at the end of
 * its definition, in order to reply the command execution result to the AV server.
 *
 * Sometimes instead of waiting for activity to occur on a field, users may want to have their
 * application notify the server of their asset data details. Asset data can also be pushed from
 * the device to the server by using le_avdata_Push().
 *
 * This code sample shows how to push asset data to the server (assuming session is opened)
 *
 * @code
 * static void PushCallbackHandler
 * (
 *     le_avdata_PushStatus_t status,
 *     void* contextPtr
 * )
 * {
 *     if (status == LE_AVDATA_PUSH_SUCCESS)
 *     {
 *         // data pushed successfully
 *     }
 * }
 *
 * COMPONENT_INIT
 * {
 *       le_result_t result;
 *       result = le_avdata_CreateResource("/livingRoom/sensor1", LE_AVDATA_ACCESS_VARIABLE);
 *       if (result != LE_OK)
 *       {
 *           LE_FATAL("Error in creating livingRoom resource.");
 *       }
 *
 *       result = le_avdata_SetInt("/livingRoom/sensor1", 5) == LE_OK);
 *       if (result != LE_OK)
 *       {
 *           LE_FATAL("Failed to set value for livingRoom resource.");
 *       }
 *
 *       le_avdata_Push("/livingRoom/sensor1", PushCallbackHandler, NULL);
 * }
 * @endcode
 *
 * If users simply want to push a data dump to the server without creating resources,
 * le_avdata_PushStream() is available.
 * @note The push stream API has a limit of 20K.
 *
 * @code
 * COMPONENT_INIT
 * {
 *       int fd = open("data.txt", O_RDONLY);
 *       if (fd == -1)
 *       {
 *          LE_FATAL("Failed to open file.");
 *       }
 *
 *       // The data dump sent to the server will be display under <Path>
 *       le_avdata_PushStream("<Path>", fd, PushCallbackHandler, NULL);
 * }
 * @endcode
 *
 * @section le_avdata_namespace Namespace
 *
 * By default all asset data paths are created and managed under the application namespace.
 * Calling @c le_avdata_GetXXX / @c le_avdata_SetXXX/ le_avdata_AddResourceEventHandler() /
 * le_avdata_Push() with the path "/a/b/c" will search for the "/a/b/c" data under the apps
 * namespace. In order to query any paths created under an application namespace, user must append
 * the app name in front of the request on the server side. If creating asset data under application
 * namespace is not desired, users must call le_avdata_SetNamespace() and set the value to global.
 * An application that changes its namespace to global will be creating asset data in the global
 * space. All asset data API will use the last namespace set until the function is called again and
 * the namespace is set to something new.
 *
 * @note All asset data paths created by the application are deleted when it is stopped, even
 * the asset data created under global namespace. No asset data can therefore be persistent after
 * an application restart, regardless of the namespace used to create it.
 *
 * Example:
 *
 * @code
 * #define RESOURCE_A         "/test/resourceA"
 *
 * COMPONENT_INIT
 * {
 *     int intVal;
 *     LE_ASSERT(le_avdata_CreateResource(RESOURCE_A, LE_AVDATA_ACCESS_VARIABLE) == LE_OK);
 *     LE_ASSERT(le_avdata_SetInt(RESOURCE_A, 1) == LE_OK);
 *     LE_ASSERT(le_avdata_GetInt(RESOURCE_A, &intVal) == LE_OK);
 *     LE_ASSERT(intVal == 1); // value obtained under the application namespace
 *
 *     // Switch to global namespace
 *     LE_ASSERT(le_avdata_SetNamespace(LE_AVDATA_NAMESPACE_GLOBAL) == LE_OK);
 *
 *     LE_ASSERT(le_avdata_CreateResource(RESOURCE_A, LE_AVDATA_ACCESS_VARIABLE) == LE_OK);
 *     LE_ASSERT(le_avdata_SetInt(RESOURCE_A, 2) == LE_OK);
 *     LE_ASSERT(le_avdata_GetInt(RESOURCE_A, &intVal) == LE_OK);
 *     LE_ASSERT(intVal == 2); // value obtained under the global namespace
 * }
 * @endcode
 *
 * An application starts by creating a path "/test/resourceA" under the default application
 * namespace and sets it to 1. Then it sets the namespace to global and creates another path
 * "/test/resourceA" with a value of 2. A read request from the server on "<appName>/test/resourceA"
 * will return 1; whereas a read request on "/test/resourceA" will return 2.
 *
 * @section le_avdata_timeseries Time Series
 *
 * This feature enables user Apps to collect and keep in memory data when the device is off-line and
 * send the data to the AirVantage Server when the device is on-line.
 *
 * Time series records can be initialized using le_avdata_CreateRecord().
 * Data can be accumulated using the following data types along with a specified time stamp
 * (milliseconds elapsed since epoch).
 * - le_avdata_RecordInt()
 * - le_avdata_RecordFloat()
 * - le_avdata_RecordBool()
 * - le_avdata_RecordString()
 *
 * User apps can then open an @c avms session, and push the collected history data using
 * le_avdata_PushRecord(). The callback used when calling le_avdata_PushRecord() will indicate
 * whether the push has been successful or not.
 *
 * This code sample shows how to collect data and send to the server (assuming session is opened)
 *
 * @code
 *
 * static void PushCallbackHandler
 * (
 *     le_avdata_PushStatus_t status,
 *     void* contextPtr
 * )
 * {
 *     if (status == LE_AVDATA_PUSH_SUCCESS)
 *     {
 *         // data pushed successfully
 *     }
 * }
 *
 * static void SendData()
 * {
 *     struct timeval tv;
 *     uint64_t utcMilliSec;
 *     le_result_t result;
 *
 *     le_avdata_RecordRef_t recRef = le_avdata_CreateRecord();
 *
 *     gettimeofday(&tv, NULL);
 *     utcMilliSec = (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000; // get current time
 *     result = le_avdata_RecordFloat(recRef, "speed", 0.08, utcMilliSec);
 *
 *     if (result == LE_OK)
 *     {
 *          le_avdata_PushRecord(recRef, PushCallbackHandler, NULL);
 *     }
 *
 *     le_avdata_DeleteRecord(recRef);
 * }
 * @endcode
 *
 * @section le_avdata_session User App Session Management
 *
 * An App can request to open a session and register a handler to get notified of a session
 * events. le_avdata_RequestSession() and le_avdata_ReleaseSession() can be used to
 * open a session and close a session respectively. If the session was initiated by an
 * user app, the session will be closed when all apps release their session reference.
 * le_avdata_AddSessionStateHandler() and le_avdata_RemoveSessionStateHandler() can be used to add
 * and remove notification handlers.
 *
 * @section le_avdata_fatal Fatal Behavior
 *
 * An invalid asset name or field name is treated as a fatal error (i.e. non-recoverable)
 * and will result in the client App being terminated.
 *
 * Copyright (C) Sierra Wireless Inc.
 */
/**
 * @file le_avdata_interface.h
 *
 * Legato @ref c_le_avdata include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#ifndef LE_AVDATA_INTERFACE_H_INCLUDE_GUARD
#define LE_AVDATA_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"


//--------------------------------------------------------------------------------------------------
/**
 * Type for handler called when a server disconnects.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_avdata_DisconnectHandler_t)(void *);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Connect the current client thread to the service providing this API. Block until the service is
 * available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_ConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Try to connect the current client thread to the service providing this API. Return with an error
 * if the service is not available.
 *
 * For each thread that wants to use this API, either ConnectService or TryConnectService must be
 * called before any other functions in this API.  Normally, ConnectService is automatically called
 * for the main thread, but not for any other thread. For details, see @ref apiFilesC_client.
 *
 * This function is created automatically.
 *
 * @return
 *  - LE_OK if the client connected successfully to the service.
 *  - LE_UNAVAILABLE if the server is not currently offering the service to which the client is
 *    bound.
 *  - LE_NOT_PERMITTED if the client interface is not bound to any service (doesn't have a binding).
 *  - LE_COMM_ERROR if the Service Directory cannot be reached.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_TryConnectService
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Set handler called when server disconnection is detected.
 *
 * When a server connection is lost, call this handler then exit with LE_FATAL.  If a program wants
 * to continue without exiting, it should call longjmp() from inside the handler.
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_SetServerDisconnectHandler
(
    le_avdata_DisconnectHandler_t disconnectHandler,
    void *contextPtr
);

//--------------------------------------------------------------------------------------------------
/**
 *
 * Disconnect the current client thread from the service providing this API.
 *
 * Normally, this function doesn't need to be called. After this function is called, there's no
 * longer a connection to the service, and the functions in this API can't be used. For details, see
 * @ref apiFilesC_client.
 *
 * This function is created automatically.
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_DisconnectService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * Define the maximum characters and bytes of a path name
 */
//--------------------------------------------------------------------------------------------------
#define LE_AVDATA_PATH_NAME_LEN 511

//--------------------------------------------------------------------------------------------------
/**
 */
//--------------------------------------------------------------------------------------------------
#define LE_AVDATA_PATH_NAME_BYTES 512

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximum characters and bytes of a string
 */
//--------------------------------------------------------------------------------------------------
#define LE_AVDATA_STRING_VALUE_LEN 255

//--------------------------------------------------------------------------------------------------
/**
 */
//--------------------------------------------------------------------------------------------------
#define LE_AVDATA_STRING_VALUE_BYTES 256

//--------------------------------------------------------------------------------------------------
/**
 * Resource access modes:
 *   - Variable: read (server), read/write (client)
 *   - Setting: read/write (server), read/write (client)
 *   - Command: execute (server)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_AVDATA_ACCESS_VARIABLE = 0,
        ///< read(server) or read/write(client)
    LE_AVDATA_ACCESS_SETTING = 1,
        ///< read/write (server) or read/write (client)
    LE_AVDATA_ACCESS_COMMAND = 2
        ///< execute (server)
}
le_avdata_AccessMode_t;


//--------------------------------------------------------------------------------------------------
/**
 * Resource access types
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_AVDATA_ACCESS_READ = 0x1,        ///< read access type
    LE_AVDATA_ACCESS_WRITE = 0x2,        ///< write access type
    LE_AVDATA_ACCESS_EXEC = 0x4        ///< execute access type
}
le_avdata_AccessType_t;


//--------------------------------------------------------------------------------------------------
/**
 * Resource namespace
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_AVDATA_NAMESPACE_APPLICATION = 0,
        ///<
    LE_AVDATA_NAMESPACE_GLOBAL = 1
        ///<
}
le_avdata_Namespace_t;


//--------------------------------------------------------------------------------------------------
/**
 * Data types
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_AVDATA_DATA_TYPE_NONE = 0,
        ///< Null Data Type
    LE_AVDATA_DATA_TYPE_INT = 1,
        ///< Integer Data Type
    LE_AVDATA_DATA_TYPE_FLOAT = 2,
        ///< Float Data Type
    LE_AVDATA_DATA_TYPE_BOOL = 3,
        ///< Boolean Data Type
    LE_AVDATA_DATA_TYPE_STRING = 4
        ///< String Data Type
}
le_avdata_DataType_t;


//--------------------------------------------------------------------------------------------------
/**
 * Status of the data push
 * @todo Provide additional status to troubleshoot delivery problems
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_AVDATA_PUSH_SUCCESS = 0,
        ///< Push was successful
    LE_AVDATA_PUSH_FAILED = 1
        ///< Push has failed
}
le_avdata_PushStatus_t;


//--------------------------------------------------------------------------------------------------
/**
 * Argument list reference, for command only.
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_avdata_ArgumentList* le_avdata_ArgumentListRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'le_avdata_ResourceEvent'
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_avdata_ResourceEventHandler* le_avdata_ResourceEventHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * A record reference
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_avdata_Record* le_avdata_RecordRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference returned by RequestSession function and used by ReleaseSession function
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_avdata_RequestSessionObj* le_avdata_RequestSessionObjRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * AVMS session state
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_AVDATA_SESSION_STARTED = 0,
        ///< AVMS session started
    LE_AVDATA_SESSION_STOPPED = 1
        ///< AVMS session stopped
}
le_avdata_SessionState_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'le_avdata_SessionState'
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_avdata_SessionStateHandler* le_avdata_SessionStateHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Handler for resource activity. Note that the path returned by this event follows the unix format
 * (e.g. "/a/b/c") even if the app uses the URL format (e.g "a.b.c") when registering
 * the handler.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_avdata_ResourceHandlerFunc_t)
(
    const char* LE_NONNULL path,
        ///<
    le_avdata_AccessType_t accessType,
        ///<
    le_avdata_ArgumentListRef_t argumentListRef,
        ///<
    void* contextPtr
        ///<
);

//--------------------------------------------------------------------------------------------------
/**
 * Handler for pushing data result.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_avdata_CallbackResultFunc_t)
(
    le_avdata_PushStatus_t status,
        ///<
    void* contextPtr
        ///<
);

//--------------------------------------------------------------------------------------------------
/**
 * Handler for AV session changes
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_avdata_SessionStateHandlerFunc_t)
(
    le_avdata_SessionState_t sessionState,
        ///< Session started or stopped?
    void* contextPtr
        ///<
);

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_avdata_ResourceEvent'
 *
 * Upon resource access on the server side, the registered handler is called.
 *
 * For "settings" (read/write), the same handler is called for both read and write access.
 *
 * For "commands", the handler function must call the "ReplyExecResult" function to send the command
 * execution result back to the AVC daemon (which then sends the proper response back to the AV
 * server).
 */
//--------------------------------------------------------------------------------------------------
le_avdata_ResourceEventHandlerRef_t le_avdata_AddResourceEventHandler
(
    const char* LE_NONNULL path,
        ///< [IN]
    le_avdata_ResourceHandlerFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'le_avdata_ResourceEvent'
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_RemoveResourceEventHandler
(
    le_avdata_ResourceEventHandlerRef_t handlerRef
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Create an asset data with the provided path. Note that asset data type and value are determined
 * upon the first call to a Set function. When an asset data is created, it contains a null value,
 * represented by the data type of none.
 *
 * @return:
 *      - LE_OK on success
 *      - LE_DUPLICATE if path has already been called by CreateResource before, or path is parent
 *        or child to an existing Asset Data path.
 *      - LE_FAULT on any other error.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_CreateResource
(
    const char* LE_NONNULL path,
        ///< [IN]
    le_avdata_AccessMode_t accessMode
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets the namespace for asset data.
 *
 * @return:
 *      - LE_OK on success
 *      - LE_BAD_PARAMETER if the namespace is unknown
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_SetNamespace
(
    le_avdata_Namespace_t namespace
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets an asset data to contain a null value, represented by the data type of none.
 *
 * @return:
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_SetNull
(
    const char* LE_NONNULL path
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Gets the integer value of an asset data.
 *
 * @return:
 *      - LE_BAD_PARAMETER - asset data being accessed is of the wrong data type
 *      - LE_UNAVAILABLE - asset data contains null value
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetInt
(
    const char* LE_NONNULL path,
        ///< [IN]
    int32_t* valuePtr
        ///< [OUT]
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets an asset data to an integer value.
 *
 * @return:
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_SetInt
(
    const char* LE_NONNULL path,
        ///< [IN]
    int32_t value
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Gets the float value of an asset data.
 *
 * @return:
 *      - LE_BAD_PARAMETER - asset data being accessed is of the wrong data type
 *      - LE_UNAVAILABLE - asset data contains null value
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetFloat
(
    const char* LE_NONNULL path,
        ///< [IN]
    double* valuePtr
        ///< [OUT]
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets an asset data to a float value.
 *
 * @return:
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_SetFloat
(
    const char* LE_NONNULL path,
        ///< [IN]
    double value
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Gets the bool value of an asset data.
 *
 * @return:
 *      - LE_BAD_PARAMETER - asset data being accessed is of the wrong data type
 *      - LE_UNAVAILABLE - asset data contains null value
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetBool
(
    const char* LE_NONNULL path,
        ///< [IN]
    bool* valuePtr
        ///< [OUT]
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets an asset data to a bool value.
 *
 * @return:
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_SetBool
(
    const char* LE_NONNULL path,
        ///< [IN]
    bool value
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Gets the string value of an asset data.
 *
 * @return:
 *      - LE_BAD_PARAMETER - asset data being accessed is of the wrong data type
 *      - LE_UNAVAILABLE - asset data contains null value
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OVERFLOW - asset data length exceeds the maximum length
 *      - LE_OK - access successful
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetString
(
    const char* LE_NONNULL path,
        ///< [IN]
    char* value,
        ///< [OUT]
    size_t valueSize
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Sets an asset data to a string value.
 *
 * @return:
 *      - LE_NOT_FOUND - if the path is invalid and does not point to an asset data
 *      - LE_NOT_PERMITTED - asset data being accessed does not have the right permission
 *      - LE_OK - access successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_SetString
(
    const char* LE_NONNULL path,
        ///< [IN]
    const char* LE_NONNULL value
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the bool argument with the specified name.
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NOT_FOUND if argument doesn't exist, or its data type doesn't match the API.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetBoolArg
(
    le_avdata_ArgumentListRef_t argumentListRef,
        ///< [IN]
    const char* LE_NONNULL argName,
        ///< [IN]
    bool* boolArgPtr
        ///< [OUT]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the float argument with the specified name.
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NOT_FOUND if argument doesn't exist, or its data type doesn't match the API.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetFloatArg
(
    le_avdata_ArgumentListRef_t argumentListRef,
        ///< [IN]
    const char* LE_NONNULL argName,
        ///< [IN]
    double* floatArgPtr
        ///< [OUT]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the int argument with the specified name.
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NOT_FOUND if argument doesn't exist, or its data type doesn't match the API.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetIntArg
(
    le_avdata_ArgumentListRef_t argumentListRef,
        ///< [IN]
    const char* LE_NONNULL argName,
        ///< [IN]
    int32_t* intArgPtr
        ///< [OUT]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the string argument with the specified name.
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NOT_FOUND if argument doesn't exist, or its data type doesn't match the API
 *      - LE_OVERFLOW - argument length exceeds the maximum length
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetStringArg
(
    le_avdata_ArgumentListRef_t argumentListRef,
        ///< [IN]
    const char* LE_NONNULL argName,
        ///< [IN]
    char* strArg,
        ///< [OUT]
    size_t strArgSize
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the length (excluding terminating null byte) of the string argument of the specified name.
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NOT_FOUND if argument doesn't exist, or its data type doesn't match the API.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_GetStringArgLength
(
    le_avdata_ArgumentListRef_t argumentListRef,
        ///< [IN]
    const char* LE_NONNULL argName,
        ///< [IN]
    int32_t* strArgLenPtr
        ///< [OUT]
);

//--------------------------------------------------------------------------------------------------
/**
 * Reply command execution result to AVC Daemon, which can then respond to AV server. This function
 * MUST be called at the end of a command execution, in order for AV server to be notified about the
 * execution status.
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_ReplyExecResult
(
    le_avdata_ArgumentListRef_t argumentListRef,
        ///< [IN]
    le_result_t result
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Push asset data to the server.
 *
 * @return:
 *      - LE_OK on success.
 *      - LE_NOT_FOUND if path doesn't exist.
 *      - LE_BUSY if push is queued and will pushed later automatically
 *      - LE_NOT_POSSIBLE if push queue is full, try again later
 *      - LE_FAULT on any other error
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_Push
(
    const char* LE_NONNULL path,
        ///< [IN]
    le_avdata_CallbackResultFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Push data dump to a specified path on the server.
 *
 * @return:
 *      - LE_OK on success.
 *      - LE_NOT_POSSIBLE if the service is busy pushing other data, try again later
 *      - LE_FAULT on any other error
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_PushStream
(
    const char* LE_NONNULL path,
        ///< [IN]
    int fd,
        ///< [IN]
    le_avdata_CallbackResultFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Create a timeseries record
 *
 * @return Reference to the record
 */
//--------------------------------------------------------------------------------------------------
le_avdata_RecordRef_t le_avdata_CreateRecord
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Delete a timeseries record
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_DeleteRecord
(
    le_avdata_RecordRef_t recordRef
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Accumulate int data
 *
 * @note The client will be terminated if the recordRef is not valid, or the resource doesn't exist
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NO_MEMORY if the current entry was NOT added because the time series buffer is full.
 *      - LE_FAULT on any other error
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_RecordInt
(
    le_avdata_RecordRef_t recordRef,
        ///< [IN]
    const char* LE_NONNULL path,
        ///< [IN]
    int32_t value,
        ///< [IN]
    uint64_t timestamp
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Accumulate float data
 *
 * @note The client will be terminated if the recordRef is not valid, or the resource doesn't exist
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NO_MEMORY if the current entry was NOT added because the time series buffer is full.
 *      - LE_FAULT on any other error
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_RecordFloat
(
    le_avdata_RecordRef_t recordRef,
        ///< [IN]
    const char* LE_NONNULL path,
        ///< [IN]
    double value,
        ///< [IN]
    uint64_t timestamp
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Accumulate boolean data
 *
 * @note The client will be terminated if the recordRef is not valid, or the resource doesn't exist
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NO_MEMORY if the current entry was NOT added because the time series buffer is full.
 *      - LE_FAULT on any other error
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_RecordBool
(
    le_avdata_RecordRef_t recordRef,
        ///< [IN]
    const char* LE_NONNULL path,
        ///< [IN]
    bool value,
        ///< [IN]
    uint64_t timestamp
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Accumulate string data
 *
 * @note The client will be terminated if the recordRef is not valid, or the resource doesn't exist
 *
 * @return:
 *      - LE_OK on success
 *      - LE_NO_MEMORY if the current entry was NOT added because the time series buffer is full.
 *      - LE_FAULT on any other error
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_RecordString
(
    le_avdata_RecordRef_t recordRef,
        ///< [IN]
    const char* LE_NONNULL path,
        ///< [IN]
    const char* LE_NONNULL value,
        ///< [IN]
    uint64_t timestamp
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Push record to the server
 *
 ** @return:
 *      - LE_OK on success.
 *      - LE_BUSY if push is queued and will pushed later automatically
 *      - LE_NOT_POSSIBLE if push queue is full, try again later
 *      - LE_FAULT on any other error
 *
 * * @note If the caller is passing a bad pointer into this function, it is a fatal error, the
 *       function will not return.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_avdata_PushRecord
(
    le_avdata_RecordRef_t recordRef,
        ///< [IN]
    le_avdata_CallbackResultFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_avdata_SessionState'
 *
 * This event provides information on AV session state changes
 */
//--------------------------------------------------------------------------------------------------
le_avdata_SessionStateHandlerRef_t le_avdata_AddSessionStateHandler
(
    le_avdata_SessionStateHandlerFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'le_avdata_SessionState'
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_RemoveSessionStateHandler
(
    le_avdata_SessionStateHandlerRef_t handlerRef
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Request the avcServer to open a session.
 *
 * @return
 *      - SessionRef if session request succeeded
 *      - NULL if session request failed
 */
//--------------------------------------------------------------------------------------------------
le_avdata_RequestSessionObjRef_t le_avdata_RequestSession
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Request the avcServer to close a session.
 *
 */
//--------------------------------------------------------------------------------------------------
void le_avdata_ReleaseSession
(
    le_avdata_RequestSessionObjRef_t requestRef
        ///< [IN] Reference to a previously opened AV session.
);

#endif // LE_AVDATA_INTERFACE_H_INCLUDE_GUARD