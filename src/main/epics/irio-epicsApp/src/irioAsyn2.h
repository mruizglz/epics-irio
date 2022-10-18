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


#include <map>
#include <functional>
#include <list>
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
//#include <cantProceed.h>
#include <iocsh.h>
#include <epicsThread.h>
#include <epicsString.h>
#include "epicsExit.h"
#include "epicsRingBytes.h"
//#include <errlog.h>
#include <epicsTime.h>
#include <registryFunction.h>
#include <epicsExport.h>
//
//#include "alarm.h"
//#include "cvtTable.h"
//#include "dbDefs.h"
//#include "dbAccess.h"
//#include "recGbl.h"
//#include "recSup.h"
//#include "devSup.h"
//#include "link.h"
//#include "aiRecord.h"
//#include "stringinRecord.h"
//#include "stringoutRecord.h"
//#include "mbboRecord.h"
//#include "boRecord.h"

#include "locale.h"

/* asyn */
//#include <asynDriver.h>
//#include <asynInt32.h>
//#include <asynUInt32Digital.h>
//#include <asynFloat64.h>
//#include <asynStandardInterfaces.h>
//#include <asynCommonSyncIO.h>
//#include <asynInt32ArraySyncIO.h>
//#include <asynDrvUser.h>
//#include <asynOctet.h>
//#include <asynFloat32ArraySyncIO.h>
#include <epicsTypes.h>
#include <asynPortDriver.h>

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

/* Status message strings */
#define DeviceSerialNumberString "device_serial_number"
#define EPICSVersionString       "epics_version"        /**< (asynOctet,    r/o) Status message */
#define IRIOVersionString		 "driver_version"
#define DeviceNameString  	     "device_name"
#define FPGAStatusString		"FPGAStatus"
#define InfoStatusString 		"InfoStatus"
#define VIversionString			"VIversion"

//asynInt32
#define RIODeviceStatusString "riodevice_status"
#define SamplingRateString "SamplingRate"
#define SR_AI_IntrString "SR_AI_Intr"
#define SR_DI_IntrString "SR_DI_Intr"
#define DebugString "debug"
#define GroupEnableString "GroupEnable"
#define FPGAStartString "FPGAStart" /** <asynInt32 */
#define DAQStartStopString "DAQStartStop"
#define DFString "DF"
#define DevQualityStatusString "DevQualityStatus"
#define DMAOverflowString "DMAOverflow"
#define AOEnableString "AOEnable"
#define SGFreqString "SGFreq"
#define SGUpdateRateString "SGUpdateRate"
#define SGSignalTypeString "SGSignalType"
#define SGPhaseString "SGPhase"
#define DIString "DI"
#define DOString "DO"
#define auxAIString "auxAI"
#define auxAOString "auxAO"
#define auxDIString "auxDI"
#define auxDOString "auxDO"
//TODOD: Añadir los del profile de imagen!!!!!!



#define DeviceTempString "DeviceTemp"
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

/**
 * Class for managing DMA thread acquisition resources
 */
class dmathread
{
public:
	dmathread(uint8_t id);
	~dmathread();
	void runthread(void);
	void *aiDMA_thread(void *p);
private:
	epicsThreadId *_thread_id;	//!< Pointer to DMA acquisition thread ID
	std::string		  _name; //!< Pointer to DMA acquisition thread name
	int _id;							//!< ID
	int _threadends; 				//!< This field will be used to terminate the thread when change to 1
	int _endAck;						//!< This field will be used to notify thread termination when change to 1
	int _DecimationFactor; 			//!< Decimation Factor
	int _SR;							//!< Sampling Rate
	int _blockSize; 					//!< Size of acquisition block (in terms of DMA NwordU64)
//	struct irioPvt* asynPvt;		//!< Pointer to data structure of RIO device resources
	epicsRingBytesId* _IdRing;		//!< Pointer to RingBuffer ID
	uint8_t _dmanumber;					//!< DMA number
};

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
///@}
class irio : asynPortDriver{
public:
	 irio(const char *namePort,
			const char *DevSerial,
			const char *PXInirioModel,
			const char *projectName,
			const char *FPGAversion,
			int verbosity);
	 ~irio();
	virtual void report(FILE *fp, int details);
	virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars,
									size_t *nActual, int *eomReason);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,
									size_t *nActual);//!< Asyn asynOctet interface
	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
//	virtual asynStatus readInt64(asynUser *pasynUser, epicsInt64 *value);
//	virtual asynStatus writeInt64(asynUser *pasynUser, epicsInt64 value);
	virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
//	virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
//	virtual asynStatus readInt8Array(asynUser *pasynUser, epicsInt8 *value,
//							size_t nElements, size_t *nIn);
//	virtual asynStatus writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
//							size_t nElements);
//
//	virtual asynStatus readInt16Array(asynUser *pasynUser, epicsInt16 *value,
//							size_t nElements, size_t *nIn);
//	virtual asynStatus writeInt16Array(asynUser *pasynUser, epicsInt16 *value,
//							size_t nElements);
//
//	virtual asynStatus readInt32Array(asynUser *pasynUser, epicsInt32 *value,
//							size_t nElements, size_t *nIn);
//	virtual asynStatus writeInt32Array(asynUser *pasynUser, epicsInt32 *value,
//							size_t nElements);
//
//	virtual asynStatus readInt64Array(asynUser *pasynUser, epicsInt64 *value,
//							size_t nElements, size_t *nIn);
//	virtual asynStatus writeInt64Array(asynUser *pasynUser, epicsInt64 *value,
//							size_t nElements);
//
//	virtual asynStatus readFloat32Array(asynUser *pasynUser, epicsFloat32 *value,
//							size_t nElements, size_t *nIn);
//	virtual asynStatus writeFloat32Array(asynUser *pasynUser, epicsFloat32 *value,
//							size_t nElements);
//
//	virtual asynStatus readFloat64Array(asynUser *pasynUser, epicsFloat64 *value,
//							size_t nElements, size_t *nIn);
//	virtual asynStatus writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value,
//							size_t nElements);

protected:



private:
	int status_func( TStatus* status);
	int resources(void);
	void nirio_shutdown();

	//		char irio_version[10];						//!< IRIO Library version used
	//		char linux_driver_version[10];				//!< NI_RIO Linux driver version used
	        std::string portName;								//!< portName
	//        int portNumber;
	        std::string sInfoStatus;							//!< Additional info of RIO device status
	        std::string sFPGAStatus;							//!< FPGA info status
	        irioDrv_t iriodrv;							//!< Main struct of irioCore. Stores all ports, the current session and the status
	//        int acq_status;								//!< Acquisition status. 1=Acquiring else=not acquiring
	        riodevice_status_name_t rio_device_status;  //!< RIO device status enum
	//        uint8_t *error_oob_array[MAX_ERROR_OOB];					//!< Array of out of bound errors
	//        int hw_err_count;  							//!< Errors in hardware configuration counter
	//        int stat_err_count;  						//!< Out of bound errors counter before FPGASTART=ON
	        int dyn_err_count; 							//!< Out of bound errors counter after FPGASTART=ON
	        //int aux2, bit, shift, shift_pbp, shift_dma, shift_ao, shift_sg,  	//!< Variables to manage out of bound errors
			unsigned int FPGAstarted;
	        int flag_close, flag_exit, closeDriver, driverInitialized, epicsExiting;		//!< Flags to close FPGA and exit IOC safely
	        std::vector<epicsFloat64> UserDefinedConversionFactor;	//!< Conversion factor to be applied to data read from DMA if appropiate FrameType is selected.
	//

	//
	//        /*IMAQ Profile*/
	        CLConfigData_t CLConfig; 	//!< Camera Link configuration resources struct
	        std::string UARTReceivedMsg;		//!< UART Message received
			int sizeX;					//!< Image x size
			int sizeY;					//!< Image Y size
	//
			/*SG Info*/
			std::vector<SGData_t> sgData;  			//!< Array of Signal generator resources structs
	//
	        /* DMA data acquisition threads */
	        std::vector<dmathread> ai_dma_thread; 	//!<  Pointer to struct of DMA data acquisition thread resources
	//
	//        /* AI I/O Intr data acquisition threads */
	//        thread_ai_t* thread_ai; 			//!<  Pointer to struct of AI data acquisition thread resources
	//         /* DI I/O Intr data acquisition threads */
	//        thread_di_t* thread_di; 			//!<  Pointer to struct of DI data acquisition thread resources
	//

	int DeviceSerialNumber;
	int EPICSVersionMessage;
	int IRIOVersionMessage;
	int DeviceName;
	int FPGAStatus;
	int InfoStatus;
	int VIversion;


	int DeviceTemp;


	int riodevice_status;
	 int SamplingRate;
	 int SR_AI_Intr;
	 int SR_DI_Intr;
	 int debug;
	 int GroupEnable;
	 int FPGAStart;

	 int DAQStartStop;
	 int DF;
	 int  DevQualityStatus;
	int DMAOverflow;
	int AOEnable;
	int SGFreq;
	int SGUpdateRate;
	int SGSignalType;
	int SGPhase;
	int DI;
	int DO;
	int auxAI;
	int auxAO;
	int auxDI;
	int auxDO;

};

std::vector<irio> instances;
uint8_t number=0;

#endif
