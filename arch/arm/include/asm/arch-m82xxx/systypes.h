/*
 * Copyright(c) 2007-2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution 
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Intel Corporation
 */

#ifndef __SYSTYPES_H__
#define __SYSTYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TRUE
#define TRUE    1
#endif /*TRUE*/

#ifndef FALSE
#define FALSE   0
#endif /*FALSE*/

#ifndef NULL
#define NULL   (void*)0
#endif /*NULL*/


/************************************************************************/
/*     UINT64, UINT32, UINT16 and UINT8 definition                      */
/************************************************************************/
#ifndef _UINT64_
#define _UINT64_
typedef unsigned long long UINT64, *PUINT64;
#endif /*_UINT64_*/

#ifndef _UINT32_
#define _UINT32_
typedef unsigned int UINT32, *PUINT32;
#endif /*_UINT32_*/

#ifndef _UINT16_
#define _UINT16_
typedef unsigned short UINT16, *PUINT16;
#endif /*_UINT16_*/

#ifndef _UINT8_
#define _UINT8_
typedef unsigned char UINT8, *PUINT8;
#endif /*_UINT8_*/

#ifndef _VUINT64_
#define _VUINT64_
typedef volatile unsigned long long VUINT64, *PVUINT64;
#endif /*_UINT64_*/

#ifndef _VUINT32_
#define _VUINT32_
typedef volatile unsigned int VUINT32, *PVUINT32;
#endif /*_UINT32_*/

#ifndef _VUINT16_
#define _VUINT16_
typedef volatile unsigned short VUINT16, *PVUINT16;
#endif /*_UINT16_*/

#ifndef _VUINT8_
#define _VUINT8_
typedef volatile unsigned char VUINT8, *PVUINT8;
#endif /*_UINT8_*/

/************************************************************************/
/*     SINT64, SINT32, SINT16 and SINT8 definition                      */
/************************************************************************/
#ifndef _SINT64_
#define _SINT64_
typedef long long SINT64, *PSINT64;
#endif /*_SINT64_*/

#ifndef _SINT32_
#define _SINT32_
typedef int SINT32, *PSINT32;
#endif /*_SINT32_*/

#ifndef _SINT16_
#define _SINT16_
typedef short SINT16, *PSINT16;
#endif /*_SINT16_*/

#ifndef _SINT8_
#define _SINT8_
typedef char SINT8, *PSINT8;
#endif /*_SINT8_*/

#ifndef _PVOID_
#define _PVOID_
typedef void *PVOID;
#endif /*_PVOID_*/

#ifndef _BOOL_
#define _BOOL_
typedef UINT8 BOOL;
#endif /*_BOOL_*/

#endif /*__SYSTYPES_H__ */

