#ifndef __VITAL_SIGNS_H__
#define __VITAL_SIGNS_H__
#include <stdint.h>
typedef struct VitalSignsDemo_OutputStats_t
{
    uint16_t rangeBinIndexMax;     // 1
    uint16_t rangeBinIndexPhase;   // 1
    float maxVal;                  //2
    uint32_t processingCyclesOut;  //3
    uint16_t rangeBinStartIndex;   //4
    uint16_t rangeBinEndIndex;     //4
    float unwrapPhasePeak_mm;           // 5
    float outputFilterBreathOut;        // 6
    float outputFilterHeartOut;         // 7
    float heartRateEst_FFT;             // 8
    float heartRateEst_FFT_4Hz;         // 9
    float heartRateEst_xCorr;           // 10
    float heartRateEst_peakCount_filtered;  // 11
    float breathingRateEst_FFT;          // 12
    float breathingRateEst_xCorr;        // 13
    float breathingRateEst_peakCount;    // 14
    float confidenceMetricBreathOut;        // 15
    float confidenceMetricBreathOut_xCorr;  // 16
    float confidenceMetricHeartOut;         // 17
    float confidenceMetricHeartOut_4Hz;     // 18
    float confidenceMetricHeartOut_xCorr;   // 19
    float sumEnergyBreathWfm;               // 20
    float sumEnergyHeartWfm;                // 21
    float motionDetectedFlag;               // 22
    float breathingRateEst_harmonicEnergy;  // 23
    float heartRateEst_harmonicEnergy;      // 24
    float reserved7;  //25
    float reserved8;  //26
    float reserved9;  //27
    float reserved10;  //28
    float reserved11;  //29
    float reserved12;  //30
    float reserved13;  //31
    float reserved14;  //32
} VitalSignsDemo_OutputStats;

#endif // __VITAL_SIGNS_H__