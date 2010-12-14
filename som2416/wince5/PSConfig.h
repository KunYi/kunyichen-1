////////////////////////////////////////////////////////////////////
// PSConfig.h
//
// Copyright (c) Cambridge Silicon Radio.  All rights reserved.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Header for lightweight WinCE port of PS update code 
//

// Standard constants
#undef  FALSE
#undef  TRUE
#undef  NULL
#define FALSE   0
#define TRUE    1
#define NULL    0

// From BCCMDPDU.H
#define	BCCMDVARID_SPECIAL_START (4096)
#define BCCMDVARID_WRITEONLY	(0x4000)
#define BCCMDVARID_READWRITE	(0x6000)
#define	BCCMDVARID_COMMANDS_START (0)
#define	BCCMDVARID_WARM_RESET	(BCCMDVARID_COMMANDS_START + BCCMDVARID_WRITEONLY+2)
#define	BCCMDVARID_PS (BCCMDVARID_SPECIAL_START + BCCMDVARID_READWRITE + 3)

// From BCCMD_TRANS.H
#define PC_SIZE(X) ((X) * sizeof(uint16))
#define BCCMDPDU_MAXBUFSIZ_XAP (94)
#define BCCMDPDU_MAXBUFSIZ_PC (PC_SIZE(BCCMDPDU_MAXBUFSIZ_XAP))
#define	BCCMDPDU_SETREQ		(2)
#define	BCCMDPDU_STAT_OK	(0)
#define XAP_SIZE(X) ((X) / sizeof(uint16))
#define BCCMDPDU_MAXBUFSIZ_XAP (94)
#define XAP_SIZEOF(X) (XAP_SIZE(sizeof(X)))
#define XAP_PS_LEN(X)  (X - XAP_SIZEOF(BCCMDPDU) + 1)
#define MAX_PS_VALUE_LEN_XAP (XAP_PS_LEN(BCCMDPDU_MAXBUFSIZ_XAP))

// Various structures
// - Adapted From BCCMDPDU.H
#define	BCCMDPDUMIN 8      // Min size of pdu

// Define PDU with static memory
// - Enables array handling
#pragma pack(push, 2)
typedef struct {
	uint16		id;		
	uint16		len;		
	uint16		stores;		
	uint16		psmem[BCCMDPDU_MAXBUFSIZ_PC];        //  Note PDU data with static memory
	} BCCMDPDU_PS;

typedef struct {
	uint16		type;
	uint16		pdulen;
	uint16		seqno;
	uint16		varid;
	uint16		status;
  union
  {
    BCCMDPDU_PS	ps;	
  }d;
} BCCMDPDU;
#pragma pack(pop)

enum channelreliability {
  UNRELIABLE,
    RELIABLE
};

// Adapted state indicator
enum ubcspstates
{
  POLL_STATE,
  SENT_STATE,
  RECEIVED_STATE,
  PEER_RESET_STATE,
  SLEEP_STATE,
  EXIT_STATE
};

// From BCCMD_HCI.H
static const size_t  LineDatabufSize = 2000;
typedef uint16 pskey;

enum BCSPreliablechannels 
{
	ACKKNOWLEDGE, 
	UNUSED_RELIABLE_CHANNEL_1, 
	BCCMD, 
	HQ,
	DEVICE_MGT,
	HCI_COMMAND,
	HCI_ACL_RELIABLE,
	HCI_SCO_RELIABLE,
	L2CAP,
	RFCOMM,
	SDD,
	RESERVED_RELIABLE_CHANNEL_11, 
	DFU,
	VM,
	UNUSED_RELIABLE_CHANNEL_14, 
	RESERVED_RELIABLE_CHANNEL_15 
};

// Command Queue
//  This is based on HCIQueue
struct uBCSPQueue
{
	uint8 NoOfOpcodes;
	uint8 CommandNumber;
	uint8 Opcode[2];
	uint8 NoOfRetries;
};

// Forward declarations   
BOOL  PSConfig(HANDLE hPort);
void  put_uart (uint8 ch);
uint8 get_uart (uint8 *ch);
int   ReadPSRFile(FILE *ist, bool bBuild);
void  ConfigPSRPDU(const pskey key, const uint16 *data, uint16 len);
void  GetReceiveState(struct ubcsp_packet *rx_packet, unsigned char *rx_buffer);
void  GetSentState(struct ubcsp_packet *sd_packet);
void  ExitMod();
void  ConfigResetPDU();

// Misc functions
static int Hex2Int(char c);
void PrintData(unsigned char *data, int length);
int get_icount();