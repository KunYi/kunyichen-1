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

#define SECONDARYGAINCLASSMAX 2

#define SECONDARYDEVICEGAINCLASSMAX 2
class DeviceContext
{
public:
    DeviceContext()
    {
        InitializeListHead(&m_StreamList);
        m_dwGain = 0xFFFF;
        m_dwDefaultStreamGain = 0xFFFFFFFF;
        for (int i=0;i<SECONDARYGAINCLASSMAX;i++)
        {
            m_dwSecondaryGainLimit[i]=0xFFFF;
        }
    }

    virtual BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    PBYTE TransferBuffer(PBYTE pBuffer, PBYTE pBufferEnd, DWORD *pNumStreams);

    void NewStream(StreamContext *pStreamContext);
    void DeleteStream(StreamContext *pStreamContext);

    DWORD GetGain()
    {
        return m_dwGain;
    }

    DWORD SetGain(DWORD dwGain);
	#if 0
    DWORD SetGain(DWORD dwGain)
    {
        m_dwGain = dwGain;
        RecalcAllGains();
        return MMSYSERR_NOERROR;
    }
	#endif

    DWORD GetDefaultStreamGain()
    {
        return m_dwDefaultStreamGain;
    }

    DWORD SetDefaultStreamGain(DWORD dwGain)
    {
        m_dwDefaultStreamGain = dwGain;
        return MMSYSERR_NOERROR;
    }

    DWORD GetSecondaryGainLimit(DWORD GainClass)
    {
        return m_dwSecondaryGainLimit[GainClass];
    }

    DWORD SetSecondaryGainLimit(DWORD GainClass, DWORD Limit)
    {
        if (GainClass>=SECONDARYGAINCLASSMAX)
        {
            return MMSYSERR_ERROR;
        }
        m_dwSecondaryGainLimit[GainClass]=Limit;
        RecalcAllGains();
        return MMSYSERR_NOERROR;
    }

    void RecalcAllGains();

    DWORD OpenStream(LPWAVEOPENDESC lpWOD, DWORD dwFlags, StreamContext **ppStreamContext);
    virtual DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual DWORD GetDevCaps(PVOID pCaps, DWORD dwSize)=0;
    virtual void StreamReadyToRender(StreamContext *pStreamContext)=0;

    virtual StreamContext *CreateStream(LPWAVEOPENDESC lpWOD)=0;

protected:
    LIST_ENTRY  m_StreamList;         // List of streams rendering to/from this device
    DWORD       m_dwGain;
    DWORD       m_dwDefaultStreamGain;
    DWORD m_dwSecondaryGainLimit[SECONDARYGAINCLASSMAX];
};

class InputDeviceContext : public DeviceContext
{
public:
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
};

class OutputDeviceContext : public DeviceContext
{
public:
    BOOL IsSupportedFormat(LPWAVEFORMATEX lpFormat);
    StreamContext *CreateStream(LPWAVEOPENDESC lpWOD);
    DWORD GetExtDevCaps(PVOID pCaps, DWORD dwSize);
    DWORD GetDevCaps(PVOID pCaps, DWORD dwSize);
    void StreamReadyToRender(StreamContext *pStreamContext);
};


