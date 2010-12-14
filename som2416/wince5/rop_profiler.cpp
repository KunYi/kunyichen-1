#include "precomp.h"
#include "rop_profiler.h"



RopProfiler RopProfiler::m_pInstance;

RopProfiler::RopProfiler() : printinterval(DEFAULT_COUNT_INTERVAL)
{
	Accel_rop = new RopCounter;
	Accel_rop->SetID(ID_ACCEL_ROP_COUNTER);
	Emul_rop = new RopCounter;
	Emul_rop->SetID(ID_EMUL_ROP_COUNTER);
}
RopProfiler::~RopProfiler()
{
	delete Accel_rop;
	delete Emul_rop;
}


bool RopProfiler::CheckCount(RopCounter *targetcounter)
{
	if( ((Accel_rop->GetTotalCount() + Emul_rop->GetTotalCount()) % ACCUML_COUNT_INTERVAL) == 0)
	{
		Print();
	}
	if( (targetcounter->GetTotalCount() % printinterval) == 0 )
	{
		targetcounter->Print();
	}
	if( targetcounter->GetTotalCount() == MAX_COUNT)
	{
//		printf("MAX_COUNT reach.. All Counter Auto Reset\n");
		Print(false);
		Print(true);

		Accel_rop->Restart();
		Emul_rop->Restart();

	}
	return true;
}

bool RopProfiler::LogLine(bool IsAccel)
{
	if(IsAccel)
	{
		if(Accel_rop->hit_LINE())
		{
			if( (Accel_rop->GetLineCount() % printinterval) == 0 )
			{
//				printf("Accelerated line_count : %d\n",	Accel_rop->GetLineCount());
			}
		}
	}
	else
	{
		if(Emul_rop->hit_LINE())
		{
			if( (Emul_rop->GetLineCount() % printinterval) == 0 )
			{
//				printf("Emulated line_count : %d\n",	Accel_rop->GetLineCount());
			}
		}
	}

	return true;
}


bool RopProfiler::Log(bool IsAccel, unsigned int MSROP)
{
	if(IsAccel)
	{
		if(Accel_rop->hit_ROP(MSROP))
		{
			return CheckCount(Accel_rop);
		}
	}
	else
	{
		if(Emul_rop->hit_ROP(MSROP))
		{
			return CheckCount(Emul_rop);
		}
	}
	return false;
}

bool RopProfiler::Print(bool IsAccel)
{
	if(IsAccel)
	{
		Accel_rop->Print();
	}
	else
	{
		Emul_rop->Print();
	}
	return true;
}

float RopProfiler::ROPPercentage(bool IsAccel, unsigned int MSRopCode)
{
	if((Emul_rop->GetRopCount(MSRopCode) + Accel_rop->GetRopCount(MSRopCode)) == 0) return 0;	
	if(IsAccel)
	{
		return (float)Accel_rop->GetRopCount(MSRopCode)*100/(Emul_rop->GetRopCount(MSRopCode) + Accel_rop->GetRopCount(MSRopCode));
	}
	else
	{
		return (float)Emul_rop->GetRopCount(MSRopCode)*100/(Emul_rop->GetRopCount(MSRopCode) + Accel_rop->GetRopCount(MSRopCode));
	}

}
bool RopProfiler::Print()
{
	printf("Accumulated ROP  (Emul:Accel, Emul%%:Accel%%)\n");
	printf("total(%d:%d,%3.1lf:%3.1lf), BLACKNESS(%d:%d,%3.1lf:%3.1lf), WHITENESS(%d:%d,%3.1lf:%3.1lf), DSTINVERT(%d:%d,%3.1lf:%3.1lf), PATINVERT(%d:%d,%3.1lf:%3.1lf), PATCOPYTEXT(%d:%d,%3.1lf:%3.1lf), PATCOPY(%d:%d,%3.1lf:%3.1lf), SRCINVERT(%d:%d,%3.1lf:%3.1lf), SRCCOPY(%d:%d,%3.1lf,%3.1lf), SRCAND(%d:%d,%3.1lf:%3.1lf), SRCPAINT(%d:%d,%3.1lf:%3.1lf), B8B8(%d:%d,%3.1lf:%3.1lf), OTHER(%d:%d,%3.1lf:%3.1lf)\n",
			 Emul_rop->GetTotalCount(), Accel_rop->GetTotalCount(),
			 (float)(Emul_rop->GetTotalCount()*100)/(Emul_rop->GetTotalCount() + Accel_rop->GetTotalCount()),
			 (float)(Accel_rop->GetTotalCount()*100)/(Emul_rop->GetTotalCount() + Accel_rop->GetTotalCount()),
			 Emul_rop->GetRopCount(0x0000), Accel_rop->GetRopCount(0x0000),
			 ROPPercentage(false, 0x0000), ROPPercentage(true, 0x0000),
			 Emul_rop->GetRopCount(0xFFFF), Accel_rop->GetRopCount(0xFFFF),			 
			 ROPPercentage(false, 0xFFFF), ROPPercentage(true, 0xFFFF),			 
			 Emul_rop->GetRopCount(0x5555), Accel_rop->GetRopCount(0x5555),	
			 ROPPercentage(false, 0x5555), ROPPercentage(true, 0x5555),
			 Emul_rop->GetRopCount(0xAAF0), Accel_rop->GetRopCount(0xAAF0),	
			 ROPPercentage(false, 0xAAF0), ROPPercentage(true, 0xAAF0),
			 Emul_rop->GetRopCount(0x5A5A), Accel_rop->GetRopCount(0x5A5A),	
			 ROPPercentage(false, 0x5A5A), ROPPercentage(true, 0x5A5A),
			 Emul_rop->GetRopCount(0xF0F0), Accel_rop->GetRopCount(0xF0F0),	
			 ROPPercentage(false, 0xF0F0), ROPPercentage(true, 0xF0F0),
			 Emul_rop->GetRopCount(0x6666), Accel_rop->GetRopCount(0x6666),	
			 ROPPercentage(false, 0x6666), ROPPercentage(true, 0x6666),
			 Emul_rop->GetRopCount(0xCCCC), Accel_rop->GetRopCount(0xCCCC),
			 ROPPercentage(false, 0xCCCC), ROPPercentage(true, 0xCCCC),
			 Emul_rop->GetRopCount(0x8888), Accel_rop->GetRopCount(0x8888),	
			 ROPPercentage(false, 0x8888), ROPPercentage(true, 0x8888),
			 Emul_rop->GetRopCount(0xEEEE), Accel_rop->GetRopCount(0xEEEE),	
			 ROPPercentage(false, 0xEEEE), ROPPercentage(true, 0xEEEE),
			 Emul_rop->GetRopCount(0xB8B8), Accel_rop->GetRopCount(0xB8B8),	
			 ROPPercentage(false, 0xB8B8), ROPPercentage(true, 0xB8B8),
			 Emul_rop->GetRopCount(1), Accel_rop->GetRopCount(1),	
			 ROPPercentage(false, 1), ROPPercentage(true, 1));
	return true;
}

unsigned int  RopProfiler::SetPrintInterval(unsigned int interval)
{
	printinterval = interval;
	return printinterval;
}


bool RopCounter::hit_ROP(unsigned int MSropCode)
{
	if(run_state == RC_STOP) return false;
	switch(MSropCode)
	{
		case 0x0000: // BLACKNESS
			return hit_BLACKNESS();
		case 0xFFFF: //WHITENESS
			return hit_WHITENESS();
		case 0x5555:	// DSTINVERT
			return hit_DSTINVERT();
		case 0xAAF0:	// PATCOPY TEXT
			return hit_PATCOPYTEXT();
		case 0x5A5A:	// PATINVERT
			return hit_PATINVERT();
		case 0xF0F0:	// PATCOPY
			return hit_PATCOPY();
		case 0x6666:	// SRCINVERT
			return hit_SRCINVERT();
		case 0xCCCC:	// SRCCOPY
			return hit_SRCCOPY();
		case 0x8888:	// SRCAND
			return	hit_SRCAND();
		case 0xEEEE:	// SRCPAINT
			return	hit_SRCPAINT();
		case 0xB8B8:	// 0xB8B8
			return	hit_B8B8();
		default:			// OTHER
			return	hit_OTHER();
	}
	return false;
}

unsigned int RopCounter::GetRopCount(unsigned int MSropCode)
{
	switch(MSropCode)
	{
		case 0x0000: // BLACKNESS
			return rop_BLACKNESS;
		case 0xFFFF: //WHITENESS
			return rop_WHITENESS;
		case 0x5555:	// DSTINVERT
			return rop_DSTINVERT;
		case 0xAAF0:	// PATCOPY TEXT
			return rop_PATCOPYTEXT;
		case 0x5A5A:	// PATINVERT
			return rop_PATINVERT;
		case 0xF0F0:	// PATCOPY
			return rop_PATCOPY;
		case 0x6666:	// SRCINVERT
			return rop_SRCINVERT;
		case 0xCCCC:	// SRCCOPY
			return rop_SRCCOPY;
		case 0x8888:	// SRCAND
			return	rop_SRCAND;
		case 0xEEEE:	// SRCPAINT
			return	rop_SRCPAINT;
		case 0xB8B8:	// 0xB8B8
			return	rop_B8B8;
		default:			// OTHER
			return	rop_OTHER;
	}
}

void RopCounter::Print(void)
{
	if(m_sID)	printf("%s\n ", m_sID);
		 printf("total : %d, BLACKNESS: %d(%3.1lf%%), WHITENESS: %d(%3.1lf%%), DSTINVERT:%d(%3.1lf%%), PATINVERT:%d(%3.1lf%%), PATCOPYTEXT:%d(%3.1lf%%), PATCOPY:%d(%3.1lf%%), SRCINVERT:%d(%3.1lf%%), SRCCOPY:%d(%3.1lf%%), SRCAND:%d(%3.1lf%%), SRCPAINT:%d(%3.1lf%%), B8B8:%d(%3.1lf%%), OTHER:%d(%3.1lf%%)\n",
			 rop_counter,
			 rop_BLACKNESS,			 (float)rop_BLACKNESS*100/rop_counter,
			 rop_WHITENESS,			 (float)rop_WHITENESS*100/rop_counter,
			 rop_DSTINVERT,			 (float)rop_DSTINVERT*100/rop_counter,
			 rop_PATINVERT,			 (float)rop_PATINVERT*100/rop_counter,
			 rop_PATCOPYTEXT,			 (float)rop_PATCOPYTEXT*100/rop_counter,
			 rop_PATCOPY,			 (float)rop_PATCOPY*100/rop_counter,
			 rop_SRCINVERT,			 (float)rop_SRCINVERT*100/rop_counter,
			 rop_SRCCOPY,			 (float)rop_SRCCOPY*100/rop_counter,
			 rop_SRCAND,			 (float)rop_SRCAND*100/rop_counter,
			 rop_SRCPAINT,			 (float)rop_SRCPAINT*100/rop_counter,
			 rop_B8B8,			 (float)rop_B8B8*100/rop_counter,
			 rop_OTHER,			 (float)rop_OTHER*100/rop_counter);
}

unsigned int RopCounter::GetTotalCount()
{ 
	return rop_counter;
}

char *RopCounter::SetID(char *ID_string)
{
	m_sID = new char[strlen(ID_string)+1];
	strcpy(m_sID, ID_string);
	return m_sID;
}
