/****************************************************************************************
 * FILE INCLUSION PROTECTION
 ******************************************************************************
 */
#ifndef __RL_DATATYPES_H__
#define __RL_DATATYPES_H__


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * TYPE-DEFINE STRUCT/ENUM/UNION DEFINITIONS
 *******************************************************************************
 */
/*! \brief
 * Standard C data types typedef
 */
typedef unsigned char                   rlUInt8_t;
typedef unsigned short                  rlUInt16_t;
typedef unsigned int                    rlUInt32_t;
typedef unsigned long long              rlUInt64_t;
typedef signed long long                rlInt64_t;
typedef char                            rlInt8_t;
typedef signed char                     rlSInt8_t;
typedef signed short                    rlInt16_t;
typedef signed int                      rlInt32_t;
typedef void                            rlVoid_t;

#ifdef __cplusplus
}
#endif

#endif // __RL_DATATYPES_H__