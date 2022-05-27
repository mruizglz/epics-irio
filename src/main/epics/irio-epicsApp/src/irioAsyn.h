/**************************************************************************//**
 * \file irioAsyn.h
 * \authors Mariano Ruiz (Universidad Politécnica de Madrid, UPM)
 * \authors Diego Sanz (Universidad Politécnica de Madrid, UPM)
 * \authors Sergio Esquembri (Universidad Politécnica de Madrid, UPM)
 * \authors Enrique Bernal (Universidad Politécnica de Madrid, UPM)
 * \authors Alvaro Bustos (Universidad Politécnica de Madrid, UPM)
 * \brief Initialization and common resources access methods for IRIOASYN Driver.
 * \date Sept., 2010 (Last Review February 2016)
 *****************************************************************************/

/**
 * @mainpage
 *
 * This document describes the NI-RIO EPICS device driver available functions.
 * This functions allow the user to initialize RIO devices
 * and interact with them to start/stop data acquisition processes.
 * Arguments of the functions are provided with the description.
 * The initialization function must be called from userPreDriver.cmd file.
 *
 */

#ifndef IRIOASYN_H_
#define IRIOASYN_H_

/* Standard includes */
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <poll.h>
#include <sys/time.h>
#include <limits.h>
/* epics */
#include <cantProceed.h>
#include <iocsh.h>
#include <epicsThread.h>
#include <epicsString.h>
#include "epicsExit.h"
#include "epicsRingBytes.h"
#include <errlog.h>
#include <epicsTime.h>
#include <registryFunction.h>
#include <epicsExport.h>

#include "alarm.h"
#include "cvtTable.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "recGbl.h"
#include "recSup.h"
#include "devSup.h"
#include "link.h"
#include "aiRecord.h"
#include "stringinRecord.h"
#include "stringoutRecord.h"
#include "mbboRecord.h"
#include "boRecord.h"

#include "locale.h"

/* asyn */
#include <asynDriver.h>
#include <asynInt32.h>
#include <asynUInt32Digital.h>
#include <asynFloat64.h>
#include <asynStandardInterfaces.h>
#include <asynCommonSyncIO.h>
#include <asynInt32ArraySyncIO.h>
#include <asynDrvUser.h>
#include <asynOctet.h>
#include <asynFloat32ArraySyncIO.h>

#include <dbStaticLib.h>
#include <initHooks.h>
#include "irioDataTypes.h"

/** @name Max number of cards
 * The maximum number of RIO cards is 10.
 */
///@{
#define MAX_NUMBER_OF_CARDS 10
///@}

/** @name RIO device name
 */
///@{
#define DEVICE_NAME "NI-RIO"
///@}

/** @name IRIO Asyn driver version
 */
///@{
#define EPICS_DRIVER_VERSION "1.1.2"
///@}

/** @name Max UARTSIZE
 */
#define MAX_UARTSIZE 40
///@}

/** @name Maximun number of commands(reasons)
 *  Number of reasons used in irioasyn driver
 */
///@{
#define SR_CMDS                     3
#define DEBUG_COMMANDS              1
#define GROUPENABLE_COMMANDS        1
#define STARTSTOP_COMMANDS			2
#define CHANNELS_COMMANDS           1
#define DECIMATIONFACTOR_COMMANDS   1
#define OTHER_COMMANDS              11
#define AO_COMMANDS             	7

#define DO_COMMANDS              	1
#define DI_COMMANDS              	1
#define AI_COMMANDS             	1

#define AUXAI_COMMANDS             	1
#define AUXAO_COMMANDS             	1
#define AUXDI_COMMANDS             	1
#define AUXDO_COMMANDS             	1
#define CLCONFIG_COMMANDS			10
#define UART_COMMANDS				6
#define USER_DEFINED_CONVERSION_FACTOR  1
#define AUX64AI_COMMANDS             	1
#define AUX64AO_COMMANDS             	1


#define MAX_IRIO_COMMANDS    (SR_CMDS+ DEBUG_COMMANDS+GROUPENABLE_COMMANDS+ STARTSTOP_COMMANDS+\
								CHANNELS_COMMANDS+ DECIMATIONFACTOR_COMMANDS+OTHER_COMMANDS + AO_COMMANDS + DO_COMMANDS + DI_COMMANDS+ AI_COMMANDS+ \
								AUXAI_COMMANDS+ +AUXAO_COMMANDS+ AUXDI_COMMANDS+ AUXDO_COMMANDS+ CLCONFIG_COMMANDS + UART_COMMANDS + USER_DEFINED_CONVERSION_FACTOR+ AUX64AI_COMMANDS+AUX64AO_COMMANDS)
///@}

#define MIN(a,b) ((a<b) ? a: b)

/* @name Constants that define out of bound errors and board status.
 *
 */
///@{
#define MAX_ERROR 9
#define MAX_ERROR_OOB 11
#define WAIT_MS 1000
///@}



/**
 * Struct to store I/O Intr records
 */
typedef struct {
	char *type;
	char *name;
	char *input;
	int addr;
	char *portName;
	char *reason;
	char *scan;
	int nelm;
	int timestamp_source;
}intr_records_t;

/**
 * Enum Type for out of bound errors.
 */
typedef enum{
	ERROR_OOB_DF=0, 		//!< Decimation Factor out of bound error
	ERROR_OOB_SR, 			//!< Sampling Rate out of bound error
	ERROR_OOB_SR_AI_INTR, 	//!< Sampling Rate for AI I/O Intr data acquisition, out of bound error
	ERROR_OOB_SR_DI_INTR, 	//!< Sampling Rate for DI I/O Intr data acquisition, out of bound error
	ERROR_OOB_CLSIZEX, 		//!< Camera Link SizeX out of bound error
	ERROR_OOB_CLSIZEY, 		//!< Camera Link SizeY out of bound error
	ERROR_OOB_AO, 			//!< Analog Output out of bound error
	ERROR_OOB_SGUPDATERATE, //!< Signal Generator UpdateRate out of bound error
	ERROR_OOB_SGAMP, 		//!< Signal Generator signal Amplitude out of bound error
	ERROR_OOB_SGFREQ, 		//!< signal Generator signal Frequency out of bound error
	ERROR_OOB_USER_DEFINED_CONVERSION_FACTOR //!< User defined conversion factor out of bounds error

}error_oob_name_t;

/**
 * Enum Type of RIO device status.
 */
typedef enum {
	STATUS_OK = 0, 						//!< RIO Device running OK.
    STATUS_INITIALIZING, 				//!< RIO Device initializing.
	STATUS_RESETTING,   				//!< N/A
    STATUS_HARDWARE_ERROR, 				//!< NIRIO_API_Error.
	STATUS_NO_BOARD,   					//!< No RIO device found connected.
	STATUS_STATIC_CONFIG_ERROR, 		//!< BitfileDownload_Error, ListRIODevicesCommand_Error, ListRIODevicesParsing_Error, SignatureNotFound_Error, MemoryAllocation_Error,
										//!< FileAccess_Error, FileNotFound_Error, FeatureNotImplemented_Error, FeatureNotImplemented_Error, ResourceValueNotValid_Error, BitfileNotFound_Error, HeaderNotFound_Error.
	STATUS_DYNAMIC_CONFIG_ERROR, 		//!< Parameters configuration error. Values out of bound.
	STATUS_CONFIGURED, 					//!< nirioinit ok, waitting to start FPGA.
	STATUS_INCORRECT_HW_CONFIGURATION, 	//!< InitDone_Error, IOModule_Error, ResourceNotFound_Error.
}riodevice_status_name_t;

/**
 * Struct to store RIO device status enum and string.
 */
typedef struct {
	riodevice_status_name_t riodevice_status_name;  //!< RIO device status enum.
	const char *riodevice_status_string;			//!< RIO device status string.
} err_dev_stat_struct_t;

/**
 * Array of RIO device {enum,string}
 */
static err_dev_stat_struct_t err_dev_stat_struct[MAX_ERROR] = {
    {STATUS_OK,"STATUS_OK"},
    {STATUS_INITIALIZING,"STATUS_INITIALIZING"},
    {STATUS_RESETTING, "STATUS_RESETTING"},
    {STATUS_HARDWARE_ERROR, "STATUS_HARDWARE_ERROR"},
	{STATUS_NO_BOARD,"STATUS_NO_BOARD"},
    {STATUS_STATIC_CONFIG_ERROR,"STATUS_STATIC_CONFIG_ERROR"},
    {STATUS_DYNAMIC_CONFIG_ERROR,"STATUS_DYNAMIC_CONFIG_ERROR"},
    {STATUS_CONFIGURED,"STATUS_CONFIGURED"},
	{STATUS_INCORRECT_HW_CONFIGURATION,"STATUS_INCORRECT_HW_CONFIGURATION"}
};

/**
 * Enum Type of RIO device platform-profiles
 */
typedef enum {
	flexRIO_DAQ=0,		//!< FlexRIO DMA data acquisition profile
	flexRIO_IMAQ,		//!< FlexRIO Image data acquisition profile
	flexRIO_DAQ_GPU,	//!< FlexRIO GPU DMA data acquisition profile
	flexRIO_IMAGE_GPU,	//!< FlexRIO GPU Image data acquisition profile
	cRIO_DAQ,			//!< cRIO DMA data acquisition profile
	cRIO_IO,			//!< cRIO input/output data acquisition profile
}platform_profile_enumt;

/**
 * Struct to store RIO device platform-profiles enum and strings.
 */
typedef struct {
	platform_profile_enumt platform_profile_name; 	//!< RIO device platforn-profile enum
	const char *platform_profile_string;			//!< RIO device platforn-profile string
} platform_profile_t;

/**
 * Array of RIO device platform-profiles {enum,string}
 */
static platform_profile_t platform_profile[6]= {
	{flexRIO_DAQ,"flexRIO DMA Data Acquisition profile."},
	{flexRIO_IMAQ,"flexRIO IMAQ Data Acquisition profile."},
	{flexRIO_DAQ_GPU,"flexRIO GPU DMA Data Acquisition profile."},
	{flexRIO_IMAGE_GPU,"flexRIO GPU IMAGE Data Acquisition profile."},
	{cRIO_DAQ,"cRIO DMA Data Acquisition profile."},
	{cRIO_IO,"cRIO Point by Point Data Acquisition profile."},
};
/**
 * Enum Type of the different reason used by irio asyn driver
 */

typedef enum  {
	SamplingRate = 0,
	SR_AI_Intr,
	SR_DI_Intr,
	debug,
	GroupEnable,
	FPGAStart,
	DAQStartStop,
	CH,
	DF,
	riodevice_status,
	DevQualityStatus,
	DeviceTemp,
	DMAsOverflow,
	FPGAStatus,
	InfoStatus,
	VIversion,
	epics_version,
	driver_version,
	device_name,
	device_serial_number,
	AO,
	AOEnable,
	SGUpdateRate,
	SGFreq,
	SGAmp,
	SGPhase,
	SGSignalType,
	DO,
	DI,
	AI,
	auxAI,
	auxAO,
	auxDI,
	auxDO,
	CLConfiguration,
	CLSignalMapping,
	CLLineScan,
	CLFVALHigh,
	CLLVALHigh,
	CLDVALHigh,
	CLSpareHigh,
	CLControlEnable,
	CLSizeX,
	CLSizeY,
	UARTTransmit,
	UARTReceive,
	UARTBaudRate,
	UARTBreakIndicator,
	UARTFrammingError,
	UARTOverrunError,
	UserDefinedConversionFactor,
	aux64AI,
	aux64AO
}Tirio_commands;

/**
 * Struct to store all reasons enum and strings.
 */
typedef struct {
	Tirio_commands command;		//!< Reason enum
	const char *commandString;	//!< Reason string
} TirioCommandStruct;

/**
 * Array of reasons {enum,string}
 */
static TirioCommandStruct commands[MAX_IRIO_COMMANDS] = {
    {SamplingRate,"SamplingRate"},
    {SR_AI_Intr,"SR_AI_Intr"},
    {SR_DI_Intr,"SR_DI_Intr"},
    {debug,"debug"},
    {GroupEnable,"GroupEnable"},
    {FPGAStart,"FPGAStart"},
    {DAQStartStop,"DAQStartStop"},
    {CH,"CH"},
    {DF, "DF"},
	{riodevice_status,"riodevice_status"},
	{DevQualityStatus, "DevQualityStatus"},
	{DeviceTemp,"DeviceTemp"},
	{DMAsOverflow,"DMAsOverflow"},
	{FPGAStatus,"FPGAStatus"},
	{InfoStatus,"InfoStatus"},
	{VIversion,"VIversion"},
	{epics_version,"epics_version"},
	{driver_version,"driver_version"},
	{device_name,"device_name"},
	{device_serial_number,"device_serial_number"},
	{AO, "AO"},
	{AOEnable,"AOEnable"},
	{SGUpdateRate,"SGUpdateRate"},
	{SGFreq,"SGFreq"},
	{SGAmp,"SGAmp"},
	{SGPhase,"SGPhase"},
	{SGSignalType,"SGSignalType"},
	{DO, "DO"},
	{DI, "DI"},
	{AI, "AI"},
	{auxAI, "auxAI"},
	{auxAO,"auxAO"},
	{auxDI,"auxDI"},
	{auxDO,"auxDO"},
	{CLConfiguration,"CLConfiguration"},
	{CLSignalMapping,"CLSignalMapping"},
	{CLLineScan,"CLLineScan"},
	{CLFVALHigh,"CLFVALHigh"},
	{CLLVALHigh,"CLLVALHigh"},
	{CLDVALHigh,"CLDVALHigh"},
	{CLSpareHigh,"CLSpareHigh"},
	{CLControlEnable,"CLControlEnable"},
	{CLSizeX,"CLSizeX"},
	{CLSizeY, "CLSizeY"},
	{UARTTransmit,"UARTTransmit"},
	{UARTReceive,"UARTReceive"},
	{UARTBaudRate,"UARTBaudRate"},
	{UARTBreakIndicator,"UARTBreakIndicator"},
	{UARTFrammingError,"UARTFrammingError"},
	{UARTOverrunError,"UARTOverrunError"},
	{UserDefinedConversionFactor,"UserDefinedConversionFactor"},
	{aux64AI, "aux64AI"},
	{aux64AO,"aux64AO"}
};

/**
 * Struct for managing DMA thread acquisition resources
 */
typedef struct threadDMA
{
	epicsThreadId *dma_thread_id;	//!< Pointer to DMA acquisition thread ID
	char		  *dma_thread_name; //!< Pointer to DMA acquisition thread name
	int id;							//!< ID
	int threadends; 				//!< This field will be used to terminate the thread when change to 1
	int endAck;						//!< This field will be used to notify thread termination when change to 1
	int DecimationFactor; 			//!< Decimation Factor
	int SR;							//!< Sampling Rate
	int blockSize; 					//!< Size of acquisition block (in terms of DMA NwordU64)
	struct irioPvt* asynPvt;		//!< Pointer to data structure of RIO device resources
	epicsRingBytesId* IdRing;		//!< Pointer to RingBuffer ID
	int dmanumber;					//!< DMA number
} irio_dmathread_t;

/**
 * Struct for managing thread acquisition resources of AI PVs with SCAN=I/O Intr
 */
typedef struct thread_ai
{
	epicsThreadId *id;	//!< Pointer to AI acquisition thread ID
	char		  *name; //!< Pointer to AI acquisition thread name
	int threadends; 				//!< This field will be used to terminate the thread when change to 1
	int endAck;						//!< This field will be used to notify thread termination when change to 1
	int SR;							//!< Sampling Rate
	int portNumber;
	struct irioPvt* asynPvt;		//!< Pointer to data structure of RIO device resources
	int *timestamp_source;			//!< Pointer to array with timestamp sources of PVs.
} thread_ai_t;

/**
 * Struct for managing thread acquisition resources of DI PVs with SCAN=I/O Intr
 */
typedef struct thread_di
{
	epicsThreadId *id;	//!< Pointer to DI acquisition thread ID
	char		  *name; //!< Pointer to DI acquisition thread name
	int threadends; 				//!< This field will be used to terminate the thread when change to 1
	int endAck;						//!< This field will be used to notify thread termination when change to 1
	int SR;							//!< Sampling Rate
	int portNumber;
	struct irioPvt* asynPvt;		//!< Pointer to data structure of RIO device resources
	int *timestamp_source;			//!< Pointer to array with timestamp sources of PVs.
} thread_di_t;

/**
 * Struct of Camera Link configuration resources
 */
typedef struct CLConfigData
{
	int Configuration;			//!< Parameter to config camera to Base, Medium or Full
	int SignalMapping;			//!< Parameter to set the signal mapping to Standard, Basler10Tap or Voskhuler10Tap
	int LineScan;				//!< Enable/Disable the line scan for Cameralink
	int FVALHigh;				//!< Enable/Disable the logic level used for the FVALHIGH signal
	int LVALHigh;				//!< Enable/Disable the logic level used for the LVALHIGH signal
	int DVALHigh;				//!< Enable/Disable the logic level used for the DVALHIGH signal
	int SpareHigh;				//!< Enable/Disable the logic level used for the Spare High signal
	int ControlEnable;			//!< Enable/Disable if the camera control lines on the Camera Link cable are driven by the CL Control signals
}CLConfigData_t;

/**
 * Struct of Signal Generator resources
 */
typedef struct SGData
{
	int32_t Freq;
	int32_t UpdateRate;
}SGData_t;

typedef struct GlobalData
{
	int *ch_nelm;
	intr_records_t *intr_records;
	int init_success;
	int io_number;
	int ai_poll_thread_created;
	int ai_poll_thread_run;
	int di_poll_thread_created;
	int di_poll_thread_run;
	int *dma_thread_created;
	int *dma_thread_run;
	int number_of_DMAs;
}globalData_t;

/**
 * Struct of RIO device resources
 */
typedef struct irioPvt
{
		char irio_version[10];						//!< IRIO Library version used
		char linux_driver_version[10];				//!< NI_RIO Linux driver version used
        char *portName;								//!< portName
        int portNumber;
        char *InfoStatus;							//!< Additional info of RIO device status
        char *FPGAStatus;							//!< FPGA info status
        irioDrv_t drvPvt;							//!< Main struct of irioCore. Stores all ports, the current session and the status
        int acq_status;								//!< Acquisition status. 1=Acquiring else=not acquiring
        riodevice_status_name_t rio_device_status;  //!< RIO device status enum
        uint8_t *error_oob_array[MAX_ERROR_OOB];					//!< Array of out of bound errors
        int hw_err_count;  							//!< Errors in hardware configuration counter
        int stat_err_count;  						//!< Out of bound errors counter before FPGASTART=ON
        int dyn_err_count; 							//!< Out of bound errors counter after FPGASTART=ON
        int aux2, bit, shift, shift_pbp, shift_dma, shift_ao, shift_sg, FPGAstarted; 	//!< Variables to manage out of bound errors
        int flag_close, flag_exit, closeDriver, driverInitialized, epicsExiting;		//!< Flags to close FPGA and exit IOC safely
        epicsFloat64 *UserDefinedConversionFactor;	//!< Conversion factor to be applied to data read from DMA if appropiate FrameType is selected.

        /* Asyn interfaces */
        asynInterface   common;				//!< Asyn common interface
        asynInterface   AsynDrvUser;		//!< Asyn asynDrvUser interface
        asynInterface   AsynOption;			//!< Asyn asynOption interface
        asynInterface	AsynOctet;			//!< Asyn asynOctet interface
        asynInterface   AsynInt8Array;		//!< Asyn asynInt8Array interface
        asynInterface   AsynInt32;			//!< Asyn asynInt32 interface
        asynInterface   AsynInt64;			//!< Asyn asynInt64 interface
        asynInterface   AsynInt32Array;		//!< Asyn asynInt32Array interface
        asynInterface   AsynFloat64;		//!< Asyn asynFloat64 interface
        asynInterface   AsynFloat32Array;	//!< Asyn asynFloat32Array interface

        /* For interrupt sources */
        void *asynOctetInterruptPvt;			//!< Asyn asynOctet interrupt source
        void *asynInt8ArrayInterruptPvt;		//!< Asyn asynInt8Array interrupt source
        void *asynInt32InterruptPvt;			//!< Asyn asynInt32 interrupt source
        void *asynInt64InterruptPvt;			//!< Asyn asynInt32 interrupt source
        void *asynInt32ArrayInterruptPvt;		//!< Asyn asynInt32Array interrupt source
        void *asynFloat32ArrayInterruptPvt;		//!< Asyn asynFloat32Array interrupt source
        void *asynFloat64InterruptPvt;			//!< Asyn asynFloat64 interrupt source

        /*IMAQ Profile*/
        CLConfigData_t CLConfig; 	//!< Camera Link configuration resources struct
        char* UARTReceivedMsg;		//!< UART Message received
		int sizeX;					//!< Image x size
		int sizeY;					//!< Image Y size

		/*SG Info*/
		SGData_t* sgData;  			//!< Signal generator resources struct

        /* DMA data acquisition threads */
        irio_dmathread_t* ai_dma_thread; 	//!<  Pointer to struct of DMA data acquisition thread resources

        /* AI I/O Intr data acquisition threads */
        thread_ai_t* thread_ai; 			//!<  Pointer to struct of AI data acquisition thread resources
         /* DI I/O Intr data acquisition threads */
        thread_di_t* thread_di; 			//!<  Pointer to struct of DI data acquisition thread resources

} irio_pvt_t;

/**
 * struct to define pointers to RIO devices structs.
 */
typedef struct pdrvPvt_array_struct {
        irio_pvt_t * ptr;
        int used;
}pdrvPvt_array_t;

/**
 * Array of pointers to RIO devices structs.
 */
static pdrvPvt_array_t pdrvPvt_array[MAX_NUMBER_OF_CARDS];
//local variable
globalData_t globalData[MAX_NUMBER_OF_CARDS]; //array of globalData_t structs to store all necessary data read from record database when gettingDBInfo method is called.

int call_gettingDBInfo; // Variable to call gettingDBInfo only once.
int all_poll_threads_finished, all_dma_threads_finished; //0: All intr/dma threads ARE finished ; 1 All intr/dma threads are NOT finished.
int epicsExiting;


/********************************************** asynCommon **********************************************/

/**
 * Report: Generates a report about the hardware device
 * 		Device name, RIO platform, RIO device model, RIO serial number, Adapter module ID, LabVIEW project name,
 * 		EPICS driver version, IRIO library version, Device profile, FPGA main loop frecquency, Min-Max Sampling Rate,
 * 		Min-Max Analog output, Coupling mode, Conversion to volts of analog inputs and Conversion from volts for analog outputs.
 *
 * Use: Execute dbior command from IOC shell
 *
 * @param[in] drvPvt    Struct pointer that includes all resources to manage a RIO device
 * @param[in] fp		Pointer to a file object that identifies the stream
 * @param[in] details	0 = Short description, 1 = Additional information and 2 = Print even more info
 * @return
 */
static void report (void *drvPvt, FILE *fp, int details);

/**
 * Connect: Connect to the hardware device
 *
 * @param[in] drvPvt 		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @return asynSuccess
 *
 */
static asynStatus connect(void *drvPvt, asynUser *pasynUser);

/**
 * Disconnect: Disconnect to the hardware device
 *
 * @param[in] drvPvt 		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @return asynStatus
 */
static asynStatus disconnect(void *drvPvt,asynUser *pasynUser);

/**
 * asynCommon interface: describes the methods that must be implemented by drivers
 */
static asynCommon asynCom = {
                report,
                connect,
                disconnect
};

/********************************************** asynInt8Array **********************************************/

/**
 * asynInt8Array interface: Communicates with the device via 8 bit integer arrays
 */
static asynInt8Array AInt8Array = {
		NULL,
		NULL,
        NULL,
        NULL,
};

/********************************************** asynInt32 **********************************************/

/**
 * int32Write: Method to write epicsInt32 values
 *
 * @param[in] drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[in] value			Value to write
 * @return asynStatus
 */
static asynStatus int32Write(void *drvPvt, asynUser *pasynUser, epicsInt32 value);

/**
 * int32Read: Method to read epicsInt32 values
 *
 * @param[in]  drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[out] value		Value read
 * @return asynStatus
 */
static asynStatus int32Read(void *drvPvt, asynUser *pasynUser, epicsInt32 *value);

/**
 * asynInt32 interface: Communicates with the device via 32 bit integers
 */
static asynInt32 AInt32 = {
        int32Write,
        int32Read,
        NULL,
        NULL,
        NULL,
};



/********************************************** asynInt64 **********************************************/

/**
 * int64Write: Method to write epicsInt32 values
 *
 * @param[in] drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[in] value			Value to write
 * @return asynStatus
 */
static asynStatus int64Write(void *drvPvt, asynUser *pasynUser, epicsInt64 value);

/**
 * int64Read: Method to read epicsInt32 values
 *
 * @param[in]  drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[out] value		Value read
 * @return asynStatus
 */
static asynStatus int64Read(void *drvPvt, asynUser *pasynUser, epicsInt64 *value);

/**
 * asynInt32 interface: Communicates with the device via 64 bit integers
 */
static asynInt64 AInt64 = {
        int64Write,
        int64Read,
        NULL,
        NULL,
        NULL,
};

/********************************************** asynInt32Array **********************************************/

/**
 * asynInt32Array interface: Communicates with the device via 32 bit integer arrays
 */
static asynInt32Array AInt32Array = {
		NULL,
		NULL,
        NULL,
        NULL,
};

/********************************************** asynFloat64 **********************************************/

/**
 * float64Write: Method to write epicsFloat64 values
 *
 * @param[in] drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[in] value			Value to write
 * @return asynStatus
 */
static asynStatus float64Write(void *drvPvt, asynUser *pasynUser, epicsFloat64 value);

/**
 * float64Read: Method to read epicsFloat64 values
 *
 * @param[in]  drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[out] value		Value read
 * @return asynStatus
 */
static asynStatus float64Read(void *drvPvt, asynUser *pasynUser, epicsFloat64 *value);

/**
 * asynFloat64 interface: Communicates with the device via double precision float values
 */
static asynFloat64 AFloat64 = {
        float64Write,
        float64Read,
        NULL,
        NULL,
};

/********************************************** asynFloat32Array **********************************************/

/**
 * float32ArrayRead: Method to read arrays of epicsFloat32 data
 *
 * @param[in]  drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[out] value		Value read
 * @param[in]  nelements	Number of elements to read
 * @param[in]  nIn			Size of the elements
 * @return asynStatus
 */
static asynStatus float32ArrayRead(void *drvPvt, asynUser *pasynUser,
        epicsFloat32 *value, size_t nelements, size_t *nIn);

/**
  * asynFloat32Array interface: Communicates with the device via arrays of double precision float values
  */
static asynFloat32Array asynFloat32Array_structure = {
				NULL,
                float32ArrayRead,
                NULL,
                NULL,
};

/********************************************** asynOctet **********************************************/

/**
 * octetRead: Method to read octet strings. The name octet is used instead ASCII because it implies that comunication is done via 8-bit bytes.
 *
 * @param[in]  drvPvt				Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser				Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[out] data					Chars read
 * @param[in]  maxchars				Max number of chars to read
 * @param[in]  nbytesTransfered		Number of bytes transfered
 * @param[in]  eomReason			Not used
 * @return asynStatus
 */
static asynStatus octetRead(void *drvPvt, asynUser *pasynUser, char *data, size_t maxchars,size_t *nbytestransfered,int *eomReason);
/**
 * octetWrite: Method to write octet strings. The name octet is used instead ASCII because it implies that comunication is done via 8-bit bytes.
 *
 * @param[in] drvPvt				Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser				Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[in] data					Chars to write
 * @param[in] nbytesTransfered		Number of bytes transfered
 * @return asynStatus
 */

static asynStatus octetWrite(void *drvPvt,asynUser *pasynUser, const char *data,size_t numchars,size_t *nbytesTransfered);

/**
 * asynOctet interface: Describes the methods implemented by drivers that use octet strings for sending commands and receiving responses from the device
 */
static asynOctet AOctet = {
		.write = octetWrite,
        .read = octetRead,
};

/********************************************** asynUser **********************************************/

/**
 * drvUserCreate: The driver can create any resources it needs by calling drvUserCreate method. It can use asynUse *pasynUser to provide access to the resources
 *
 * @param[in] drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[in] drvInfo		TODO
 * @param[in] pptypeName	TODO
 * @param[in] psize			TODO
 * @return asynStatus
 */
static asynStatus drvUserCreate(void *drvPvt, asynUser *pasynUser, const char *drvInfo, const char **pptypeName, size_t *psize);

/**
 * drvUserGetType: If other code wants to access asynUser *pasynUser it must call drvUserGetType method and verify that typeName and size are what it expects
 *
 * @param[in] drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @param[in] pptypeName	TODO
 * @param[in] psize			TODO
 * @return asynStatus
 */
static asynStatus drvUserGetType(void *drvPvt, asynUser *pasynUser, const char **pptypeName, size_t *psize);

/**
 * drvUserDestroy: Method to destroy the resources created by drvUserCreate
 *
 * @param[in] drvPvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in] pasynUser		Pointer to asynUser, struct that contains generic information and is the "handle" for calling most methods.
 * @return asynStatus
 */
static asynStatus drvUserDestroy(void *drvPvt, asynUser *pasynUser);

/**
 * asynDrvUser interface: Provides methods that allow an asynUser to communicate user specific information to/from a port driver
 */
static asynDrvUser drvUser = {
        drvUserCreate,
        drvUserGetType,
        drvUserDestroy,
};

/*******************************************************************************************************************/

/**
 * initializes NI-RIO EPICS device support
 *
 *
 * @param namePort aysn portname to be asigned
 * @param DevSerial serial number of the RIO device (Manufacturer Serial Number)
 * @param PXInirioModel Model as defined by manufacturer eg: PXIe7966R
 * @param projectName name identifying the VI programmed to be programmed in the FPGAto be loaded in the FPGA
 * @param FPGAversion parameter indicating the version of the bitfile
 * @return Operation status. @see {TStatusCodes}
 */
int nirioinit(const char *namePort,const char *DevSerial,const char *PXInirioModel, const char *projectName, const char *FPGAversion,int verbosity);

/**
 * status_func: Method to set RIO device status. After any function call, it evaluates any error, if occurred, in IRIO Library or EPICS driver  and
 * 				sets values of next PVs: STATUS (RIO device status) and STATUS-STR (Additional info about error obtained).
 *
 * @param[in] drvPvt	Struct pointer that includes all resources to manage a RIO device
 * @param[in] status	Status returned by any previous function call
 * @return asynStatus
 */
int status_func(void *drvPvt, TStatus* status);

/**
 * ai_intr_thread:
 *
 *  * @param[in] p		Pointer to thread_ai struct
 *
 */
static void ai_intr_thread(void *p);

/**
 * di_intr_thread:
 *
 *  * @param[in] p		Pointer to thread_di struct
 *
 */
static void di_intr_thread(void *p);

/**
 * aiDMA_thread: Body function of the EPICS thread created per DMA in nirioinit function.

 *		-Creates necessary resources for acquisition thread.
 * 		-Identifies the data acquisition profile (DMA or imageDMA)
 * 		-Identifies if there is at least one consumer (Waveform PV) to read the data acquired by a channel of the DMA. If there is not PVs to publish the data, the thread ends.
 * 		-Thread initialization:
 * 			-ImageDMA profile:
 * 		 		-Sets buffersize of the acquisition thread.
 * 		 		-No epicsRingBuffer needed. No publishing thread needed.
 * 		 	-DMA profile:
 * 		 		-Sets the buffersize of the acquisition thread and the number of samples per channel in each data block acquired.
 * 		 		-If there is consumer (Waveform PV loaded) associated:
 * 		 			-Creates one ringbuffer, per channel in the DMA. The size of each ringbuffer is: samples_per_channel*Sample_Size*4096
 * 		  			-Creates one publishing thread per consumer (Waveform PV) and points its epicsRingBytesID to the ringbuffer of the channel.
 *		-Data acquisition main loop (do-while, flag to safe exit thread is 0 or there is no error in data acquisition process):
 *			-ImageDMA profile:
 *				-Checks that imageSize is not bigger than PV container. If bigger, acquisition stops.
 *				-getDMATtoHostImage call to acquire images.
 *				-Send the image acquired to consumer subscribed (Image waveform PV), taking into account the block decimation.
 *			-DMA profile:
 *				-getDMATtoHostData call to acquire one data block per call.
 *				-Organize by channels the data received, depending on the sample size.
 *				-Copy the data of each channel on its own ringbuffer from where each consumer thread will read the data to send it to EPICS.
 *		-End of acquisition:
 *				 -Send flags to consumer threads advising that acquisition has finished and wait to ack.
 *				 -Delete ringbuffer of each channel.
 *				 -Free all memory allocated and finishes.
 *
 * @param[in] p		Pointer to irio_dmathread struct
 */
static void aiDMA_thread(void *p);

/**
 * ai_pv_thread: Body function of the EPICS thread created per channel in each DMA in aiDMA_thread function.
 *
 *		-Creates necessary resources for consumer thread.
 *		-Wait till acquisition thread is running
 *		-Gets data from the ringbuffer pointed by the epicsRingBytsID.
 *		-Publish the data to EPICS through Waveform PVs
 *		-Waits for acquisition thread ends, free all memory allocated and finishes.
 *
 * @param[in] p 	Struct pointer that includes all resources to manage a RIO device
 */
static void ai_pv_thread(void *p);

/**
 * nirio_epicsExit: Method to manage correct IOC exit
 *
 * @param[in] drvPvt 	Struct pointer that includes all resources to manage a RIO device
 */
void nirio_epicsExit(void *drvPvt);

/**
 * nirio_shutdown: Method to manage correct threads finish, correct free memory allocations and correct FPGA shutdown.
 *
 * @param[in] drvPvt 	Struct pointer that includes all resources to manage a RIO device
 */
void nirio_shutdown(irio_pvt_t *pdrvPvt);

/**
 * callbackOctet:
 *
 * @param[in]  asynirio_Pvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in]  reason			Reason that calls the method
 * @param[out] pv_data			Data read
 * @param[in]  pv_nelem			Number of elements to read
 */
int callbackOctet(irio_pvt_t* asynirio_Pvt, int reason, char* pv_data, int pv_nelem);

/**
 * callbackInt32
 *
 * @param[in]  asynirio_Pvt 	Struct pointer that includes all resources to manage a RIO device
 * @param[in]  reason			Reason that calls the method
 * @param[in]  addr				Address
 * @param[out] pv_data			Data Read
 */
int callbackInt32(irio_pvt_t* asynirio_Pvt, int reason,int addr, epicsInt32 pv_data);

/**
 * callbackFloat64
 *
 * @param[in]  asynirio_Pvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in]  reason			Reason that calls the method
 * @param[in]  addr				Address
 * @param[out] pv_data			Data Read
 */
int callbackFloat64(irio_pvt_t* asynirio_Pvt, int reason,int addr, epicsFloat64 pv_data);

/**
 * CallAIInsInt8Array
 *
 * @param[in]  asynirio_Pvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in]  reason			Reason that calls the method
 * @param[in]  addr				Address
 * @param[out] pv_data			Data read
 * @param[in]  pv_nelem			Number of elements to read
 */
int CallAIInsInt8Array(irio_pvt_t* asynirio_Pvt, int reason, int addr, epicsInt8* pv_data, int pv_nelem);

/**
 * CallAIInsFloat32Array
 *
 * @param[in]  asynirio_Pvt		Struct pointer that includes all resources to manage a RIO device
 * @param[in]  reason			Reason that calls the method
 * @param[in]  addr 			Address
 * @param[out] pv_data			Data read
 * @param[in]  pv_nelem			Nember of elements to read
 */
int CallAIInsFloat32Array(irio_pvt_t* asynirio_Pvt, int reason, int addr, float* pv_data, int pv_nelem);

/**
 * getChannelDataU8: Method to reorganize by channels, U8 data acquired in DMA.
 *
 * @param[in] nChannels			Number of channels in the DMA
 * @param[in] nSamples			Number of samples per channel per data block
 * @param[in] inBuffer			Input buffer
 * @param[in] outBuffer			Output buffer
 * @param[in] CVADC				Analog to digital conversion value
 */
void getChannelDataU8(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC);

/**
 * getChannelDataU16: Method to reorganize by channels, U16 data acquired in DMA.
 *
 * @param[in] nChannels			Number of channels in the DMA
 * @param[in] nSamples			Number of samples per channel per data block
 * @param[in] inBuffer			Input buffer
 * @param[in] outBuffer			Output buffer
 * @param[in] CVADC				Analog to digital conversion value
 */
void getChannelDataU16(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC);

/**
 * getChannelDataU32: Method to reorganize by channels, U32 data acquired in DMA.
 *
 * @param[in] nChannels			Number of channels in the DMA
 * @param[in] nSamples			Number of samples per channel per data block
 * @param[in] inBuffer			Input buffer
 * @param[in] outBuffer			Output buffer
 * @param[in] CVADC				Analog to digital conversion value
 */
void getChannelDataU32(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC);

/**
 * getChannelDataU64: Method to reorganize by channels, U64 data acquired in DMA.
 *
 * @param[in] nChannels			Number of channels in the DMA
 * @param[in] nSamples			Number of samples per channel per data block
 * @param[in] inBuffer			Input buffer
 * @param[in] outBuffer			Output buffer
 * @param[in] CVADC				Analog to digital conversion value
 */
void getChannelDataU64(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC);

static void irioTimeStamp(void *userPvt,epicsTimeStamp *pTimeStamp);

#endif
