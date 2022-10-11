/*
 * Generated with the FPGA Interface C API Generator 17.0.0
 * for NI-RIO 17.0.0 or later.
 */

#ifndef __NiFpga_FlexRIOMod5761_7966v1_1_h__
#define __NiFpga_FlexRIOMod5761_7966v1_1_h__

#ifndef NiFpga_Version
   #define NiFpga_Version 1700
#endif

#include "NiFpga.h"

/**
 * The filename of the FPGA bitfile.
 *
 * This is a #define to allow for string literal concatenation. For example:
 *
 *    static const char* const Bitfile = "C:\\" NiFpga_FlexRIOMod5761_7966v1_1_Bitfile;
 */
#define NiFpga_FlexRIOMod5761_7966v1_1_Bitfile "NiFpga_FlexRIOMod5761_7966v1_1.lvbitx"

/**
 * The signature of the FPGA bitfile.
 */
static const char* const NiFpga_FlexRIOMod5761_7966v1_1_Signature = "DF0CEC5FAB9D31B1B900DC13C3F8AB60";

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_InitDone = 0xEE,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_RIOAdapterCorrect = 0xF2,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_auxDI0 = 0x7A,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_auxDI1 = 0x76,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_auxDI2 = 0x72,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_auxDI3 = 0x6E,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_auxDI4 = 0x6A,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool_auxDI5 = 0x66,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorBool;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU8_DevProfile = 0xE2,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU8_DevQualityStatus = 0x5A,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU8_Platform = 0xFE,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU8_SGNo = 0x62,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU8;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI16_DevTemp = 0x56,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI16;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU16_DMATtoHOSTOverflows = 0x4A,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU16;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI0 = 0xA8,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI1 = 0xA4,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI10 = 0x18,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI2 = 0xA0,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI3 = 0x9C,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI4 = 0x98,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI5 = 0x94,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32_auxAI9 = 0x14,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI32;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU32_ExpectedIOModuleID = 0xF8,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU32_Fref = 0xE8,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU32_InsertedIOModuleID = 0xF4,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU32_SGFref0 = 0xC4,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU32_SGFref1 = 0x5C,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorU32;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI64_aux64AI0 = 0xC8,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorI64;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_AOEnable0 = 0x3A,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_AOEnable1 = 0x1E,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_DAQStartStop = 0x4E,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_DMATtoHOSTEnable0 = 0x42,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_DebugMode = 0x52,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_auxDO0 = 0x92,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_auxDO1 = 0x8E,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_auxDO2 = 0x8A,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_auxDO3 = 0x7E,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_auxDO4 = 0x82,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlBool_auxDO5 = 0x86,
} NiFpga_FlexRIOMod5761_7966v1_1_ControlBool;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU8_SGSignalType0 = 0x3E,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU8_SGSignalType1 = 0x26,
} NiFpga_FlexRIOMod5761_7966v1_1_ControlU8;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU16_DMATtoHOSTSamplingRate0 = 0x46,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU16_SGAmp0 = 0x12,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU16_SGAmp1 = 0x36,
} NiFpga_FlexRIOMod5761_7966v1_1_ControlU16;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_AO0 = 0x4,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_AO1 = 0x28,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_auxAO0 = 0xC0,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_auxAO1 = 0xBC,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_auxAO2 = 0xB8,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_auxAO3 = 0xB4,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_auxAO4 = 0xB0,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI32_auxAO5 = 0xAC,
} NiFpga_FlexRIOMod5761_7966v1_1_ControlI32;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU32_SGFreq0 = 0xC,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU32_SGFreq1 = 0x30,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU32_SGPhase0 = 0x8,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU32_SGPhase1 = 0x2C,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU32_SGUpdateRate0 = 0x0,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU32_SGUpdateRate1 = 0x20,
   NiFpga_FlexRIOMod5761_7966v1_1_ControlU32_TabControl = 0x104,
} NiFpga_FlexRIOMod5761_7966v1_1_ControlU32;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_ControlI64_aux64AO0 = 0xCC,
} NiFpga_FlexRIOMod5761_7966v1_1_ControlI64;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8_DMATtoHOSTFrameType = 0xD6,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8_DMATtoHOSTSampleSize = 0xDA,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8_FPGAVIversion = 0xE6,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8Size_DMATtoHOSTFrameType = 1,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8Size_DMATtoHOSTSampleSize = 1,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8Size_FPGAVIversion = 2,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU8Size;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU16_DMATtoHOSTBlockNWords = 0xDE,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU16_DMATtoHOSTNCh = 0xD2,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU16;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU16Size_DMATtoHOSTBlockNWords = 1,
   NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU16Size_DMATtoHOSTNCh = 1,
} NiFpga_FlexRIOMod5761_7966v1_1_IndicatorArrayU16Size;

typedef enum
{
   NiFpga_FlexRIOMod5761_7966v1_1_TargetToHostFifoU64_DMATtoHOST0 = 0,
} NiFpga_FlexRIOMod5761_7966v1_1_TargetToHostFifoU64;

#endif
