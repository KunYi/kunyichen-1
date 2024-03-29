#pragma once
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000 Microsoft Corporation.  All rights reserved.
//
// -----------------------------------------------------------------------------

#define VOLSHIFT (32-BITSPERSAMPLE)

extern const DWORD GainMap[];
#define DELTAINT  (17)
#define DELTAFRAC (32 - DELTAINT)

// Define 1.0 value for Delta
#define DELTA_OVERFLOW (1<<DELTAFRAC)

class StreamContext
{
public:
    LIST_ENTRY  m_Link;               // Link into list of streams

    StreamContext() {};
    virtual ~StreamContext() {};

    LONG AddRef();
    LONG Release();

    virtual HRESULT Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    virtual DWORD Close();
    virtual DWORD GetPos(PMMTIME pmmt);

    virtual DWORD Run();
    virtual DWORD Stop();
    virtual DWORD Reset();
    virtual PBYTE Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)=0;

    BOOL StillPlaying() {return (m_lpWaveHdrHead!=NULL);}
    DWORD GetByteCount() {return m_dwByteCount;}
	WAVEFORMATEX *GetWaveFormat() {return &m_WaveFormat;}
    DeviceContext *GetDeviceContext() { return m_pDeviceContext; }

    void DoDriverCallback(UINT msg, DWORD dwParam1, DWORD dwParam2)
    {
        m_pfnCallback(m_hWave,msg,m_dwInstance,dwParam1,dwParam2);
    }
    virtual DoCallbackReturnBuffer(LPWAVEHDR lpHdr)
    {
        DoDriverCallback(WOM_DONE,(DWORD)lpHdr,0);
    }
    virtual DoCallbackStreamOpened()
    {
        DoDriverCallback(WOM_OPEN,0,0);
    }
    virtual DoCallbackStreamClosed()
    {
        DoDriverCallback(WOM_CLOSE,0,0);
    }

    virtual DWORD QueueBuffer(LPWAVEHDR lpWaveHdr);
    PBYTE GetNextBuffer();

    // Default implementation
    void ReturnBuffer(LPWAVEHDR lpHdr)
    {
        lpHdr->dwFlags &= ~WHDR_INQUEUE;
        lpHdr->dwFlags |= WHDR_DONE;
        DoCallbackReturnBuffer(lpHdr);
    }

    DWORD GetGain()
    {
        return m_dwGain;
    }

    DWORD SetGain(DWORD dwGain)
    {
        m_dwGain = dwGain;
        GainChange();
        return MMSYSERR_NOERROR;
    }

    DWORD SetSecondaryGainClass(DWORD GainClass)
    {
        if (GainClass>=SECONDARYGAINCLASSMAX)
        {
            return MMSYSERR_ERROR;
        }
        m_SecondaryGainClass=GainClass;
        GainChange();
        return MMSYSERR_NOERROR;
    }

    DWORD MapGain(DWORD Gain, DWORD Channel);

    virtual void GainChange()
    {
        for (int i=0; i<2; i++)
        {
            m_fxpGain[i] = MapGain(m_dwGain,i);
        }
    }

    static void ClearBuffer(PBYTE pStart, PBYTE pEnd) {memset(pStart,0,pEnd-pStart);}

    DWORD BreakLoop();

protected:
    LONG            m_RefCount;
    BOOL            m_bRunning;         // Is stream running or stopped

    DWORD			m_dwFlags;          // allocation flags
    HWAVE			m_hWave;            // handle for stream
    DRVCALLBACK*    m_pfnCallback;      // client's callback
    DWORD			m_dwInstance;       // client's instance data

	WAVEFORMATEX	m_WaveFormat;	    // Format of wave data

    LPWAVEHDR   m_lpWaveHdrHead;
    LPWAVEHDR   m_lpWaveHdrCurrent;
    LPWAVEHDR   m_lpWaveHdrTail;
    PBYTE       m_lpCurrData;            // position in current buffer
    PBYTE       m_lpCurrDataEnd;         // end of current buffer

    DWORD       m_dwByteCount;          // byte count since last reset

    DeviceContext *m_pDeviceContext;   // Device which this stream renders to

    // Loopcount shouldn't really be here, since it's really for wave output only, but it makes things easier
    DWORD       m_dwLoopCount;          // Number of times left through loop

    DWORD       m_dwGain;
    DWORD       m_SecondaryGainClass;
    DWORD       m_fxpGain[2];
};

class WaveStreamContext : public StreamContext
{
public:
    HRESULT Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags);

    DWORD GetRate(DWORD *pdwMultiplier);
    virtual DWORD SetRate(DWORD dwMultiplier) = 0;
    PBYTE Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
    virtual PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)=0;

protected:
    PCM_TYPE m_SampleType;       // Enum of sample type, e.g. M8, M16, S8, S16
    ULONG    m_SampleSize;       // # of bytes per sample in client buffer
    DWORD    m_DeltaT;   // Sample rate conversion factor
    DWORD    m_dwMultiplier;
    LONG     m_PrevSamp[OUTCHANNELS];
    LONG     m_CurrSamp[OUTCHANNELS];
    LONG     m_CurrT;
};

class InputStreamContext : public WaveStreamContext
{
public:
    DWORD SetRate(DWORD dwMultiplier);
    DWORD Stop();   // On input, stop has special handling
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
    virtual DoCallbackReturnBuffer(LPWAVEHDR lpHdr)
    {
        DoDriverCallback(WIM_DATA,(DWORD)lpHdr,0);
    }
    virtual DoCallbackStreamOpened()
    {
        DoDriverCallback(WIM_OPEN,0,0);
    }
    virtual DoCallbackStreamClosed()
    {
        DoDriverCallback(WIM_CLOSE,0,0);
    }
};

class OutputStreamContext : public WaveStreamContext
{
public:
    HRESULT Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags);
    DWORD Reset();

    DWORD SetRate(DWORD dwMultiplier);
};

class OutputStreamContextM8 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};
class OutputStreamContextM16 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};
class OutputStreamContextS8 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};
class OutputStreamContextS16 : public OutputStreamContext
{
public:
    PBYTE Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast);
};


