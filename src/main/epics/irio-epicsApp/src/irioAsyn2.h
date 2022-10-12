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
//#include "epicsExit.h"
//#include "epicsRingBytes.h"
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
	virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars,
									size_t *nActual, int *eomReason);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,
									size_t *nActual);//!< Asyn asynOctet interface
	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
//	virtual asynStatus readInt64(asynUser *pasynUser, epicsInt64 *value);
//	virtual asynStatus writeInt64(asynUser *pasynUser, epicsInt64 value);
//	virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
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
	int EPICSVerisonMessage;
	int FPGAStart;
	int DevQualityStatus;
	template <typename T>
			using handler_t = std::function<int(asynUser *, T*)>;
	template <typename T>
			using handlerMap_t = std::map<int, handler_t<T>>;
	handlerMap_t<char *> mapa;
	int echoHandler(asynUser *, char *);

private:
	int status_func(void *drvPvt, TStatus* status);

	void nirio_shutdown();
	//typedef struct irioPvt
	//{
	//		char irio_version[10];						//!< IRIO Library version used
	//		char linux_driver_version[10];				//!< NI_RIO Linux driver version used
	//        char *portName;								//!< portName
	//        int portNumber;
	//        char *InfoStatus;							//!< Additional info of RIO device status
	        std::string FPGAStatus;							//!< FPGA info status
	        irioDrv_t iriodrv;							//!< Main struct of irioCore. Stores all ports, the current session and the status
	//        int acq_status;								//!< Acquisition status. 1=Acquiring else=not acquiring
	        riodevice_status_name_t rio_device_status;  //!< RIO device status enum
	//        uint8_t *error_oob_array[MAX_ERROR_OOB];					//!< Array of out of bound errors
	//        int hw_err_count;  							//!< Errors in hardware configuration counter
	//        int stat_err_count;  						//!< Out of bound errors counter before FPGASTART=ON
	//        int dyn_err_count; 							//!< Out of bound errors counter after FPGASTART=ON
	//        int aux2, bit, shift, shift_pbp, shift_dma, shift_ao, shift_sg, FPGAstarted; 	//!< Variables to manage out of bound errors
	        int flag_close, flag_exit, closeDriver, driverInitialized, epicsExiting;		//!< Flags to close FPGA and exit IOC safely
	//        epicsFloat64 *UserDefinedConversionFactor;	//!< Conversion factor to be applied to data read from DMA if appropiate FrameType is selected.
	//
	//        /* Asyn interfaces */
	//        asynInterface   common;				//!< Asyn common interface
	//        asynInterface   AsynDrvUser;		//!< Asyn asynDrvUser interface
	//        asynInterface   AsynOption;			//!< Asyn asynOption interface

//        asynInterface   AsynInt8Array;		//!< Asyn asynInt8Array interface
	//        asynInterface   AsynInt32;			//!< Asyn asynInt32 interface
	//        asynInterface   AsynInt32Array;		//!< Asyn asynInt32Array interface
	//        asynInterface   AsynFloat64;		//!< Asyn asynFloat64 interface
	//        asynInterface   AsynFloat32Array;	//!< Asyn asynFloat32Array interface
	//
	//        /* For interrupt sources */
	//        void *asynOctetInterruptPvt;			//!< Asyn asynOctet interrupt source
	//        void *asynInt8ArrayInterruptPvt;		//!< Asyn asynInt8Array interrupt source
	//        void *asynInt32InterruptPvt;			//!< Asyn asynInt32 interrupt source
	//        void *asynInt32ArrayInterruptPvt;		//!< Asyn asynInt32Array interrupt source
	//        void *asynFloat32ArrayInterruptPvt;		//!< Asyn asynFloat32Array interrupt source
	//        void *asynFloat64InterruptPvt;			//!< Asyn asynFloat64 interrupt source
	//
	//        /*IMAQ Profile*/
	//        CLConfigData_t CLConfig; 	//!< Camera Link configuration resources struct
	//        char* UARTReceivedMsg;		//!< UART Message received
	//		int sizeX;					//!< Image x size
	//		int sizeY;					//!< Image Y size
	//
	//		/*SG Info*/
	//		SGData_t* sgData;  			//!< Signal generator resources struct
	//
	//        /* DMA data acquisition threads */
	//        irio_dmathread_t* ai_dma_thread; 	//!<  Pointer to struct of DMA data acquisition thread resources
	//
	//        /* AI I/O Intr data acquisition threads */
	//        thread_ai_t* thread_ai; 			//!<  Pointer to struct of AI data acquisition thread resources
	//         /* DI I/O Intr data acquisition threads */
	//        thread_di_t* thread_di; 			//!<  Pointer to struct of DI data acquisition thread resources
	//
	//} irio_pvt_t;
};

/* Status message strings */
#define EPICSVersionString       "epics_version"        /**< (asynOctet,    r/o) Status message */
#define FPGAStartString "FPGAStart" /** <asynInt32
//#define ADStringToServerString      "STRING_TO_SERVER"      /**< (asynOctet,    r/o) String sent to server for message-based drivers */
//#define ADStringFromServerString    "STRING_FROM_SERVER"    /**< (asynOctet,    r/o) String received from server for message-based drivers */
#define DevQualityStatusString "DevQualityStatus"

#endif
