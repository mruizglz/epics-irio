/**************************************************************************//**
 * \file irioAsyn.h
 * \authors Mariano Ruiz (Universidad Politécnica de Madrid, UPM)
 * \authors Enrique Bernal ()
 * \brief Initialization and common resources access methods for IRIOASYN Driver.
 * \date Sept., 2022
 *****************************************************************************/


#include "irioAsyn.h"

#include <math.h>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <regex>
#include <stdexcept>
#include <iostream>
//static const char *driverName = "irio";
#define ERR(msg) asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "[%s-%d] %s::%s: %s\n", \
    __func__, __LINE__, driverName, functionName, msg)
#define FLOW(msg) asynPrint(pasynUserSelf, ASYN_TRACE_FLOW, "[%s-%d] %s::%s: %s\n", \
	__func__, __LINE__,driverName, functionName, msg)

static void irioTimeStamp(void *userPvt, epicsTimeStamp *pTimeStamp) {
	epicsTimeGetCurrent(pTimeStamp);
}

extern "C" {
int irio_configure(const char *namePort, const char *DevSerial,
		const char *PXInirioModel, const char *projectName,
		const char *FPGAversion, int verbosity) {
	try {
		new irio(namePort, DevSerial, PXInirioModel, projectName, FPGAversion,
				verbosity);
	} catch (std::exception& e ) {
		//if the initialization goes wrong display error an exit
		std::cerr<<"Error in the initialization of: "<<namePort<<"RIO Device with Serial number:"<<DevSerial;
		exit(-1);
	}
	return 0;
}
/**
 *	Next, Registration of the irioinit function and its 6 parameters
 */
static const iocshArg nirioInitArg0 = { "portName", iocshArgString }; //!< i.e, "RIO12" first number "1" the number of chassis from top to down
//!< i.e, second number "2" is the number of slot.
static const iocshArg nirioInitArg1 = { "DevSerial", iocshArgString }; //!< i.e, Serial Number of the device Card
static const iocshArg nirioInitArg2 = { "irioDevice", iocshArgString }; //!< i.e, "PXIe-7965R"
static const iocshArg nirioInitArg3 = { "projectName", iocshArgString }; //!< i.e, "7952fpga"
static const iocshArg nirioInitArg4 = { "FPGAVersion", iocshArgString };//!< i.e, "V2.12"
static const iocshArg nirioInitArg5 = { "verbosity", iocshArgInt };

static const iocshArg *nirioInitArgs[] = { &nirioInitArg0, &nirioInitArg1,
		&nirioInitArg2, &nirioInitArg3, &nirioInitArg4, &nirioInitArg5 };

static const iocshFuncDef nirioInitFuncDef = { "nirioinit", 6, nirioInitArgs };

static void nirioInitCallFunc(const iocshArgBuf *args) {
	irio_configure(args[0].sval, args[1].sval, args[2].sval, args[3].sval,
			args[4].sval, args[5].ival);
}

static void nirioRegister(void) {
	static int firstTime = 1;
	if (firstTime == 1) {
		firstTime = 0;
		iocshRegister(&nirioInitFuncDef, nirioInitCallFunc);
	}
}

epicsExportRegistrar(nirioRegister);
epicsRegisterFunction(irioTimeStamp);


}
