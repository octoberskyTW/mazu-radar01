/**
 *   @file  mmw_mss.h
 *
 *   @brief
 *      This is the main header file for the Millimeter Wave Demo
 *
 *  \par
 *  NOTE:
 *      (C) Copyright 2019 Texas Instruments, Inc.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __MMW_MSS_H__
#define __MMW_MSS_H__
#include "rl_sensor.h"
/*!
 * @brief
 * Structure holds temperature information from Radar front end.
 *
 * @details
 *  The structure holds temperature stats information. 
 */
typedef struct MmwDemo_temperatureStats_t
{

    /*! @brief   retVal from API rlRfTempData_t - can be used to know 
                 if values in temperatureReport are valid */
    int32_t        tempReportValid;

    /*! @brief   detailed temperature report - snapshot taken just 
                 before shipping data over UART */
    rlRfTempData_t temperatureReport;

} MmwDemo_temperatureStats;

#ifdef __cplusplus
}
#endif

#endif /* __MMW_MSS_H__ */

