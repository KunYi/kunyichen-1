//
// Copyright (c) Cambridge Silicon Radio.  All rights reserved.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Module Name: PSConfig.cpp
//
//
// Abstract: This module is a lightweight WinCE port of PS update code 
//
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "ubcsp.h"
#include "PSConfig.h"
#include "SerialCSR.h"

#define  LINKTIMEOUT   8000//0xffff
//4000

// Declarations
char*                        gchFileName         = "\\Windows\\PSConfig.psr";    // Persistent store keys
int                          giPDUCount          = 0;                 // Count of pdu entries
int                          giPDUIndex          = 0;                 // Index of pdu entries
BCCMDPDU                     *gPDU;                                   // Array of pdu entries to update persistent store with
int                          giResetSent         = 0;
HANDLE                       ghPort              = NULL;
static struct uBCSPQueue     uBCSP_TXQueue;
static struct uBCSPQueue     uBCSP_RXQueue;
uint16 stores_; 
int							 giStartTime         = 0;
bool                         gbLinkTimeOut       = FALSE;
bool                         gbLinked            = FALSE;
int icount=0;
// Monitor states
extern struct ubcsp_configuration ubcsp_config;            //  NB uBCSP decl changed from static

/////////////////////////////////////////////////////////////////////////
BOOL PSConfig(HANDLE hPort)
{

    int           iDelay;
    BOOL          bLastSentPacket;
    unsigned char chActivity = 0;
    unsigned char chSendBuffer[512],	chReceiveBuffer[512];
    struct        ubcsp_packet  SendPacket, ReceivePacket;

    giPDUIndex          = 0;
    giResetSent         = 0;

    DebugMessage(1,(_T("++PSConfig++ Enter\r\n")));

    // Construct pdus
    ghPort = hPort;
    FILE *stream;
    stream = fopen(gchFileName, "r");

    if(stream)
    {
    	DebugMessage(1,(_T("PSConfig: open PSConfig.psr file,  line =  %d\r\n"),__LINE__));
        // Count entries
        giPDUCount     = 0;

        giPDUCount     = ReadPSRFile(stream, false);    // Count only

        if(giPDUCount  == 0)
        {
            DebugMessage(1,(_T("++ERROR++:: PSConfig++ PSR file No data  \r\n")));
            return FALSE;                                 // No data
        }
        
        gPDU = new BCCMDPDU[giPDUCount + 1];            // Last pdu is warm reset
//	gPDU = (BCCMDPDU*)malloc(sizeof(BCCMDPDU));

        if(!gPDU)
        {
            DebugMessage(1,(_T("new BCCMDPDU[giPDUCount + 1] error  %d\r\n"),__LINE__));
        }

        fseek(stream, 0, SEEK_SET);                     // Seek start of stream
        giPDUCount = 0;                                 // Reset to zero and load pdu array

        ReadPSRFile(stream, true);                      // Adds .psr file pdu's

        giPDUCount++;                                   // Extra reset pdu

        ConfigResetPDU();                               // Construct Reset PDU

        // Close file
        fclose(stream);
    }
    else
    {
        DebugMessage(1,(_T("++ERROR++:: PSConfig++ Can not open PSR file.\r\n")));
        return FALSE;                                   // File access problem
    }

    // Setup packets payload
    SendPacket.payload          = chSendBuffer;
    ReceivePacket.payload       = chReceiveBuffer;
    ReceivePacket.length        = 512;

    // Ask ubcsp to receive a packet
    ubcsp_initialize();
    ubcsp_receive_packet (&ReceivePacket);
    uBCSP_TXQueue.CommandNumber = RECEIVED_STATE;
    uBCSP_TXQueue.NoOfOpcodes   = 0;
    uBCSP_TXQueue.NoOfRetries   = 0;

    // Start Link establishment timer
    giStartTime  = GetTickCount();

    // Reset packet counter
    bLastSentPacket = FALSE;

    // Enter loop to 'listen' to uBCSP commands
    while(TRUE)
    {
        switch(uBCSP_TXQueue.CommandNumber)
        {
            case POLL_STATE:
            {
//                if(icount >= 9)
//                    DebugMessage(1,(_T("++PSConfig++:: POLL_STATE.\r\n")));

                // Poll uBCSP for activity
                iDelay = ubcsp_poll (&chActivity);
                uBCSP_TXQueue.CommandNumber = RECEIVED_STATE;

                // Link Init check
                if((GetTickCount() - giStartTime > LINKTIMEOUT) && !gbLinked)
                    uBCSP_TXQueue.CommandNumber = EXIT_STATE;

                if(ubcsp_config.link_establishment_state > 0)
                    gbLinked = TRUE;
            }
            break;

            case RECEIVED_STATE:
            {
//                DebugMessage(1,(_T("++PSConfig++:: RECEIVED_STATE.\r\n")));
                uBCSP_TXQueue.CommandNumber = PEER_RESET_STATE;
                if(chActivity & UBCSP_PACKET_RECEIVED)
                {
                    DebugMessage(1,(_T("++PSConfig++:: UBCSP_PACKET_RECEIVED\r\n")));
                    GetReceiveState(&ReceivePacket, chReceiveBuffer);
                }
                if(chActivity & UBCSP_PACKET_SENT)
                {
                    // Acknowledge sent package
                    DebugMessage(1,(_T("++PSConfig++:: UBCSP_PACKET_SENT icount=%d\r\n"),icount));
//                    if(icount == 10)
//                       uBCSP_TXQueue.CommandNumber = EXIT_STATE;
                        
                    icount++;
                    if(!bLastSentPacket)
                    {
                        // Link is established
                        DebugMessage(1,(_T("++PSConfig++:: Link is now Established\r\n")));
                        uBCSP_TXQueue.CommandNumber = SENT_STATE;
                    }
                    bLastSentPacket = TRUE;
                }
 //               DebugMessage(1,(_T("++PSConfig++:: RECEIVED_STATE chActivity =0x%x.\r\n"),chActivity));

            }
            break;

            case SENT_STATE:
            {
//                DebugMessage(1,(_T("++PSConfig++:: SENT_STATE.\r\n")));
                // Returns the appropriate next state
                GetSentState(&SendPacket);
            }
            break;

            case PEER_RESET_STATE:
            {
                //DebugMessage(1,(_T("++PSConfig++:: PEER_RESET_STATE.\r\n")));
                if((chActivity & UBCSP_PEER_RESET) && giResetSent > 0)
                {
                    // Peer reset detected
                    DebugMessage(1, (_T("++PSConfig++:: uBCSP PEER RESET RECEIVED.\r\n")));
                    uBCSP_TXQueue.CommandNumber = EXIT_STATE;
                    break;
                }
                uBCSP_TXQueue.CommandNumber = SLEEP_STATE;
            }
            break;

            case SLEEP_STATE:
            {
//                DebugMessage(1,(_T("++PSConfig++:: SLEEP_STATE. sleep %d\r\n"),iDelay));
                if(iDelay)
                {
                    // If we need to delay, sleep for minimum period
                    Sleep(1);
                }
                uBCSP_TXQueue.CommandNumber = POLL_STATE;
            }
            break;

            case EXIT_STATE:
            {
                DebugMessage(1,(_T("++PSConfig++:: EXIT_STATE.\r\n")));
                // Exit module
                ExitMod();
                return TRUE;
            }
            break;
        }//endof switch
    }//endof while
	return TRUE;
}

int ReadPSRFile(FILE* ist, bool bBuild)
{
	char linebuf[LineDatabufSize];
	uint16 data[LineDatabufSize];
	uint16 *pdata;
	char *p, *end;
	int key;
	bool query_mode = false;

	while (fgets(linebuf, LineDatabufSize, ist))
	{
		// Does it start with an &?
		p = linebuf;
		linebuf[LineDatabufSize - 1] = '\0';
		end = &linebuf[strlen(linebuf)];
		while ((p < end) && isspace(*p))
			p++;
		if (p == end)
			continue;
		if (*p == '?')
			query_mode = true;
		else if (*p != '&')
			continue; // Not a valid line.
		p++;

		// Are there digits next?
		key = 0;
		while ((p < end) && isxdigit(*p))
		{
			key = (key * 16) + Hex2Int(*p);
			p++;
		}
		if (key == 0)
			continue;
		while ((p < end) && isspace(*p))
			p++;
		if (p == end)
		{   // No operation?  
			continue;  // Next line please.
		}

		// Read the operation
		if (*p != '=')
			continue;  // Next line please.

		if (p < end)
			p++;
    
		// Key counter
		giPDUCount++;

		if(bBuild)   //  If building pdu array read data and set key
		{
			// Read the data
			pdata = data;
			for (;;)
			{
			  while ((p < end) && isspace(*p))
				  p++;
			  if (p == end)
				  break;
			  if (*p == '\"')
			  {
				  p++;
				  while (p < end)
				  {
					  *pdata++ = *p++;
				  }
				  break;
			  }
			  else
			  {
				  if (p >= end || (!isxdigit(*p)))
					  break;
				  *pdata = 0;
				  while ((p < end) && isxdigit(*p))
				  {
					  *pdata = (*pdata * 16) + Hex2Int(*p);
					  p++;
				  }
				  pdata++;
			  }
			}

			// Create pdu
			ConfigPSRPDU(key, data, pdata-data);
		}
	}
	return giPDUCount;
}

void ConfigPSRPDU(const pskey key, const uint16 *data, uint16 len )
{
    // Construct data unit
    //BCCMDPDU *p    = (BCCMDPDU *)malloc(BCCMDPDU_MAXBUFSIZ_PC);
    BCCMDPDU *p = (BCCMDPDU*)malloc(sizeof(BCCMDPDU));
    
    if(!p)
    {
        DebugMessage(1,(_T("malloc error %d\r\n"),__LINE__));
    }
    
    memset(p, 0, BCCMDPDU_MAXBUFSIZ_PC);
    p->type        = BCCMDPDU_SETREQ;
    if (len > MAX_PS_VALUE_LEN_XAP)
		len = (uint16)MAX_PS_VALUE_LEN_XAP;
    p->pdulen      = BCCMDPDUMIN + len;    // XAP_PDU_LEN(len);
    p->seqno       = giPDUCount;
    p->varid       = BCCMDVARID_PS;                         // Persistent store
    p->status      = BCCMDPDU_STAT_OK;
    p->d.ps.id     = key;
    p->d.ps.len    = len;
    p->d.ps.stores = stores_;

    DebugMessage(1,(_T("Debug Data p =  %x\r\n"),p));

    // Assign pdu to global array
    gPDU[giPDUCount-1] = *p;                                    // *** Confirm doing this doesn't clobber anything
    //memcpy(&(gPDU[giPDUCount-1]), p, BCCMDPDU_MAXBUFSIZ_PC);
    
    memcpy(gPDU[giPDUCount-1].d.ps.psmem, data, PC_SIZE(len));  // *** ie.  pointer to something already malloc'd ?

    // Add by henry, we need to free the memory
    if(p)
        free(p);
 }

void ConfigResetPDU()
{
	// Write reset pdu
	//BCCMDPDU *p          = (BCCMDPDU *)malloc(BCCMDPDU_MAXBUFSIZ_PC);
    BCCMDPDU *p = (BCCMDPDU*)malloc(sizeof(BCCMDPDU));
    memset(p, 0, BCCMDPDU_MAXBUFSIZ_PC);
	//memset(p, 0, 18);
	p->type              = BCCMDPDU_SETREQ;
    p->seqno = giPDUCount;  // add by henry.
	p->pdulen            = BCCMDPDUMIN + 1;
//        p->pdulen            = BCCMDPDUMIN;
	p->varid             = BCCMDVARID_WARM_RESET;
	p->status            = BCCMDPDU_STAT_OK;

    // add by henry
    p->d.ps.id     = 0x00;
    p->d.ps.len    = 0x00;
    p->d.ps.stores = stores_;
            
	gPDU[giPDUCount-1]   = *p;

    gPDU[giPDUCount-1].d.ps.psmem[0] = 0x00;
    DebugMessage(1,(_T("giPDUCount = %d\r\n"), giPDUCount));

    // free the memory
    if(p)
        free(p);

	return;
}

void GetReceiveState(struct ubcsp_packet *rx_packet, unsigned char *rx_buffer)
{
 //     DebugMessage(1,(_T("++GetReceiveState++:: Enter =.\r\n")));

	// Only print out BCCMD information
	if(rx_packet->channel == BCCMD)
	{
		switch(rx_buffer[0] & 0xFF)
		{ 
			case 0x01:
			{
                DebugMessage(1,(_T("++GetReceiveState++:: BCCMD COMMAND RECEIVED =.\r\n")));
 				PrintData(rx_buffer, rx_packet->length);

				// Check to see if Command Complete is the PS from TX Queue
				// If it is allow next BCCMD Message
				if(((rx_buffer[6] & 0xFF) == uBCSP_TXQueue.Opcode[0])
						&& ((rx_buffer[7] & 0xFF) == uBCSP_TXQueue.Opcode[1]))
				{
                    DebugMessage(1,(_T("++GetReceiveState++:: COMMAND COMPLETE =.\r\n")));
					uBCSP_TXQueue.CommandNumber = SENT_STATE;
					uBCSP_TXQueue.NoOfOpcodes = 0;
				}
				else
				{
                    DebugMessage(1,(_T("++GetReceiveState _WARNING++:: COMMAND DOES NOT MATCH QUEUE.\r\n")));
				}
			}
			break;

			case 0x05:
			{
				// Output to Debug any disconnection attempts
                DebugMessage(1,(_T("++GetReceiveState++:: DISCONNECTION COMPLETE =.\r\n")));
				PrintData(rx_buffer, rx_packet->length);
			}
			break;
							
			case 0x10:
			{
				// Output to Debug any detected error messages
                DebugMessage(1,(_T("++GetReceiveState_WARNING++:: HARDWARE ERROR =.\r\n")));
				PrintData(rx_buffer, rx_packet->length);
			}
			break;

			case 0x0f:
			{
                DebugMessage(1,(_T("COMMAND STATUS.\r\n")));
				PrintData(rx_buffer, rx_packet->length);
			}
			break;  
      
			default:
			{
				// Output to Debug any other message
//				OutputDebugString("\nUNKNOWN COMMAND: ");
//				PrintData(rx_buffer, rx_packet->length);
			}
		}
	}
	else
	{
		// Reset received
   		if(rx_packet->channel == HCI_COMMAND && (rx_buffer[0] & 0xFF) == 0x0f )
		{
			if(giPDUIndex == giPDUCount)
			{
                DebugMessage(1,(_T("++GetReceiveState++::...Reset complete....\r\n")));
				uBCSP_TXQueue.CommandNumber = EXIT_STATE;
			}
		}
		else
		{
			// Check any data received on other channels
//			OutputDebugString("\n|_WARNING: DATA RECEIVED ON NON BCCMD CHANNEL:");
//			sprintf(sTemp, "\n|_Packet Received, Channel = %02x ", rx_packet->channel);
//			OutputDebugString(sTemp);
//			OutputDebugString("\n|_Data: ");
//			PrintData(rx_buffer, rx_packet->length);
		}
	}

	// Allow another packet to be received up to 512 bytes long
	rx_packet->length = 512;
	memset(rx_packet->payload, 0, 512);

	// Setup the receive again 
	ubcsp_receive_packet(rx_packet);
}

void GetSentState(struct ubcsp_packet *sd_packet)
{
	static uint8 i, j;
	int iLength;
	//char sTemp[24];

	// Write data
	DebugMessage(1,(_T("\r\nGetSentState: enter \r\n "))); 

	// Construct uBCSP packet header from PDU - ensures correct byte ordering
	// Header
	DebugMessage(1,(_T("GetSentState line =  %d, giPDUIndex = %d\r\n"),__LINE__, giPDUIndex));
	sd_packet->payload[0] = (uint8)gPDU[giPDUIndex].type;
	DebugMessage(1,(_T("GetSentState line =  %d\r\n"),__LINE__));
	sd_packet->payload[1] = (uint8)(gPDU[giPDUIndex].type >> 8);
	sd_packet->payload[2] = (uint8)gPDU[giPDUIndex].pdulen;
	sd_packet->payload[3] = (uint8)(gPDU[giPDUIndex].pdulen >> 8);
	sd_packet->payload[4] = (uint8)gPDU[giPDUIndex].seqno;
	sd_packet->payload[5] = (uint8)(gPDU[giPDUIndex].seqno >> 8);
	sd_packet->payload[6] = (uint8)gPDU[giPDUIndex].varid;
	sd_packet->payload[7] = (uint8)(gPDU[giPDUIndex].varid >> 8);
	sd_packet->payload[8] = (uint8)gPDU[giPDUIndex].status;
	sd_packet->payload[9] = (uint8)(gPDU[giPDUIndex].status >> 8);

	// PS specific
	sd_packet->payload[10] = (uint8)gPDU[giPDUIndex].d.ps.id;
	sd_packet->payload[11] = (uint8)(gPDU[giPDUIndex].d.ps.id >> 8);
	sd_packet->payload[12] = (uint8)gPDU[giPDUIndex].d.ps.len;
	sd_packet->payload[13] = (uint8)(gPDU[giPDUIndex].d.ps.len >> 8);
	sd_packet->payload[14] = (uint8)gPDU[giPDUIndex].d.ps.stores;
	sd_packet->payload[15] = (uint8)(gPDU[giPDUIndex].d.ps.stores >> 8);

	// Data
	j = 16;
	for(i = 0; i < gPDU[giPDUIndex].d.ps.len; i++)
	{
		sd_packet->payload[i + j]     = (uint8)gPDU[giPDUIndex].d.ps.psmem[i];
		sd_packet->payload[i + j + 1] = (uint8)(gPDU[giPDUIndex].d.ps.psmem[i] >> 8);
		j++;
	}

	// Packet length
	iLength = 15 + (gPDU[giPDUIndex].d.ps.len * sizeof(uint16)) + 1;

    // If we are sending warm_reset packet    
    DebugMessage(1,(_T("\n giPDUCount = %d \r\n"),giPDUCount));      
    if(giPDUCount -1 == giPDUIndex )
    {
        sd_packet->payload[16] = (uint8) 0x00;
        sd_packet->payload[17] = (uint8) 0x00;
        iLength = 18;
    }
        
	sd_packet->length = iLength;
       
	// Set packet defaults
	sd_packet->channel  = BCCMD;
	sd_packet->reliable = RELIABLE;
	sd_packet->use_crc  = TRUE;

	// Configure uBCSP queue handler
	uBCSP_TXQueue.NoOfRetries   = 0;
	uBCSP_TXQueue.Opcode[0]     = sd_packet->payload[6]; 
	uBCSP_TXQueue.Opcode[1]     = sd_packet->payload[7]; 
	uBCSP_TXQueue.CommandNumber = RECEIVED_STATE;
	uBCSP_TXQueue.NoOfOpcodes   = 1;

	DebugMessage(1,(_T("\n BCCMD COMMAND SENT  CMDID   = 0x%x \r\n"),sd_packet->payload[6])); 

	// Send the Transmit Packet
	ubcsp_send_packet(sd_packet);
	giPDUIndex++;                              // Increase pdu index
	if(giPDUIndex == giPDUCount)               // Temp
		giResetSent = 1;

    // Some Debug Information
    
/*	for (i = 0; i < sd_packet->length; i++)
	{
		sprintf(sTemp, "%02x ", sd_packet->payload[i]);
               DebugMessage(1,(_T("++GetSentState++ %02x \r\n "),(wchar_t*)sTemp));
	}
	DebugMessage(1,(_T("\n")));*/
}

void ExitMod()
{
	// Delete global pdu array
	delete [] gPDU;

	// Exiting
	DebugMessage(1,(_T("EXITING\r\n")));

	return;
}

//////////////////////////////////////////////////////////////////////////
#define PSCONFIG_DBG_OUTPUT  0

// Called externally by uBCSP
//
void put_uart (uint8 ch)
{
    unsigned long lWritten = 0;
    
  #if PSCONFIG_DBG_OUTPUT
    DebugMessage(1,(_T(">%2x "),ch));
    if(SLIP_FRAME == ch)
        DebugMessage(1,(_T("\r\n")));
  #endif        
//       DebugMessage(1,(_T("++Put to COM =%x.\r\n"),ch));
	lWritten = CSR_Write(ghPort, &ch, 1);
}

// Called externally by uBCSP
uint8 get_uart (uint8 *ch)
{
	static uint8 fifo_buffer[256];
	static uint8 read_index = 0, write_index = 0;
	unsigned long lDummy = 0, lRead = 0;
    static uint8 dLen=254;
    //int i;
	// Read the UART in large chunks
	if (read_index == write_index)
	{
		read_index = 0;
		write_index = 0;

		// Read 254 octets
		//unsigned long int lRead = CSR_Read(ghPort, fifo_buffer, dLen, &lDummy);
        uint lRead = CSR_Read(ghPort, fifo_buffer, dLen, &lDummy);
        
        if(lRead)
            // DebugMessage(1,(_T("Read number = %x\r\n",lRead)));
            DebugMessage(1,(_T("\r\nRead number =0x%x\r\n"),lRead));
		if(lRead > 0)
		{
			write_index += (uint8)lRead;

			// Read the first octet returned
			*ch = fifo_buffer[read_index ++];
          #if PSCONFIG_DBG_OUTPUT
            DebugMessage(1,(_T("<%2x "),*ch));
            if(SLIP_FRAME == *ch)
                DebugMessage(1,(_T("\r\n")));
          #endif
			return 1;
		}
	}
	else
	{
		// Read the next octet in the fifo_buffer
		*ch = fifo_buffer[read_index ++];	
      #if PSCONFIG_DBG_OUTPUT
        DebugMessage(1,(_T("<%2x "),*ch));
        if(SLIP_FRAME == *ch)
            DebugMessage(1,(_T("\r\n")));
      #endif          
		return 1;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//  Miscellaneous functions
//
void PrintData(unsigned char *data, int length)
{
	static int i;
	//char sTemp[48];
	int iLen;

	// Size to max packet length
	iLen = 15 + gPDU[giPDUIndex-1].d.ps.len * sizeof(uint16) + 1;
/*
	for (i = 0; i < iLen; i++)
	{
		sprintf(sTemp, "%02x ", data[i] & 0xFF);
              DebugMessage(1,(_T("++PrintData++ %02x \r\n  "),(wchar_t*)sTemp));

	}
	DebugMessage(1,(_T("\n")));
*/	
}

static int Hex2Int(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}

int get_icount()
{
    return icount;
}

