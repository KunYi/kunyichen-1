#ifndef __ROPPROFILER_H__
#define __ROPPROFILER_H__

#include <stdio.h>

#define	MAX_COUNT		1000000000L
#define ACCUML_COUNT_INTERVAL		1000000L
#define	DEFAULT_COUNT_INTERVAL		20000
#define RC_STOP	0
#define RC_RUN	1
#define ID_EMUL_ROP_COUNTER		"Emulated ROP"
#define ID_ACCEL_ROP_COUNTER	"Accelerated ROP"

class RopCounter
{
	protected:
		unsigned int rop_counter;		
		unsigned int line_counter;
		unsigned int rop_BLACKNESS;
		unsigned int rop_WHITENESS;
		unsigned int rop_DSTINVERT;
		unsigned int rop_PATINVERT;
		unsigned int rop_PATCOPYTEXT;
		unsigned int rop_PATCOPY;
		unsigned int rop_SRCINVERT;
		unsigned int rop_SRCCOPY;
		unsigned int rop_SRCAND;
		unsigned int rop_SRCPAINT;
		unsigned int rop_B8B8;
		unsigned int rop_OTHER;

		inline bool hit_BLACKNESS() { rop_BLACKNESS++; rop_counter++; return true;}
		inline bool hit_WHITENESS() { rop_WHITENESS++; rop_counter++; return true;}
		inline bool hit_DSTINVERT() { rop_WHITENESS++; rop_counter++; return true;}
		inline bool hit_PATINVERT() { rop_PATINVERT++; rop_counter++; return true;}
		inline bool hit_PATCOPYTEXT() { rop_PATCOPYTEXT++; rop_counter++; return true;}
		inline bool hit_PATCOPY() { rop_PATCOPY++; rop_counter++; return true;}
		inline bool hit_SRCINVERT() { rop_SRCINVERT++; rop_counter++; return true;}
		inline bool hit_SRCCOPY() { rop_SRCCOPY++; rop_counter++; return true;}
		inline bool hit_SRCAND() { rop_SRCAND++;; rop_counter++; return true;}
		inline bool hit_SRCPAINT() { rop_SRCPAINT++; rop_counter++; return true;}
		inline bool hit_B8B8() { rop_B8B8++; rop_counter++; return true;}
		inline bool hit_OTHER() { rop_OTHER++; rop_counter++; return true;}


	private:
		RopCounter& operator=(RopCounter const & );		
		bool run_state;
		char *m_sID;
		
	public:
		RopCounter() : 	rop_counter(0),
										line_counter(0),
										rop_BLACKNESS(0),
										rop_WHITENESS(0),
										rop_DSTINVERT(0),
										rop_PATINVERT(0),
										rop_PATCOPYTEXT(0),
										rop_PATCOPY(0),
										rop_SRCINVERT(0),
										rop_SRCCOPY(0),
										rop_SRCAND(0),
										rop_SRCPAINT(0),
										rop_B8B8(0),
										rop_OTHER(0),
										run_state(RC_RUN),
										m_sID(NULL)	{};
		~RopCounter() { if(m_sID) delete m_sID; }
		void Reset() { rop_counter = 
										line_counter = 
										rop_BLACKNESS = 
										rop_WHITENESS =
										rop_DSTINVERT =
										rop_PATINVERT =
										rop_PATCOPYTEXT =
										rop_PATCOPY =
										rop_SRCINVERT =
										rop_SRCCOPY =
										rop_SRCAND =
										rop_SRCPAINT =
										rop_B8B8 =
										rop_OTHER = 0;}
		bool Stop() { run_state = RC_STOP; return run_state; }
		bool Restart() { Stop(); Reset(); Run(); return run_state;}
		bool Run() { run_state = RC_RUN; return run_state; }
		bool hit_ROP(unsigned int MSropCode); 		
		inline bool hit_LINE()	{ line_counter++; return true; }		
		char *SetID(char *ID_string);
		unsigned int GetTotalCount();
		unsigned int GetLineCount() { return line_counter;}
		unsigned int GetRopCount(unsigned int rop);		
		void Print();
};

class RopProfiler
{
	private:
		unsigned int printinterval;
		RopCounter *Accel_rop;
		RopCounter *Emul_rop;
		RopProfiler();
		RopProfiler(RopProfiler const &);		
		~RopProfiler();				
		static RopProfiler m_pInstance;
#if 0
		RopProfiler& operator=(RopProfiler const & );
#endif
	public:
#if 0
		static RopProfiler& Instance()
		{
			static RopProfiler obj;
//			printf("allocation is done. return pointer : %x..\n", obj);	
			return obj;
		}
#endif
		static RopProfiler* Instance() {
			return &m_pInstance;
		}
		bool CheckCount(RopCounter *targetcounter);
		bool SearchROP(unsigned int ropCode);
		unsigned int SetPrintInterval(unsigned int interval);
		bool Log(bool IsAccel, unsigned int MSROP);
		bool LogLine(bool IsAccel);
		float ROPPercentage(bool IsAccel, unsigned int rop);		
		bool Print(bool IsAccel);
		bool Print();
};

#endif __ROPPROFILER_H__
