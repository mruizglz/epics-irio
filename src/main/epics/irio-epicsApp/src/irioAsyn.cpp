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

//local variable
globalData_t globalData[MAX_NUMBER_OF_CARDS]; //array of globalData_t structs to store all necessary data read from record database when gettingDBInfo method is called.
int call_gettingDBInfo; // Variable to call gettingDBInfo only once.

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

static void gettingDBInfo(initHookState state)
{
	DBENTRY             dbentry;
	DBENTRY				*pdbentry= &dbentry;
	dbRecordType        *pdbRecordType=NULL;
	dbRecordNode        *pdbRecordNode=NULL;
	int i=0,j=0;

	switch (state){
	case initHookAfterInitDatabase:
		if (call_gettingDBInfo==0){
			for(j = 0; j < MAX_NUMBER_OF_CARDS; j++){
				if(globalData[j].init_success == 1){
					std::string recInpString;
					std::string auxrecInpString;
					std::string recScan;
					std::string recInp;
					std::string recPortName;
					std::string portname;
					std::string recReason;
					std::string recChString;

					printf("##################################################################################################\n");
					printf("##       Reading record database for RIO device with portName:RIO_%d at initHookBeginning       ##\n",j);
					printf("##################################################################################################\n");

					dbInitEntry(pdbbase, &dbentry); //pdbase is an EPICS global variable
					pdbRecordType = (dbRecordType *)ellFirst(&pdbbase->recordTypeList);

					while (pdbRecordType != NULL){
						int recordType_ai = strcmp(pdbRecordType->name, "ai");
						int recordType_bi = strcmp(pdbRecordType->name, "bi");
						int recordType_waveform = strcmp(pdbRecordType->name, "waveform");
						if((recordType_ai==0)||(recordType_bi==0)||(recordType_waveform==0)){
							pdbRecordNode = (dbRecordNode *)ellFirst(&pdbRecordType->recList);
							while(pdbRecordNode != NULL){
								if (!dbFindRecord(&dbentry,pdbRecordNode->recordname)){
									if(!dbFindField(pdbentry, "SCAN")){
										recInpString = dbGetString(pdbentry);
										if(!recInpString.compare("I/O Intr")){   				//Current record is I/O Intr.
											recScan = recInpString;
											if(!dbFindField(pdbentry, "INP")){
												recInpString = dbGetString(pdbentry);
												recInpString = strstr(recInpString.c_str(),"@asyn");
												if(!recInpString.empty()){							//Current record is asyndriver based on.
													recInp = recInpString;
													recInpString = strstr(recInpString.c_str(),"RIO_");
													if(!recInpString.empty()){						//Current record is IRIOdriver based on.
														auxrecInpString = recInpString;
														recPortName = strtok((char*)auxrecInpString.c_str(), ",");
														portname = "RIO_" + std::to_string(j);
														if(!recPortName.compare(portname)){
															int recChNumber = 0;
															recChString = strstr(recInpString.c_str(),",");///this is the separator for the channel
															recChString = recChString.substr(1);
															recReason = recChString.substr(2);			//this is the separator for the reason
															recChNumber = stoi(recChString);
															int reason_AI = recReason.compare("AI");
															int reason_DI = recReason.compare("DI");
															int reason_CH = recReason.compare("CH");
															if((reason_AI == 0)||(reason_DI == 0)||(reason_CH == 0)){    //only AI,DI and waveform PVs belonging to IRIO devices using asynDriver with SCAN=I/O Intr are considered.
																int aux=0;
																asprintf(&globalData[j].intr_records[i].scan, recScan.c_str());
																asprintf(&globalData[j].intr_records[i].type, pdbRecordType->name);
																asprintf(&globalData[j].intr_records[i].name, pdbRecordNode->recordname);
																asprintf(&globalData[j].intr_records[i].input, recInp.c_str());
																globalData[j].intr_records[i].addr = recChNumber;
																asprintf(&globalData[j].intr_records[i].reason, recReason.c_str());
																asprintf(&globalData[j].intr_records[i].portName, recPortName.c_str());
																if(!recReason.compare("CH")){
																	if(!dbFindField(pdbentry, "NELM")){
																		recInpString = dbGetString(pdbentry);
																		if(!recInpString.empty()){
																			aux = stoi(recInpString);
																			globalData[j].intr_records[i].nelm = aux;
																			globalData[j].ch_nelm[recChNumber] = aux;
																		}
																	}
																}else{
																	globalData[j].intr_records[i].nelm = 1;
																}
																dbFindField(pdbentry, "TSE");
																recInpString = dbGetString(pdbentry);
																aux=stoi(recInpString);
																globalData[j].intr_records[i].timestamp_source = aux;
																errlogSevPrintf(errlogInfo,"[%s-%d][RIO_%d]globalData[%d].intr_records[%d]=Type:%s;Name:%s;INP:%s;PortName:%s;Addr:%d;Reason:%s;SCAN:%s;NELM:%d;TSE:%d\n",__func__,__LINE__,j,j,i,globalData[j].intr_records[i].type,globalData[j].intr_records[i].name,globalData[j].intr_records[i].input,globalData[j].intr_records[i].portName,globalData[j].intr_records[i].addr,globalData[j].intr_records[i].reason,globalData[j].intr_records[i].scan,globalData[j].intr_records[i].nelm,globalData[j].intr_records[i].timestamp_source);
																i++;
															}
														}
													}

												}
											}
										}
									}
								}
								pdbRecordNode = (dbRecordNode *)ellNext(&pdbRecordNode->node);
							}
						}
						pdbRecordType = (dbRecordType *)ellNext(&pdbRecordType->node);
					}
					globalData[j].io_number = i;
					i = 0;
					errlogSevPrintf(errlogInfo,"[%s-%d][RIO_%d]Number of AI,DI or CH records belonging to IRIO device with port:RIO_%d using asynDriver with SCAN=I/O Intr is: %d\n",__func__,__LINE__,j,j,globalData[j].io_number);
				}
			}
		}
		call_gettingDBInfo = 1;
		break;

	case initHookAfterIocRunning:
		if(call_gettingDBInfo == 1){
			for(i = 0; i < MAX_NUMBER_OF_CARDS; ++i){
				for(j = 0; j < globalData[i].number_of_DMAs; ++j){
					if(globalData[i].dma_thread_created[j] == 1){
						globalData[i].dma_thread_run[j] = 1;
					}
				}
			}
			call_gettingDBInfo = 0;
		}
		break;

	default:
		break;
	}
}


irio::irio(const char *namePort, const char *DevSerial,
		const char *PXInirioModel, const char *projectName,
		const char *FPGAversion, int verbosity) :
		asynPortDriver(namePort, 16,
				asynOctetMask | asynInt32Mask | asynInt32ArrayMask
						| asynFloat64Mask | asynFloat32ArrayMask | asynEnumMask
						| asynDrvUserMask, // interface mask,
				asynOctetMask | asynInt32Mask | asynInt32ArrayMask
						| asynFloat64Mask | asynFloat32ArrayMask | asynEnumMask, // interrupt mask,
				ASYN_MULTIDEVICE, 1, 0, 0), driverInitialized(0), flag_close(0), flag_exit(0), epicsExiting(
				0), closeDriver(0), _rio_device_status(STATUS_INITIALIZING) {

	const char *functionName = "irio constructor";

	if (createIRIOParams()) throw std::logic_error("Problem creating the asynPort driver params"); //create all asynPort needed parameters
	epicsAtExit(nirio_epicsExit, this);

	initHookRegister(gettingDBInfo);  //for database analisys
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

	_portNumber = _portName.back() - '0';


	/* Initialize global data for each port Number */
	globalData[_portNumber].ch_nelm = NULL;
	globalData[_portNumber].intr_records = NULL;
	globalData[_portNumber].init_success = 0;
	globalData[_portNumber].io_number = 0;
	globalData[_portNumber].ai_poll_thread_created = 0;
	globalData[_portNumber].ai_poll_thread_run = 0;
	globalData[_portNumber].di_poll_thread_created = 0;
	globalData[_portNumber].di_poll_thread_run = 0;
	globalData[_portNumber].dma_thread_created = NULL;
	globalData[_portNumber].dma_thread_run = NULL;
	globalData[_portNumber].number_of_DMAs = 0;

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
	//TODO
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


	globalData[_portNumber].intr_records = (intr_records_t*) calloc((_iriodrv.max_analogoutputs+_iriodrv.max_digitalsinputs+number_dma_ch),sizeof(intr_records_t));
	globalData[_portNumber].ch_nelm = (int*) calloc(64,sizeof(int));
	globalData[_portNumber].dma_thread_created = (int*) calloc(_iriodrv.DMATtoHOSTNo.value,sizeof(int));
	globalData[_portNumber].dma_thread_run = (int*) calloc(_iriodrv.DMATtoHOSTNo.value,sizeof(int));

	int st = resources();
	globalData[_portNumber].init_success = 1;

}

irio::~irio(void) {
	//TODO:Analyze the functions to execute in object destruction
	//parar threads

}
//TODO: find a method to detect erros in the calls to CreateParam and accumulate to simplify the error checking
int irio::createIRIOParams(void) {
	int status = asynSuccess;  // asynStatus

	status |= createParam(DeviceSerialNumberString, asynParamOctet, &DeviceSerialNumber);
	status |= createParam(EPICSVersionString, asynParamOctet, &EPICSVersionMessage);
	status |= setStringParam(EPICSVersionMessage, "Hola mundo");
	status |= createParam(IRIOVersionString, asynParamOctet, &IRIOVersionMessage);
	status |= createParam(DeviceNameString, asynParamOctet, &DeviceName);
	status |= createParam(FPGAStatusString, asynParamOctet, &FPGAStatus);
	status |= createParam(InfoStatusString, asynParamOctet, &InfoStatus);
	status |= createParam(VIversionString, asynParamOctet, &VIversion);

	status |= createParam(DeviceTempString, asynParamFloat64, &DeviceTemp);
	status |= createParam(AIString, asynParamFloat64, &AI);
	status |= createParam(AOString, asynParamFloat64, &AO);
	status |= createParam(SGAmpString, asynParamFloat64, &SGAmp);
	status |= createParam(UserDefinedConversionFactorString, asynParamFloat64, &UsrDefinedConversionFactor);
	status |= createParam(CHString, asynParamFloat32Array, &CH);

		//asynInt32

	status |= createParam(RIODeviceStatusString, asynParamInt32, &riodevice_status);
	status |= createParam(SamplingRateString, asynParamInt32, &SamplingRate);
	status |= createParam(SR_AI_IntrString, asynParamInt32, &SR_AI_Intr);
	status |= createParam(SR_DI_IntrString, asynParamInt32, &SR_DI_Intr);
	status |= createParam(DebugString, asynParamInt32, &debug);
	status |= createParam(GroupEnableString, asynParamInt32, &GroupEnable);
	status |= createParam(FPGAStartString, asynParamInt32, &FPGAStart);
	status |=setIntegerParam(FPGAStart, 0);
	status |= createParam(DAQStartStopString, asynParamInt32, &DAQStartStop);
	status |= createParam(DFString, asynParamInt32, &DF);
	status |= createParam(DevQualityStatusString, asynParamInt32, &DevQualityStatus);
	status |= setIntegerParam(DevQualityStatus, 255);
	status |= createParam(DMAsOverflowString, asynParamInt32, &DMAsOverflow);
	status |= createParam(AOEnableString, asynParamInt32, &AOEnable);
	status |= createParam(SGFreqString, asynParamInt32, &SGFreq);
	status |= createParam(SGUpdateRateString, asynParamInt32, &SGUpdateRate);
	status |= createParam(SGSignalTypeString, asynParamInt32, &SGSignalType);
	status |= createParam(SGPhaseString, asynParamInt32, &SGPhase);
	status |= createParam(DIString, asynParamInt32, &DI);
	status |= createParam(DOString, asynParamInt32, &DO);
	status |= createParam(auxAIString, asynParamInt32, &auxAI);
	status |= createParam(auxAOString, asynParamInt32, &auxAO);
	status |= createParam(auxDIString, asynParamInt32, &auxDI);
	status |= createParam(auxDOString, asynParamInt32, &auxDO);

	if(status > 0)
		return asynError;

	return asynSuccess; //Manage the error
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
	int i = 0;
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
			ai_dma_thread.push_back(dmathread(_portName, j, this));
		}
		for (std::vector<dmathread>::iterator it = ai_dma_thread.begin();
				it != ai_dma_thread.end(); ++it) {
			it->runthread();
			globalData[_portNumber].dma_thread_created[i] = 1;
			i++;
		}
		globalData[_portNumber].number_of_DMAs = _iriodrv.DMATtoHOSTNo.value;

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
				ai_dma_thread.push_back(dmathread(_portName, j, this));
			}
			for (std::vector<dmathread>::iterator it = ai_dma_thread.begin();
					it != ai_dma_thread.end(); ++it) {
				it->runthread();
				globalData[_portNumber].dma_thread_created[i] = 1;
				i++;
			}
			globalData[_portNumber].number_of_DMAs = _iriodrv.DMATtoHOSTNo.value;

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


asynStatus irio::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason) {
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "readOctet";


	if(epicsExiting == 1){
		if(flag_exit == 0){
			flag_exit = 1;
			//errlogSevPrintf
		}
		*nActual = 0;
		return asynSuccess;
	}

	if(function != InfoStatus){
		if(driverInitialized == 0){
			if(flag_close == 0){
				flag_close = 1;
				//errlogSevprintf
			}
			*nActual = 0;
			return asynSuccess;
		}
	}

	/* Set the parameter in the parameter library. */

	status = (asynStatus) getStringParam(function, (int) maxChars, value);
	//ojo con las sobrecargas de las funciones de getSttringParam????

	/* Fetch the parameter string name for possible use in debugging */
	getParamName(function, &paramName);
	asynPrint(pasynUser, function, "%s", paramName);


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

asynStatus irio::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual) {
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
	int function;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "readInt32";
	int addr;
	int st = IRIO_success;	//TIRIOStatusCode??
	TStatus irio_status;
	irio_initStatus(&irio_status);

	//TODO: revisar el uso de la funcion de abajo
	status = parseAsynUser(pasynUser, &function, &addr, &paramName);

	if(epicsExiting == 1){
		if(flag_exit == 0){
			flag_exit = 1;
			//errlogSevPrintf
		}
		*value = 0;
		return asynSuccess;
	}

	if(function != riodevice_status){
		if(driverInitialized == 0){
			if(flag_close == 0){
				flag_close = 1;
				//errlogSevprintf
			}
			*value = 0;
			return asynSuccess;
		}
	}

	/* Set the parameter in the parameter library. */
	status = (asynStatus) getIntegerParam(function, value);

	//asynPrint(pasynUser, function, "%s", paramName);


	if (function == riodevice_status) {
		*value = _rio_device_status;

	} else if (function == SamplingRate) {
		if (_iriodrv.platform == IRIO_cRIO && _iriodrv.devProfile == 1) {
			st = irio_getSamplingRate(&_iriodrv, addr, value, &irio_status);
			if (st == IRIO_success) {
				if (*value == 0)
					*value = 1;
				*value = _iriodrv.Fref / (*value);
			}

		} else {
			st = irio_getDMATtoHostSamplingRate(&_iriodrv, addr, value,
					&irio_status);
			if (st == IRIO_success) {
				if (*value == 0)
					*value = 1;
				*value = _iriodrv.Fref / (*value);
				ai_dma_thread[addr]._SR = *value;
			}
		}
		//if IRIO_success

	} else if (function == SR_AI_Intr) {

	} else if (function == SR_DI_Intr) {

	} else if (function == debug) {
		st = irio_getDebugMode(&_iriodrv, value, &irio_status);

	} else if (function == GroupEnable) {
		st = irio_getDMATtoHostEnable(&_iriodrv, addr, value, &irio_status);


	} else if (function == FPGAStart) {
		st = irio_getFPGAStart(&_iriodrv, value, &irio_status);

	} else if (function == DAQStartStop) {
		st = irio_getDAQStartStop(&_iriodrv, value, &irio_status);
		if(st==IRIO_success){
			acq_status=*value;
		}

	} else if (function == DF) {
		ai_dma_thread[addr]._DecimationFactor = 1;
		*value = 1;
		//errlog

	} else if (function == DevQualityStatus) {
		st = irio_getDevQualityStatus(&_iriodrv, value, &irio_status);

	} else if (function == DMAsOverflow) {
		int aux;
		st = irio_getDMATtoHostOverflow(&_iriodrv, &aux, &irio_status);
		if(st == IRIO_success){
			*value = ((uint32_t)aux>>(uint32_t)addr)&0x00000001u;
		}

	} else if (function == AOEnable) {
		st = irio_getAOEnable(&_iriodrv, addr, value, &irio_status);

	} else if (function == SGFreq) {
		if(_iriodrv.NoOfSG > addr){
			if(sgData[addr].Freq != 0){
				*value =sgData[addr].Freq;
			}
			else {
				st = irio_getSGFreq(&_iriodrv, addr, value, &irio_status);
				if(st == IRIO_success){
					if(sgData[addr].UpdateRate == 0){
						int32_t aux=0;
						st = irio_getSGUpdateRate(&_iriodrv, addr, &aux, &irio_status);
						if (st == IRIO_success){
							if(aux==0){
								sgData[addr].UpdateRate = 0;
							}
							else{
								sgData[addr].UpdateRate = _iriodrv.SGfref[addr]/aux;
							}
						}
					}
					*value=(*value)*(epicsInt32)(((int64_t)sgData[addr].UpdateRate)/4294967296);
					sgData[addr].Freq=*value;
				}
			}
		}
		//else irio_mergestatus

	} else if (function == SGUpdateRate) {
		if(_iriodrv.NoOfSG > addr){
			if(sgData[addr].UpdateRate != 0){
				*value = sgData[addr].UpdateRate;
			}
			else{
				st = irio_getSGUpdateRate(&_iriodrv,addr, value, &irio_status);
				if (st == IRIO_success){
					if(*value == 0){
						sgData[addr].UpdateRate = 0;
					}
					else{
						*value = _iriodrv.SGfref[addr]/(*value);
						sgData[addr].UpdateRate = *value;
					}
				}
			}
		}
		//TODO: else irio_mergestatus

	} else if (function == SGSignalType) {
		st=irio_getSGSignalType(&_iriodrv, addr, value, &irio_status);

	} else if (function == SGPhase) {
		st=irio_getSGPhase(&_iriodrv, addr, value, &irio_status);

	} else if (function == DI) {
		st = irio_getDI(&_iriodrv, addr, value, &irio_status);

	} else if (function == DO) {
		st = irio_getDO(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxAI) {
		st = irio_getAuxAI(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxAO) {
		st = irio_getAuxAO(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxDI) {
		st = irio_getAuxDI(&_iriodrv, addr, value, &irio_status);


	} else if (function == auxDO) {
		st = irio_getAuxDO(&_iriodrv, addr, value, &irio_status);


	}
	//TODO: Camera Link and UART reasons

	setIntegerParam(function, *value);
	return status;
}

asynStatus irio::writeInt32(asynUser *pasynUser, epicsInt32 value) {
	int function;
	int addr;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "writeInt32";
	int st = IRIO_success;	//TIRIOStatusCode??
	TStatus irio_status;
	irio_initStatus(&irio_status);

	status = parseAsynUser(pasynUser, &function, &addr, &paramName);

	if(epicsExiting == 1){
		if(flag_exit == 0){
			flag_exit = 1;
			//errlogSevPrintf
		}
		return asynSuccess;
	}

	if(driverInitialized == 0){
		if(flag_close == 0){
			flag_close = 1;
			//errlogSevprintf
		}
		return asynSuccess;
	}

	/* Set the parameter in the parameter library. */
	status = (asynStatus) setIntegerParam(function, value);


	//asynPrint(pasynUser, function, "%s", paramName);

	if (function == SamplingRate) {
		if(value >= _iriodrv.minSamplingRate && value <= _iriodrv.maxSamplingRate){
			if(_iriodrv.platform == IRIO_cRIO && _iriodrv.devProfile == 1){ //CRIO IO
				value = _iriodrv.Fref / value;
				st = irio_setSamplingRate(&_iriodrv, addr, value, &irio_status);
			}
			else {
				ai_dma_thread[addr]._SR = value;
				value = _iriodrv.Fref / value;
				st = irio_setDMATtoHostSamplingRate(&_iriodrv, addr, value, &irio_status);
			}
			if(st == IRIO_success){
				// error_oob_array
			}
		}
		else {
			// irio_mergestatus + error_oob_array
		}

	} else if (function == SR_AI_Intr) {

	} else if (function == SR_DI_Intr) {

	} else if (function == debug) {
		st = irio_setDebugMode(&_iriodrv, value,&irio_status);

	} else if (function == GroupEnable) {
		st = irio_setDMATtoHostEnable(&_iriodrv, addr, value, &irio_status);

	} else if (function == FPGAStart) {
		st = irio_setFPGAStart(&_iriodrv, (int32_t) value, &irio_status);
		if (st == IRIO_success) {
			FPGAstarted = 1;
			//errlogSevPrintf(errlogInfo,"[%s-%d][%s]FPGAStart (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
	} else if (function == DAQStartStop) {
		st = irio_setDAQStartStop(&_iriodrv,value,&irio_status);
				if (st==IRIO_success){
					acq_status = value;
				}

	} else if (function == DF) {
		if(value >= 1){
			ai_dma_thread[addr]._DecimationFactor = value;
			//errlog + error oob array
		}
		else{
			//merge status + error_oob_array
		}

	} else if (function == AOEnable) {
		st = irio_setAOEnable(&_iriodrv, addr, value, &irio_status);


	} else if (function == SGFreq) {
		if(value > 0){
			if(_iriodrv.NoOfSG > addr){
				sgData[addr].Freq = value;

				if(sgData[addr].UpdateRate == 0){
					value = 0;
				}
				else{
					value= sgData[addr].Freq *( 4294967296 / sgData[addr].UpdateRate);
				}
				st = irio_setSGFreq(&_iriodrv, addr, value, &irio_status);
				if(st == IRIO_success){
					//errlogsevprintf SGfreq value+ error_oob_array
				}
				else{
					//errlogsevprint SGfreq error in irio_setSGFreq
				}
			}
			else{
				//iriomergestatus resourcenotfound
			}
		}
		else{
			//iriomergestatus out of bounds + error_oob_array
		}

	} else if (function == SGUpdateRate) {
		if(_iriodrv.NoOfSG > addr){
			if(value <= 0){
				/*irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SGUpdateRate (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
				if((pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
					pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
					pdrvPvt->dyn_err_count++;
				}*/
			}else{
				if(value > _iriodrv.SGfref[addr]){
					//irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SGUpdateRate (addr=%d) value: %d truncated to %d\n",__func__,__LINE__,pdrvPvt->portName,addr,value,pdrvPvt->drvPvt.SGfref[addr]);
					value = _iriodrv.SGfref[addr];
				}
				sgData[addr].UpdateRate = value;
				//Value contains the frequency desired to update analog output
				value = _iriodrv.SGfref[addr] / value;
				st = irio_setSGUpdateRate(&_iriodrv, addr, value, &irio_status);
				if(st == IRIO_success){
					//errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGUpdateRate (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->sgData[addr].UpdateRate);
				}
				//Update SGFreq to keep the relation between SGFreq y SGUpdateRate
				int32_t aux = 0;

				aux = sgData[addr].Freq * (4294967296/sgData[addr].UpdateRate);
				st=irio_setSGFreq(&_iriodrv, addr, aux, &irio_status);
				if(st == IRIO_success){
					//errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGFreq (addr=%d) value updated to: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,aux);
				}
				/*if((pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
					pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
					pdrvPvt->dyn_err_count--;
				}*/
			}

		}else{
			//irio_mergeStatus(&irio_status,ResourceNotFound_Warning,0,"[%s-%d][%s]ResourceNotFound_Warning: SG (addr=%d) out of bounds. Number of Signal Generators in FPGA = %d\n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->drvPvt.NoOfSG);
		}

	} else if (function == SGSignalType) {
		st=irio_setSGSignalType(&_iriodrv, addr, value, &irio_status);

	} else if (function == SGPhase) {
		st=irio_setSGPhase(&_iriodrv, addr, (int32_t)value, &irio_status);

	} else if (function == DO) {
		st = irio_setDO(&_iriodrv, addr, value, &irio_status);

	} else if (function == auxAO) {
		st = irio_setAuxAO(&_iriodrv, addr, value, &irio_status);

	} else if (function == auxDO) {
		st = irio_setAuxDO(&_iriodrv, addr, value, &irio_status);

	}

	//TODO: Camera Link and UART reasons
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
	int32_t vaux;
	int st = IRIO_success;	//TIRIOStatusCode??
	TStatus irio_status;
	irio_initStatus(&irio_status);

	status = parseAsynUser(pasynUser, &function, &addr, &paramName);

	/* Set the parameter in the parameter library. */
	status = (asynStatus) getDoubleParam(function, value);

	if(epicsExiting == 1){
		if(flag_exit == 0){
			flag_exit = 1;
			//errlogSevPrintf
		}
		*value = 0;
		return asynSuccess;
	}

	if(driverInitialized == 0){
		if(flag_close == 0){
			flag_close = 1;
			//errlogSevprintf
		}
		*value = 0;
		return asynSuccess;
	}
	//asynPrint(pasynUser,function, "%s",paramName);

	if (function == DeviceTemp) {
		st = irio_getDevTemp(&_iriodrv, &vaux, &irio_status);
		if (st == IRIO_success) {
			*value = (epicsFloat64) vaux * 0.25;
			//errlogSevPrintf(errlogInfo,"[%s-%d][%s]DeviceTemp (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
		} else
			*value = 0;

	} else if (function == AI) {
		st = irio_getAI(&_iriodrv,addr,&vaux,&irio_status);
		if (st == IRIO_success) {
			*value = (epicsFloat64) vaux * _iriodrv.CVADC;
		} else
			*value = 0;

	} else if (function == AO) {
		st = irio_getAO(&_iriodrv,addr,&vaux,&irio_status);
		if (st == IRIO_success) {
			*value = (epicsFloat64) vaux / _iriodrv.CVDAC;
		} else
			*value = 0;
	} else if (function == SGAmp) {
		st = irio_getSGAmp(&_iriodrv, addr, &vaux, &irio_status);
		if(st ==  IRIO_success){
			*value = (epicsFloat64)vaux/_iriodrv.CVDAC;
			//errlogsevprintf
		}
	} else if (function == UsrDefinedConversionFactor) {
		if(UserDefinedConversionFactor[addr] <= 0){
			UserDefinedConversionFactor[addr] = 1;
		}
		*value = UserDefinedConversionFactor[addr];
	}
	setDoubleParam(function, *value);
	return status;
}


asynStatus irio::writeFloat64(asynUser *pasynUser, epicsFloat64 value) {
	int function;
	asynStatus status = asynSuccess;
	const char *paramName;
	const char *functionName = "writeFloat64";
	int addr = 0;
	int32_t vaux;
	int st = IRIO_success;	//TIRIOStatusCode??
	TStatus irio_status;
	irio_initStatus(&irio_status);


	status = parseAsynUser(pasynUser, &function, &addr, &paramName);

	if(epicsExiting == 1){
		if(flag_exit == 0){
			flag_exit = 1;
			//errlogSevPrintf
		}
		return asynSuccess;
	}

	if(driverInitialized == 0){
		if(flag_close == 0){
			flag_close = 1;
			//errlogSevprintf
		}
		return asynSuccess;
	}

	/* Set the parameter in the parameter library. */
	status = (asynStatus) setDoubleParam(function, value);

	//asynPrint(pasynUser,function, "%s",paramName);

	if (function == AO) {
		if (value > _iriodrv.minAnalogOut && value < _iriodrv.maxAnalogOut){
			st = irio_setAO(&_iriodrv, addr, (int32_t)(value * _iriodrv.CVDAC), &irio_status);
			//TODO: Error Out of bounds
		}
		//else{
		//}
	} else if (function == SGAmp){
		if(value > _iriodrv.minAnalogOut && value < _iriodrv.maxAnalogOut){
			int st = irio_setSGAmp(&_iriodrv, addr, (int32_t) (value * _iriodrv.CVDAC), &irio_status);

			if(st == IRIO_success){
				//errlogsevprintf + error_oob_array
			}
		}
		else{
			//iriomergestatus oob + error_oob_array
		}
	} else if (function == UsrDefinedConversionFactor) {
		if(_iriodrv.DMATtoHOSTFrameType[addr] >= 128){
			if(value > 0){
				UserDefinedConversionFactor[addr] = value;
				//error oob array
			}
			else{
				//irio merge tatus oob + error_oob_array
			}
		}
		else{
			//errlogSev.... UserConversionFactor disabled
		}
	}

	return status;
}

asynStatus irio::readFloat32Array(asynUser *pasynUser, epicsFloat32 *value,
             size_t nElements, size_t *nIn) {

       int function ;
       asynStatus status = asynSuccess;
       const char *paramName;
       const char *functionName = "readFloat64";
       int addr = 0;
       int32_t vaux;
       int st = IRIO_success;  //TIRIOStatusCode??
       TStatus irio_status;
       irio_initStatus(&irio_status);

       status = parseAsynUser(pasynUser, &function, &addr, &paramName);

       if(function == CH){

       }
       return status;

}

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
//             size_t nElements, size_t *nIn) {
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

	int status = IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	std::vector<uint64_t> dataBuffer;
	std::vector<uint64_t> cleanbuffer;
	std::vector<pvthread> ai_pv_publish;
	float **aux = NULL;
	int i = 0;
	size_t buffersize; //in u64
	int found = 0;
	uint16_t samples_per_channel = 0;
	int chIndex;
	double *ConversionFactor=NULL;

	auto dma_pt = static_cast <dmathread*>(p);
	auto irioPvt = static_cast <irio*>(dma_pt->_asynPvt);

	int imgProfile = irioPvt->_iriodrv.platform == IRIO_FlexRIO && (irioPvt->_iriodrv.devProfile == 1
			|| irioPvt->_iriodrv.devProfile == 3);

	while (globalData[irioPvt->_portNumber].dma_thread_run[dma_pt->_id] == 0) {usleep(10000);}

	if(imgProfile == 1){
		chIndex = dma_pt->_id;
	}
	else{
		chIndex = irioPvt->_iriodrv.DMATtoHOSTChIndex[dma_pt->_id];
	}

	while(!found && i<irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id]){
		if(globalData[irioPvt->_portNumber].ch_nelm[chIndex + i] != 0){
			found = 1;
		}
		++i;
	}

	if(!found){ //No channels to publish -> End thread
		globalData[irioPvt->_portNumber].dma_thread_run[dma_pt->_id] = 0;
		dma_pt->_endAck = 1;
		return;
	}
	//Set data conversion factor.
	if(irioPvt->_iriodrv.DMATtoHOSTFrameType[dma_pt->_id] < 128){
		//I/O Module conversion factor to Volts
		ConversionFactor = &irioPvt->_iriodrv.CVADC;
	}
	else{
		//User defined conversion factor. At IOC init its value is 1 by default.
		ConversionFactor = &irioPvt->UserDefinedConversionFactor[dma_pt->_id];

	}

	//Thread initialization
	if(imgProfile == 1){
	}else{

		ai_pv_publish.reserve(irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id]);
		samples_per_channel = irioPvt->_iriodrv.DMATtoHOSTBlockNWords[dma_pt->_id]*8; //Bytes per block
		samples_per_channel = samples_per_channel/irioPvt->_iriodrv.DMATtoHOSTSampleSize[dma_pt->_id]; //Samples per block
		samples_per_channel = samples_per_channel/irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id];//Samples per channel per block

		// Ring Buffers for Waveforms PVs
		dma_pt->_IdRing.reserve(irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id]);
		aux = (float **) malloc(sizeof(float*)*irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id]);
		for(int i = 0; i < irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id]; i++){
			aux[i] = (float *) malloc(sizeof(float)*samples_per_channel);
			//ai_pv_publish_thread initializing
			if(globalData[irioPvt->_portNumber].ch_nelm[chIndex + i] != 0){
				dma_pt->_IdRing.push_back(epicsRingBytesCreate(samples_per_channel * irioPvt->_iriodrv.DMATtoHOSTSampleSize[dma_pt->_id]*4096));//!<Ring buffer to store manage the waveforms.
				ai_pv_publish.push_back(pvthread(irioPvt->_portName, i, irioPvt, dma_pt->_id, dma_pt->_IdRing[i], dma_pt->_SR));
			}

		}
		for (std::vector<pvthread>::iterator it = ai_pv_publish.begin(); it != ai_pv_publish.end(); ++it) {
			it->runpvthread();
		}

		//If irioasyn driver is used iriolib:
		//From DMATtoHOSTFrameType=0 to DMATtoHOSTFrameType=127 data conversion factor used is: I/O Module conversion factor.
		//From DMATtoHOSTFrameType=128 to DMATtoHOSTFrameType=255 data conversion factor used is: user defined conversion factor.
		switch(irioPvt->_iriodrv.DMATtoHOSTFrameType[dma_pt->_id]){
		case 0:
			buffersize = irioPvt->_iriodrv.DMATtoHOSTBlockNWords[dma_pt->_id];
			break;
		case 1:
			buffersize = irioPvt->_iriodrv.DMATtoHOSTBlockNWords[dma_pt->_id]+2; //each DMA data block includes two extra U64 words to include timestamp
			break;
		case 128:
			buffersize = irioPvt->_iriodrv.DMATtoHOSTBlockNWords[dma_pt->_id];
			break;
		case 129:
			buffersize = irioPvt->_iriodrv.DMATtoHOSTBlockNWords[dma_pt->_id]+2; //each DMA data block includes two extra U64 words to include timestamp
			break;
		default:
			buffersize = irioPvt->_iriodrv.DMATtoHOSTBlockNWords[dma_pt->_id];
			break;
		}
	}

    // Data acquisition from DMA main loop
	dataBuffer.resize(buffersize);
	cleanbuffer.resize(buffersize);

	int timeout=0,acqInProgress=0,timeoutLimit=100;

	int count = 0;
	int DFcount = 1;
	float twaitus = 0.0;
	do{
		if(irioPvt->acq_status == 0 && dma_pt->_threadends == 0){
			timeout = 0;
			DFcount = 1;
			do{
				usleep(1000);
			}while(irioPvt->acq_status == 0 && dma_pt->_threadends == 0);
			if(imgProfile == 1){
				//currImgSize = irioPvt->sizeX*irioPvt->sizeY;
				/*if(currImgSize>imgBufferSize){
					printf("[%s-%d] ERROR Current Image Size bigger than PV Container\n",__func__,__LINE__);
					ai_dma_thread->asynPvt->acq_status=0;
				}
				continue;*/
			}
		}
		//Not data received yet
		if(imgProfile == 1){
			//status=irio_getDMATtoHostImage(irioPvt, currImgSize, ai_dma_thread->id, dataBuffer, &count, &irio_status);
		}else{
			status = irio_getDMATtoHostData(&irioPvt->_iriodrv, 1, dma_pt->_id,dataBuffer.data(), &count, &irio_status);
		}
		if(status == IRIO_success){
			if(acqInProgress == 0){
				if(count != 0){//Now acquiring data
					printf("ACQ Started. Reading data from DMA%d\n",dma_pt->_id);
					acqInProgress = 1;
					timeout = 0;
				}
			}else{
				//No data received
				if(count == 0){
					//ACQ finished?
					if(timeout > timeoutLimit){
						irio_cleanDMATtoHost(&irioPvt->_iriodrv, dma_pt->_id, cleanbuffer.data(), buffersize, &irio_status);
						acqInProgress = 0;
						printf("ACQ Stopped in DMA%d\n",dma_pt->_id);
					}else{
						timeout++;
					}
				}else{
					timeout = 0;
				}
			}
			if(count!=0){
				//Data Read. Decimate blocks read
				if(DFcount >= dma_pt->_DecimationFactor){
					//Send block
					DFcount = 1;

					if(imgProfile == 1){
						//CallAIInsInt8Array(asynPvt,CH,ai_dma_thread->id,(epicsInt8*)dataBuffer,currImgSize);
					}else{
						switch(irioPvt->_iriodrv.DMATtoHOSTSampleSize[dma_pt->_id]){
						case(1):
							dma_pt->getChannelDataU8(irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id],samples_per_channel,dataBuffer.data(),aux,*ConversionFactor);
							break;
						case(2):
							dma_pt->getChannelDataU16(irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id],samples_per_channel,dataBuffer.data(),aux,*ConversionFactor);
							break;
						case(4):
							dma_pt->getChannelDataU32(irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id],samples_per_channel,dataBuffer.data(),aux,*ConversionFactor);
							break;
						case(8):
							dma_pt->getChannelDataU64(irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id],samples_per_channel,dataBuffer.data(),aux,*ConversionFactor);
							break;
						default:
							//irio_mergeStatus(&irio_status,ResourceNotFound_Error,0,"[%s,%d]-(%s) ERROR Getting channel data from DMA. Sample size %d not allowed.\n",__func__,__LINE__,irioPvt->_iriodrv.appCallID,irioPvt->_iriodrv.DMATtoHOSTSampleSize[pt->_id]);
							break;

						}

						//Copy data to ring buffer
						for(int i = 0; i < irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id]; i++){
							if(globalData[irioPvt->_portNumber].ch_nelm[chIndex + i] != 0){
								if((sizeof(float)*samples_per_channel) != (epicsRingBytesPut(dma_pt->_IdRing[i], (char*)aux[i], sizeof(float)*samples_per_channel))){
									//errlogSevPrintf(errlogFatal,"[%s-%d][%s]Error putting to ringBuffer%d\n",__func__,__LINE__,asynPvt->_portName,i);
								}
							}
						}
					}

				}else{
					//Ignore block
					DFcount++;
				}
			}
			///This twaitus have to be done only if data is not available
			else {
				if(imgProfile == 1){
					//TODO Review this
					usleep(1000);
				}else{
					twaitus = 500000*(float)samples_per_channel/(dma_pt->_SR);
					usleep(twaitus);
				}
			}

		}
	}while(dma_pt->_threadends == 0);

	if (irio_status.code == IRIO_error){
			irioPvt->status_func(&irio_status);
		}
		//errlogSevPrintf(errlogInfo,"[%s-%d][%s]Exiting Thread: %s\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);
		if(!imgProfile){
			for(int i = 0; i < irioPvt->_iriodrv.DMATtoHOSTNCh[dma_pt->_id]; i++){
				/*if(globalData[asynPvt->portNumber].ch_nelm[chIndex+i]!=0){
					ai_pv_publish[i].threadends=1; //finishing thread publishing PV
					while((ai_pv_publish[i].endAck!=1)&&(timeout==0)){
						usleep(1000);
						count++;
						if(count==WAIT_MS){
							timeout=1;
							errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s timeout at exit.\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);
						}
					}
					timeout=0;
					count=0;
					epicsRingBytesDelete(ai_dma_thread->IdRing[i]);//!<Ring buffer destroy.
				}*/
				free(aux[i]);
			}
			free(aux);
		}
		//free(ai_pv_publish);
		//free(imgBuffer);
		//free(dataBuffer);
		//free(cleanbuffer);
		//errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s Terminated.\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);

		if(irio_status.code != IRIO_success){
			//errlogSevPrintf(errlogFatal,"[%s-%d][%s]FPGA ERROR in acquisition thread:%s\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);
		}
		globalData[irioPvt->_portNumber].dma_thread_run[dma_pt->_id] = 0;

		irio_resetStatus(&irio_status);
		dma_pt->_endAck = 1;
}

void pvthread::aiPV_thread(void *p) {
	//There is one thread per ringbuffer (per DMA channel)
	auto pv_pt = static_cast <pvthread*>(p);
    auto irioPvt = static_cast <irio*>(pv_pt->_asynPvt);
    float* pv_data;
    int pv_nelem = 4096,aux;

    while (globalData[irioPvt->_portNumber].dma_thread_run[pv_pt->_dmanumber] == 0) {usleep(10000);}
    aux = irioPvt->_iriodrv.DMATtoHOSTChIndex[pv_pt->_dmanumber] + pv_pt->_id;
    pv_nelem = globalData[irioPvt->_portNumber].ch_nelm[aux];
    pv_data = (float*) malloc(sizeof(float)*pv_nelem);
    float twaitus = 0.0;

    do{
    	int NbytesDecimated;
        NbytesDecimated = epicsRingBytesUsedBytes(pv_pt->_IdRing);
        if(NbytesDecimated >= (sizeof(float)*pv_nelem)){
            if(epicsRingBytesIsFull(pv_pt->_IdRing) == 1){
            	errlogSevPrintf(errlogFatal, "\nRING BYTES Channel %d FULL\n", pv_pt->_id);
            }
            int aux2 = 0;
            aux2 = epicsRingBytesGet(pv_pt->_IdRing, (char*)pv_data, sizeof(float)*pv_nelem);
            if(aux2 != pv_nelem * sizeof(float)){
            	errlogSevPrintf(errlogFatal, "Requested %lu elems, get %d", pv_nelem * sizeof(float), aux2);
                usleep(1000);
                continue;
            }
            irioPvt->doCallbacksFloat32Array(pv_data, pv_nelem, irioPvt->CH, pv_pt->_id);

         }else{
        	 twaitus = 500000*(float)pv_nelem/(pv_pt->_SR);
             usleep(twaitus);
         }
    }while (pv_pt->_threadends == 0);
    free(pv_data);
    usleep(1000);
    pv_pt->_endAck = 1;

}

dmathread::dmathread(const std::string &device, uint8_t id, irio *irio_pvt) {
	_thread_id = NULL;
	_name = device + "-DMA_" + std::to_string(id);
	_id = id;
	_threadends = 0;
	_endAck = 0;
	_DecimationFactor = 1;
	_SR = 1;
	_blockSize = 1;
	_asynPvt = irio_pvt;
	_dmanumber = id;
}
pvthread::pvthread(const std::string &device, uint8_t id, irio *irio_pvt, uint8_t dma_id, epicsRingBytesId IdRing, int SR) {
       _thread_id = NULL;
       _name = device + "-DMA_" + std::to_string(dma_id) + "-CH" + std::to_string(id);
       _id = id;
       _threadends = 0;
       _endAck = 0;
       _DecimationFactor = 1;
       _SR = SR;
       _blockSize = 1;
       _IdRing = IdRing;
       _asynPvt = irio_pvt;
       _dmanumber = dma_id;
}
dmathread::~dmathread() {

}
pvthread::~pvthread() {

}
void dmathread::runthread(void) {
	_thread_id = (epicsThreadId*) epicsThreadCreate(_name.c_str(),
			epicsThreadPriorityHigh,
			epicsThreadGetStackSize(epicsThreadStackBig), /*(EPICSTHREADFUNC)*/
			(this->aiDMA_thread), (void*) this);
}

void pvthread::runpvthread(void) {
       _thread_id = (epicsThreadId*) epicsThreadCreate(_name.c_str(),
                       epicsThreadPriorityHigh,
                       epicsThreadGetStackSize(epicsThreadStackBig), /*(EPICSTHREADFUNC)*/
                       (this->aiPV_thread), (void*) this);
}

void dmathread::getChannelDataU8 (int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word = 0;
	int channel = 0;
	int sample = 0;
	int8_t* auxBuffer = (int8_t*) inBuffer;
	int offset = 0;
	for(sample = 0; sample < nSamples; sample++){
		for(channel = 0; channel < nChannels; channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[(u64word*8)+offset])*CVADC;
			if(offset == 7){
				offset = 0;
				u64word++;
			}else{
				offset++;
			}
		}
	}
}

void dmathread::getChannelDataU16(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word = 0;
	int channel = 0;
	int sample = 0;
	int16_t* auxBuffer = (int16_t*) inBuffer;
	int offset = 0;
	for(sample = 0; sample < nSamples; sample++){
		for(channel = 0; channel < nChannels; channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[(u64word*4)+offset])*CVADC;
			if(offset == 3){
				offset = 0;
				u64word++;
			}else{
				offset++;
			}
		}
	}
}
void dmathread::getChannelDataU32(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word = 0;
	int channel = 0;
	int sample = 0;
	int32_t* auxBuffer = (int32_t*) inBuffer;
	int offset = 0;
	for(sample = 0; sample < nSamples; sample++){
		for(channel = 0; channel < nChannels; channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[(u64word*2)+offset])*CVADC;
			if(offset == 1){
				offset = 0;
				u64word++;
			}else{
				offset++;
			}
		}
	}
}
void dmathread::getChannelDataU64(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word = 0;
	int channel = 0;
	int sample = 0;
	int64_t* auxBuffer = (int64_t*) inBuffer;
	for(sample = 0; sample < nSamples; sample++){
		for(channel = 0; channel < nChannels; channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[u64word])*CVADC; //TODO: Review this casting
			u64word++;
		}
	}
}

