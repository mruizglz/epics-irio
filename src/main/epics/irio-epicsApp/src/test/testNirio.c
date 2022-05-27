//#include "nirioDriver.h"
//
//int initTest();
//int fileAccessTest();
//
//int main(void) {
//	printf("fileAccessTest\n");
//	fileAccessTest();
//
//	printf("initTest\n");
//	initTest();
//	return 0;
//}
//
//
//int initTest(){
//	irioDrv_t* p_DrvPvt = malloc(sizeof(irioDrv_t));
//	irio_initDriver("Test0","014EDEA2","PXI-7952R","mainDAQ","V1.0", p_DrvPvt);
//	if(p_DrvPvt->ErrorNo!=success){
//		printf("TEST ERROR: Test %s\n",__func__);
//	}else{
//		printf("TEST PASSED: Test %s\n",__func__);
//	}
//
//	return p_DrvPvt->ErrorNo;
//}
//
//int fileAccessTest(){
//	irioDrv_t* p_DrvPvt = malloc(sizeof(irioDrv_t));
//	char* filePath= malloc(200);
//	strcpy(filePath,"nirio/headerfiles/NiFpga_mainDAQ.h");
//	int fd;
//
//	fd = open(filePath,O_RDONLY);
//	if(fd==-1){
//		printf("TEST ERROR: Test %s open\n",__func__);
//	}else{
//		close(fd);
//		printf("TEST PASSED: Test %s open\n",__func__);
//	}
//	return p_DrvPvt->ErrorNo;
//}
