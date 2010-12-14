//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Header:  s3c2450_cam.h
//
//  Defines the Camera Interface control registers and definitions.
//
#ifndef __S3C2450_CAM_H
#define __S3C2450_CAM_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef struct {
	UINT32 		CISRCFMT;		// 00
	UINT32 		CIWDOFST;		// 04
	UINT32 		CIGCTRL;		// 08
	UINT32 		CIFCTRL1;			// 0c
	UINT32 		CIFCTRL2;			// 10
	UINT32 		CIDOWSFT2;			// 14	
	UINT32 		CICOYSA1;		// 18
	UINT32 		CICOYSA2;		// 1c
	UINT32 		CICOYSA3;		// 20
	UINT32 		CICOYSA4;		// 24
	UINT32 		CICOCBSA1;		// 28
	UINT32 		CICOCBSA2;		// 2c
	UINT32 		CICOCBSA3;		// 30
	UINT32 		CICOCBSA4;		// 34
	UINT32 		CICOCRSA1;		// 38
	UINT32 		CICOCRSA2;		// 3c
	UINT32 		CICOCRSA3;		// 40
	UINT32 		CICOCRSA4;		// 44
	UINT32 		CICOTRGFMT;		// 48
	UINT32 		CICOCTRL;		// 4c
	UINT32 		CICOSCPRERATIO;	// 50
	UINT32 		CICOSCPREDST;	// 54
	UINT32 		CICOSCCTRL;		// 58
	UINT32 		CICOTAREA;		// 5c
	UINT32 		PAD4;			// 60
	UINT32 		CICOSTATUS;		// 64
	UINT32 		PAD5;			// 68
	UINT32 		CIPRCLRSA1;		// 6c
	UINT32 		CIPRCLRSA2;		// 70
	UINT32 		CIPRCLRSA3;		// 74
	UINT32 		CIPRCLRSA4;		// 78
	UINT32 		CIPRTRGFMT;		// 7c
	UINT32 		CIPRCTRL;		// 80
	UINT32 		CIPRSCPRERATIO;	// 84
	UINT32 		CIPRSCPREDST;	// 88
	UINT32 		CIPRSCCTRL;		// 8c
	UINT32 		CIPRTAREA;		// 90
	UINT32 		PAD6;			// 94
	UINT32 		CIPRSTATUS;		// 98
	UINT32 		PAD7;			// 9c
	UINT32 		CIIMGCPT;		// a0
	UINT32 		CICOCPTSEQ;		// a4
	UINT32 		CICOSCOS;		// a8
	UINT32 		CIPRSCOS;		// ac
	UINT32 		CIIMGEFF;		// b0
	UINT32 		CIMSYSA;		// b4
	UINT32 		CIMSCBSA;		// b8
	UINT32 		CIMSCRSA;		// bc
	UINT32 		CIMSYEND;		// c0
	UINT32 		CIMSCBEND;		// c4
	UINT32 		CIMSCREND;		// c8
	UINT32 		CIMSYOFF;		// cc
	UINT32 		CIMSCBOFF;		// d0
	UINT32 		CIMSCROFF;		// d4
	UINT32 		CIMSWIDTH;		// d8
	UINT32 		CIMSCTRL;		// dc

	
}S3C2450_CAM_REG,*PS3C2450_CAM_REG;
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif // __S3C2450_CAM_H
