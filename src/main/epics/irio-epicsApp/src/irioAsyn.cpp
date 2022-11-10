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
static const char *driverName = "irio";
#define ERR(msg) asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "[%s-%d] %s::%s: %s\n", \
    __func__, __LINE__, driverName, functionName, msg)
#define DRIVER(msg) asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER, "[%s-%d] %s::%s: %s\n", \
	__func__, __LINE__,driverName, functionName, msg)

//static void irioTimeStamp(void *userPvt, epicsTimeStamp *pTimeStamp) {
//	epicsTimeGetCurrent(pTimeStamp);
//}
//This function is called by EPICS and needs external linking in C
extern "C" {
void nirio_epicsExit(void *ptr) {

	irio *pirio=static_cast<irio*>(ptr);
	delete pirio;
}
}

irio::irio(const char *namePort, const char *DevSerial,
		const char *PXInirioModel, const char *projectName,
		const char *FPGAversion, int verbosity) :
		asynPortDriver(namePort, 1,
				asynOctetMask | asynInt32Mask | asynInt32ArrayMask
						| asynFloat64Mask | asynFloat32ArrayMask | asynEnumMask
						| asynDrvUserMask, // interface mask,
				asynOctetMask | asynInt32Mask | asynInt32ArrayMask
						| asynFloat64Mask | asynFloat32ArrayMask | asynEnumMask, // interrupt mask,
				0, 1, 0, 0), driverInitialized(0), flag_close(0), flag_exit(0), epicsExiting(
				0), closeDriver(0), _rio_device_status(STATUS_INITIALIZING) {

	const char *functionName = "irio constructor";

	if (createIRIOParams()) throw std::logic_error("Problem creating the asynPort driver params"); //create all asynPort needed parameters
	epicsAtExit(nirio_epicsExit, this);

//	TODO: initHookRegister(gettingDBInfo);  for database analisys
//

	_portName = namePort;
	if (_portName.empty()) {
		ERR("PORTNAME is empty!");
		throw std::logic_error("");
	}
	std::regex pat("RIO_\\d+$"); //see https://regex101.com/ for help

	bool match = std::regex_match(_portName, pat);
	if (!match) {
		ERR("PORTNAME format invalid: use RIO_<n> !");
		throw std::logic_error("");
	}

	std::string appCallID;
	std::string currentdir;
	std::string bitfilepath;
	std::string OSdirFW;
	OSdirFW = "/opt/codac/firmware/ni/irio/"; //default path
	currentdir = get_current_dir_name();
	bitfilepath = currentdir + "/irio/";
	_rio_device_status = STATUS_INITIALIZING;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	// We search for the bitfile in two locations. a) local folder b) system CODAC folder
	unsigned int retry = 2;
	do {
		int st = irio_initDriver(appCallID.c_str(), DevSerial, PXInirioModel,
				projectName, FPGAversion, verbosity, bitfilepath.c_str(),
				bitfilepath.c_str(), &_iriodrv, &irio_status);
		switch (st) {
		case IRIO_success:
			DRIVER("Device initialization OK.\n");
			driverInitialized = 1;
			status_func(&irio_status);
			retry = 0;
			break;
		case IRIO_error:
			if (irio_status.detailCode == HeaderNotFound_Error)
				retry--;
			else {
				status_func(&irio_status);
				nirio_shutdown();
				DRIVER("Device initialization Fails. Bitfile or header file not found.");
				bitfilepath = OSdirFW;
			}
			break;
		case IRIO_warning:
			status_func(&irio_status);
			retry = 0;
			break;
		}
	} while (retry > 0);
	if (irio_status.code ==IRIO_error) throw std::logic_error("");

//resizing vectors
	sgData.resize(_iriodrv.NoOfSG);
	UserDefinedConversionFactor.resize(_iriodrv.DMATtoHOSTNo.value);

//Total number of channels in DMAs


//
//	//Allocate memory for AI+DI+CH records
//	//i=0;
	uint32_t number_dma_ch = 0;
	for (uint32_t i = 0; i < _iriodrv.DMATtoHOSTNo.value; i++) {
		number_dma_ch += (uint32_t) _iriodrv.DMATtoHOSTNCh[i];
//	    errlogSevPrintf(errlogInfo,"[%s-%d][%s]Number of channels of DMA%d is:%d\n",__func__,__LINE__,pdrvPvt->portName,i,(int)pdrvPvt->drvPvt.DMATtoHOSTNCh[i]);
//
	}
//    errlogSevPrintf(errlogInfo,"[%s-%d][%s]Total number of channels in ALL DMAs is:%d\n",__func__,__LINE__,pdrvPvt->portName,number_dma_ch);
//    errlogSevPrintf(errlogInfo,"[%s-%d][%s]Total number of maxAI(%d)+maxDI(%d)+CH(%d) is:%d\n",__func__,__LINE__,pdrvPvt->portName,pdrvPvt->drvPvt.max_analogoutputs, pdrvPvt->drvPvt.max_digitalsinputs, number_dma_ch, (pdrvPvt->drvPvt.max_analogoutputs+pdrvPvt->drvPvt.max_digitalsinputs+number_dma_ch));
//

	int st = resources();

}

irio::~irio(void) {
	//TODO:Analyze the functions to execute in object destruction
	//parar threads

}
//TODO: find a method to detect erros in the calls to CreateParam and accumulate to simplify the error checking
int irio::createIRIOParams(void) {
	asynStatus status=asynSuccess;

		status=createParam(DeviceSerialNumberString, asynParamOctet,
				&DeviceSerialNumber);


		status=createParam(EPICSVersionString, asynParamOctet,
				&EPICSVersionMessage);
		setStringParam(EPICSVersionMessage, "Hola mundo");
		status= createParam(IRIOVersionString, asynParamOctet,
				&IRIOVersionMessage);
		status= createParam(DeviceNameString, asynParamOctet, &DeviceName);
		status= createParam(FPGAStatusString, asynParamOctet, &FPGAStatus);
		status= createParam(InfoStatusString, asynParamOctet, &InfoStatus);
		status= createParam(VIversionString, asynParamOctet, &VIversion);

		status= createParam(DeviceTempString, asynParamFloat64, &DeviceTemp);
		status = createParam(AIString, asynParamFloat64, &AI);

		//asynInt32

		status= createParam(RIODeviceStatusString, asynParamInt32,
				&riodevice_status);
		status= createParam(SamplingRateString, asynParamInt32, &SamplingRate);
		status= createParam(SR_AI_IntrString, asynParamInt32, &SR_AI_Intr);
		status= createParam(SR_DI_IntrString, asynParamInt32, &SR_DI_Intr);
		status= createParam(DebugString, asynParamInt32, &debug);
		status= createParam(GroupEnableString, asynParamInt32, &GroupEnable);
		status= createParam(FPGAStartString, asynParamInt32, &FPGAStart);
		setIntegerParam(FPGAStart, 0);
		status= createParam(DAQStartStopString, asynParamInt32, &DAQStartStop);
		status= createParam(DFString, asynParamInt32, &DF);
		status= createParam(DevQualityStatusString, asynParamInt32,
				&DevQualityStatus);
		setIntegerParam(DevQualityStatus, 255);
		status= createParam(DMAOverflowString, asynParamInt32, &DMAOverflow);

		status = createParam(AOEnableString, asynParamInt32, &AOEnable);
		status = createParam(SGFreqString, asynParamInt32, &SGFreq);
		status = createParam(SGUpdateRateString, asynParamInt32, &SGUpdateRate);

		status = createParam(SGSignalTypeString, asynParamInt32, &SGSignalType);
		status = createParam(SGPhaseString, asynParamInt32, &SGPhase);
		status = createParam(DIString, asynParamInt32, &DI);
		status = createParam(DOString, asynParamInt32, &DO);
		status = createParam(auxAIString, asynParamInt32, &auxAI);
		status = createParam(auxAOString, asynParamInt32, &auxAO);
		status = createParam(auxDIString, asynParamInt32, &auxDI);
		status = createParam(auxDOString, asynParamInt32, &auxDO);
		return 0; //Manage the error
}

int irio::status_func(TStatus *status) {
	//irio_pvt_t* pdrvPvt=(irio_pvt_t *)drvPvt;
	char *error_string = NULL;
	std::string msg;

	riodevice_status_name_t next_status = _rio_device_status;
	int update = 0;
	switch (status->detailCode) {
	case Generic_Error:
		next_status = STATUS_DYNAMIC_CONFIG_ERROR;
		msg = "One of the values configured is out of bounds";
		error_string = (char*) msg.c_str();
		update = 1;
		break;
	case HardwareNotFound_Error:
		next_status = STATUS_NO_BOARD;
		irio_getErrorString(status->detailCode, &error_string);
		update = 1;
		break;
	case BitfileDownload_Error:
	case ListRIODevicesCommand_Error:
	case ListRIODevicesParsing_Error:
	case SignatureNotFound_Error:
	case MemoryAllocation_Error:
	case FileAccess_Error:
	case FileNotFound_Error:
	case FeatureNotImplemented_Error:
		next_status = STATUS_STATIC_CONFIG_ERROR;
		irio_getErrorString(status->detailCode, &error_string);
		update = 1;
		break;
	case NIRIO_API_Error:
		if ((_rio_device_status != STATUS_STATIC_CONFIG_ERROR)
				&& (_rio_device_status != STATUS_NO_BOARD)) {
			next_status = STATUS_HARDWARE_ERROR;
			irio_getErrorString(status->detailCode, &error_string);
			update = 1;
		}

		break;
	case InitDone_Error:
	case IOModule_Error:
	case ResourceNotFound_Error:
		next_status = STATUS_INCORRECT_HW_CONFIGURATION;
		irio_getErrorString(status->detailCode, &error_string);
		update = 1;
		break;
	case SignatureValueNotValid_Error:
	case ResourceValueNotValid_Error:
	case BitfileNotFound_Error:
	case HeaderNotFound_Error:
		next_status = STATUS_STATIC_CONFIG_ERROR; //CHANGE FOR STATUS_CONFIG_FILES_ERROR
		irio_getErrorString(status->detailCode, &error_string);
		update = 1;
		break;
	case Success:
		if (dyn_err_count == 0) {
			if (FPGAstarted == 1
					&& (_rio_device_status == STATUS_CONFIGURED
							|| _rio_device_status == STATUS_DYNAMIC_CONFIG_ERROR)) {
				next_status = STATUS_OK;
				msg = "Device is OK and running";
				error_string = (char*) msg.c_str();
				update = 1;
			}
			if (FPGAstarted == 0 && driverInitialized == 1
					&& (_rio_device_status == STATUS_DYNAMIC_CONFIG_ERROR
							|| _rio_device_status == STATUS_INITIALIZING)) {
				next_status = STATUS_CONFIGURED;
				msg = "Device is configured";
				error_string = (char*) msg.c_str();
				update = 1;
			}
		}
		break;
	case FPGAAlreadyRunning_Warning:
		if (FPGAstarted == 1 && _rio_device_status == STATUS_CONFIGURED) {
			next_status = STATUS_OK;
			msg = "FPGA started before IOC.";
			error_string = (char*) msg.c_str();
			update = 1;
		}
		break;
	case TemporaryFileDelete_Warning:
	case ResourceNotFound_Warning:
	case Read_NIRIO_Warning:
	case Read_Resource_Warning:
	case Write_NIRIO_Warning:
	case Write_Resource_Warning:
	case ConfigDMA_Warning:
	case ConfigUART_Warning:
	case ValueOOB_Warning:
	case Generic_Warning:
	case DAQtimeout_Warning:
		irio_getErrorString(status->detailCode, &error_string);
		update = 1;
		break;
	default:
		break;
	}

	_rio_device_status = next_status;
//	if(update==1){
//		callbackInt32(pdrvPvt, riodevice_status, 0, (epicsInt32)err_dev_stat_struct[pdrvPvt->rio_device_status].riodevice_status_name);
//		char* aux = pdrvPvt->InfoStatus;
//		pdrvPvt->InfoStatus=error_string;
//		callbackOctet(pdrvPvt,InfoStatus,pdrvPvt->InfoStatus,strlen(pdrvPvt->InfoStatus));
//		free(aux);
//	}else{
//		free(error_string);
//	}

	int retVal = asynSuccess;
	switch (status->code) {
	case IRIO_warning:
		//errlogSevPrintf(errlogMinor,"[status_func][%s]IRIO WARNING Code:%d. Description:%s. IRIO Messages:%s",pdrvPvt->portName,status->detailCode,pdrvPvt->InfoStatus,status->msg);
		retVal = asynError;
		break;
	case IRIO_error:
		//errlogSevPrintf(errlogFatal,"[status_func][%s]IRIO ERROR Code:%d. Description:%s. IRIO Messages:%s",pdrvPvt->portName,status->detailCode,pdrvPvt->InfoStatus,status->msg);
		retVal = asynError;
		break;
	default:
		retVal = asynSuccess;
		break;
	}
	irio_resetStatus(status);
	return retVal;

}

void irio::report(FILE *fp, int details) {
	char *platform[2] = { "FlexRIO", "cRIO" };
	char *couplingMode[3] = { "AC", "DC", "N/A" };

	int i = _iriodrv.devProfile;
	if (_iriodrv.platform) { // cRIOs
		i = i + 4;
	}
	fprintf(fp,
			"\t---------------------------------------------------------- \n");
	fprintf(fp, "\tNI-RIO EPICS device support \n");
	fprintf(fp,
			"\t---------------------------------------------------------- \n");
	fprintf(fp, "\tDevice Name                         : %s \n", DEVICE_NAME);
	fprintf(fp, "\tRIO platform                        : %s \n",
			platform[_iriodrv.platform]);
	fprintf(fp, "\tRIO Device Model                    : %s \n",
			_iriodrv.RIODeviceModel);
	fprintf(fp, "\tRIO Serial Number                   : %s \n",
			_iriodrv.DeviceSerialNumber);
	fprintf(fp, "\tModule ID (xxx:FPGASTART must be ON): %u \n",
			_iriodrv.moduleValue);
	fprintf(fp, "\tLabVIEW Project Name (VI)           : %s \n",
			_iriodrv.projectName);
	fprintf(fp, "\tEPICS Driver Version                : %s \n",
			EPICS_DRIVER_VERSION);
	std::string tmp;
	getStringParam(IRIOVersionMessage, tmp);
	fprintf(fp, "\tIRIO Library Version                : %s \n", tmp.c_str());
	fprintf(fp, "\tDevice Profile                      : %s \n",
			platform_profile[i].platform_profile_string);

	switch (details) {
	case 0:
		break;
	case 1:
	case 2:

		fprintf(fp,
				"\t---------------------------------------------------------- \n");
		fprintf(fp, "\tAdditional Details \n");
		fprintf(fp,
				"\t---------------------------------------------------------- \n");
		fprintf(fp, "\tFPGA Main Loop Frequency                 : %u \n",
				_iriodrv.Fref);
		fprintf(fp, "\tMin Sampling Rate                        : %f \n",
				_iriodrv.minSamplingRate);
		fprintf(fp, "\tMax Sampling Rate                        : %f \n",
				_iriodrv.maxSamplingRate);
		fprintf(fp, "\tMin Analog Output                        : %f \n",
				_iriodrv.minAnalogOut);
		fprintf(fp, "\tMax Analog Output                        : %f \n",
				_iriodrv.maxAnalogOut);
		fprintf(fp, "\tCoupling Mode                            : %s \n",
				couplingMode[_iriodrv.couplingMode]);
		fprintf(fp, "\tConversion to Volts of analog inputs     : %g \n",
				_iriodrv.CVADC);
		fprintf(fp, "\tConversion from Volts for analog outputs : %g \n",
				_iriodrv.CVDAC);
		//	     for(i=0;pdrvPvt->drvPvt.DMATtoHOSTNo.value;i++){
		//		 	 fprintf(fp, "\t---------------------------------------------------------- \n" );
		//		 	 fprintf(fp, "\tDMA%d details \n",i);
		//		 	 fprintf(fp, "\t---------------------------------------------------------- \n" );
		//	     	 fprintf(fp, "\tSize of DMA%d data blocks in terms of DMA(U64) words : %d \n",i, pdrvPvt->drvPvt.DMATtoHOSTBlockNWords[i]);
		//		 	 fprintf(fp, "\tSample size used by the DMA%d : %d \n",i, pdrvPvt->drvPvt.DMATtoHOSTSampleSize[i]);
		//		 	 fprintf(fp, "\tFrame type used by the DMA%d : %d \n",i, pdrvPvt->drvPvt.DMATtoHOSTFrameType[i]);
		//		 	 fprintf(fp, "\tNumber of Channels in DMA%d : %d \n",i, pdrvPvt->drvPvt.DMATtoHOSTNCh[i]);
		//	     }

		break;

	default:
		break;
	}

	fprintf(fp,
			"\t---------------------------------------------------------- \n");
	fprintf(fp, "\t asynReport params \n");
	fprintf(fp,
			"\t---------------------------------------------------------- \n");
	asynPortDriver::report(fp, details); //This displays the info of the parameters registered

}

int irio::resources(void) {
	int st;
	switch (_iriodrv.platform) {
	case IRIO_FlexRIO:
		if (_iriodrv.devProfile == 1 || _iriodrv.devProfile == 3) {
			//IMAQ Profile
			UARTReceivedMsg.resize(MAX_UARTSIZE);
			UARTReceivedMsg.empty();
			sizeX = 256;
			sizeY = 256;
		}
		//Reserving the needed space for the specific number of DMA
		ai_dma_thread.reserve(_iriodrv.DMATtoHOSTNo.value);
		//initializing every object with the number
		// we need to fill the vector with the elements (iterator cannot be used)
		for (uint8_t j = 0; j < _iriodrv.DMATtoHOSTNo.value; j++) {
			ai_dma_thread.push_back(dmathread(_portName, j));
		}
		for (std::vector<dmathread>::iterator it = ai_dma_thread.begin();
				it != ai_dma_thread.end(); ++it) {
			it->runthread();
		}
		TStatus irio_status;
		st = irio_setUpDMAsTtoHost(&_iriodrv, &irio_status);
		if (st != IRIO_success) {
			status_func(&irio_status);
			if (st == IRIO_error) {
				return st;
			}
		}
		break;
	case IRIO_cRIO:
		if (_iriodrv.devProfile != 1) {
			//Reserving the needed space for the specific number of DMA
			ai_dma_thread.reserve(_iriodrv.DMATtoHOSTNo.value);
			//initializing every object with the number
			// we need to fill the vector with the elements (iterator cannot be used)
			for (uint8_t j = 0; j < _iriodrv.DMATtoHOSTNo.value; j++) {
				ai_dma_thread.push_back(dmathread(_portName, j));
			}
			for (std::vector<dmathread>::iterator it = ai_dma_thread.begin();
					it != ai_dma_thread.end(); ++it) {
				it->runthread();
			}
			TStatus irio_status;
			st = irio_setUpDMAsTtoHost(&_iriodrv, &irio_status);
			if (st != IRIO_success) {
				status_func(&irio_status);
				if (st == IRIO_error) {
					return st;
				}
			}
		}
		break;
	default:
		st = IRIO_error;
		return st;
		break;
	}
	return IRIO_success;


}


asynStatus irio::readOctet(asynUser *pasynUser, char *value, size_t maxChars,
		size_t *nActual, int *eomReason) {
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "readOctet";

	/* Set the parameter in the parameter library. */

	status = (asynStatus) getStringParam(function, (int) maxChars, value);
	//ojo con las sobrecargas de las funciones de getSttringParam????

	/* Fetch the parameter string name for possible use in debugging */
	getParamName(function, &paramName);
	asynPrint(pasynUser, function, "%s", paramName);

	//Hay que introducir cierta lógica que impida acceder a la fpga sino esta inicializada
	if (driverInitialized == 0) {
		*nActual = 0;
		return asynSuccess;
	}
	//Nose puede usar switch porque funtion es una variable
	if (function == DeviceSerialNumber) {
		std::string tmp(this->_iriodrv.DeviceSerialNumber);
		std::strcpy(value, tmp.c_str());
		*nActual = tmp.length();
	} else if (function == EPICSVersionMessage) {
		std::string tmp(EPICS_DRIVER_VERSION);
		std::strcpy(value, tmp.c_str());
		*nActual = tmp.length();
	} else if (function == IRIOVersionMessage) {
		std::string tmp(40, ' ');
		TStatus status;
		int st = irio_getVersion((char*) tmp.c_str(), &status);
		std::strcpy(value, tmp.c_str());
		*nActual = tmp.length();
	} else if (function == DeviceName) {
		std::string tmp(DEVICE_NAME);
		std::strcpy(value, tmp.c_str());
		*nActual = tmp.length();
	} else if (function == FPGAStatus) {
		std::string tmp(this->_sFPGAStatus);
		std::strcpy(value, tmp.c_str());
		*nActual = tmp.length();
	} else if (function == InfoStatus) {
		std::string tmp(this->_sInfoStatus);
		std::strcpy(value, tmp.c_str());
		*nActual = tmp.length();
	} else if (function == VIversion) {
		std::string tmp(40, ' ');
		TStatus status;
		int st = irio_getFPGAVIVersion(&(this->_iriodrv), (char*) tmp.c_str(),
				maxChars, nActual, &status);
		std::strcpy(value, tmp.c_str());
		*nActual = tmp.length();
	}
	setStringParam(function, value); //updating the Param for AsynPortDriver
	return status;

}

asynStatus irio::writeOctet(asynUser *pasynUser, const char *value,
		size_t maxChars, size_t *nActual) {
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "readOctet";

	/* Set the parameter in the parameter library. */
	status = (asynStatus) setStringParam(function, (char*) value);

	/* Fetch the parameter string name for possible use in debugging */
	getParamName(function, &paramName);
	asynPrint(pasynUser, function, "%s", paramName);
	*nActual = maxChars;
	return status;
}

asynStatus irio::readInt32(asynUser *pasynUser, epicsInt32 *value) {
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "readInt32";
	int addr = 0;
	TStatus irio_status;
	/* Set the parameter in the parameter library. */
	status = (asynStatus) getIntegerParam(function, value);

	/* Fetch the parameter string name for possible use in debugging */
	getParamName(function, &paramName);
	asynPrint(pasynUser, function, "%s", paramName);
	//TODO: revisar el uso de la funcion de abajo
	status = parseAsynUser(pasynUser, &function, &addr, &paramName);
	if (function == riodevice_status) {
		*value = _rio_device_status;
	} else if (function == SamplingRate) {
		if (_iriodrv.platform == IRIO_cRIO && _iriodrv.devProfile == 1) {
			int st = irio_getSamplingRate(&_iriodrv, addr, value, &irio_status);
			if (st == IRIO_success) {
				if (*value == 0)
					*value = 1;
				*value = _iriodrv.Fref / (*value);
			}

		} else {
			int st = irio_getDMATtoHostSamplingRate(&_iriodrv, addr, value,
					&irio_status);
			if (st == IRIO_success) {
				if (*value == 0)
					*value = 1;
				*value = _iriodrv.Fref / (*value);
				//TODO: iriodrv.ai_dma_thread[addr].SR=*value;
			}
		}

	} else if (function == SR_AI_Intr) {

	} else if (function == SR_DI_Intr) {

	} else if (function == debug) {
		int st = irio_getDebugMode(&_iriodrv, value, &irio_status);

	} else if (function == GroupEnable) {

	} else if (function == FPGAStart) {

	} else if (function == DAQStartStop) {
		int st = irio_getDAQStartStop(&_iriodrv, value, &irio_status);
		if(st==IRIO_success){
			acq_status=*value;
		}

	} else if (function == DF) {

	} else if (function == DevQualityStatus) {

	} else if (function == DMAOverflow) {

	} else if (function == AOEnable) {

	} else if (function == SGFreq) {

	} else if (function == SGUpdateRate) {

	} else if (function == SGSignalType) {

	} else if (function == SGPhase) {

	} else if (function == DI) {
		int st = irio_getDI(&_iriodrv, addr, value, &irio_status);

	} else if (function == DO) {
		int st = irio_getDO(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxAI) {
		int st = irio_getAuxAI(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxAO) {
		int st = irio_getAuxAO(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxDI) {
		int st = irio_getAuxDI(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxDO) {
		int st = irio_getAuxDO(&_iriodrv, addr, value, &irio_status);


	}
	setIntegerParam(function, *value);
	return status;
}

asynStatus irio::writeInt32(asynUser *pasynUser, epicsInt32 value) {
	int function;
	int addr;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "writeInt32";
	TStatus irio_status;

	status = parseAsynUser(pasynUser, &function, &addr, &paramName);

	/* Set the parameter in the parameter library. */
	status = (asynStatus) setIntegerParam(function, value);


	asynPrint(pasynUser, function, "%s", paramName);

	if (function == riodevice_status) {

	} else if (function == SamplingRate) {

	} else if (function == SR_AI_Intr) {

	} else if (function == SR_DI_Intr) {

	} else if (function == debug) {
		int st = irio_setDebugMode(&_iriodrv, value,&irio_status);

	} else if (function == GroupEnable) {

	} else if (function == FPGAStart) {
		int st = irio_setFPGAStart(&_iriodrv, (int32_t) value, &irio_status);
		if (st == IRIO_success) {
			FPGAstarted = 1;
			//errlogSevPrintf(errlogInfo,"[%s-%d][%s]FPGAStart (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
	} else if (function == DAQStartStop) {
		int st = irio_setDAQStartStop(&_iriodrv,value,&irio_status);
				if (st==IRIO_success){
					acq_status = value;
				}

	} else if (function == DF) {

	} else if (function == DevQualityStatus) {

	} else if (function == DMAOverflow) {

	} else if (function == AOEnable) {

	} else if (function == SGFreq) {

	} else if (function == SGUpdateRate) {

	} else if (function == SGSignalType) {

	} else if (function == SGPhase) {

	} else if (function == DO) {
		int st = irio_setDO(&_iriodrv, addr, value, &irio_status);

	} else if (function == auxAO) {
		int st = irio_setAuxAO(&_iriodrv, addr, value, &irio_status);

	} else if (function == auxDO) {
		int st = irio_setAuxDO(&_iriodrv, addr, value, &irio_status);

	}
	return status;
}
//
//asynStatus irio::readInt64(asynUser *pasynUser, epicsInt64 *value) {
//}
//
//asynStatus irio::writeInt64(asynUser *pasynUser, epicsInt64 value) {
//}
//
asynStatus irio::readFloat64(asynUser *pasynUser, epicsFloat64 *value) {
	int function ;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "readFloat64";
	int addr = 0;
	TStatus irio_status;
	int32_t vaux;

	irio_initStatus(&irio_status);

	status = parseAsynUser(pasynUser, &function, &addr, &paramName);

	/* Set the parameter in the parameter library. */
	status = (asynStatus) getDoubleParam(function, value);

	//asynPrint(pasynUser,function, "%s",paramName);

	if (function == DeviceTemp) {
		int st = irio_getDevTemp(&_iriodrv, &vaux, &irio_status);
		if (st == IRIO_success) {
			*value = (epicsFloat64) vaux * 0.25;
			//errlogSevPrintf(errlogInfo,"[%s-%d][%s]DeviceTemp (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
		} else
			*value = 0;

	} else if (function == AI) {
		int st = irio_getAI(&_iriodrv,addr,&vaux,&irio_status);
		if (st == IRIO_success) {
			*value = (epicsFloat64) vaux * this->_iriodrv.CVADC;
		} else
			*value = 0;
	}
	setDoubleParam(function, *value);
	return status;
}

//
//asynStatus irio::writeFloat64(asynUser *pasynUser, epicsFloat64 value) {
//}
//
//asynStatus irio::readInt8Array(asynUser *pasynUser, epicsInt8 *value,
//		size_t nElements, size_t *nIn) {
//}
//
//asynStatus irio::writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
//		size_t nElements) {
//}
//
//asynStatus irio::readInt16Array(asynUser *pasynUser, epicsInt16 *value,
//		size_t nElements, size_t *nIn) {
//}
//
//asynStatus irio::writeInt16Array(asynUser *pasynUser, epicsInt16 *value,
//		size_t nElements) {
//}
//
//asynStatus irio::readInt32Array(asynUser *pasynUser, epicsInt32 *value,
//		size_t nElements, size_t *nIn) {
//}
//
//asynStatus irio::writeInt32Array(asynUser *pasynUser, epicsInt32 *value,
//		size_t nElements) {
//}
//
//asynStatus irio::readInt64Array(asynUser *pasynUser, epicsInt64 *value,
//		size_t nElements, size_t *nIn) {
//}
//
//asynStatus irio::writeInt64Array(asynUser *pasynUser, epicsInt64 *value,
//		size_t nElements) {
//}
//
//asynStatus irio::readFloat32Array(asynUser *pasynUser, epicsFloat32 *value,
//		size_t nElements, size_t *nIn) {
//}
//
//asynStatus irio::writeFloat32Array(asynUser *pasynUser, epicsFloat32 *value,
//		size_t nElements) {
//}
//
//asynStatus irio::readFloat64Array(asynUser *pasynUser, epicsFloat64 *value,
//		size_t nElements, size_t *nIn) {
//}
//
//asynStatus irio::writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value,
//		size_t nElements) {
//}
//

void irio::nirio_shutdown() {
}

void dmathread::aiDMA_thread(void *p) {
	//auto pt = (dmathread*) p;
	auto pt =static_cast<dmathread*>(p);
	do {
		usleep(1000);
	} while (pt->_threadends == 0);
}
dmathread::dmathread(const std::string &device, uint8_t id) {
	_thread_id = NULL;
	_name = device + "-DMA_" + std::to_string(id);
	_id = id;
	_threadends = 0;
	_endAck = 0;
	_DecimationFactor = 1;
	_SR = 1;
	_blockSize = 1;
	_IdRing = NULL;
	_dmanumber = id;
}
dmathread::~dmathread() {

}
void dmathread::runthread(void) {

	_thread_id = (epicsThreadId*) epicsThreadCreate(_name.c_str(),
			epicsThreadPriorityHigh,
			epicsThreadGetStackSize(epicsThreadStackBig), /*(EPICSTHREADFUNC)*/
			(this->aiDMA_thread), (void*) this);
}

