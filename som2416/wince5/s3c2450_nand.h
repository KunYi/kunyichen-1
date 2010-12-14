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
//  Header: s3c2450_nand.h
//
//  Defines the NAND controller CPU register layout and definitions.
//
#ifndef __S3C2450_NAND_H
#define __S3C2450_NAND_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C2450_NAND_REG    
//
//  NAND Flash controller register layout. This register bank is located 
//  by the constant CPU_BASE_REG_XX_NAND in the configuration file 
//  cpu_base_reg_cfg.h.
//

typedef struct  
{
    UINT32  NFCONF;             //0x00         // configuration reg
    UINT32  NFCONT;             //0x04
    UINT8  	NFCMD;          //0x08    // command set reg
    UINT8	d0[3];
    UINT8 	NFADDR;         //0x0C    // address set reg
    UINT8	d1[3];
    UINT8  	NFDATA;         //0x10    // data reg
    UINT8	d2[3];
    UINT32  NFMECCD0;           //0x14
    UINT32  NFMECCD1;           //0x18
    UINT32  NFSECCD;            //0x1C 
    UINT32  NFSBLK;             //0x20
    UINT32  NFEBLK;             //0x24  // error correction code 2
    UINT32  NFSTAT;             //0x28  // operation status reg
    UINT32  NFECCERR0;           //0x2C   
    UINT32  NFECCERR1;           //0x30   
    UINT32  NFMECC0;            //0x34  // error correction code 0
    UINT32  NFMECC1;            //0x38  // error correction code 1
    UINT32  NFSECC;             //0x3C
    UINT32  NFMLCBITPT;         //0x40
    UINT32  NF8ECCERR0;	//0x44
    UINT32  NF8ECCERR1;	//0x48
    UINT32  NF8ECCERR2;	//0x4c
    UINT32  NF8MECC0;		//0x50
    UINT32  NF8MECC1;		//0x54
    UINT32  NF8MECC2;		//0x58
    UINT32  NF8MECC3;		//0x5c
    UINT32 NFMLC8BITPT0;	//0x60	
    UINT32 NFMLC8BITPT1;	//0x64

} S3C2450_NAND_REG, *PS3C2450_NAND_REG;    


#if __cplusplus
    }
#endif

#endif 
