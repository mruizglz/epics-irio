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

#include "irioHandlerSG.h"
#include "irioDataTypes.h"
#include "irioDriver.h"
#include "irioResourceFinder.h"
#include "irioHandlerAnalog.h"
#include "irioHandlerDigital.h"
#include "irioHandlerDMA.h"
#include "irioHandlerImage.h"
#include "irioError.h"
#include <math.h>
#include "irioAsynold.h"

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
			for(j=0;j<MAX_NUMBER_OF_CARDS;j++){
				if(globalData[j].init_success==1){
					char *recInpString = NULL;
					char *auxrecInpString = NULL;
					char *recScan=NULL;
					char *recInp=NULL;
					char *recPortName=NULL;
					char *portname=NULL;
					char *recReason=NULL;
					char *recChString=NULL;
					printf("##################################################################################################\n");
					printf("##       Reading record database for RIO device with portName:RIO_%d at initHookBeginning       ##\n",j);
					printf("##################################################################################################\n");
					dbInitEntry(pdbbase,&dbentry); //pdbase is an EPICS global variable
					pdbRecordType = (dbRecordType *)ellFirst(&pdbbase->recordTypeList);
					while (pdbRecordType!=NULL){
						int recordType_ai=strcmp(pdbRecordType->name, "ai");
						int recordType_bi=strcmp(pdbRecordType->name, "bi");
						int recordType_waveform=strcmp(pdbRecordType->name, "waveform");
						if((recordType_ai==0)||(recordType_bi==0)||(recordType_waveform==0)){
							pdbRecordNode = (dbRecordNode *)ellFirst(&pdbRecordType->recList);
							while(pdbRecordNode!=NULL){
								if (!dbFindRecord(&dbentry,pdbRecordNode->recordname)){
									if(!dbFindField(pdbentry, "SCAN")){
										recInpString = dbGetString(pdbentry);
										if(!strcmp(recInpString, "I/O Intr")){   				//Current record is I/O Intr.
											asprintf(&recScan,recInpString);
											if(!dbFindField(pdbentry, "INP")){
												recInpString = dbGetString(pdbentry);
												recInpString=strstr(recInpString,"@asyn");
												if(NULL!=recInpString){							//Current record is asyndriver based on.
													asprintf(&recInp,recInpString);
													recInpString=strstr(recInpString,"RIO_");
													if(NULL!=recInpString){						//Current record is IRIOdriver based on.
														asprintf(&auxrecInpString,recInpString);
														recPortName=strtok(auxrecInpString,",");//store IRIO portName(RIO_#)
														asprintf(&portname,"RIO_%d",j);
														if(!strcmp(recPortName,portname)){
															int recChNumber=0;
															recChString=strstr(recInpString,","); //this is the separator for the channel
															recChString=recChString+1;
															recReason=recChString+2;			//this is the separator for the reason
															recChNumber=atoi(recChString);
															int reason_AI=strcmp(recReason, "AI");
															int reason_DI=strcmp(recReason, "DI");
															int reason_CH=strcmp(recReason, "CH");
															if((reason_AI==0)||(reason_DI==0)||(reason_CH==0)){    //only AI,DI and waveform PVs belonging to IRIO devices using asynDriver with SCAN=I/O Intr are considered.
																int aux=0;
																asprintf(&globalData[j].intr_records[i].scan,recScan);
																asprintf(&globalData[j].intr_records[i].type,pdbRecordType->name);
																asprintf(&globalData[j].intr_records[i].name,pdbRecordNode->recordname);
																asprintf(&globalData[j].intr_records[i].input,recInp);
																globalData[j].intr_records[i].addr=recChNumber;
																asprintf(&globalData[j].intr_records[i].reason,recReason);
																asprintf(&globalData[j].intr_records[i].portName,recPortName);
																if(!strcmp(recReason, "CH")){
																	if(!dbFindField(pdbentry, "NELM")){
																		recInpString = dbGetString(pdbentry);
																		if(NULL!=recInpString){
																			aux=atoi(recInpString);
																			globalData[j].intr_records[i].nelm=aux;
																			globalData[j].ch_nelm[recChNumber]=aux;
																		}
																	}
																}else{
																	globalData[j].intr_records[i].nelm=1;
																}
																dbFindField(pdbentry, "TSE");
																recInpString = dbGetString(pdbentry);
																aux=atoi(recInpString);
																globalData[j].intr_records[i].timestamp_source=aux;
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
					globalData[j].io_number=i;
					i=0;
					errlogSevPrintf(errlogInfo,"[%s-%d][RIO_%d]Number of AI,DI or CH records belonging to IRIO device with port:RIO_%d using asynDriver with SCAN=I/O Intr is: %d\n",__func__,__LINE__,j,j,globalData[j].io_number);
				}
			}
		}
		call_gettingDBInfo=1;
		break;
	case initHookAfterIocRunning:
		if(call_gettingDBInfo==1){
			for(i=0;i<MAX_NUMBER_OF_CARDS;++i){
				for(j=0;j<globalData[i].number_of_DMAs;++j){
					if(globalData[i].dma_thread_created[j]==1){
						globalData[i].dma_thread_run[j]=1;
					}
				}
			}
			call_gettingDBInfo=0;
		}
		break;
	default:
		break;
	}
}

int nirioinit(const char *namePort,const char *DevSerial,const char *PXInirioModel, const char *projectName, const char *FPGAversion,int verbosity)
{
	irio_pvt_t *pdrvPvt;
	asynStatus status;
	asynUser *pasynUser = NULL;
	int i=0, number_dma_ch=0;

	TIRIOStatusCode st=IRIO_success;
	initHookRegister(gettingDBInfo);

	/* Define port name */
	pdrvPvt = callocMustSucceed(sizeof(irio_pvt_t) + 1,sizeof (char),"iriosInit error");
	memset (pdrvPvt, 0, sizeof(*pdrvPvt));
	pdrvPvt->InfoStatus=callocMustSucceed(sizeof(char)*100 + 1,sizeof(char),"irioInit_port error");
	strcpy(pdrvPvt->InfoStatus ,"-");
	pdrvPvt->FPGAStatus=callocMustSucceed(sizeof(char)*100 + 1,sizeof(char),"irioInit_port error");
	strcpy(pdrvPvt->FPGAStatus ,"OK");
	pdrvPvt->portName = callocMustSucceed(strlen(namePort) + 1,sizeof(char),"irioInit_port error");

    /* Checking correct format of portName. It must be RIO_$(MODULEIDX), where $(MODULEIDX) must be a value between 0 and MAX_NUMBER_OF_CARDS*/
    strcpy(pdrvPvt->portName ,namePort);
    if(pdrvPvt->portName==NULL){
		printf("\n\n[%s-%d]PortName format is wrong. Correct format is: RIO_$(MODULEIDX)\n\n",__func__,__LINE__);
		return asynError;
    }
    char portcmp[]="RIO_";
    if(strncmp(pdrvPvt->portName,portcmp,4)!=0){
		printf("\n\n[%s-%d]PortName %s format is wrong. Correct format is: RIO_$(MODULEIDX)\n\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
    }
	char *portNumber=NULL;
	portNumber=strstr(pdrvPvt->portName,"_");
	if(portNumber==NULL){
		printf("\n\n[%s-%d]PortName %s format is wrong. Correct format is: RIO_$(MODULEIDX)\n\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}
	portNumber=portNumber+1;
	int portnumberlenth=strlen(portNumber);
	char validchars[]="0123456789";
	if(strspn(portNumber,validchars)!=portnumberlenth){
		printf("\n\n[%s-%d]PortName %s format is wrong. Correct format is: RIO_$(MODULEIDX)\n\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}
	if((strncmp(portNumber,"0",1)==0) && portnumberlenth!=1){
		printf("\n\n[%s-%d]PortName %s format is wrong. Correct format is: RIO_$(MODULEIDX)\n\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}
	pdrvPvt->portNumber=atoi(portNumber);
	if((pdrvPvt->portNumber>=MAX_NUMBER_OF_CARDS)||(pdrvPvt->portNumber<0)){
		printf("\n\n[%s-%d]PortName %s must be set between RIO_0 and RIO_%d\n\n",__func__,__LINE__,pdrvPvt->portName,(MAX_NUMBER_OF_CARDS-1));
		return asynError;
	}
	call_gettingDBInfo=0;
	all_poll_threads_finished=0;
	all_dma_threads_finished=0;
	epicsExiting=0;

	/* Initialize global data for each port Number */
	globalData[pdrvPvt->portNumber].ch_nelm=NULL;
	globalData[pdrvPvt->portNumber].intr_records=NULL;
	globalData[pdrvPvt->portNumber].init_success=0;
	globalData[pdrvPvt->portNumber].io_number=0;
	globalData[pdrvPvt->portNumber].ai_poll_thread_created=0;
	globalData[pdrvPvt->portNumber].ai_poll_thread_run=0;
	globalData[pdrvPvt->portNumber].di_poll_thread_created=0;
	globalData[pdrvPvt->portNumber].di_poll_thread_run=0;
	globalData[pdrvPvt->portNumber].dma_thread_created=NULL;
	globalData[pdrvPvt->portNumber].dma_thread_run=NULL;
	globalData[pdrvPvt->portNumber].number_of_DMAs=0;

    epicsAtExit(nirio_epicsExit,(void*)pdrvPvt);


	/* create asyn user */
	pasynUser = pasynManager->createAsynUser(0,0);
	if(pasynUser==NULL){
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]Error createAsynUser.\n",__func__,__LINE__,pdrvPvt->portName);
	    return asynError;
	}
	pasynUser->userPvt = pdrvPvt;
	/* Check if port name is OK */
	if (!strlen(pdrvPvt->portName)) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]No port name received.\n",__func__,__LINE__,pdrvPvt->portName);
	    return asynError;
	}

    /* register port */
    ///@TODO: review asyn parameters
     status = pasynManager->registerPort(pdrvPvt->portName,
    		 	 	 	 	 ASYN_MULTIDEVICE, /* ASYN_MULTIDEVIC | ASYN_CANBLOCK */
    		 	 	 	 	 1,  /* autoconnect = 1 */
    		 	 	 	 	 1,  /* medium priority */
    		 	 	 	 	 0); /* default stacksize */

    if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]IRIO registerDriver failed.\n",__func__,__LINE__,pdrvPvt->portName);
    	return asynError;
    }

    /*Register interfaces and interrupts*/
	/* Commons */
	pdrvPvt->common.interfaceType = asynCommonType;
	pdrvPvt->common.pinterface = (void*)&asynCom;
	pdrvPvt->common.drvPvt = pdrvPvt;
	/* user */
	pdrvPvt->AsynDrvUser.interfaceType = asynDrvUserType;
	pdrvPvt->AsynDrvUser.pinterface = (void *)&drvUser;
	pdrvPvt->AsynDrvUser.drvPvt = pdrvPvt;
	/*Octet*/
	pdrvPvt->AsynOctet.interfaceType = asynOctetType;
	pdrvPvt->AsynOctet.pinterface = (void *)&AOctet;
	pdrvPvt->AsynOctet.drvPvt = pdrvPvt;
	/*Int16Array*/
	pdrvPvt->AsynInt8Array.interfaceType = asynInt8ArrayType;
	pdrvPvt->AsynInt8Array.pinterface = (void *)&AInt8Array;
	pdrvPvt->AsynInt8Array.drvPvt = pdrvPvt;
	/* Int32 */
	pdrvPvt->AsynInt32.interfaceType = asynInt32Type;
	pdrvPvt->AsynInt32.pinterface = (void *)&AInt32;
	pdrvPvt->AsynInt32.drvPvt = pdrvPvt;
	/* Int32Array */
	pdrvPvt->AsynInt32Array.interfaceType = asynInt32ArrayType;
	pdrvPvt->AsynInt32Array.pinterface = (void *)&AInt32Array;
	pdrvPvt->AsynInt32Array.drvPvt = pdrvPvt;
	/* Float64 */
	pdrvPvt->AsynFloat64.interfaceType = asynFloat64Type;
	pdrvPvt->AsynFloat64.pinterface = (void *)&AFloat64;
	pdrvPvt->AsynFloat64.drvPvt = pdrvPvt;
	/* Float32Array */
	pdrvPvt->AsynFloat32Array.interfaceType = asynFloat32ArrayType;
	pdrvPvt->AsynFloat32Array.pinterface = (void *)&asynFloat32Array_structure;
	pdrvPvt->AsynFloat32Array.drvPvt = pdrvPvt;

	/* register Common interface */
	status = pasynManager->registerInterface(pdrvPvt->portName, &pdrvPvt->common);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for common interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/* Register User Interface */
	status = pasynManager->registerInterface(pdrvPvt->portName,&pdrvPvt->AsynDrvUser);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for asynDrvUser interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/*register AsynOctet*/
	status = pasynOctetBase->initialize(pdrvPvt->portName,&pdrvPvt->AsynOctet,0,0,0);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for asynOctet interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/*register AsynInt8Array*/
	status = pasynInt8ArrayBase->initialize(pdrvPvt->portName,&pdrvPvt->AsynInt8Array);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for asynInt8Array interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/* register AsynInt32 */
	status = pasynInt32Base->initialize(pdrvPvt->portName, &pdrvPvt->AsynInt32);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for asynInt32 interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/* register AsynInt32Array */
	status = pasynInt32ArrayBase->initialize(pdrvPvt->portName, &pdrvPvt->AsynInt32Array);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for asynInt32Array interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/* register AsynFloat64 */
	status = pasynFloat64Base->initialize(pdrvPvt->portName, &pdrvPvt->AsynFloat64);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for asynFloat64 interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/* register AsynFloat32Array */
	status = pasynFloat32ArrayBase->initialize(pdrvPvt->portName, &pdrvPvt->AsynFloat32Array);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterface failed for asynFloat32Array interface.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/*Register Octet interrupt sources*/
	status = pasynManager->registerInterruptSource(pdrvPvt->portName, &pdrvPvt->AsynOctet, &pdrvPvt->asynOctetInterruptPvt);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterruptSource asynOctetInterruptPvt failed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/*Register Int8 Array interrupt sources*/
	status = pasynManager->registerInterruptSource(pdrvPvt->portName, &pdrvPvt->AsynInt8Array, &pdrvPvt->asynInt8ArrayInterruptPvt);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterruptSource asynInt8ArrayInterruptPvt failed\n",__func__,__LINE__,pdrvPvt->portName);
    	return asynError;
	}
	/* Register Int32 interrupt sources */
	status = pasynManager->registerInterruptSource(pdrvPvt->portName, &pdrvPvt->AsynInt32, &pdrvPvt->asynInt32InterruptPvt);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterruptSource asynInt32InterruptPvt failed\n",__func__,__LINE__,pdrvPvt->portName);
    	return asynError;
	}

	/* Register Int32Array interrupt sources */
	status = pasynManager->registerInterruptSource(pdrvPvt->portName, &pdrvPvt->AsynInt32Array, &pdrvPvt->asynInt32ArrayInterruptPvt);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterruptSource asynInt32ArrayInterruptPvt failed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/* Register Float64 interrupt sources */
	status = pasynManager->registerInterruptSource(pdrvPvt->portName, &pdrvPvt->AsynFloat64, &pdrvPvt->asynFloat64InterruptPvt);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterruptSource asynFloat64InterruptPvt failed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/* Register Float32Array interrupt sources */
	status = pasynManager->registerInterruptSource(pdrvPvt->portName, &pdrvPvt->AsynFloat32Array, &pdrvPvt->asynFloat32ArrayInterruptPvt);
	if (status != asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]irioInit registerInterruptSource asynFloat32ArrayInterruptPvt failed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynError;
	}

	/*Device status before init device*/
	pdrvPvt->rio_device_status=STATUS_NO_BOARD;

    char* appCallID=NULL;
    char *currentdir=NULL;
    char *bitfilepath=NULL;
    char *OSdirFW=NULL;
    int retry = 0;

    OSdirFW="/opt/codac/firmware/ni/irio/";
    asprintf(&appCallID,"%s%s","asyn",pdrvPvt->portName);
    // Generating the path for locating headerfile and bitfile
    currentdir=(char *)get_current_dir_name();
    asprintf(&bitfilepath,"%s/irio/",currentdir);
    errlogSevPrintf(errlogInfo,"[%s-%d][%s]bitfilepath=%s\n",__func__,__LINE__,pdrvPvt->portName,bitfilepath);
    fflush(NULL);

    pdrvPvt->rio_device_status=STATUS_INITIALIZING;

	TStatus irio_status;
	irio_initStatus(&irio_status);
    st=irio_initDriver(appCallID,DevSerial,PXInirioModel,projectName,FPGAversion,verbosity,bitfilepath,bitfilepath,&pdrvPvt->drvPvt,&irio_status);
    if(st==IRIO_success){
    	asynPrint(pasynUser,ASYN_TRACE_FLOW,"[%s-%d][%s]Device initialization OK.\n",__func__,__LINE__,pdrvPvt->portName);
	pdrvPvt->driverInitialized=1;
    	status_func(pdrvPvt,&irio_status);
    }
    if(st==IRIO_error){
	if (irio_status.detailCode == HeaderNotFound_Error )
	{
                OSdirFW="/opt/codac/firmware/ni/irio/";
                asprintf(&bitfilepath,OSdirFW);
                irio_initStatus(&irio_status);
                pdrvPvt->rio_device_status=STATUS_INITIALIZING;
                retry=1;
	}
	else{
	    	status_func(pdrvPvt,&irio_status);
        	nirio_shutdown(pdrvPvt);
	    	return asynError;
	}
    }
    if(st==IRIO_warning){
    	status_func(pdrvPvt,&irio_status);
    }

    if (retry){
        st=irio_initDriver(appCallID,DevSerial,PXInirioModel,projectName,FPGAversion,verbosity,bitfilepath,bitfilepath,&pdrvPvt->drvPvt,&irio_status);
        if(st==IRIO_success){
                asynPrint(pasynUser,ASYN_TRACE_FLOW,"[%s-%d][%s]Device initialization OK.\n",__func__,__LINE__,pdrvPvt->portName);
                pdrvPvt->driverInitialized=1;
                status_func(pdrvPvt,&irio_status);
        }
        if(st==IRIO_error){
                status_func(pdrvPvt,&irio_status);
                nirio_shutdown(pdrvPvt);
                return asynError;
        }

        if(st==IRIO_warning){
                status_func(pdrvPvt,&irio_status);
        }
    }



    free(appCallID);

	/* find first unused slot in array of pointers */
	for (i=0; i<MAX_NUMBER_OF_CARDS; i++) {
		if (pdrvPvt_array[i].used == 0) {
			/* Save pointer to structure to global array of pointers */
			pdrvPvt_array[i].used = 1;
			pdrvPvt_array[i].ptr = pdrvPvt;
		}
	}

	//Allocate Signal Generators
	if(pdrvPvt->drvPvt.NoOfSG>0){
		pdrvPvt->sgData = (SGData_t*) calloc(pdrvPvt->drvPvt.NoOfSG,sizeof(SGData_t));
	}

	//Allocate memory for User_Defined_Conversion_Factors
	if(pdrvPvt->drvPvt.DMATtoHOSTNo.value!=0){
		pdrvPvt->UserDefinedConversionFactor=calloc(pdrvPvt->drvPvt.DMATtoHOSTNo.value, sizeof(epicsFloat64));
	}

	//Allocate memory for AI+DI+CH records
	//i=0;
	for(i=0;i<pdrvPvt->drvPvt.DMATtoHOSTNo.value;i++){
		number_dma_ch+=(int)pdrvPvt->drvPvt.DMATtoHOSTNCh[i];
	    errlogSevPrintf(errlogInfo,"[%s-%d][%s]Number of channels of DMA%d is:%d\n",__func__,__LINE__,pdrvPvt->portName,i,(int)pdrvPvt->drvPvt.DMATtoHOSTNCh[i]);

	}
    errlogSevPrintf(errlogInfo,"[%s-%d][%s]Total number of channels in ALL DMAs is:%d\n",__func__,__LINE__,pdrvPvt->portName,number_dma_ch);
    errlogSevPrintf(errlogInfo,"[%s-%d][%s]Total number of maxAI(%d)+maxDI(%d)+CH(%d) is:%d\n",__func__,__LINE__,pdrvPvt->portName,pdrvPvt->drvPvt.max_analogoutputs, pdrvPvt->drvPvt.max_digitalsinputs, number_dma_ch, (pdrvPvt->drvPvt.max_analogoutputs+pdrvPvt->drvPvt.max_digitalsinputs+number_dma_ch));

	globalData[pdrvPvt->portNumber].intr_records=calloc((pdrvPvt->drvPvt.max_analogoutputs+pdrvPvt->drvPvt.max_digitalsinputs+number_dma_ch),sizeof(intr_records_t));
	globalData[pdrvPvt->portNumber].ch_nelm=calloc(64,sizeof(int));
	globalData[pdrvPvt->portNumber].dma_thread_created=calloc(pdrvPvt->drvPvt.DMATtoHOSTNo.value,sizeof(int));
	globalData[pdrvPvt->portNumber].dma_thread_run=calloc(pdrvPvt->drvPvt.DMATtoHOSTNo.value,sizeof(int));


	//IMAQ Profile
	if(pdrvPvt->drvPvt.platform==IRIO_FlexRIO && (pdrvPvt->drvPvt.devProfile==1 || pdrvPvt->drvPvt.devProfile==3)){
		pdrvPvt->UARTReceivedMsg= malloc(sizeof(char)*MAX_UARTSIZE);
		strcpy(pdrvPvt->UARTReceivedMsg,"");
		pdrvPvt->sizeX=256;
		pdrvPvt->sizeY=256;
	}

	//If not cRIO PBP Profile create DMA threads
	if(!(pdrvPvt->drvPvt.platform==IRIO_cRIO && pdrvPvt->drvPvt.devProfile==1)){
		pdrvPvt->ai_dma_thread = malloc(sizeof(irio_dmathread_t)*pdrvPvt->drvPvt.DMATtoHOSTNo.value);
		//Setup all DMAs:
		st=irio_setUpDMAsTtoHost(&pdrvPvt->drvPvt,&irio_status);
		if (st!=IRIO_success){
			status_func(pdrvPvt,&irio_status);
			if(st==IRIO_error){
				return asynError;
			}
		}
		//Create and launch threads attending each DMA
		for (i=0;i<(pdrvPvt->drvPvt.DMATtoHOSTNo.value) ;i++){
				pdrvPvt->ai_dma_thread[i].dma_thread_id=NULL;
				pdrvPvt->ai_dma_thread[i].dma_thread_name=NULL;
				asprintf(&pdrvPvt->ai_dma_thread[i].dma_thread_name,"RIO%d_DMA%d",pdrvPvt->portNumber,i);
				pdrvPvt->ai_dma_thread[i].id=i;
				pdrvPvt->ai_dma_thread[i].threadends=0;
				pdrvPvt->ai_dma_thread[i].endAck=0;
				pdrvPvt->ai_dma_thread[i].DecimationFactor=1;
				pdrvPvt->ai_dma_thread[i].SR=1;
				pdrvPvt->ai_dma_thread[i].blockSize=1;
				pdrvPvt->ai_dma_thread[i].asynPvt=pdrvPvt;
				pdrvPvt->ai_dma_thread[i].IdRing=NULL;
				pdrvPvt->ai_dma_thread[i].dmanumber=i;
				pdrvPvt->ai_dma_thread[i].dma_thread_id=(epicsThreadId*)epicsThreadCreate(pdrvPvt->ai_dma_thread[i].dma_thread_name,
									epicsThreadPriorityHigh,
									epicsThreadGetStackSize(epicsThreadStackBig),
									(EPICSTHREADFUNC)aiDMA_thread,(void *)(&pdrvPvt->ai_dma_thread[i]));
				globalData[pdrvPvt->portNumber].dma_thread_created[i]=1;
		}
		globalData[pdrvPvt->portNumber].number_of_DMAs=pdrvPvt->drvPvt.DMATtoHOSTNo.value;

	}

	/*Control of device status error parameters*/
	if(pdrvPvt->drvPvt.platform==IRIO_cRIO && pdrvPvt->drvPvt.devProfile==1){
		pdrvPvt->shift_pbp = 8;
		pdrvPvt->error_oob_array[ERROR_OOB_DF]=calloc(pdrvPvt->shift_pbp/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_SR]=calloc(pdrvPvt->shift_pbp/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEX]=calloc(pdrvPvt->shift_pbp/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEY]=calloc(pdrvPvt->shift_pbp/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_USER_DEFINED_CONVERSION_FACTOR]=calloc(pdrvPvt->shift_pbp/8,sizeof(char));
	}else{
		pdrvPvt->aux2=((pdrvPvt->drvPvt.DMATtoHOSTNo.value%8)>0)?(pdrvPvt->drvPvt.DMATtoHOSTNo.value/8+1):pdrvPvt->drvPvt.DMATtoHOSTNo.value/8;
		pdrvPvt->shift_dma = pdrvPvt->aux2*8;
		pdrvPvt->error_oob_array[ERROR_OOB_DF]=calloc(pdrvPvt->shift_dma/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_SR]=calloc(pdrvPvt->shift_dma/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEX]=calloc(pdrvPvt->shift_dma/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEY]=calloc(pdrvPvt->shift_dma/8,sizeof(char));
		pdrvPvt->error_oob_array[ERROR_OOB_USER_DEFINED_CONVERSION_FACTOR]=calloc(pdrvPvt->shift_dma/8,sizeof(char));
	}
	pdrvPvt->error_oob_array[ERROR_OOB_SR_AI_INTR]=calloc(1,sizeof(char));
	pdrvPvt->error_oob_array[ERROR_OOB_SR_DI_INTR]=calloc(1,sizeof(char));
	pdrvPvt->aux2=((pdrvPvt->drvPvt.max_analogoutputs%8)>0)?(pdrvPvt->drvPvt.max_analogoutputs/8+1):pdrvPvt->drvPvt.max_analogoutputs/8;
	pdrvPvt->shift_ao = pdrvPvt->aux2*8;
	pdrvPvt->error_oob_array[ERROR_OOB_AO]=calloc(pdrvPvt->shift_ao/8,sizeof(char));

	pdrvPvt->aux2=((pdrvPvt->drvPvt.NoOfSG%8)>0)?(pdrvPvt->drvPvt.NoOfSG/8+1):pdrvPvt->drvPvt.NoOfSG/8;
	pdrvPvt->shift_sg = pdrvPvt->aux2*8;
	pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE]=calloc(pdrvPvt->shift_sg/8,sizeof(char));
	pdrvPvt->error_oob_array[ERROR_OOB_SGAMP]=calloc(pdrvPvt->shift_sg/8,sizeof(char));
	pdrvPvt->error_oob_array[ERROR_OOB_SGFREQ]=calloc(pdrvPvt->shift_sg/8,sizeof(char));

	pdrvPvt->rio_device_status=STATUS_CONFIGURED;
    if(st==IRIO_success){
    	printf("======================================================================================\n");
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]Finish Initialization. Device configured successfully!.\n",__func__,__LINE__,pdrvPvt->portName);
    	printf("======================================================================================\n");
		globalData[pdrvPvt->portNumber].init_success=1;
    }
	irio_resetStatus(&irio_status);
    return asynSuccess;
}

/**
 * asynCommon
 */
static asynStatus connect (void *drvPvt, asynUser *pasynUser) {

        irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
        asynStatus status;
        int addr;

        asynPrint(pasynUser,ASYN_TRACE_FLOW,"[%s-%d][%s]Connect to port.\n",__func__,__LINE__,pdrvPvt->portName);

        status = pasynManager->getAddr(pasynUser, &addr);
        if (status != asynSuccess) {
            asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]Error getAddr connect. Error '%d'.\n",__func__,__LINE__,pdrvPvt->portName,(int)status);
        }

        status = pasynManager->exceptionConnect(pasynUser);
        if (status != asynSuccess) {
            asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]Error running exceptionConnect. Error '%d'.\n",__func__,__LINE__,pdrvPvt->portName,(int)status);
        }

        return asynSuccess;
}

static asynStatus disconnect(void *drvPvt,asynUser *pasynUser) {

        irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
        asynStatus status;
        int addr;

        asynPrint(pasynUser,ASYN_TRACE_FLOW,"[%s-%d][%s]Disconnect to port.\n",__func__,__LINE__,pdrvPvt->portName);

        status = pasynManager->getAddr(pasynUser, &addr);
        if (status != asynSuccess){
            asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]Error getAddr connect. Error '%d'.\n",__func__,__LINE__,pdrvPvt->portName,(int)status);
        	return status;
        }

        status=pasynManager->exceptionDisconnect(pasynUser);
        if (status != asynSuccess) {
            asynPrint(pasynUser,ASYN_TRACE_ERROR,"[%s-%d][%s]Error running exceptionDisconnect. Error '%d'\n",__func__,__LINE__,pdrvPvt->portName,(int)status);
        }

        return asynSuccess;
}

static void report (void *drvPvt, FILE *fp, int details) {

	//details could be: 0 Short description
	//					1 Additional information
	//					2 Print even more info
 	 char *platform[2]={"FlexRIO","cRIO"};
 	 char *couplingMode[3]={"AC","DC","N/A"};

	 irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	 int i=0;

	 if(pdrvPvt->drvPvt.platform==0){//FlexRIO
		 switch (pdrvPvt->drvPvt.devProfile){
		 case 0://DAQ
			 i=0;
			 break;
		 case 1://IMAQ
			 i=1;
			 break;
		 case 2://GPU DAQ
		 	 i=2;
			 break;
		 case 3://GPU IMAQ
			 i=3;
			 break;
		 default:
			 break;
		 }
	 }
	 else{
		 switch (pdrvPvt->drvPvt.devProfile){
		 case 0://DAQ
			 i=4;
			 break;
		 case 1://IO
			 i=5;
			 break;
		 default:
			 break;
		 }
	 }
     fprintf(fp, "\t---------------------------------------------------------- \n" );
     fprintf(fp, "\tNI-RIO EPICS device support \n" );
     fprintf(fp, "\t---------------------------------------------------------- \n" );
     fprintf(fp, "\tDevice Name                         : %s \n", DEVICE_NAME);
     fprintf(fp, "\tRIO platform                        : %s \n", platform[pdrvPvt->drvPvt.platform]);
     fprintf(fp, "\tRIO Device Model                    : %s \n", pdrvPvt->drvPvt.RIODeviceModel);
     fprintf(fp, "\tRIO Serial Number                   : %s \n", pdrvPvt->drvPvt.DeviceSerialNumber);
     fprintf(fp, "\tModule ID (xxx:FPGASTART must be ON): %u \n", pdrvPvt->drvPvt.moduleValue);
     fprintf(fp, "\tLabVIEW Project Name (VI)           : %s \n", pdrvPvt->drvPvt.projectName);
     fprintf(fp, "\tEPICS Driver Version                : %s \n", EPICS_DRIVER_VERSION );
     fprintf(fp, "\tIRIO Library Version                : %s \n", pdrvPvt->irio_version);
	 fprintf(fp, "\tDevice Profile                      : %s \n", platform_profile[i].platform_profile_string);

	 switch (details){
	 case 0:
		 break;
	 case 1:
	 case 2:

	     fprintf(fp, "\t---------------------------------------------------------- \n" );
		 fprintf(fp, "\tAdditional Details \n" );
	     fprintf(fp, "\t---------------------------------------------------------- \n" );
		 fprintf(fp, "\tFPGA Main Loop Frequency                 : %u \n", pdrvPvt->drvPvt.Fref);
		 fprintf(fp, "\tMin Sampling Rate                        : %f \n", pdrvPvt->drvPvt.minSamplingRate);
		 fprintf(fp, "\tMax Sampling Rate                        : %f \n", pdrvPvt->drvPvt.maxSamplingRate);
		 fprintf(fp, "\tMin Analog Output                        : %f \n", pdrvPvt->drvPvt.minAnalogOut);
		 fprintf(fp, "\tMax Analog Output                        : %f \n", pdrvPvt->drvPvt.maxAnalogOut);
		 fprintf(fp, "\tCoupling Mode                            : %s \n", couplingMode[pdrvPvt->drvPvt.couplingMode]);
		 fprintf(fp, "\tConversion to Volts of analog inputs     : %g \n", pdrvPvt->drvPvt.CVADC);
		 fprintf(fp, "\tConversion from Volts for analog outputs : %g \n", pdrvPvt->drvPvt.CVDAC);
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
	 return;
}

/* asynUser */

static asynStatus drvUserCreate(void *drvPvt, asynUser *pasynUser,
		const char *drvInfo, const char **pptypeName, size_t *psize) {

	asynPrint(pasynUser,ASYN_TRACE_FLOW,"drvUserCreate \n");
	int i;

	if (!drvInfo){
		return(asynSuccess);
	}
	for (i=0; i<MAX_IRIO_COMMANDS; i++) {
		const char *pstring;
		pstring = commands[i].commandString;
		if (epicsStrCaseCmp(drvInfo, pstring) == 0) {
			pasynUser->reason = commands[i].command;
			if (pptypeName!=NULL){
				*pptypeName = epicsStrDup(pstring);
			}
			if (psize!=NULL){
				*psize = sizeof(commands[i].command);
			}
			asynPrint(pasynUser,ASYN_TRACE_FLOW,"user created\n");
			return(asynSuccess);
		}
	}

	asynPrint(pasynUser,ASYN_TRACE_ERROR,"provided command %s not found in base!\n", drvInfo);
	return(asynError);
}

static asynStatus drvUserGetType(void *drvPvt, asynUser *pasynUser,
                                const char **pptypeName, size_t *psize) {

        asynPrint(pasynUser,ASYN_TRACE_FLOW,"drvUserGetType \n");

        return asynSuccess;
}

static asynStatus drvUserDestroy(void *drvPvt, asynUser *pasynUser) {

        asynPrint(pasynUser,ASYN_TRACE_FLOW,"drvUserDestroy \n");

        return asynSuccess;
}

static asynStatus octetRead(void *drvPvt, asynUser *pasynUser, char *data, size_t maxchars,size_t *nbytestransfered,int *eomReason)
{
	irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	int addr;
	asynStatus asyn_status;
	size_t nbytes=0;
	TIRIOStatusCode st=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	*nbytestransfered=0;

	if(epicsExiting==1 && pdrvPvt->flag_exit==0){
		pdrvPvt->flag_exit=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from octetRead interface at IOC exit.\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;

	}
	if(epicsExiting==1 && pdrvPvt->flag_exit==1){
		return asynSuccess;
	}

	if(pasynUser->reason!=InfoStatus){
		if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==0){
			pdrvPvt->flag_close=1;
	    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from octetRead interface with driver closed\n",__func__,__LINE__,pdrvPvt->portName);
			return asynSuccess;
		}
		if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==1){
			return asynSuccess;
		}
	}

	if(maxchars==0){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]OctetRead call from reason %s with maxChars=0 (%d)\n",__func__,__LINE__,pdrvPvt->portName,commands[pasynUser->reason].commandString,(int)maxchars);
		return status_func(pdrvPvt,&irio_status);
	}

	asyn_status = pasynManager->getAddr(pasynUser,&addr);
	if(asyn_status!=asynSuccess){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]Error getAddr connect. Asyn_Status: '%d'.\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason,asyn_status);
		return status_func(pdrvPvt,&irio_status);
	}

	switch (pasynUser->reason) {

	case device_serial_number:
		nbytes=(MIN(maxchars,(strlen(pdrvPvt->drvPvt.DeviceSerialNumber)+1)));
		strncpy(data, pdrvPvt->drvPvt.DeviceSerialNumber, nbytes-1);
		data[nbytes-1]='\0';
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]RIO Device serial number(addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
		*nbytestransfered=nbytes;
		break;

	case epics_version:
		nbytes=(MIN(maxchars,(strlen(EPICS_DRIVER_VERSION)+1)));
		strncpy(data, EPICS_DRIVER_VERSION, nbytes-1);
		data[nbytes-1]='\0';
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]EPICS device support Version (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
		*nbytestransfered=nbytes;
		break;

	case driver_version:
		st=irio_getVersion(data, &irio_status);
		if(st==IRIO_success){
			nbytes=(MIN(maxchars,(strlen(data))));
			strncpy(pdrvPvt->irio_version, data, nbytes);
	    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]IRIO library Version (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
			*nbytestransfered=nbytes;
		}
		break;

	case device_name:
	    nbytes=(MIN(maxchars,(strlen(DEVICE_NAME)+1)));
		strncpy(data, DEVICE_NAME, nbytes-1);
		data[nbytes-1]='\0';
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Device Name (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
		*nbytestransfered=nbytes;
		break;

	case FPGAStatus:
	    nbytes=(MIN(maxchars,(strlen(pdrvPvt->FPGAStatus)+1)));
		strncpy(data, pdrvPvt->FPGAStatus, nbytes-1);
		data[nbytes-1]='\0';
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]FPGAStatus (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
		*nbytestransfered=nbytes;
		break;

	case InfoStatus:
	    nbytes=(MIN(maxchars,(strlen(pdrvPvt->InfoStatus)+1)));
		strncpy(data,pdrvPvt->InfoStatus,nbytes-1);
		data[nbytes-1]='\0';
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]InfoStatus (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
		*nbytestransfered=nbytes;
		break;

	case VIversion:
		st=irio_getFPGAVIVersion(&pdrvPvt->drvPvt,data,maxchars,nbytestransfered,&irio_status);
		if(st==IRIO_success){
	    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]VIversion (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
		}
		break;

	case UARTTransmit:
		break;

	case UARTReceive:

		if(pdrvPvt->UARTReceivedMsg!=NULL){
			nbytes=(MIN(maxchars,(strlen(pdrvPvt->UARTReceivedMsg)+1)));
			strncpy(data,pdrvPvt->UARTReceivedMsg,maxchars);
			data[nbytes-1]='\0';
			if(strlen(pdrvPvt->UARTReceivedMsg)>maxchars-1){
				irio_mergeStatus(&irio_status,ValueOOB_Warning,0,"[%s-%d][%s]UART Message Received (addr=%d) message truncated.\n",__func__,__LINE__,pdrvPvt->portName,addr);
			}
			*nbytestransfered=nbytes;
	    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]UART Message Received (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,data);
		}
		break;

	default:
		irio_mergeStatus(&irio_status,FeatureNotImplemented_Error,0,"[%s-%d][%s]Reason:(%d) NOT FOUND\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason);
		break;
	}

	return status_func(pdrvPvt,&irio_status);
}


static asynStatus octetWrite(void *drvPvt,asynUser *pasynUser, const char *data,size_t numchars,size_t *nbytesTransfered){

	irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	int addr;
	asynStatus asyn_status;
	TIRIOStatusCode st=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	*nbytesTransfered=0;

	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==0){
		pdrvPvt->flag_exit=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Trying to write in octetWrite interface at IOC exit\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;
	}
	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==1){
		return asynSuccess;
	}

	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==0){
		pdrvPvt->flag_close=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Trying to write in octetWrite interface with driver closed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;
	}
	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==1){
		return asynSuccess;
	}

	asyn_status = pasynManager->getAddr(pasynUser,&addr);
	if(asyn_status!=asynSuccess){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]Error getAddr connect. Asyn_Status: '%d'.\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason,asyn_status);
		return status_func(pdrvPvt,&irio_status);
	}

	switch (pasynUser->reason) {

		case UARTTransmit:
			st=irio_sendCLuart(&pdrvPvt->drvPvt, data, numchars,&irio_status);
			if(st==IRIO_success){
				*nbytesTransfered=numchars;
				char* msg = malloc(40*sizeof(char));
				int len=0;
				usleep(100000);
				irio_getCLuart(&pdrvPvt->drvPvt,msg,&len,&irio_status);
				msg[len]='\0';
				char* aux = pdrvPvt->UARTReceivedMsg;
				pdrvPvt->UARTReceivedMsg=msg;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]UART Message Transmited (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,msg);
				callbackOctet(drvPvt, UARTReceive, pdrvPvt->UARTReceivedMsg, len+1);
				free(aux);
			}
			break;
		default:
			irio_mergeStatus(&irio_status,FeatureNotImplemented_Error,0,"[%s-%d][%s]Reason:(%d) NOT FOUND\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason);
			break;
	}

	return status_func(pdrvPvt,&irio_status);
}


/**
 * int32 asyndriver interface
 *
 *
 * @param *drvPvt pointer to device structure
 * @param pasynUser pointer to asynuser.
 * @param value [out] pointer to read value int int32 data type
 * @return asynStatus
 */
static asynStatus int32Read(void *drvPvt,asynUser *pasynUser, epicsInt32 *value)
{

	irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	int addr;
	asynStatus asyn_status;
	int32_t aux=0;
	TIRIOStatusCode st=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);

	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==0){
		pdrvPvt->flag_exit=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from int32Read interface at IOC exit\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;
	}
	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==1){
		return asynSuccess;
	}

	if(pasynUser->reason!=riodevice_status){
		if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==0){
			pdrvPvt->flag_close=1;
			*value=0;
	    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from int32Read interface with driver closed\n",__func__,__LINE__,pdrvPvt->portName);
			return asynSuccess;
		}
		if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==1){
			*value=0;
			return asynSuccess;
		}
	}

	asyn_status = pasynManager->getAddr(pasynUser,&addr);
	if(asyn_status!=asynSuccess){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]Error getAddr connect. Asyn_Status: '%d'.\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason,asyn_status);
		return status_func(pdrvPvt,&irio_status);
	}

	switch (pasynUser->reason) {

		case riodevice_status:
			*value=pdrvPvt->rio_device_status;
	    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]riodevice_status (addr=%d) : %s \n",__func__,__LINE__,pdrvPvt->portName,addr,err_dev_stat_struct[pdrvPvt->rio_device_status].riodevice_status_string);
		break;

		case SamplingRate:
			if(pdrvPvt->drvPvt.platform==IRIO_cRIO && pdrvPvt->drvPvt.devProfile==1){//CRIO IO profile
				st=irio_getSamplingRate(&pdrvPvt->drvPvt,addr, &aux,&irio_status);
				if(st==IRIO_success){
					if (aux==0){
						aux=1;
					}
					*value=pdrvPvt->drvPvt.Fref/aux;
				}

			}else{
				st=irio_getDMATtoHostSamplingRate(&pdrvPvt->drvPvt,addr, &aux,&irio_status);
				if(st==IRIO_success){
					if (aux==0){
						aux=1;
					}
					*value=pdrvPvt->drvPvt.Fref/aux;
					pdrvPvt->ai_dma_thread[addr].SR=*value;
				}
			}
			if(st==IRIO_success){
		    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]SamplingRate (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case SR_AI_Intr:
			if(globalData[pdrvPvt->portNumber].ai_poll_thread_created==1){
				*value=pdrvPvt->thread_ai->SR;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SR_AI_Intr (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			else{
				*value=10;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SR_AI_Intr (addr=%d) default value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case SR_DI_Intr:
			if(globalData[pdrvPvt->portNumber].di_poll_thread_created==1){
				*value=pdrvPvt->thread_di->SR;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SR_DI_Intr (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			else{
				*value=10;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SR_DI_Intr (addr=%d) default value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case debug:
			st=irio_getDebugMode(&pdrvPvt->drvPvt,value,&irio_status);
			if(st==IRIO_success){
		    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]debug (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case GroupEnable:
			st=irio_getDMATtoHostEnable(&pdrvPvt->drvPvt,addr,value,&irio_status);
			if(st==IRIO_success){
		    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]GroupEnable (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case FPGAStart:
			st=irio_getFPGAStart(&pdrvPvt->drvPvt,value,&irio_status);
			if(st==IRIO_success){
		    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]FPGAStart (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case DAQStartStop:
			st=irio_getDAQStartStop(&pdrvPvt->drvPvt,value,&irio_status);
			if(st==IRIO_success){
		    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]DAQStartStop (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
				pdrvPvt->acq_status=*value;
			}
			break;

		case DF:
			//TODO: Review
			pdrvPvt->ai_dma_thread[addr].DecimationFactor=1;
			*value=1;
	    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]DF (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case DevQualityStatus:
			st=irio_getDevQualityStatus(&pdrvPvt->drvPvt,value,&irio_status);
			if(st==IRIO_success){
		    	//errlogSevPrintf(errlogInfo,"[%s-%d][%s]DevQualityStatus (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case DMAsOverflow:
			st=irio_getDMATtoHostOverflow(&pdrvPvt->drvPvt,&aux, &irio_status);
			if (st==IRIO_success){
				*value=((uint32_t)aux>>(uint32_t)addr)&0x00000001u;
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]DMAsOverflow (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case AOEnable:
			st=irio_getAOEnable(&pdrvPvt->drvPvt,addr,value,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]AOEnable (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case SGFreq:
			if(pdrvPvt->drvPvt.NoOfSG>addr){
				if(pdrvPvt->sgData[addr].Freq!=0){
					*value=pdrvPvt->sgData[addr].Freq;
				}else{
					st=irio_getSGFreq(&pdrvPvt->drvPvt,addr,value, &irio_status);
					if (st==IRIO_success){
						if(pdrvPvt->sgData[addr].UpdateRate==0){
							st=irio_getSGUpdateRate(&pdrvPvt->drvPvt,addr, &aux, &irio_status);//TODO UpdateRate in FPGA = 0??
							if (st==IRIO_success){
								if(aux==0){
									pdrvPvt->sgData[addr].UpdateRate=0;
								}else{
									pdrvPvt->sgData[addr].UpdateRate=pdrvPvt->drvPvt.SGfref[addr]/aux;
								}
							}

						}
						*value=(*value)*(epicsInt32)(((int64_t)pdrvPvt->sgData[addr].UpdateRate)/4294967296);
						pdrvPvt->sgData[addr].Freq=*value;
						errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGFreq (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
					}
				}
			}else{
				irio_mergeStatus(&irio_status,ResourceNotFound_Warning,0,"[%s-%d][%s]ResourceNotFound_Warning: SG (addr=%d) out of bounds. Number of Signal Generators in FPGA = %d\n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->drvPvt.NoOfSG);
			}
			break;

		case SGUpdateRate:
			if(pdrvPvt->drvPvt.NoOfSG>addr){
				if(pdrvPvt->sgData[addr].UpdateRate!=0){
					*value=pdrvPvt->sgData[addr].UpdateRate;
				}else{
					st=irio_getSGUpdateRate(&pdrvPvt->drvPvt,addr, value, &irio_status);
					if (st==IRIO_success){
						errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGUpdateRate (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
						if(*value==0){
							pdrvPvt->sgData[addr].UpdateRate=0;
						}else{
							*value=pdrvPvt->drvPvt.SGfref[addr]/(*value);
							pdrvPvt->sgData[addr].UpdateRate=*value;
						}
					}
				}

			}else{
				irio_mergeStatus(&irio_status,ResourceNotFound_Warning,0,"[%s-%d][%s]ResourceNotFound_Warning: SG (addr=%d) out of bounds. Number of Signal Generators in FPGA = %d\n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->drvPvt.NoOfSG);
			}
			break;

		case SGSignalType:
			st=irio_getSGSignalType(&pdrvPvt->drvPvt, addr, value,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGSignalType (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case SGPhase:
			st=irio_getSGPhase(&pdrvPvt->drvPvt, addr, value,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGPhase (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case DI:
			st=irio_getDI(&pdrvPvt->drvPvt, addr,value,&irio_status);
			if (st==IRIO_success){
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]DI (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case DO:
			st=irio_getDO(&pdrvPvt->drvPvt, addr,value,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]DO (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case auxAI:
			st=irio_getAuxAI(&pdrvPvt->drvPvt, addr,value,&irio_status);
			if (st==IRIO_success){
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]auxAI (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case auxAO:
			st=irio_getAuxAO(&pdrvPvt->drvPvt, addr,value,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]auxAO (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case auxDI:
			st=irio_getAuxDI(&pdrvPvt->drvPvt, addr,value,&irio_status);
			if (st==IRIO_success){
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]auxDI (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case auxDO:
			st=irio_getAuxDO(&pdrvPvt->drvPvt, addr,value,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]auxDO (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case CLConfiguration:
			*value=pdrvPvt->CLConfig.Configuration;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLConfiguration (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLSignalMapping:
			*value=pdrvPvt->CLConfig.SignalMapping;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSignalMapping (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLLineScan:
			*value=pdrvPvt->CLConfig.LineScan;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLLineScan (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLFVALHigh:
			*value=pdrvPvt->CLConfig.FVALHigh;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLFVALHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLLVALHigh:
			*value=pdrvPvt->CLConfig.LVALHigh;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLLVALHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLDVALHigh:
			*value=pdrvPvt->CLConfig.DVALHigh;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLDVALHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLSpareHigh:
			*value=pdrvPvt->CLConfig.SpareHigh;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSpareHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLControlEnable:
			*value=pdrvPvt->CLConfig.ControlEnable;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLControlEnable (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLSizeX:
			*value=pdrvPvt->sizeX;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSizeX (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case CLSizeY:
			*value=pdrvPvt->sizeY;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSizeY (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		case UARTBaudRate:
			st=irio_getUARTBaudRate(&pdrvPvt->drvPvt,value,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]UARTBaudRate (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case UARTBreakIndicator:
			st=irio_getUARTBreakIndicator(&pdrvPvt->drvPvt,value,&irio_status);
			if (st==IRIO_success){
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]UARTBreakIndicator (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case UARTFrammingError:
			st=irio_getUARTFrammingError(&pdrvPvt->drvPvt,value,&irio_status);
			if (st==IRIO_success){
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]UARTFrammingError (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case UARTOverrunError:
			st=irio_getUARTOverrunError(&pdrvPvt->drvPvt,value,&irio_status);
			if (st==IRIO_success){
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]UARTOverrunError (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		default:
			irio_mergeStatus(&irio_status,FeatureNotImplemented_Error,0,"[%s-%d][%s]Reason:(%d) NOT FOUND\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason);
			break;
	}

	return status_func(pdrvPvt,&irio_status);
}

/**
*	asyn interface int32Write is used to write in the RIO FPGA Device
*	This function gives support to the next reasons:
*		SR[15-0]
*		GroupEnable[15-0]
*		debug
*		DAQStartStop
*		WFFrequency[1-0]
*		AOEnable[1-0]
*		AOEnable[1-0]
*		SignalType[1-0]
*		DF[15-0]
*		DO[95-0]
*/
static asynStatus int32Write(void *drvPvt,asynUser *pasynUser, epicsInt32 value) {

	irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	int addr;
	TIRIOStatusCode st=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	asynStatus asyn_status;
	int i=0,j=0;

	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==0){
		pdrvPvt->flag_exit=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Trying to write in int32Write interface at IOC exit\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;
	}
	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==1){
		return asynSuccess;
	}

	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==0){
		pdrvPvt->flag_close=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Trying to write in int32Write interface with driver closed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;
	}
	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==1){
		return asynSuccess;
	}

	asyn_status = pasynManager->getAddr(pasynUser,&addr);
	if(asyn_status!=asynSuccess){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]Error getAddr connect. Asyn_Status: '%d'.\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason,asyn_status);
		return status_func(pdrvPvt,&irio_status);
	}

	switch (pasynUser->reason) {

	case SamplingRate:

		if (value>=pdrvPvt->drvPvt.minSamplingRate  &&  value<= pdrvPvt->drvPvt.maxSamplingRate) {

			if(pdrvPvt->drvPvt.platform==IRIO_cRIO && pdrvPvt->drvPvt.devProfile==1){//CRIO IO profile
				value=pdrvPvt->drvPvt.Fref/value;
				st=irio_setSamplingRate(&pdrvPvt->drvPvt,addr, value,&irio_status);
			}else{
				pdrvPvt->ai_dma_thread[addr].SR=value;
				value=pdrvPvt->drvPvt.Fref/value;
				st=irio_setDMATtoHostSamplingRate(&pdrvPvt->drvPvt,addr, value,&irio_status);
			}
			if(st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SamplingRate (addr=%d) value: %u \n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->drvPvt.Fref/value);

				if((pdrvPvt->error_oob_array[ERROR_OOB_SR][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
					pdrvPvt->error_oob_array[ERROR_OOB_SR][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
					pdrvPvt->dyn_err_count--;
				}
			}
		}else {
			irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SamplingRate (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_SR][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
				pdrvPvt->error_oob_array[ERROR_OOB_SR][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
				pdrvPvt->dyn_err_count++;
			}
		}
		break;

	case SR_AI_Intr:
		if(globalData[pdrvPvt->portNumber].ai_poll_thread_created==1){
			if((value<=1000)&&(value>=10)){
				pdrvPvt->thread_ai->SR=value;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SR_AI_Intr (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->thread_ai->SR);
				if((pdrvPvt->error_oob_array[ERROR_OOB_SR_AI_INTR][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
					pdrvPvt->error_oob_array[ERROR_OOB_SR_AI_INTR][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
					pdrvPvt->dyn_err_count--;
				}
			}
			else{
				irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SR_AI_Intr (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
				if((pdrvPvt->error_oob_array[ERROR_OOB_SR_AI_INTR][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
					pdrvPvt->error_oob_array[ERROR_OOB_SR_AI_INTR][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
					pdrvPvt->dyn_err_count++;
				}
			}
		}
		else{
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread for AI acquisition by I/O Intr is not created yet \n",__func__,__LINE__,pdrvPvt->portName);
		}
		break;

	case SR_DI_Intr:
		if(globalData[pdrvPvt->portNumber].di_poll_thread_created==1){
			if((value<=1000)&&(value>=10)){
				pdrvPvt->thread_di->SR=value;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SR_DI_Intr (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->thread_di->SR);
				if((pdrvPvt->error_oob_array[ERROR_OOB_SR_DI_INTR][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
					(pdrvPvt->error_oob_array[ERROR_OOB_SR_DI_INTR][addr/8])&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
					pdrvPvt->dyn_err_count--;
				}
			}
			else{
				irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SR_DI_Intr (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
				if((pdrvPvt->error_oob_array[ERROR_OOB_SR_DI_INTR][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
					(pdrvPvt->error_oob_array[ERROR_OOB_SR_DI_INTR][addr/8])|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
					pdrvPvt->dyn_err_count++;
				}
			}
		}
		else{
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread for DI acquisition by I/O Intr is not created yet \n",__func__,__LINE__,pdrvPvt->portName);
		}
		break;

	case debug:

		st=irio_setDebugMode(&pdrvPvt->drvPvt, value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]debug (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case GroupEnable:

		st=irio_setDMATtoHostEnable(&pdrvPvt->drvPvt,addr,value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]GroupEnable (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case FPGAStart:

		//Create acquisition threads for AI and DI with SCAN=I/O Intr
		for(j=0;j<MAX_NUMBER_OF_CARDS;j++){
					if((globalData[j].init_success==1)&&(pdrvPvt->portNumber==j)){
						for (i=0;i<globalData[j].io_number;i++){
							int scan=strcmp(globalData[j].intr_records[i].scan,"I/O Intr");
							int reason_ai=strcmp(globalData[j].intr_records[i].reason,"AI");
							int reason_di=strcmp(globalData[j].intr_records[i].reason,"DI");
							if((scan==0)&&(reason_ai==0)){
								if((globalData[j].ai_poll_thread_created==0)){
									pdrvPvt->thread_ai=malloc(sizeof(thread_ai_t)*1); //One thread for all AI I/O Intr PVs
									pdrvPvt->thread_ai->name=NULL;
									asprintf(&pdrvPvt->thread_ai->name,"RIO%d_ai_poll",pdrvPvt->portNumber);
									pdrvPvt->thread_ai->threadends=0;
									pdrvPvt->thread_ai->portNumber=j;
									pdrvPvt->thread_ai->endAck=0;
									pdrvPvt->thread_ai->SR=10;
									pdrvPvt->thread_ai->asynPvt=pdrvPvt;
									pdrvPvt->thread_ai->id=(epicsThreadId*)epicsThreadCreate(pdrvPvt->thread_ai->name,
											epicsThreadPriorityHigh,
											epicsThreadGetStackSize(epicsThreadStackBig),
											(EPICSTHREADFUNC)ai_intr_thread,pdrvPvt->thread_ai);
									errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread: %s created successfully\n",__func__,__LINE__,pdrvPvt->portName,pdrvPvt->thread_ai->name);
									globalData[j].ai_poll_thread_created=1;
								}
							}
							if((scan==0)&&(reason_di==0)){
								if(globalData[j].di_poll_thread_created==0){
									pdrvPvt->thread_di=malloc(sizeof(thread_di_t)*1); //One thread for all DI I/O Intr PVs
									pdrvPvt->thread_di->name=NULL;
									asprintf(&pdrvPvt->thread_di->name,"RIO%d_di_poll",pdrvPvt->portNumber);
									pdrvPvt->thread_di->threadends=0;
									pdrvPvt->thread_di->portNumber=j;
									pdrvPvt->thread_di->endAck=0;
									pdrvPvt->thread_di->SR=10;
									pdrvPvt->thread_di->asynPvt=pdrvPvt;
									pdrvPvt->thread_di->id=(epicsThreadId*)epicsThreadCreate(pdrvPvt->thread_di->name,
											epicsThreadPriorityHigh,
											epicsThreadGetStackSize(epicsThreadStackBig),
											(EPICSTHREADFUNC)di_intr_thread,pdrvPvt->thread_di);
									errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread: %s created successfully\n",__func__,__LINE__,pdrvPvt->portName,pdrvPvt->thread_di->name);
									globalData[j].di_poll_thread_created=1;
								}
							}
						}
					}
				}

		//IMAQ Profile
		if(pdrvPvt->drvPvt.platform==IRIO_FlexRIO && (pdrvPvt->drvPvt.devProfile==1 || pdrvPvt->drvPvt.devProfile==3)){
			irio_configCL(&pdrvPvt->drvPvt,pdrvPvt->CLConfig.FVALHigh,pdrvPvt->CLConfig.LVALHigh,
					pdrvPvt->CLConfig.DVALHigh,pdrvPvt->CLConfig.SpareHigh,	pdrvPvt->CLConfig.ControlEnable,
					pdrvPvt->CLConfig.LineScan,pdrvPvt->CLConfig.SignalMapping,pdrvPvt->CLConfig.Configuration,&irio_status);
		}

		st=irio_setFPGAStart(&pdrvPvt->drvPvt,value,&irio_status);
		if (st==IRIO_success){
			pdrvPvt->FPGAstarted=1;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]FPGAStart (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}

		if(pdrvPvt->drvPvt.platform==IRIO_FlexRIO && (pdrvPvt->drvPvt.devProfile==1 || pdrvPvt->drvPvt.devProfile==3)){
			st=irio_setUARTBaudRate(&pdrvPvt->drvPvt,0,&irio_status);
			if (st==IRIO_success){
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]UARTBaudRate (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			}
		}

		break;

	case DAQStartStop:

		st=irio_setDAQStartStop(&pdrvPvt->drvPvt,value,&irio_status);
		if (st==IRIO_success){
			pdrvPvt->acq_status=value;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]DAQStartStop (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case SGFreq:

		if (value>0) {
			if(pdrvPvt->drvPvt.NoOfSG>addr ){

				pdrvPvt->sgData[addr].Freq = value;

				if(pdrvPvt->sgData[addr].UpdateRate==0){
					value=0;
				}else{
					value=pdrvPvt->sgData[addr].Freq*(4294967296/pdrvPvt->sgData[addr].UpdateRate);
				}
				st=irio_setSGFreq(&pdrvPvt->drvPvt,addr,value, &irio_status);
				if(st==IRIO_success){
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGFreq (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->sgData[addr].Freq);
					if((pdrvPvt->error_oob_array[ERROR_OOB_SGFREQ][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
						pdrvPvt->error_oob_array[ERROR_OOB_SGFREQ][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
						pdrvPvt->dyn_err_count--;
					}
				}else{
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGFreq (addr=%d) error in irio_setSGFreq \n",__func__,__LINE__,pdrvPvt->portName,addr);
				}
			}else{
				irio_mergeStatus(&irio_status,ResourceNotFound_Warning,0,"[%s-%d][%s]ResourceNotFound_Warning: SG (addr=%d) out of bounds. Number of Signal Generators in FPGA = %d\n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->drvPvt.NoOfSG);
			}
		}
		else {
			irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SGFreq (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_SGFREQ][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
				pdrvPvt->error_oob_array[ERROR_OOB_SGFREQ][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
				pdrvPvt->dyn_err_count++;
			}
		}

		break;

	case SGUpdateRate:

		if(pdrvPvt->drvPvt.NoOfSG>addr){
			if(value<=0){
				irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SGUpdateRate (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
				if((pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
					pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
					pdrvPvt->dyn_err_count++;
				}
			}else{
				if(value>pdrvPvt->drvPvt.SGfref[addr]){
					irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SGUpdateRate (addr=%d) value: %d truncated to %d\n",__func__,__LINE__,pdrvPvt->portName,addr,value,pdrvPvt->drvPvt.SGfref[addr]);
					value=pdrvPvt->drvPvt.SGfref[addr];
				}
				pdrvPvt->sgData[addr].UpdateRate= value;
				//Value contains the frequency desired to update analog output
				value=pdrvPvt->drvPvt.SGfref[addr]/value;
				st=irio_setSGUpdateRate(&pdrvPvt->drvPvt, addr, value, &irio_status);
				if(st==IRIO_success){
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGUpdateRate (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->sgData[addr].UpdateRate);
				}
				//Update SGFreq to keep the relation between SGFreq y SGUpdateRate
				int32_t aux = 0;

				aux=pdrvPvt->sgData[addr].Freq*(4294967296/pdrvPvt->sgData[addr].UpdateRate);
				st=irio_setSGFreq(&pdrvPvt->drvPvt,addr,aux, &irio_status);
				if(st==IRIO_success){
					//errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGFreq (addr=%d) value updated to: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,aux);
				}
				if((pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
					pdrvPvt->error_oob_array[ERROR_OOB_SGUPDATERATE][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
					pdrvPvt->dyn_err_count--;
				}
			}

		}else{
			irio_mergeStatus(&irio_status,ResourceNotFound_Warning,0,"[%s-%d][%s]ResourceNotFound_Warning: SG (addr=%d) out of bounds. Number of Signal Generators in FPGA = %d\n",__func__,__LINE__,pdrvPvt->portName,addr,pdrvPvt->drvPvt.NoOfSG);
		}
		break;

	case SGPhase:

		st=irio_setSGPhase(&pdrvPvt->drvPvt, addr,(int32_t)value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGPhase (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case AOEnable:

		st=irio_setAOEnable(&pdrvPvt->drvPvt,addr,value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]AOEnable (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case SGSignalType:

		st=irio_setSGSignalType(&pdrvPvt->drvPvt,addr, value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGSignalType (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case DF:

		if (value>=1) {
			pdrvPvt->ai_dma_thread[addr].DecimationFactor=value;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]DF (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_DF][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
				pdrvPvt->error_oob_array[ERROR_OOB_DF][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
				pdrvPvt->dyn_err_count--;
			}
		}
		else {
			irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]DF (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_DF][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
				pdrvPvt->error_oob_array[ERROR_OOB_DF][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
				pdrvPvt->dyn_err_count++;
			}
		}
		break;

	case DO:

		st=irio_setDO(&pdrvPvt->drvPvt, addr,value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]DO (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case auxAO:

		st=irio_setAuxAO(&pdrvPvt->drvPvt, addr, value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]auxAO (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case auxDO:

		st=irio_setAuxDO(&pdrvPvt->drvPvt, addr, value,&irio_status);
		if (st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]auxDO (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	case CLConfiguration:

		pdrvPvt->CLConfig.Configuration=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLConfiguration (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLSignalMapping:

		pdrvPvt->CLConfig.SignalMapping=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSignalMapping (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLLineScan:

		pdrvPvt->CLConfig.LineScan=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLLineScan (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLFVALHigh:

		pdrvPvt->CLConfig.FVALHigh=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLFVALHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLLVALHigh:

		pdrvPvt->CLConfig.LVALHigh=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLLVALHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLDVALHigh:

		pdrvPvt->CLConfig.DVALHigh=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLDVALHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLSpareHigh:

		pdrvPvt->CLConfig.SpareHigh=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSpareHigh (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLControlEnable:

		pdrvPvt->CLConfig.ControlEnable=value;
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLControlEnable (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		break;

	case CLSizeX:

		if (value>=1 && value<=1690) {
			pdrvPvt->sizeX=value;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSizeX (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEX][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
				pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEX][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
				pdrvPvt->dyn_err_count--;
			}
		}
		else {
			irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]CLSizeX (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEX][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
				pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEX][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
				pdrvPvt->dyn_err_count++;
			}
		}
		break;

	case CLSizeY:

		if (value>=1 && value<=1710) {
			pdrvPvt->sizeY=value;
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]CLSizeY (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEY][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
				pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEY][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
				pdrvPvt->dyn_err_count--;
			}
		}
		else {
			irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]CLSizeY (addr=%d) value : %d, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
			if((pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEY][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
				pdrvPvt->error_oob_array[ERROR_OOB_CLSIZEY][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
				pdrvPvt->dyn_err_count++;
			}
		}
		break;

	case UARTBaudRate:

		st=irio_setUARTBaudRate(&pdrvPvt->drvPvt,value,&irio_status);
		if(st==IRIO_success){
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]UARTBaudRate (addr=%d) value: %d \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
		}
		break;

	default:
		irio_mergeStatus(&irio_status,FeatureNotImplemented_Error,0,"[%s-%d][%s]Reason:(%d) NOT FOUND\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason);
		break;
	}
	return status_func(pdrvPvt,&irio_status);
}

/**
*	asyn interface float64Write is used to write in the RIO FPGA Device
*	This function gives support to the next reasons:
*		cRIO:aoutput[10,-10], WFAmplitude[10,-10]
*		fRIO:aoutput[1,-1], WFAmplitude[1,-1]
*
*/
static asynStatus float64Write(void *drvPvt,asynUser *pasynUser, epicsFloat64 value)
{
	irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	int addr;
	TIRIOStatusCode st=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	asynStatus asyn_status;

	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==0){
		pdrvPvt->flag_exit=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Trying to write in float64Write interface at IOC exit\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;

	}
	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==1){
		return asynSuccess;
	}

	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==0){
		pdrvPvt->flag_close=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Trying to write in float64Write interface with driver closed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;

	}
	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==1){
		return asynSuccess;
	}



	asyn_status = pasynManager->getAddr(pasynUser,&addr);
	if(asyn_status!=asynSuccess){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]Error getAddr connect. Asyn_Status: '%d'.\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason,asyn_status);
		return status_func(pdrvPvt,&irio_status);
	}

	switch (pasynUser->reason)
	{
		case AO:

			/**
			*	The value wrote on the record is rounded to the maximum or minimum value
			*	due to the maximum and minimum analog output supported by the RIO card.
			*	If the value written is more or less than the limits then occurs an error
			*	And is marked in the flagError variable with a "1". This Error will produce
			*	A WRITE ALARM with state INVALID
			*/

			if (value>pdrvPvt->drvPvt.minAnalogOut && value <pdrvPvt->drvPvt.maxAnalogOut){
				st=irio_setAO(&pdrvPvt->drvPvt,addr, (int32_t)(value*pdrvPvt->drvPvt.CVDAC),&irio_status);
				if(st==IRIO_success){
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]AO (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
					if((pdrvPvt->error_oob_array[ERROR_OOB_AO][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
						pdrvPvt->error_oob_array[ERROR_OOB_AO][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
						pdrvPvt->dyn_err_count--;
					}
				}
			}
			else {
				irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]AO (addr=%d) value : %f, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
				if((pdrvPvt->error_oob_array[ERROR_OOB_AO][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
					pdrvPvt->error_oob_array[ERROR_OOB_AO][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
					pdrvPvt->dyn_err_count++;
				}
			}
			break;

		case SGAmp:

			if (value>pdrvPvt->drvPvt.minAnalogOut && value<pdrvPvt->drvPvt.maxAnalogOut){
				st=irio_setSGAmp(&pdrvPvt->drvPvt,addr,(int32_t)(value*pdrvPvt->drvPvt.CVDAC),&irio_status);
				if(st==IRIO_success){
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGAmp (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
					if((pdrvPvt->error_oob_array[ERROR_OOB_SGAMP][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
						pdrvPvt->error_oob_array[ERROR_OOB_SGAMP][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
						pdrvPvt->dyn_err_count--;
					}
				}
			}
			else {
				irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]SGAmp (addr=%d) value : %f, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
				if((pdrvPvt->error_oob_array[ERROR_OOB_SGAMP][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
					pdrvPvt->error_oob_array[ERROR_OOB_SGAMP][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
					pdrvPvt->dyn_err_count++;
				}
			}

			break;

		case UserDefinedConversionFactor:
			if(pdrvPvt->drvPvt.DMATtoHOSTFrameType[addr]>=128){
				if(value>0){
					pdrvPvt->UserDefinedConversionFactor[addr]=value;
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]UserConversionFactor (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,value);
					if((pdrvPvt->error_oob_array[ERROR_OOB_USER_DEFINED_CONVERSION_FACTOR][addr/8]&(0x01u<<(uint8_t)(addr%8)))>0){
						pdrvPvt->error_oob_array[ERROR_OOB_USER_DEFINED_CONVERSION_FACTOR][addr/8]&=(~(0x01u<<(uint8_t)(addr%8))); //Put bit to zero
						pdrvPvt->dyn_err_count--;
					}
				}
				else{
					irio_mergeStatus(&irio_status,Generic_Error,0,"[%s-%d][%s]UserConversionFactor (addr=%d) value : %f, out of bounds.\n",__func__,__LINE__,pdrvPvt->portName,addr,value);
					if((pdrvPvt->error_oob_array[ERROR_OOB_USER_DEFINED_CONVERSION_FACTOR][addr/8]&(0x01u<<(uint8_t)(addr%8)))==0){
						pdrvPvt->error_oob_array[ERROR_OOB_USER_DEFINED_CONVERSION_FACTOR][addr/8]|=(0x01u<<(uint8_t)(addr%8)); //Put bit to one
						pdrvPvt->dyn_err_count++;
					}
				}
			}
			else{
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]UserConversionFactor (addr=%d) is disabled in FPGA implementation. Default value of I/O Module used by default \n",__func__,__LINE__,pdrvPvt->portName,addr);
			}
			break;

		default:
			irio_mergeStatus(&irio_status,FeatureNotImplemented_Error,0,"[%s-%d][%s]Reason:(%d) NOT FOUND\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason);
			break;
	}

	return status_func(pdrvPvt,&irio_status);

}

static asynStatus float64Read(void *drvPvt,asynUser *pasynUser, epicsFloat64 *value) {
	irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	int addr;   //PROBABLY TO BE ERASED
	TStatus irio_status;
	irio_initStatus(&irio_status);
	TIRIOStatusCode st = IRIO_success;
	asynStatus asyn_status;

	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==0){
		pdrvPvt->flag_exit=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from float64Read interface at IOC exit\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;

	}
	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==1){
		return asynSuccess;
	}

	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==0){
		pdrvPvt->flag_close=1;
		*value=0;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from float64Read interface with driver closed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;

	}
	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==1){
		*value=0;
		return asynSuccess;
	}

	asyn_status = pasynManager->getAddr(pasynUser,&addr);
	if(asyn_status!=asynSuccess){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]Error getAddr connect. Asyn_Status: '%d'.\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason,asyn_status);
		return status_func(pdrvPvt,&irio_status);
	}

	int32_t vaux;
	switch (pasynUser->reason) {

		case SGAmp:
			//This is used to initialize the value of the PV with the default value of the FPGA
			st = irio_getSGAmp(&pdrvPvt->drvPvt,addr,&vaux,&irio_status);
			if(st==IRIO_success){
				*value=(epicsFloat64)vaux/pdrvPvt->drvPvt.CVDAC;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]SGAmp (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case AO:

			//This is used to initialize the value of the PV with the default value of the FPGA
			st = irio_getAO(&pdrvPvt->drvPvt,addr,&vaux,&irio_status);
			if(st==IRIO_success){
				*value=(epicsFloat64)vaux/pdrvPvt->drvPvt.CVDAC;
				errlogSevPrintf(errlogInfo,"[%s-%d][%s]AO (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case DeviceTemp:

			//This is used to initialize the value of the PV with the default value of the FPGA
			st = irio_getDevTemp(&pdrvPvt->drvPvt,&vaux,&irio_status);
			if(st==IRIO_success){
				*value=(epicsFloat64)vaux*0.25;
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]DeviceTemp (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case AI:
			//This is used to initialize the value of the PV with the default value of the FPGA
			st = irio_getAI(&pdrvPvt->drvPvt,addr,&vaux,&irio_status);
			if(st==IRIO_success){
				*value=((epicsFloat64)vaux)*pdrvPvt->drvPvt.CVADC;
				//errlogSevPrintf(errlogInfo,"[%s-%d][%s]AI (addr=%d) value: %f \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			}
			break;

		case UserDefinedConversionFactor:

			if(pdrvPvt->UserDefinedConversionFactor[addr]<=0){
				pdrvPvt->UserDefinedConversionFactor[addr]=1;
			}
			*value=pdrvPvt->UserDefinedConversionFactor[addr];
			errlogSevPrintf(errlogInfo,"[%s-%d][%s]UserConversionFactor (addr=%d) value %f \n",__func__,__LINE__,pdrvPvt->portName,addr,*value);
			break;

		default:
			irio_mergeStatus(&irio_status,FeatureNotImplemented_Error,0,"[%s-%d][%s]Reason:(%d) NOT FOUND\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason);
			break;
	}

	return status_func(pdrvPvt,&irio_status);
}

				/*   FLOAT 32 ARRAY READ   */

static asynStatus float32ArrayRead(void *drvPvt, asynUser *pasynUser, epicsFloat32 *value, size_t nelements,  size_t *nIn){

	irio_pvt_t *pdrvPvt = (irio_pvt_t *) drvPvt;
	asynStatus asyn_status;
	TStatus irio_status;
	int addr;
	irio_initStatus(&irio_status);

	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==0){
		pdrvPvt->flag_exit=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from float32ArrayRead interface at IOC exit\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;

	}
	if(pdrvPvt->epicsExiting==1 && pdrvPvt->flag_exit==1){
		return asynSuccess;
	}

	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==0){
		pdrvPvt->flag_close=1;
    	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Reading from float32ArrayRead interface with driver closed\n",__func__,__LINE__,pdrvPvt->portName);
		return asynSuccess;

	}
	if(pdrvPvt->driverInitialized==0 && pdrvPvt->flag_close==1){
		return asynSuccess;
	}

	if (!value){
		return asynError;
	}

	if (!nIn){
		return asynError;
	}

	asyn_status = pasynManager->getAddr(pasynUser,&addr);
	if(asyn_status!=asynSuccess){
		irio_mergeStatus(&irio_status,Generic_Warning,0,"[%s-%d][%s]Error getAddr connect. Asyn_Status: '%d'.\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason,asyn_status);
		return status_func(pdrvPvt,&irio_status);
	}

//	if (nelements > 4096)
//		nelements = 4096;

	switch(pasynUser->reason){
	case CH:
		break;
	default:
		irio_mergeStatus(&irio_status,FeatureNotImplemented_Error,0,"[%s-%d][%s]Reason:(%d) NOT FOUND\n",__func__,__LINE__,pdrvPvt->portName,pasynUser->reason);
		break;
	}
	return status_func(pdrvPvt,&irio_status);
}

/**
*	Next, Registration of the irioinit function and its 4 parameters
*/
static const iocshArg nirioInitArg0 = { "portName", iocshArgString }; 	//!< i.e, "RIO12" first number "1" the number of chassis from top to down
									//!< i.e, second number "2" is the number of slot.
static const iocshArg nirioInitArg1 = { "DevSerial", iocshArgString };   //!< i.e, Serial Number of the device Card
static const iocshArg nirioInitArg2 = { "irioDevice", iocshArgString }; //!< i.e, "PXIe-7965R"
static const iocshArg nirioInitArg3 = { "projectName", iocshArgString }; //!< i.e, "7952fpga"
static const iocshArg nirioInitArg4 = { "FPGAVersion", iocshArgString };	//!< i.e, "V2.12"
static const iocshArg nirioInitArg5 = { "verbosity", iocshArgInt };

static const iocshArg *nirioInitArgs[] = {
						&nirioInitArg0,
						&nirioInitArg1,
						&nirioInitArg2,
						&nirioInitArg3,
						&nirioInitArg4,
						&nirioInitArg5};

static const iocshFuncDef nirioInitFuncDef = {"nirioinit",6, nirioInitArgs};

static void nirioInitCallFunc (const iocshArgBuf *args) {
	nirioinit(args[0].sval, args[1].sval ,args[2].sval,args[3].sval,args[4].sval,args[5].ival);
}

static void nirioRegister (void)
{
	static int firstTime = 1;
	if (firstTime==1) {
		firstTime = 0;
		iocshRegister(&nirioInitFuncDef, nirioInitCallFunc);
	}
}

epicsExportRegistrar(nirioRegister);

epicsRegisterFunction(irioTimeStamp);



int status_func(void *drvPvt, TStatus* status){
	irio_pvt_t* pdrvPvt=(irio_pvt_t *)drvPvt;
	char *error_string=NULL;
	riodevice_status_name_t next_status=pdrvPvt->rio_device_status;
	int update=0;
	switch(status->detailCode){
	case Generic_Error:
		next_status=STATUS_DYNAMIC_CONFIG_ERROR;
		asprintf(&error_string,"One of the values configured is out of bounds");
		update=1;
		break;
	case HardwareNotFound_Error:
		next_status=STATUS_NO_BOARD;
		irio_getErrorString(status->detailCode,&error_string);
		update=1;
		break;
	case BitfileDownload_Error:
	case ListRIODevicesCommand_Error:
	case ListRIODevicesParsing_Error:
	case SignatureNotFound_Error:
	case MemoryAllocation_Error:
	case FileAccess_Error:
	case FileNotFound_Error:
	case FeatureNotImplemented_Error:
		next_status=STATUS_STATIC_CONFIG_ERROR;
		irio_getErrorString(status->detailCode,&error_string);
		update=1;
		break;
	case NIRIO_API_Error:
		if((pdrvPvt->rio_device_status!=STATUS_STATIC_CONFIG_ERROR) && (pdrvPvt->rio_device_status!=STATUS_NO_BOARD) ){
			next_status=STATUS_HARDWARE_ERROR;
			irio_getErrorString(status->detailCode,&error_string);
			update=1;
		}

		break;
	case InitDone_Error:
	case IOModule_Error:
	case ResourceNotFound_Error:
		next_status=STATUS_INCORRECT_HW_CONFIGURATION;
		irio_getErrorString(status->detailCode,&error_string);
		update=1;
		break;
	case SignatureValueNotValid_Error:
	case ResourceValueNotValid_Error:
	case BitfileNotFound_Error:
	case HeaderNotFound_Error:
		next_status=STATUS_STATIC_CONFIG_ERROR; //CHANGE FOR STATUS_CONFIG_FILES_ERROR
		irio_getErrorString(status->detailCode,&error_string);
		update=1;
		break;
	case Success:
		if(pdrvPvt->dyn_err_count==0){
			if(pdrvPvt->FPGAstarted==1 && (pdrvPvt->rio_device_status==STATUS_CONFIGURED || pdrvPvt->rio_device_status==STATUS_DYNAMIC_CONFIG_ERROR)){
				next_status=STATUS_OK;
				asprintf(&error_string,"Device is OK and running");
				update=1;
			}
			if(pdrvPvt->FPGAstarted==0 && pdrvPvt->driverInitialized==1 && (pdrvPvt->rio_device_status==STATUS_DYNAMIC_CONFIG_ERROR || pdrvPvt->rio_device_status==STATUS_INITIALIZING)){
				next_status=STATUS_CONFIGURED;
				asprintf(&error_string,"Device is configured");
				update=1;
			}
		}
		break;
	case FPGAAlreadyRunning_Warning:
		if(pdrvPvt->FPGAstarted==1 && pdrvPvt->rio_device_status==STATUS_CONFIGURED){
			next_status=STATUS_OK;
			asprintf(&error_string,"FPGA started before IOC.");
			update=1;
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
		irio_getErrorString(status->detailCode,&error_string);
		update=1;
		break;
	default:
		break;
	}

	pdrvPvt->rio_device_status=next_status;
	if(update==1){
		callbackInt32(pdrvPvt, riodevice_status, 0, (epicsInt32)err_dev_stat_struct[pdrvPvt->rio_device_status].riodevice_status_name);
		char* aux = pdrvPvt->InfoStatus;
		pdrvPvt->InfoStatus=error_string;
		callbackOctet(pdrvPvt,InfoStatus,pdrvPvt->InfoStatus,strlen(pdrvPvt->InfoStatus));
		free(aux);
	}else{
		free(error_string);
	}

	int retVal=asynSuccess;
	switch(status->code){
	case IRIO_warning:
		errlogSevPrintf(errlogMinor,"[status_func][%s]IRIO WARNING Code:%d. Description:%s. IRIO Messages:%s",pdrvPvt->portName,status->detailCode,pdrvPvt->InfoStatus,status->msg);
		retVal=asynError;
		break;
	case IRIO_error:
		errlogSevPrintf(errlogFatal,"[status_func][%s]IRIO ERROR Code:%d. Description:%s. IRIO Messages:%s",pdrvPvt->portName,status->detailCode,pdrvPvt->InfoStatus,status->msg);
		retVal=asynError;
		break;
	default:
		retVal=asynSuccess;
		break;
	}
	irio_resetStatus(status);
	return retVal;
}

void nirio_epicsExit(void *drvPvt){
	irio_pvt_t* pdrvPvt=(irio_pvt_t *)drvPvt;
	pdrvPvt->epicsExiting=1;
	nirio_shutdown(pdrvPvt);
}

void nirio_shutdown(irio_pvt_t *pdrvPvt){
	TStatus irio_status;
	irio_initStatus(&irio_status);
	int count=0,timeout=0;
	int i=0;
//	int j=0;

	if(pdrvPvt->driverInitialized==1){
		pdrvPvt->driverInitialized=0;
		if(!(pdrvPvt->drvPvt.platform==IRIO_cRIO && pdrvPvt->drvPvt.devProfile==1)){
			if(pdrvPvt->ai_dma_thread!=NULL){
				for(i=0;i<pdrvPvt->drvPvt.DMATtoHOSTNo.value;i++){
					pdrvPvt->ai_dma_thread[i].threadends=1;
					while(pdrvPvt->ai_dma_thread[i].endAck!=1&&timeout==0){
						usleep(1000);
						count++;
						if(count==(2*WAIT_MS*pdrvPvt->drvPvt.DMATtoHOSTNCh[i])){
							timeout=1;
							errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s timeout at exit.\n",__func__,__LINE__,pdrvPvt->portName,pdrvPvt->ai_dma_thread[i].dma_thread_name);
						}
					}
					timeout=0;
					count=0;
				}
			}
			irio_closeDMAsTtoHost(&pdrvPvt->drvPvt,&irio_status);

		}
	}
	for(i=0;i<MAX_NUMBER_OF_CARDS;++i){
		if((globalData[i].ai_poll_thread_created==1)&&(pdrvPvt->portNumber==i)){
			pdrvPvt->thread_ai->threadends=1;
			while(pdrvPvt->thread_ai->endAck!=1&&timeout==0){
				usleep(1000);
				count++;
				if(count==2*WAIT_MS){
					timeout=1;
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s timeout at exit.\n",__func__,__LINE__,pdrvPvt->portName,pdrvPvt->thread_ai->name);
				}
			}
			timeout=0;
			count=0;
		}

		if((globalData[i].di_poll_thread_created==1)&&(pdrvPvt->portNumber==i)){
			pdrvPvt->thread_di->threadends=1;
			while(pdrvPvt->thread_di->endAck!=1&&timeout==0){
				usleep(1000);
				count++;
				if(count==2*WAIT_MS){
					timeout=1;
					errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s timeout at exit.\n",__func__,__LINE__,pdrvPvt->portName,pdrvPvt->thread_di->name);
				}
			}
			timeout=0;
			count=0;
		}
	}
	if(pdrvPvt->closeDriver==0){
		irio_closeDriver(&pdrvPvt->drvPvt,0,&irio_status);
		pdrvPvt->closeDriver=1;
	}

	if(irio_status.code != IRIO_success){
		status_func(pdrvPvt,&irio_status);
	}

	if(pdrvPvt->drvPvt.platform==IRIO_FlexRIO && (pdrvPvt->drvPvt.devProfile==1 || pdrvPvt->drvPvt.devProfile==3)){ //Only for IMAGE profile
		free(pdrvPvt->UARTReceivedMsg);
	}

	if(epicsExiting==1){
		int j=0;
		for(i=0;i<8;i++){
			free(pdrvPvt->error_oob_array[i]);
		}
		for(i=0;i<MAX_NUMBER_OF_CARDS;++i){
			if((globalData[i].ai_poll_thread_created==1)&&(globalData[i].ai_poll_thread_run==0)&&(pdrvPvt->portNumber==i)){
				free(pdrvPvt->thread_ai->id);
				free(pdrvPvt->thread_ai->name);
				free(pdrvPvt->thread_ai->timestamp_source);
				free(pdrvPvt->thread_ai);
				globalData[i].ai_poll_thread_created=0;
			}
			if((globalData[i].di_poll_thread_created==1)&&(globalData[i].di_poll_thread_run==0)&&(pdrvPvt->portNumber==i)){
				free(pdrvPvt->thread_di->id);
				free(pdrvPvt->thread_di->name);
				free(pdrvPvt->thread_di->timestamp_source);
				free(pdrvPvt->thread_di);
				globalData[i].di_poll_thread_created=0;
			}
			for(j=0;j<globalData[i].number_of_DMAs;++j){
				if((globalData[i].dma_thread_created[j]==1)&&(globalData[i].dma_thread_run[j]==0)&&(pdrvPvt->portNumber==i)){
					free(pdrvPvt->ai_dma_thread[j].dma_thread_id);
					free(pdrvPvt->ai_dma_thread[j].dma_thread_name);
					free(pdrvPvt->ai_dma_thread[j].IdRing);
					globalData[i].dma_thread_created[j]=0;
				}
			}
			free(pdrvPvt->ai_dma_thread);
		}
		for(i=0;i<MAX_NUMBER_OF_CARDS;++i){
			if((globalData[i].ai_poll_thread_created!=0)||(globalData[i].di_poll_thread_created!=0)){
				all_poll_threads_finished=1; //All intr threads are not finished.
			}
			for(j=0;j<globalData[i].number_of_DMAs;++j){
				if(globalData[i].dma_thread_created[j]!=0){
					all_dma_threads_finished=1;//Not all dma threads are finished
				}
			}
		}
		if((all_poll_threads_finished==0)&&(all_dma_threads_finished==0)){
			for(j=0;j<MAX_NUMBER_OF_CARDS;++j){
				for (i=0;i<globalData[j].io_number;++i){
					free(globalData[j].intr_records[i].type);
					free(globalData[j].intr_records[i].name);
					free(globalData[j].intr_records[i].input);
					free(globalData[j].intr_records[i].reason);
					free(globalData[j].intr_records[i].scan);
					free(globalData[j].intr_records[i].portName);
				}
				free(globalData[j].ch_nelm);
				free(globalData[j].intr_records);
				free(globalData[j].dma_thread_created);
				free(globalData[j].dma_thread_run);
			}
		}
		free(pdrvPvt->UserDefinedConversionFactor);
		free(pdrvPvt->InfoStatus);
		free(pdrvPvt->portName);
		free(pdrvPvt->sgData);
		//free(pdrvPvt);
	}

	irio_resetStatus(&irio_status);
}

static void ai_intr_thread(void *p){
	TIRIOStatusCode status=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	thread_ai_t *thread_ai=(thread_ai_t *)p;
	struct irioPvt* asynPvt = thread_ai->asynPvt;
	irioDrv_t *irioPvt = &thread_ai->asynPvt->drvPvt;
	int32_t vaux;
	double *aux=calloc(globalData[asynPvt->portNumber].io_number,sizeof(epicsFloat64));
	epicsFloat64 data=0;
	int i=0;

	while (globalData[asynPvt->portNumber].ai_poll_thread_created==0) {usleep(10000);}
	globalData[asynPvt->portNumber].ai_poll_thread_run=1;

	do{
		int SR=0;
		int time_sleep=0;
		SR=(thread_ai->SR);
		time_sleep=1000000/SR;
		for(i=0;i<globalData[asynPvt->portNumber].io_number;i++){
			int reason=0;
			int scan=0;
			reason=strcmp(globalData[thread_ai->portNumber].intr_records[i].reason, "AI");
			scan=strcmp(globalData[thread_ai->portNumber].intr_records[i].scan, "I/O Intr");
			if((reason==0)&&(scan==0)){
				status=irio_getAI(irioPvt,globalData[thread_ai->portNumber].intr_records[i].addr,&vaux,&irio_status);
				data=(epicsFloat64)(vaux*(asynPvt->drvPvt.CVADC));
				if(aux[i]!=data){
					aux[i]=data;
					if(status==IRIO_success){
						callbackFloat64(asynPvt,AI,globalData[thread_ai->portNumber].intr_records[i].addr, data);
					}
				}
			}
		}
		usleep(time_sleep);
	}while(thread_ai->threadends==0 && irio_status.code < IRIO_error);
	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Exiting Thread: %s\n",__func__,__LINE__,asynPvt->portName,thread_ai->name);
	thread_ai->endAck=1;
	if(irio_status.code != IRIO_success){
		errlogSevPrintf(errlogFatal,"[%s-%d][%s]FPGA ERROR in :%s\n",__func__,__LINE__,asynPvt->portName,thread_ai->name);
	}else{
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s Terminated.\n",__func__,__LINE__,asynPvt->portName,thread_ai->name);
		globalData[i].ai_poll_thread_run=0;
	}
	irio_resetStatus(&irio_status);
	free(aux);
}

static void di_intr_thread(void *p){
	TIRIOStatusCode status=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	thread_di_t *thread_di=(thread_di_t *)p;
	struct irioPvt* asynPvt = thread_di->asynPvt;
	irioDrv_t *irioPvt = &thread_di->asynPvt->drvPvt;
	int32_t vaux=0;
	int32_t *aux=calloc(globalData[asynPvt->portNumber].io_number,sizeof(epicsInt32));
	epicsInt32 data=0;
	int i=0;

	while (globalData[asynPvt->portNumber].di_poll_thread_created==0) {usleep(10000);}
	globalData[asynPvt->portNumber].di_poll_thread_run=1;

	do{
		int SR=0;
		int time_sleep=0;
		SR=(thread_di->SR);
		time_sleep=1000000/SR;
		for(i=0;i<globalData[asynPvt->portNumber].io_number;i++){
			int reason=0;
			int scan=0;
			reason=strcmp(globalData[thread_di->portNumber].intr_records[i].reason, "DI");
			scan=strcmp(globalData[thread_di->portNumber].intr_records[i].scan, "I/O Intr");
			if((reason==0)&&(scan==0)){
				status=irio_getDI(irioPvt,globalData[thread_di->portNumber].intr_records[i].addr,&vaux,&irio_status);
				data=(epicsInt32)vaux;
				if(aux[i]!=data){
					aux[i]=data;
					if(status==IRIO_success){
						callbackInt32(asynPvt,DI,globalData[thread_di->portNumber].intr_records[i].addr,(epicsInt32)data);
					}
				}
			}
		}
		usleep(time_sleep);
	}while(thread_di->threadends==0 && irio_status.code < IRIO_error);
	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Exiting Thread: %s\n",__func__,__LINE__,asynPvt->portName,thread_di->name);
	thread_di->endAck=1;
	if(irio_status.code != IRIO_success){
		errlogSevPrintf(errlogFatal,"[%s-%d][%s]FPGA ERROR in :%s\n",__func__,__LINE__,asynPvt->portName,thread_di->name);
	}else{
		errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s Terminated.\n",__func__,__LINE__,asynPvt->portName,thread_di->name);
		globalData[i].di_poll_thread_run=0;
	}
	irio_resetStatus(&irio_status);
	free(aux);
}

static void aiDMA_thread(void *p){
	TIRIOStatusCode status=IRIO_success;
	TStatus irio_status;
	irio_initStatus(&irio_status);
	irio_dmathread_t *ai_dma_thread=NULL;
	irio_dmathread_t *ai_pv_publish=NULL;
	size_t buffersize; //in u64
	uint64_t *dataBuffer=NULL;
	uint64_t *cleanbuffer=NULL;
	int16_t *imgBuffer=NULL;
	size_t imgBufferSize=0;
	size_t currImgSize=0; //in u64
	float **aux=NULL;
	int i=0;
	uint16_t samples_per_channel=0;
	ai_dma_thread=(irio_dmathread_t *)p;
	struct irioPvt* asynPvt = ai_dma_thread->asynPvt;
	irioDrv_t *irioPvt = &ai_dma_thread->asynPvt->drvPvt;
	int imgProfile = irioPvt->platform==IRIO_FlexRIO && (irioPvt->devProfile==1 || irioPvt->devProfile==3);
	double *ConversionFactor=NULL;

	while (globalData[asynPvt->portNumber].dma_thread_run[ai_dma_thread->id]==0) {usleep(10000);}
	int found=0;
	int chIndex;
	if(imgProfile==1){
		chIndex = ai_dma_thread->id;
	}else{
		chIndex = irioPvt->DMATtoHOSTChIndex[ai_dma_thread->id];
	}
	while(!found && i<irioPvt->DMATtoHOSTNCh[ai_dma_thread->id]){
		if(globalData[asynPvt->portNumber].ch_nelm[chIndex + i]!=0){
			found=1;
		}
		++i;
	}

	if(!found){ //No channels to publish -> End thread
		errlogSevPrintf(errlogFatal,"[%s-%d][%s]Thread:%s Terminated. No PVs (waveform CH) subscribed to read data acquired\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);
		globalData[asynPvt->portNumber].dma_thread_run[ai_dma_thread->id]=0;
		ai_dma_thread->endAck=1;
		return;
	}

	//Set data conversion factor.
	if(irioPvt->DMATtoHOSTFrameType[ai_dma_thread->id]<128){
		//I/O Module conversion factor to Volts
		ConversionFactor=&irioPvt->CVADC;
	}
	else{
		//User defined conversion factor. At IOC init its value is 1 by default.
		ConversionFactor=&asynPvt->UserDefinedConversionFactor[ai_dma_thread->id];

	}

	//Thread initialization
	if(imgProfile==1){
		buffersize=globalData[asynPvt->portNumber].ch_nelm[ai_dma_thread->id]/8; //Channel elements are pixels (u8 published as int32), we alloc in terms of u64 words
		imgBufferSize=globalData[asynPvt->portNumber].ch_nelm[ai_dma_thread->id];
		imgBuffer=malloc(sizeof(int16_t)*imgBufferSize);
	}else{

		ai_pv_publish= malloc(sizeof(irio_dmathread_t)*irioPvt->DMATtoHOSTNCh[ai_dma_thread->id]);

		samples_per_channel=irioPvt->DMATtoHOSTBlockNWords[ai_dma_thread->id]*8; //Bytes per block
		samples_per_channel= samples_per_channel/irioPvt->DMATtoHOSTSampleSize[ai_dma_thread->id]; //Samples per block
		samples_per_channel= samples_per_channel/irioPvt->DMATtoHOSTNCh[ai_dma_thread->id];//Samples per channel per block

		// Ring Buffers for Waveforms PVs
		ai_dma_thread->IdRing= malloc(sizeof(epicsRingBytesId)*irioPvt->DMATtoHOSTNCh[ai_dma_thread->id]);
		// Creation and Launching of threads working as consumers for EPICS PVs publishing
		aux=malloc(sizeof(float*)*irioPvt->DMATtoHOSTNCh[ai_dma_thread->id]);
		for(i=0;i<irioPvt->DMATtoHOSTNCh[ai_dma_thread->id];i++){
			aux[i]=malloc(sizeof(float)*samples_per_channel);
			ai_dma_thread->IdRing[i]=NULL;
			ai_pv_publish[i].IdRing=NULL;
			ai_pv_publish[i].dma_thread_id=NULL;
			ai_pv_publish[i].dma_thread_name=NULL;
			ai_pv_publish[i].id=0;
			ai_pv_publish[i].dmanumber=0;
			ai_pv_publish[i].threadends=0;
			ai_pv_publish[i].endAck=0;
			ai_pv_publish[i].blockSize=0;
			ai_pv_publish[i].DecimationFactor=0;
			ai_pv_publish[i].dma_thread_id=NULL;
			if(globalData[asynPvt->portNumber].ch_nelm[chIndex+i]!=0){
				ai_dma_thread->IdRing[i]=epicsRingBytesCreate(samples_per_channel*irioPvt->DMATtoHOSTSampleSize[ai_dma_thread->id]*4096);//!<Ring buffer to store manage the waveforms.
				ai_pv_publish[i].IdRing=&ai_dma_thread->IdRing[i];
				asprintf(&ai_pv_publish[i].dma_thread_name,"RIO%d_CH%d_DMA%d",asynPvt->portNumber,i,ai_dma_thread->id);
				ai_pv_publish[i].id=i; //channel identifier
				ai_pv_publish[i].dmanumber=ai_dma_thread->id; //dma identifier
				ai_pv_publish[i].asynPvt=ai_dma_thread->asynPvt;
				ai_pv_publish[i].SR=ai_dma_thread->SR;
				ai_pv_publish[i].blockSize=1;
				ai_pv_publish[i].DecimationFactor=1;
				ai_pv_publish[i].dma_thread_id=(epicsThreadId*)epicsThreadCreate(ai_pv_publish[i].dma_thread_name,
						epicsThreadPriorityHigh,epicsThreadGetStackSize(epicsThreadStackBig),
						(EPICSTHREADFUNC)ai_pv_thread,
						(void *)&ai_pv_publish[i]);
			}
		}
		//If irioasyn driver is used iriolib:
		//From DMATtoHOSTFrameType=0 to DMATtoHOSTFrameType=127 data conversion factor used is: I/O Module conversion factor.
		//From DMATtoHOSTFrameType=128 to DMATtoHOSTFrameType=255 data conversion factor used is: user defined conversion factor.
		switch(irioPvt->DMATtoHOSTFrameType[ai_dma_thread->id]){
		case 0:
			buffersize=irioPvt->DMATtoHOSTBlockNWords[ai_dma_thread->id];
			break;
		case 1:
			buffersize=irioPvt->DMATtoHOSTBlockNWords[ai_dma_thread->id]+2; //each DMA data block includes two extra U64 words to include timestamp
			break;
		case 128:
			buffersize=irioPvt->DMATtoHOSTBlockNWords[ai_dma_thread->id];
			break;
		case 129:
			buffersize=irioPvt->DMATtoHOSTBlockNWords[ai_dma_thread->id]+2; //each DMA data block includes two extra U64 words to include timestamp
			break;
		default:
			buffersize=irioPvt->DMATtoHOSTBlockNWords[ai_dma_thread->id];
			break;
		}
	}


    // Data acquisition from DMA main loop
	dataBuffer=malloc(sizeof(uint64_t)*buffersize);
	cleanbuffer=malloc(sizeof(uint64_t)*(buffersize));


	int timeout=0,acqInProgress=0,timeoutLimit=100;

	int count=0;
	int DFcount=1;

	float twaitus=0.0;
	do{
		if(ai_dma_thread->asynPvt->acq_status==0 && ai_dma_thread->threadends==0){
			timeout=0;
			DFcount=1;
			do{
				usleep(1000);
			}while(ai_dma_thread->asynPvt->acq_status==0 && ai_dma_thread->threadends==0);
			if(imgProfile==1){
				currImgSize=asynPvt->sizeX*asynPvt->sizeY;
				if(currImgSize>imgBufferSize){
					printf("[%s-%d] ERROR Current Image Size bigger than PV Container\n",__func__,__LINE__);
					ai_dma_thread->asynPvt->acq_status=0;
				}
				continue;
			}
		}
		//Not data received yet
		if(imgProfile==1){
			status=irio_getDMATtoHostImage(irioPvt, currImgSize, ai_dma_thread->id, dataBuffer, &count, &irio_status);
		}else{
			status=irio_getDMATtoHostData(irioPvt,1,ai_dma_thread->id,dataBuffer,&count,&irio_status);
		}
		if(status==IRIO_success){
			if(acqInProgress==0){
				if(count!=0){//Now acquiring data
					printf("ACQ Started. Reading data from DMA%d\n",ai_dma_thread->id);
					acqInProgress=1;
					timeout=0;
				}
			}else{
				//No data received
				if(count==0){
					//ACQ finished?
					if(timeout>timeoutLimit){
						irio_cleanDMATtoHost(irioPvt,ai_dma_thread->id,cleanbuffer,buffersize,&irio_status);
						acqInProgress=0;
						printf("ACQ Stopped in DMA%d\n",ai_dma_thread->id);
					}else{
						timeout++;
					}
				}else{
					timeout=0;
				}
			}
			if(count!=0){
				//Data Read. Decimate blocks read
				if(DFcount>=ai_dma_thread->DecimationFactor){
					//Send block
					DFcount=1;

					if(imgProfile==1){
						CallAIInsInt8Array(asynPvt,CH,ai_dma_thread->id,(epicsInt8*)dataBuffer,currImgSize);
					}else{
						switch(irioPvt->DMATtoHOSTSampleSize[ai_dma_thread->id]){
						case(1):
							getChannelDataU8(irioPvt->DMATtoHOSTNCh[ai_dma_thread->id],samples_per_channel,dataBuffer,aux,*ConversionFactor);
							break;
						case(2):
							getChannelDataU16(irioPvt->DMATtoHOSTNCh[ai_dma_thread->id],samples_per_channel,dataBuffer,aux,*ConversionFactor);
							break;
						case(4):
							getChannelDataU32(irioPvt->DMATtoHOSTNCh[ai_dma_thread->id],samples_per_channel,dataBuffer,aux,*ConversionFactor);
							break;
						case(8):
							getChannelDataU64(irioPvt->DMATtoHOSTNCh[ai_dma_thread->id],samples_per_channel,dataBuffer,aux,*ConversionFactor);
							break;
						default:
							irio_mergeStatus(&irio_status,ResourceNotFound_Error,0,"[%s,%d]-(%s) ERROR Getting channel data from DMA. Sample size %d not allowed.\n",__func__,__LINE__,irioPvt->appCallID,irioPvt->DMATtoHOSTSampleSize[ai_dma_thread->id]);
							break;

						}
						//Copy data to ring buffer
						for(i=0;i<irioPvt->DMATtoHOSTNCh[ai_dma_thread->id];i++){
							if(globalData[asynPvt->portNumber].ch_nelm[chIndex+i]!=0){
								if((sizeof(float)*samples_per_channel)!=(epicsRingBytesPut(ai_dma_thread->IdRing[i],(char*)aux[i],sizeof(float)*samples_per_channel))){
									errlogSevPrintf(errlogFatal,"[%s-%d][%s]Error putting to ringBuffer%d\n",__func__,__LINE__,asynPvt->portName,i);
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
				if(imgProfile==1){
					//TODO Review this
					usleep(1000);
				}else{
					twaitus=500000*(float)samples_per_channel/(ai_dma_thread->SR);
					usleep(twaitus);
				}
			}

		}
	}while(ai_dma_thread->threadends==0 && irio_status.code < IRIO_error);
	//here we are considering IRIO_success and IRIO_warning. IRIO warning, for instance, can be obtained if irio_getDMATtoHostData returns a warning because data is not avaliable
	// see bug 7804. Currently irio_getDMATtoHostData is managing the bug 7808 because if the call to NiFpa_ReadFIFOU64 returns the error -61219 the call is repeated up to 3 times. if not successfully  irio_getDMATtoHostDat returns IRIO_warning
	if (irio_status.code==IRIO_error){
		status_func(asynPvt,&irio_status);
	}
	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Exiting Thread: %s\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);
	if(!imgProfile){
		for(i=0;i<irioPvt->DMATtoHOSTNCh[ai_dma_thread->id];i++){
			if(globalData[asynPvt->portNumber].ch_nelm[chIndex+i]!=0){
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
			}
			free(aux[i]);
		}
		free(aux);
	}
	free(ai_pv_publish);
	free(imgBuffer);
	free(dataBuffer);
	free(cleanbuffer);
	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s Terminated.\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);

	if(irio_status.code != IRIO_success){
		errlogSevPrintf(errlogFatal,"[%s-%d][%s]FPGA ERROR in acquisition thread:%s\n",__func__,__LINE__,asynPvt->portName,ai_dma_thread->dma_thread_name);
	}
	globalData[asynPvt->portNumber].dma_thread_run[ai_dma_thread->id]=0;

	irio_resetStatus(&irio_status);
	ai_dma_thread->endAck=1;
}

static void ai_pv_thread(void *p){
	//There is one thread per ringbuffer (per DMA channel)

	irio_dmathread_t *pv_thread;
	pv_thread=(irio_dmathread_t *)p;
	irioDrv_t *irioPvt = &pv_thread->asynPvt->drvPvt;
	float* pv_data;
	int pv_nelem=4096,aux;
	while (globalData[pv_thread->asynPvt->portNumber].dma_thread_run[pv_thread->dmanumber]==0) {usleep(10000);}
	aux=irioPvt->DMATtoHOSTChIndex[pv_thread->dmanumber]+pv_thread->id;
	pv_nelem=globalData[pv_thread->asynPvt->portNumber].ch_nelm[aux];
	pv_data = malloc(sizeof(float)*pv_nelem);
	errlogSevPrintf(errlogInfo,"[%s-%d][%s]ch%d_nelm:%d \n",__func__,__LINE__,pv_thread->asynPvt->portName,aux,globalData[pv_thread->asynPvt->portNumber].ch_nelm[aux]);
	float twaitus=0.0;

	do{
		int NbytesDecimated;
		NbytesDecimated=epicsRingBytesUsedBytes(*pv_thread->IdRing);
		if(NbytesDecimated>=(sizeof(float)*pv_nelem)){
			if(epicsRingBytesIsFull(*pv_thread->IdRing)==1){
				errlogSevPrintf(errlogFatal,"\nRING BYTES Channel %d FULL\n",pv_thread->id);
			}
			int aux2=0;
			aux2=epicsRingBytesGet(*pv_thread->IdRing,(char*)pv_data,sizeof(float)*pv_nelem);
			if(aux2!=pv_nelem*sizeof(float)){
				errlogSevPrintf(errlogFatal,"Requested %lu elems, get %d",pv_nelem*sizeof(float),aux2);
				usleep(1000);
				continue;
			}
			CallAIInsFloat32Array(pv_thread->asynPvt,CH,irioPvt->DMATtoHOSTChIndex[pv_thread->dmanumber]+pv_thread->id,pv_data,pv_nelem);
		}else{
			twaitus=500000*(float)pv_nelem/(pv_thread->SR);
			usleep(twaitus);
		}
	}while (pv_thread->threadends==0);
	errlogSevPrintf(errlogInfo,"[%s-%d][%s]Thread %s Terminated \n",__func__,__LINE__,pv_thread->asynPvt->portName,pv_thread->dma_thread_name);
	free(pv_data);
	usleep(1000);
	pv_thread->endAck=1;
}

int callbackOctet(irio_pvt_t* asynirio_Pvt, int reason, char* pv_data, int pv_nelem){
	ELLLIST *clients;
	interruptNode * node;
	asynStatus status=asynSuccess;

	pasynManager->interruptStart(asynirio_Pvt->asynOctetInterruptPvt,&clients);
	node = (interruptNode*)ellFirst(clients);

	while (node!=NULL){
		asynOctetInterrupt *pint;
		pint = (asynOctetInterrupt*)node->drvPvt;
		if (pint!=NULL){

			if (pint->pasynUser->reason==reason){
				status=pasynManager->updateTimeStamp(pint->pasynUser);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}
				pint->callback(pint->userPvt,pint->pasynUser,pv_data,pv_nelem,ASYN_EOM_EOS);
				status=pasynManager->getTimeStamp(pint->pasynUser,&pint->pasynUser->timestamp);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}

			}
		}
		node = (interruptNode*)ellNext((ELLNODE*)node);
	}
	pasynManager->interruptEnd(asynirio_Pvt->asynOctetInterruptPvt);
	return 0;
}

int callbackInt32(irio_pvt_t* asynirio_Pvt, int reason,int addr, epicsInt32 pv_data){
	ELLLIST *clients;
	interruptNode * node;
	asynStatus status=asynSuccess;

	pasynManager->interruptStart(asynirio_Pvt->asynInt32InterruptPvt,&clients);
	node = (interruptNode*)ellFirst(clients);

	while (node!=NULL){
		asynInt32Interrupt *pint;
		pint = (asynInt32Interrupt*)node->drvPvt;
		if (pint!=NULL){

			if (addr==pint->addr && pint->pasynUser->reason==reason){
				status=pasynManager->updateTimeStamp(pint->pasynUser);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}
				pint->callback(pint->userPvt,pint->pasynUser,pv_data);
				status=pasynManager->getTimeStamp(pint->pasynUser,&pint->pasynUser->timestamp);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}

			}
		}
		node = (interruptNode*)ellNext((ELLNODE*)node);
	}
	pasynManager->interruptEnd(asynirio_Pvt->asynInt32InterruptPvt);
	return 0;
}

int callbackFloat64(irio_pvt_t* asynirio_Pvt, int reason,int addr, epicsFloat64 pv_data){
	ELLLIST *clients;
	interruptNode * node;
	asynStatus status=asynSuccess;

	pasynManager->interruptStart(asynirio_Pvt->asynFloat64InterruptPvt,&clients);
	node = (interruptNode*)ellFirst(clients);

	while (node!=NULL){
		asynFloat64Interrupt *pint;
		pint = (asynFloat64Interrupt*)node->drvPvt;
		if (pint!=NULL){

			if (addr==pint->addr && pint->pasynUser->reason==reason){
				status=pasynManager->updateTimeStamp(pint->pasynUser);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}
				pint->callback(pint->userPvt,pint->pasynUser,pv_data);
				status=pasynManager->getTimeStamp(pint->pasynUser,&pint->pasynUser->timestamp);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}

			}
		}
		node = (interruptNode*)ellNext((ELLNODE*)node);
	}
	pasynManager->interruptEnd(asynirio_Pvt->asynFloat64InterruptPvt);
	return 0;
}

int CallAIInsInt8Array(irio_pvt_t* asynirio_Pvt, int reason, int addr, epicsInt8* pv_data, int pv_nelem){
	ELLLIST *clients;
	interruptNode * node;
	asynStatus status=asynSuccess;
	pasynManager->interruptStart(asynirio_Pvt->asynInt8ArrayInterruptPvt,&clients);
	node = (interruptNode*)ellFirst(clients);

	while (node!=NULL){
		asynInt8ArrayInterrupt *pint;
		pint = (asynInt8ArrayInterrupt*)node->drvPvt;
		if (pint!=NULL){

				if (addr==pint->addr && pint->pasynUser->reason==reason){
					status=pasynManager->updateTimeStamp(pint->pasynUser);
					if(status!=asynSuccess){
						printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
					}
					pint->callback(pint->userPvt,pint->pasynUser,pv_data,pv_nelem);
					status=pasynManager->getTimeStamp(pint->pasynUser,&pint->pasynUser->timestamp);
					if(status!=asynSuccess){
						printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
					}

				}
		}
		node = (interruptNode*)ellNext((ELLNODE*)node);
	}
	pasynManager->interruptEnd(asynirio_Pvt->asynInt8ArrayInterruptPvt);
	return 0;
}

int CallAIInsFloat32Array(irio_pvt_t* asynirio_Pvt, int reason, int addr, float* pv_data, int pv_nelem){
	ELLLIST *clients;
	interruptNode * node;
	asynStatus status=asynSuccess;
	pasynManager->interruptStart(asynirio_Pvt->asynFloat32ArrayInterruptPvt,&clients);
	node = (interruptNode*)ellFirst(clients);
	while (node!=NULL){
		asynFloat32ArrayInterrupt *pint;
		pint = (asynFloat32ArrayInterrupt*)node->drvPvt;
		if (pint!=NULL){

			if (addr==pint->addr && pint->pasynUser->reason==reason){
				status=pasynManager->updateTimeStamp(pint->pasynUser);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}
				pint->callback(pint->userPvt,pint->pasynUser,pv_data,pv_nelem);
				status=pasynManager->getTimeStamp(pint->pasynUser,&pint->pasynUser->timestamp);
				if(status!=asynSuccess){
					printf("pasynUser->errorMessage=%s",pint->pasynUser->errorMessage);
				}

			}
		}
		node = (interruptNode*)ellNext((ELLNODE*)node);
	}
	pasynManager->interruptEnd(asynirio_Pvt->asynFloat32ArrayInterruptPvt);
	return 0;
}

void getChannelDataU8 (int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word=0;
	int channel=0;
	int sample=0;
	int8_t* auxBuffer= (int8_t*) inBuffer;
	int offset=0;
	for(sample=0;sample<nSamples;sample++){
		for(channel=0;channel<nChannels;channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[(u64word*8)+offset])*CVADC;
			if(offset==7){
				offset=0;
				u64word++;
			}else{
				offset++;
			}
		}
	}
}

void getChannelDataU16(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word=0;
	int channel=0;
	int sample=0;
	int16_t* auxBuffer= (int16_t*) inBuffer;
	int offset=0;
	for(sample=0;sample<nSamples;sample++){
		for(channel=0;channel<nChannels;channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[(u64word*4)+offset])*CVADC;
			if(offset==3){
				offset=0;
				u64word++;
			}else{
				offset++;
			}
		}
	}
}
void getChannelDataU32(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word=0;
	int channel=0;
	int sample=0;
	int32_t* auxBuffer= (int32_t*) inBuffer;
	int offset=0;
	for(sample=0;sample<nSamples;sample++){
		for(channel=0;channel<nChannels;channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[(u64word*2)+offset])*CVADC;
			if(offset==1){
				offset=0;
				u64word++;
			}else{
				offset++;
			}
		}
	}
}
void getChannelDataU64(int nChannels,int nSamples,uint64_t* inBuffer,float** outBuffer, double CVADC){
	int u64word=0;
	int channel=0;
	int sample=0;
	int64_t* auxBuffer= (int64_t*) inBuffer;
	for(sample=0;sample<nSamples;sample++){
		for(channel=0;channel<nChannels;channel++){
			outBuffer[channel][sample] = (float)(auxBuffer[u64word])*CVADC; //TODO: Review this casting
			u64word++;
		}
	}
}

static void irioTimeStamp(void *userPvt,epicsTimeStamp *pTimeStamp){
	epicsTimeGetCurrent(pTimeStamp);
}

