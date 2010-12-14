// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000 Microsoft Corporation.  All rights reserved.
//
// -----------------------------------------------------------------------------
#include "wavemain.h"

#define OUT_SCALE	128	//128  decrease value to increase a volume 

#define BITS_8_TO_16(x)      (short) ( ( (LONG) ( (UINT8)(x) - 128) ) << 8 )
#define BITS_16_TO_8(x)       ( (UINT8) (( (x) >> 8 ) +128)) 

#define UAM_TEST 0

HRESULT OutputStreamContext::Open(DeviceContext *pDeviceContext, LPWAVEOPENDESC lpWOD, DWORD dwFlags)
{
    HRESULT Result;

    Result = WaveStreamContext::Open(pDeviceContext, lpWOD, dwFlags);

    if (Result==MMSYSERR_NOERROR)
    {
        // Note: Output streams should be initialized in the run state.
        Run();
    }

    return Result;
}

DWORD OutputStreamContext::Reset()
{
    HRESULT Result;

    Result = WaveStreamContext::Reset();

    if (Result==MMSYSERR_NOERROR)
    {
        // Note: Output streams should be reset to the run state.
        Run();
    }

    return Result;
};

// Init m_DeltaT with (SampleRate/HWSampleRate) calculated in 16.16 fixed point form
// Note that we need to hold the result in a 64-bit value until we're done shifting,
// since the result of the multiply will overflow 32 bits for sample rates greater than
// or equal to the hardware's sample rate.

DWORD OutputStreamContext::SetRate(DWORD dwMultiplier)
{
    m_dwMultiplier = dwMultiplier;

    UINT64 Delta = (m_WaveFormat.nSamplesPerSec * m_dwMultiplier) >> 16;
    Delta = (Delta * INVSAMPLERATE) >> 17;  // Convert to 17.15 format
    m_DeltaT = (DWORD)Delta;
    return MMSYSERR_NOERROR;
}

// Originally, this code used to be in each renderer, and each one would call GetNextBuffer as needed.
// Pulling this code out of each low level renderer allows the inner loop to be in a leaf routine (ie no
// subroutine calls out of that routine), which helps the compiler optimize the inner loop.
PBYTE WaveStreamContext::Render(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    if (!m_bRunning || !m_lpCurrData)
    {
        return pBuffer;
    }

    while (pBuffer < pBufferEnd)
    {
        while (m_lpCurrData>=m_lpCurrDataEnd)
        {
            if (!GetNextBuffer())
            {
                return pBuffer;
            }
        }

        _try
        {

            pBuffer = Render2(pBuffer,pBufferEnd,pBufferLast);
        }
        _except (EXCEPTION_EXECUTE_HANDLER)
        {
            RETAILMSG(1, (TEXT("EXCEPTION IN IST for stream 0x%x, buffer 0x%x!!!!\r\n"), this, m_lpCurrData));
            m_lpCurrData=m_lpCurrDataEnd; // Pretend we finished reading the application buffer
        }
    }

    return pBuffer;
}




PBYTE OutputStreamContextM8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];

	//RETAILMSG(1,(TEXT("OutputStreamContextM8\n")));	
    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= 0x8000)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= 0x8000;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->m8.sample;
            CurrSamp0 = (CurrSamp0 - OUT_SCALE) << 8;
            pCurrData+=1;
        }

        LONG OutSamp0;
        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> 15);
        CurrT += DeltaT;
        
#if (OUTCHANNELS==2)
        LONG OutSamp1;
        OutSamp1=OutSamp0;
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;
        pBuffer += 2*sizeof(HWSAMPLE);
#else
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        pBuffer += sizeof(HWSAMPLE);
#endif
    }

    Exit:

    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}

PBYTE OutputStreamContextM16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];
    LONG OutSamp0;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= 0x8000)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= 0x8000;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->m16.sample;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> 15);
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

#if (OUTCHANNELS==2)
        LONG OutSamp1;
        OutSamp1=OutSamp0;
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;
        pBuffer += 2*sizeof(HWSAMPLE);
#else
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        pBuffer += sizeof(HWSAMPLE);
#endif
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}

#if (OUTCHANNELS==2)
PBYTE OutputStreamContextS8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG CurrSamp1 = m_CurrSamp[1];
    LONG PrevSamp0 = m_PrevSamp[0];
    LONG PrevSamp1 = m_PrevSamp[1];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];
    LONG OutSamp0;
    LONG OutSamp1;

//RETAILMSG(1,(TEXT("ch2 OutputStreamContextM8\n")));	
    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= 0x8000)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= 0x8000;

            PrevSamp0 = CurrSamp0;
            PrevSamp1 = CurrSamp1;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s8.sample_left;
            CurrSamp0 = (CurrSamp0 - OUT_SCALE) << 8;
            CurrSamp1 = (LONG)pSampleSrc->s8.sample_right;
            CurrSamp1 = (CurrSamp1 - OUT_SCALE) << 8;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> 15);
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;

        OutSamp1 = PrevSamp1 + (((CurrSamp1 - PrevSamp1) * CurrT) >> 15);
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

        pBuffer += 2*sizeof(HWSAMPLE);

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_PrevSamp[1] = PrevSamp1;
    m_CurrSamp[0] = CurrSamp0;
    m_CurrSamp[1] = CurrSamp1;
    return pBuffer;
}

PBYTE OutputStreamContextS16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG CurrSamp1 = m_CurrSamp[1];
    LONG PrevSamp0 = m_PrevSamp[0];
    LONG PrevSamp1 = m_PrevSamp[1];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain[2];
    fxpGain[0] = m_fxpGain[0];
    fxpGain[1] = m_fxpGain[1];
    LONG OutSamp0=0;
    LONG OutSamp1=0;

	// for test
    static LONG len = 0;
    static int fBingo = 0;
    LONG TmpSamp0;
    LONG OutSamp0_old_1,OutSamp0_old_2,OutSamp0_old_3,OutSamp0_old_4;

    while (pBuffer < pBufferEnd)
    {
		if ((fBingo == 0) && ((len++ % 10000) == 1))
		{
			fBingo = 1;
		}

        while (CurrT >= 0x8000)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= 0x8000;

            PrevSamp0 = CurrSamp0;
            PrevSamp1 = CurrSamp1;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 = (LONG)pSampleSrc->s16.sample_left;
            CurrSamp1 = (LONG)pSampleSrc->s16.sample_right;
            pCurrData+=4;
        }

		OutSamp0_old_1 = OutSamp0;
        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> 15);

		OutSamp0_old_2 = OutSamp0;
        OutSamp0 = (OutSamp0 * fxpGain[0]) >> VOLSHIFT;

		OutSamp0_old_3 = OutSamp0;
        OutSamp1 = PrevSamp1 + (((CurrSamp1 - PrevSamp1) * CurrT) >> 15);
        OutSamp1 = (OutSamp1 * fxpGain[1]) >> VOLSHIFT;
        CurrT += DeltaT;

        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));
		OutSamp0_old_4 = OutSamp0;
		TmpSamp0 = ((HWSAMPLE *)pBuffer)[0];
		
        if (pBuffer < pBufferLast)
        {
            OutSamp0 += ((HWSAMPLE *)pBuffer)[0];
            OutSamp1 += ((HWSAMPLE *)pBuffer)[1];
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
            if (OutSamp1>AUDIO_SAMPLE_MAX)
            {
                OutSamp1=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp1<AUDIO_SAMPLE_MIN)
            {
                OutSamp1=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        ((HWSAMPLE *)pBuffer)[0] = (HWSAMPLE)OutSamp0;
        ((HWSAMPLE *)pBuffer)[1] = (HWSAMPLE)OutSamp1;

        pBuffer += 2*sizeof(HWSAMPLE);

		if (fBingo)
		{
			fBingo++;
			if (fBingo > 2) fBingo = 0;
        	RETAILMSG(UAM_TEST,(_T("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, fxpGain=0x%x\r\n"),PrevSamp0,CurrSamp0,CurrT,fxpGain));
        	RETAILMSG(UAM_TEST,(_T("OutSamp0=0x%x->0x%x->0x%x->0x%x->0x%x, TmpSamp0=0x%x\r\n"),OutSamp0_old_1,OutSamp0_old_2,OutSamp0_old_3,OutSamp0_old_4,OutSamp0,TmpSamp0));
		}

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_PrevSamp[1] = PrevSamp1;
    m_CurrSamp[0] = CurrSamp0;
    m_CurrSamp[1] = CurrSamp1;
    return pBuffer;
}

#else

PBYTE OutputStreamContextS8::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain = m_fxpGain[0];
    LONG OutSamp0;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= 0x8000)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= 0x8000;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s8.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s8.sample_right;
            CurrSamp0 = (CurrSamp0 - 256) << 7;
            pCurrData+=2;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> 15);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += *(HWSAMPLE *)pBuffer;
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        *(HWSAMPLE *)pBuffer = (HWSAMPLE)OutSamp0;

        pBuffer += sizeof(HWSAMPLE);
    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}

PBYTE OutputStreamContextS16::Render2(PBYTE pBuffer, PBYTE pBufferEnd, PBYTE pBufferLast)
{
    LONG CurrT = m_CurrT;
    LONG DeltaT = m_DeltaT;
    LONG CurrSamp0 = m_CurrSamp[0];
    LONG PrevSamp0 = m_PrevSamp[0];
    PBYTE pCurrData = m_lpCurrData;
    PBYTE pCurrDataEnd = m_lpCurrDataEnd;
    LONG fxpGain = m_fxpGain[0];
    LONG OutSamp0;

    while (pBuffer < pBufferEnd)
    {
        while (CurrT >= 0x8000)
        {
            if (pCurrData>=pCurrDataEnd)
            {
                goto Exit;
            }

            CurrT -= 0x8000;

            PrevSamp0 = CurrSamp0;

            PPCM_SAMPLE pSampleSrc = (PPCM_SAMPLE)pCurrData;
            CurrSamp0 =  (LONG)pSampleSrc->s16.sample_left;
            CurrSamp0 += (LONG)pSampleSrc->s16.sample_right;
            CurrSamp0 = CurrSamp0>>1;
            pCurrData+=4;
        }

        OutSamp0 = PrevSamp0 + (((CurrSamp0 - PrevSamp0) * CurrT) >> 15);
        OutSamp0 = (OutSamp0 * fxpGain) >> VOLSHIFT;
        CurrT += DeltaT;
        // DEBUGMSG(1, (TEXT("PrevSamp0=0x%x, CurrSamp0=0x%x, CurrT=0x%x, OutSamp0=0x%x\r\n"), PrevSamp0,CurrSamp0,CurrT,OutSamp0));

        if (pBuffer < pBufferLast)
        {
            OutSamp0 += *(HWSAMPLE *)pBuffer;
#if USE_MIX_SATURATE
            // Handle saturation
            if (OutSamp0>AUDIO_SAMPLE_MAX)
            {
                OutSamp0=AUDIO_SAMPLE_MAX;
            }
            else if (OutSamp0<AUDIO_SAMPLE_MIN)
            {
                OutSamp0=AUDIO_SAMPLE_MIN;
            }
#endif
        }
        *(HWSAMPLE *)pBuffer = (HWSAMPLE)OutSamp0;

        pBuffer += sizeof(HWSAMPLE);

    }

    Exit:
    m_dwByteCount += (pCurrData - m_lpCurrData);
    m_lpCurrData = pCurrData;
    m_CurrT = CurrT;
    m_PrevSamp[0] = PrevSamp0;
    m_CurrSamp[0] = CurrSamp0;
    return pBuffer;
}
#endif
