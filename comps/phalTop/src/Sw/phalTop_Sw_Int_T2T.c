/*
*         Copyright (c), NXP Semiconductors Bangalore / India
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Internal functions for Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author: nxp66431 $
* $Revision: 336 $
* $Date: 2013-08-08 20:14:50 +0530 (Thu, 08 Aug 2013) $
*/
#include <ph_TypeDefs.h>
#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phacDiscLoop.h>
#include <phalMful.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW

#include "phalTop_Sw_Int_T2T.h"

phStatus_t phalTop_Sw_Int_T2T_SetReadOnly(
    phalTop_T2T_t * pT2T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bData[16];
    uint16_t   PH_MEMLOC_REM wBlockNo;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint16_t   PH_MEMLOC_REM wTempIndex;
    uint16_t   PH_MEMLOC_REM wOffset;
    uint16_t   PH_MEMLOC_REM wLockBytesCount = 0;
    uint16_t   PH_MEMLOC_REM wDefaultLockAddress;
    uint16_t   PH_MEMLOC_REM wDefaultLockLength;

    /* Check tag state, set to ready-only possible only in read-write state */
    if(pT2T->bTagState != PHAL_TOP_T2T_STATE_READWRITE)
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Read static lock bytes and CC */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
        pT2T,
        PHAL_TOP_T2T_STATIC_LOCK_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        bData));

    /* Update R/W access byte of CC to RO */
    bData[7] = PHAL_TOP_T2T_CC_RWA_RO;

    /* Write back CC */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
        pT2T,
        PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        &bData[4]));

    /* Set static lock bytes */
    bData[2] = 0xFF;
    bData[3] = 0xFF;

    /* Write back static lock bytes */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
        pT2T,
        PHAL_TOP_T2T_STATIC_LOCK_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        bData));

    /* For static tags only static lock bits are present, so exit */
    if(pT2T->bTagMemoryType == PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
    {
        /* Update state */
        pT2T->bTagState = PHAL_TOP_T2T_STATE_READONLY;

        return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
    }

    /* Update dynamic lock bytes */
    if(pT2T->bLockTlvCount)
    {
        for(wIndex = 0; wIndex < pT2T->bLockTlvCount; wIndex++)
        {
            wBlockNo = (pT2T->asLockCtrlTvl[wIndex].wByteAddr / PHAL_TOP_T2T_BYTES_PER_BLOCK);
            for(wLockBytesCount = 0;
                wLockBytesCount < (pT2T->asLockCtrlTvl[wIndex].bSizeInBits / 8);
                wBlockNo += 4)
            {
                /* Read dynamic lock bytes */
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
                    pT2T,
                    (wBlockNo * PHAL_TOP_T2T_BYTES_PER_BLOCK),
                    bData));
                if(wBlockNo == (pT2T->asLockCtrlTvl[wIndex].wByteAddr / PHAL_TOP_T2T_BYTES_PER_BLOCK))
                {
                    wOffset = pT2T->asLockCtrlTvl[wIndex].wByteAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK;
                }
                else
                {
                    wOffset = 0;
                }

                for(wTempIndex = wOffset;
                    ((wLockBytesCount < (pT2T->asLockCtrlTvl[wIndex].bSizeInBits / 8)) && (wTempIndex <= 16));
                    wTempIndex++, wLockBytesCount++)
                {
                    if(wTempIndex != 16)
                    {
                        bData[wTempIndex] = 0xFF;
                    }
                    /* If one block is updated with lock bytes, write that block */
                    if((wTempIndex >= 4) && ((wTempIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK) == 0))
                    {
                        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                            pT2T,
                            ((wBlockNo + ((wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK) - 1)) * PHAL_TOP_T2T_BYTES_PER_BLOCK),
                            &bData[(((wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK) - 1) * 4)]));
                    }
                }

                wTempIndex--;

                /* Write partially filled lock bytes */
                if((wTempIndex <= 16) && ((wTempIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK) != 0))
                {
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                        pT2T,
                        ((wBlockNo + (wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK)) * PHAL_TOP_T2T_BYTES_PER_BLOCK),
                        &bData[((wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK) * 4)]));
                }
            }
        }
    }
    /* If lock TLVs are not present, set default dynamic lock bits */
    else
    {
        /* Calculate default dynamic lock bytes starting address (data area + first 4 blocks of sector 0) */
        wDefaultLockAddress = (pT2T->bTms * 8) + 16;

        /* Calculate default dynamic lock length (in bits) */
        wDefaultLockLength = ((pT2T->bTms * 8) - 48) / 8;

        /* If there are one or more block(s) of dynamic lock bytes, set it */
        for(wIndex = 0; wIndex < (wDefaultLockLength / 8) / PHAL_TOP_T2T_BYTES_PER_BLOCK; wIndex++)
        {
            memset(bData, 0xFF, 4); /* PRQA S 3200 */
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                pT2T,
                wDefaultLockAddress,
                bData));
            wDefaultLockAddress += 4;
        }

        /* Check for partial dynamic lock block/byte */
        if(((wDefaultLockLength / 8) % PHAL_TOP_T2T_BYTES_PER_BLOCK) || (wDefaultLockLength % 8))
        {
            memset(bData, 0x00, 4); /* PRQA S 3200 */
            wIndex = 0;

            /* Calculate partial lock bytes (less than one block) */
            if((wDefaultLockLength / 8) % PHAL_TOP_T2T_BYTES_PER_BLOCK)
            {
                for(wIndex = 0; wIndex < (wDefaultLockLength / 8) % PHAL_TOP_T2T_BYTES_PER_BLOCK; wIndex++)
                {
                    bData[wIndex] = 0xFF;
                }
            }

            /* Calculate partial lock bits (less than one byte) */
            if(wDefaultLockLength % 8)
            {
                bData[wIndex] = (uint8_t)((1 << (wDefaultLockLength % 8)) - 1);
            }

            /* Set partial dynamic lock block/byte */
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                pT2T,
                wDefaultLockAddress,
                bData));
        }
    }

    /* Update state */
    pT2T->bTagState = PHAL_TOP_T2T_STATE_READONLY;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_ClearState(
    phalTop_T2T_t * pT2T
    )
{
    /* Reset parameters */
    pT2T->bLockTlvCount = 0;
    pT2T->bMemoryTlvCount = 0;
    pT2T->bNdefFormatted = 0;
    pT2T->bNdefPresence = 0;
    pT2T->bProprietaryTlvCount = 0;
    pT2T->bTagMemoryType = 0;
    pT2T->bTagState = 0;
    pT2T->bTerminatorTlvPresence = 0;
    pT2T->wMaxNdefLength = 0;
    pT2T->wNdefHeaderAddr = 0;
    pT2T->wNdefLength = 0;
    pT2T->wNdefMsgAddr = 0;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

/* Finds the count and position of memory, lock and NDEF TLV */
phStatus_t phalTop_Sw_Int_T2T_DetectTlvBlocks(
    phalTop_T2T_t * pT2T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint16_t   PH_MEMLOC_REM wIndexInit;
    uint8_t    PH_MEMLOC_REM bPageAddr;
    uint8_t    PH_MEMLOC_REM bByteOffset;
    uint8_t    PH_MEMLOC_REM bBytesPerPage;
    uint8_t    PH_MEMLOC_REM bLength[3];
    uint8_t    PH_MEMLOC_REM bExitLoop = 0;
    uint8_t    PH_MEMLOC_REM bBlockNum;
    uint8_t    PH_MEMLOC_REM bData[20];
    uint8_t    PH_MEMLOC_REM bDataLen;

    /* Set maximum data length for looping */
    bDataLen = 16;

    /* Set initial data start index */
    wIndexInit = 4;

    /* Check for TLVs only in first sector */
    for(bBlockNum = 4;
        ((bBlockNum < (((pT2T->bTms * 8) / PHAL_TOP_T2T_BYTES_PER_BLOCK) + 4)) && (bBlockNum <= 252));
        bBlockNum += 4)
    {
        /* Read data only if we don't have to skip */
        if(wIndexInit < bDataLen)
        {
            /* Read data and store it from bData[4] - bData[19] */
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
                pT2T,
                bBlockNum * PHAL_TOP_T2T_BYTES_PER_BLOCK,
                &bData[PHAL_TOP_T2T_BYTES_PER_BLOCK]));

            /* Set data starting index for loop */
            wIndex = wIndexInit;

            /* Check for TLVs in read data */
            while((wIndex < bDataLen) && (!bExitLoop))
            {
                switch(bData[wIndex])
                {
                case PHAL_TOP_T2T_NULL_TLV:
                    break;

                case PHAL_TOP_T2T_LOCK_CTRL_TLV:
                    if(pT2T->bLockTlvCount >= PHAL_TOP_T2T_MAX_LOCK_CTRL_TLV)
                    {
                        return PH_ADD_COMPCODE(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
                    }
                    if(pT2T->bTagMemoryType != PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
                    {
                        pT2T->asLockCtrlTvl[pT2T->bLockTlvCount].wOffset =
                            (bBlockNum * PHAL_TOP_T2T_BYTES_PER_BLOCK) + wIndex - PHAL_TOP_T2T_BYTES_PER_BLOCK;
                        bPageAddr = (uint8_t)((bData[wIndex + 2] & 0xF0) >> 4);
                        bByteOffset = bData[wIndex + 2] & 0x0F;
                        pT2T->asLockCtrlTvl[pT2T->bLockTlvCount].bSizeInBits =
                            bData[wIndex + 3];
                        bBytesPerPage = bData[wIndex + 4] & 0x0F;
                        pT2T->asLockCtrlTvl[pT2T->bLockTlvCount].bBytesLockedPerBit =
                            (uint8_t)((bData[wIndex + 4] & 0xF0) >> 4);
                        pT2T->asLockCtrlTvl[pT2T->bLockTlvCount].wByteAddr =
                            (bPageAddr * (1 << bBytesPerPage)) + bByteOffset;
                        pT2T->bLockTlvCount++;
                        wIndex = wIndex + 4;
                    }
                    break;

                case PHAL_TOP_T2T_MEMORY_CTRL_TLV:
                    if(pT2T->bMemoryTlvCount >= PHAL_TOP_T2T_MAX_MEM_CTRL_TLV)
                    {
                        return PH_ADD_COMPCODE(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
                    }
                    if(pT2T->bTagMemoryType != PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
                    {
                        pT2T->asMemCtrlTvl[pT2T->bMemoryTlvCount].wOffset =
                            (bBlockNum * PHAL_TOP_T2T_BYTES_PER_BLOCK) + wIndex - PHAL_TOP_T2T_BYTES_PER_BLOCK;
                        bPageAddr = (uint8_t)((bData[wIndex + 2] & 0xF0) >> 4);
                        bByteOffset = bData[wIndex + 2] & 0x0F;
                        pT2T->asMemCtrlTvl[pT2T->bMemoryTlvCount].bSizeInBytes =
                            bData[wIndex + 3];
                        bBytesPerPage = bData[wIndex + 4] & 0x0F;
                        pT2T->asMemCtrlTvl[pT2T->bMemoryTlvCount].wByteAddr =
                            (bPageAddr * (1 << bBytesPerPage)) + bByteOffset;
                        pT2T->bMemoryTlvCount++;
                        wIndex = wIndex + 4;
                    }
                    break;

                case PHAL_TOP_T2T_NDEF_TLV:
                    pT2T->wNdefHeaderAddr = (bBlockNum * PHAL_TOP_T2T_BYTES_PER_BLOCK) + wIndex - PHAL_TOP_T2T_BYTES_PER_BLOCK;
                    bLength[0] = bData[wIndex + 1];
                    if(bLength[0] == 0xFF)
                    {
                        bLength[1] = bData[wIndex + 2];
                        bLength[2] = bData[wIndex + 3];
                        pT2T->wNdefLength = ((uint16_t)bLength[1] << 8) | bLength[2];
                        pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 4;
                        wIndex = (uint8_t)(wIndex + pT2T->wNdefLength + 3);
                    }
                    else
                    {
                        pT2T->wNdefLength = bLength[0];
                        pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 2;
                        wIndex = (uint8_t)(wIndex + pT2T->wNdefLength + 1);
                    }

                    /* Only one NDEF TLV is supported, exit loop */
                    bExitLoop = 1;
                    break;

                case PHAL_TOP_T2T_PROPRIETARY_TLV:
                    if(pT2T->bProprietaryTlvCount >= PHAL_TOP_T2T_MAX_PROPRIETARY_TLV)
                    {
                        return PH_ADD_COMPCODE(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
                    }

                    pT2T->asPropTvl[pT2T->bProprietaryTlvCount].wOffset =
                        (bBlockNum * PHAL_TOP_T2T_BYTES_PER_BLOCK) + wIndex - PHAL_TOP_T2T_BYTES_PER_BLOCK;
                    bLength[0] = bData[wIndex + 1];
                    if(bLength[0] == 0xFF)
                    {
                        bLength[1] = bData[wIndex + 2];
                        bLength[2] = bData[wIndex + 3];
                        pT2T->asPropTvl[pT2T->bProprietaryTlvCount].wLength =
                            ((uint16_t)bLength[1] << 8) | bLength[2];
                        wIndex = (uint8_t)(wIndex + (((uint16_t)bLength[1] << 8) | bLength[2]) + 3);
                    }
                    else
                    {
                        pT2T->asPropTvl[pT2T->bProprietaryTlvCount].wLength = bLength[0];
                        wIndex = wIndex + bLength[0] + 1;
                    }
                    break;

                case PHAL_TOP_T2T_TERMINATOR_TLV:
                    pT2T->bTerminatorTlvPresence = PH_ON;

                    /* Exit loop after when terminator TLV is detected */
                    bExitLoop = 1;
                    break;

                default:
                    /* Invalid TLV, exit loop */
                    bExitLoop = 1;
                    break;
                }
                wIndex++;
            }

            if(!bExitLoop)
            {
                /* Copy last block of data as first block for next loop */
                memcpy(bData, &bData[bDataLen], 4);         /* PRQA S 3200 */

                /* Update starting index of data for next loop */
                if(wIndex >= bDataLen)
                {
                    wIndexInit = wIndex - bDataLen;
                }
            }
        }
        else
        {
            wIndexInit -= bDataLen;
        }

        /* Exit condition set, exit loop */
        if(bExitLoop)
        {
            break;
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_CheckNdef(
                                        phalTop_T2T_t * pT2T,
                                        uint8_t * pNdefPresence
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bData[16];

    pT2T->bNdefFormatted = PH_UNSUPPORTED;
    pT2T->bNdefPresence = false;
    *pNdefPresence = false;

    /* Clear values from previous detection, if any */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_ClearState(pT2T));

    /* Read CC */
    status = phalTop_Sw_Int_T2T_Read(
        pT2T,
        PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        bData);
    if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
    }

    /* Set default tag state */
    pT2T->bTagState = PHAL_TOP_T2T_STATE_UNKNOWN;

    /* Check for NDEF Magic Number */
    if(bData[0] == 0x00)
    {
        pT2T->bNdefFormatted = PH_SUPPORTED;
        pT2T->bNdefPresence = false;
        *pNdefPresence = false;
    }
    else if(bData[0] == PHAL_TOP_T2T_NDEF_MAGIC_NUMBER)
    {
        pT2T->bNdefFormatted = PH_SUPPORTED;

        pT2T->bVno = bData[1];
        pT2T->bTms = bData[2];

        if((pT2T->bTms * 8) == 48)
        {
            pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC;
        }
        else if((pT2T->bTms * 8) > 48)
        {
            pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_DYNAMIC;
        }
        else
        {
            return PH_ADD_COMPCODE(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
        }

        pT2T->bRwa = bData[3];

        /* Check for TLVs only if tag has a supported version number and has read access */
        if((uint8_t)(pT2T->bVno & 0xF0) <= (uint8_t)(PHAL_TOP_T2T_NDEF_SUPPORTED_VNO & 0xF0)
            && (uint8_t)(pT2T->bRwa & 0xF0) == 0x00)
        {
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_DetectTlvBlocks(
                pT2T));
        }

        /* Update NDEF presence */
        if(pT2T->wNdefHeaderAddr)
        {
            pT2T->bNdefPresence = true;
            *pNdefPresence = true;
        }

        /* Update tag state */
        if((pT2T->bNdefPresence == true) && (pT2T->wNdefLength != 0) &&
            ((uint8_t)(pT2T->bVno & 0xF0) <= (uint8_t)(PHAL_TOP_T2T_NDEF_SUPPORTED_VNO & 0xF0)))
        {
            if(pT2T->bRwa == PHAL_TOP_T2T_CC_RWA_RW)
            {
                pT2T->bTagState = PHAL_TOP_T2T_STATE_READWRITE;
            }
            else if(pT2T->bRwa == PHAL_TOP_T2T_CC_RWA_RO)
            {
                pT2T->bTagState = PHAL_TOP_T2T_STATE_READONLY;
            }
            else
            {
                /* RFU */
            }
        }
        else if((pT2T->bRwa == PHAL_TOP_T2T_CC_RWA_RW) && (pT2T->bNdefPresence == true) &&
          (pT2T->wNdefLength == 0) && (pT2T->wNdefHeaderAddr != 0))
        {
            pT2T->bTagState = PHAL_TOP_T2T_STATE_INITIALIZED;
        }
        else
        {
            pT2T->bTagState = PHAL_TOP_T2T_STATE_UNKNOWN;
            pT2T->bNdefPresence = false;
            *pNdefPresence = false;
        }
    }
    else
    {
        pT2T->bNdefPresence = false;
        *pNdefPresence = false;
    }

    if(pT2T->bTagState != PHAL_TOP_T2T_STATE_UNKNOWN)
    {
        /* Update reserved/lock bytes status for first sector */
        pT2T->sSector.bAddress = 0;
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(pT2T));
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_EraseNdef(
                                        phalTop_T2T_t * pT2T
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bData[16];

    if((pT2T->bNdefPresence == true) && (pT2T->bTagState == PHAL_TOP_T2T_STATE_READWRITE))
    {
        /* Make NDEF Magic Number value '0' */
        bData[0] = 0x00;
        bData[1] = pT2T->bVno;
        bData[2] = pT2T->bTms;
        bData[3] = pT2T->bRwa;

        /* Write CC with NMN value as '0' */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
            pT2T,
            PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
            bData));

        /* Read CC */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
            pT2T,
            PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
            bData));

        /* Check if CC/NMN erase was successful. If CC is OTP, write will fail */
        if(bData[0] != 0x00)
        {
            return PH_ADD_COMPCODE(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
        }
    }
    else
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Reset state and NDEF presence */
    pT2T->bTagState = PHAL_TOP_T2T_STATE_UNKNOWN;
    pT2T->bNdefFormatted = PH_SUPPORTED;
    pT2T->bNdefPresence = false;
    pT2T->wNdefHeaderAddr = 0x00;
    pT2T->wNdefMsgAddr = 0x00;
    pT2T->wNdefLength = 0x00;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_FormatNdef(
    phalTop_T2T_t * pT2T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM pData[16];
    uint8_t    PH_MEMLOC_REM bNdefData[16] = {0xD1, 0x01, 0x0C, 0x55, 0x00, 0x77,
                                              0x77, 0x77, 0x2E, 0x6E, 0x78, 0x70,
                                              0x2E, 0x63, 0x6F, 0x6D};

    /* Check for NDEF support and NDEF presence */
    if((pT2T->bNdefFormatted != PH_SUPPORTED) || (pT2T->bNdefPresence != false))
    {
        return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Set NMN */
    pData[0] = 0xE1;

    /* If version is not set, set default version number */
    if(pT2T->bVno < PHAL_TOP_T2T_NDEF_SUPPORTED_VNO)
    {
        pT2T->bVno = PHAL_TOP_T2T_NDEF_SUPPORTED_VNO;
        pData[1] = pT2T->bVno;
    }
    else
    {
        pData[1] = pT2T->bVno;
    }

    /* If TMS is not set/wrong, set TMS of static tag  */
    if(pT2T->bTms < 0x06)
    {
        pT2T->bTms = 0x06;
        pData[2] = pT2T->bTms;
    }
    else
    {
        pData[2] = pT2T->bTms;
    }

    /* Set R/W access */
    pT2T->bRwa = 0x00;
    pData[3] = pT2T->bRwa;

    /* Validate and update tag memory type */
    if((pT2T->bTms * 8) == 48)
    {
        pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC;
    }
    else
    {
        pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_DYNAMIC;
    }

    /* Write CC (return value not checked because if CC is OTP this may fail) */
    status = phalTop_Sw_Int_T2T_Write(
        pT2T,
        PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        pData);

    /* Read back CC */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
        pT2T,
        PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        pData));

    /* Check if CC write was successful */
    if((pData[0] != 0xE1) || (pData[1] != pT2T->bVno) ||
        (pData[2] != pT2T->bTms) || (pData[3] != pT2T->bRwa))
    {
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_TOP);
    }

    /* Update NDEF header address */
    if((pT2T->bLockTlvCount !=0) &&
        (pT2T->bMemoryTlvCount != 0))
    {
        pT2T->wNdefHeaderAddr =
            (pT2T->asLockCtrlTvl[pT2T->bLockTlvCount - 1].wOffset >
            pT2T->asMemCtrlTvl[pT2T->bMemoryTlvCount - 1].wOffset?
            pT2T->asLockCtrlTvl[pT2T->bLockTlvCount - 1].wOffset:
        pT2T->asMemCtrlTvl[pT2T->bMemoryTlvCount - 1].wOffset) + 5;
    }
    else if(pT2T->bLockTlvCount !=0)
    {
        pT2T->wNdefHeaderAddr = pT2T->asLockCtrlTvl[pT2T->bLockTlvCount - 1].wOffset + 5;
    }
    else if(pT2T->bMemoryTlvCount != 0)
    {
        pT2T->wNdefHeaderAddr = pT2T->asMemCtrlTvl[pT2T->bMemoryTlvCount - 1].wOffset + 5;
    }
    else
    {
        pT2T->wNdefHeaderAddr = 16;
    }

    /* Update the values for using WriteNdef API for writing default NDEF */
    pT2T->bTagState = PHAL_TOP_T2T_STATE_INITIALIZED;
    pT2T->bNdefPresence = true;

    /* Write default NDEF Message (www.nxp.com) */
    status = phalTop_Sw_Int_T2T_WriteNdef(pT2T, bNdefData, 16);

    if((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        /* Reset state on error */
        pT2T->bTagState = PHAL_TOP_T2T_STATE_UNKNOWN;
        pT2T->wNdefHeaderAddr = 0x00;
        pT2T->bNdefPresence = false;

        return status;
    }

    /* Update state and NDEF presence */
    pT2T->bTagState = PHAL_TOP_T2T_STATE_READWRITE;
    pT2T->bNdefFormatted = PH_SUPPORTED;
    pT2T->bNdefPresence = true;
    pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 2;
    pT2T->wNdefLength = 0x10;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_Read(
                                   phalTop_T2T_t * pT2T,
                                   uint16_t wOffset,
                                   uint8_t * pData
                                   )
{
    phStatus_t   PH_MEMLOC_REM status;

    /* Check if read offset is in current sector */
    if(pT2T->sSector.bAddress != (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE))
    {
        PH_CHECK_SUCCESS_FCT(status, phalMful_SectorSelect(
            pT2T->phalT2TDataParams,
            (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE)));

        pT2T->sSector.bAddress = (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE);
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(pT2T));
    }

    /* Read data */
    PH_CHECK_SUCCESS_FCT(status, phalMful_Read(
        pT2T->phalT2TDataParams,
        (uint8_t)((wOffset % PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK),
        pData));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_Write(
                                    phalTop_T2T_t * pT2T,
                                    uint16_t wOffset,
                                    uint8_t * pData
                                    )
{
    phStatus_t   PH_MEMLOC_REM status;

    /* Check if write offset is in current sector */
    if(pT2T->sSector.bAddress != (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE))
    {
        PH_CHECK_SUCCESS_FCT(status, phalMful_SectorSelect(
            pT2T->phalT2TDataParams,
            (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE)));

        pT2T->sSector.bAddress = (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE);
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(pT2T));
    }

    /* Write data */
    PH_CHECK_SUCCESS_FCT(status, phalMful_Write(
        pT2T->phalT2TDataParams,
        (uint8_t)((wOffset % PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK),
        pData));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_ReadNdef(
                                       phalTop_T2T_t * pT2T,
                                       uint8_t * pData,
                                       uint16_t * pLength
                                       )
{
    phStatus_t   PH_MEMLOC_REM status;
    uint16_t     PH_MEMLOC_REM wIndex;
    uint16_t     PH_MEMLOC_REM wTempIndex;
    uint16_t     PH_MEMLOC_REM wCount;
    uint8_t      PH_MEMLOC_REM bData[16];
    uint16_t     PH_MEMLOC_REM wIndexInit = 0;

    if(pT2T->bNdefPresence != PH_ON)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
    }
    else if((pT2T->bTagState != PHAL_TOP_T2T_STATE_READWRITE) &&
       (pT2T->bTagState != PHAL_TOP_T2T_STATE_READONLY))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
    }
    else
    {
        pT2T->sSector.bAddress = (uint8_t)(pT2T->wNdefMsgAddr / PHAL_TOP_T2T_SECTOR_SIZE);
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(pT2T));
        wIndexInit = pT2T->wNdefMsgAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK;

        for(wIndex = pT2T->wNdefMsgAddr, wTempIndex = 0;
            wTempIndex < pT2T->wNdefLength;
            wIndex += 16)
        {
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
                pT2T,
                wIndex,
                bData));

            for(wCount = wIndexInit; ((wCount < 16) &&
               (wTempIndex < pT2T->wNdefLength)); wCount++)
            {
                if((phalTop_Sw_Int_T2T_CheckLockReservedOtp(pT2T,
                  ((wIndex + wCount) / PHAL_TOP_T2T_BYTES_PER_BLOCK))) == PH_ERR_SUCCESS)
                {
                    pData[wTempIndex] = bData[wCount];
                    wTempIndex++;
                }
            }
            wIndexInit = 0;
        }

        if(wTempIndex == pT2T->wNdefLength)
        {
            *pLength = pT2T->wNdefLength;
        }
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_WriteNdef(
                                        phalTop_T2T_t * pT2T,
                                        uint8_t * pData,
                                        uint16_t wLength
                                        )
{
    phStatus_t  PH_MEMLOC_REM status;
    uint16_t    PH_MEMLOC_REM wIndex = 0;
    uint16_t    PH_MEMLOC_REM wTempIndex;
    uint8_t     PH_MEMLOC_REM bTempData[4];
    uint8_t     PH_MEMLOC_REM bData[16];
    uint16_t    PH_MEMLOC_REM wCount = 0;
    uint16_t    PH_MEMLOC_REM wOffset;
    uint16_t    PH_MEMLOC_REM wTernimatorTlvLen = 0;

    /* Calculate Max. NDEF size */
    phalTop_Sw_Int_T2T_CalculateMaxNdefSize(pT2T);

    /* Check NDEF length */
    if((wLength > pT2T->wMaxNdefLength) || (wLength == 0x00))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    if(pT2T->bTagState == PHAL_TOP_T2T_STATE_READONLY)
    {
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_TOP);
    }

    if((pT2T->bNdefPresence) && (pT2T->bTagState != PHAL_TOP_T2T_STATE_UNKNOWN) &&
       (pT2T->wNdefHeaderAddr))
    {
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
            pT2T,
            pT2T->wNdefHeaderAddr,
            bData));

        wOffset = pT2T->wNdefHeaderAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK;
        bData[wOffset++] = PHAL_TOP_T2T_NDEF_TLV;
        bData[wOffset++] = 0x00;

        if(wLength > 0xFE)
        {
            pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 4;
            wOffset += 2;
        }
        else
        {
            pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 2;
        }

        for(wCount = 0;
            ((wOffset % PHAL_TOP_T2T_BYTES_PER_BLOCK) != 0);
            wCount++, wOffset++)
        {
            bData[wOffset] = pData[wCount];
        }

        /* Write the first block with NDEF TLV */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
            pT2T,
            pT2T->wNdefHeaderAddr,
            bData));

        if(wOffset > PHAL_TOP_T2T_BYTES_PER_BLOCK)
        {
            for(wIndex = (pT2T->wNdefHeaderAddr + PHAL_TOP_T2T_BYTES_PER_BLOCK); wIndex < pT2T->wMaxNdefLength; wIndex += 4)
            {
                /* Check for reserved/lock blocks */
                if((phalTop_Sw_Int_T2T_CheckLockReservedOtp(pT2T, (wIndex / 4))) == PH_ERR_SUCCESS)
                {
                    /* Write the next block */
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                        pT2T->phalT2TDataParams,
                        wIndex,
                        &bData[4]));
                    break;
                }
            }
        }

        if(pT2T->bTerminatorTlvPresence == PH_ON)
        {
            wTernimatorTlvLen = 1;
        }

        /* Write NDEF data */
        for(wIndex = (pT2T->wNdefMsgAddr + wCount), wTempIndex = wCount;
            wTempIndex < (wLength + wTernimatorTlvLen);
            wIndex += 4)
        {
            if((wIndex/PHAL_TOP_T2T_SECTOR_SIZE) != pT2T->sSector.bAddress)
            {
                PH_CHECK_SUCCESS_FCT(status, phalMful_SectorSelect(
                    pT2T->phalT2TDataParams,
                    (uint8_t)(wIndex / PHAL_TOP_T2T_SECTOR_SIZE)));
                pT2T->sSector.bAddress = (uint8_t)(wIndex / PHAL_TOP_T2T_SECTOR_SIZE);
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
                    pT2T));
            }

            if((phalTop_Sw_Int_T2T_CheckLockReservedOtp(pT2T, (wIndex / 4))) == PH_ERR_SUCCESS)
            {
                if((wLength - wTempIndex) >= PHAL_TOP_T2T_BYTES_PER_BLOCK)
                {
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                        pT2T,
                        wIndex,
                        &pData[wTempIndex]));
                    wTempIndex += 4;
                }
                else
                {
                    memset(bTempData, 0, 4);    /* PRQA S 3200 */
                    for(wOffset = 0; wLength > wTempIndex; wOffset++, wTempIndex++)
                    {
                        bTempData[wOffset] = pData[wTempIndex];
                    }
                    bTempData[wOffset] = 0xFE;
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                        pT2T,
                        wIndex,
                        bTempData));
                    wTempIndex++;

                }
            }
        }

        /* Update NDEF Length */
        pT2T->wNdefLength = wLength;
        wOffset = pT2T->wNdefHeaderAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK;

        /* If length more than 254, use 3 byte length format */
        if(wLength > 0xFE)
        {
            bData[wOffset] = PHAL_TOP_T2T_NDEF_TLV;
            bData[++wOffset] = 0xFF;
            bData[++wOffset] = (uint8_t)(wLength >> 8);
            bData[++wOffset] = (uint8_t)wLength;
        }
        else
        {
            bData[wOffset] = PHAL_TOP_T2T_NDEF_TLV;
            bData[++wOffset] = (uint8_t)wLength;
        }

        /* Write the first block with NDEF TLV */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
            pT2T,
            pT2T->wNdefHeaderAddr,
            bData));

        if(wOffset > PHAL_TOP_T2T_BYTES_PER_BLOCK)
        {
            /* Write the next block */
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                pT2T,
                (pT2T->wNdefHeaderAddr + PHAL_TOP_T2T_BYTES_PER_BLOCK),
                &bData[4]));
        }

        pT2T->bTagState = PHAL_TOP_T2T_STATE_READWRITE;
        pT2T->bNdefPresence = true;
    }
    else
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T2T_CheckLockReservedOtp(
    phalTop_T2T_t * pT2T,
    uint16_t wIndex
    )
{
    if((pT2T->sSector.bLockReservedOtp[wIndex / 8]) &
        (uint8_t)(1 << (wIndex % 8)))
    {
        return PH_ERR_USE_CONDITION;
    }
    else
    {
        return PH_ERR_SUCCESS;
    }
}

void phalTop_Sw_Int_T2T_CalculateMaxNdefSize(
    phalTop_T2T_t * pT2T
    )
{
    uint16_t PH_MEMLOC_REM wIndex;

    if(pT2T->bTagMemoryType == PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
    {
        pT2T->wMaxNdefLength = 46;
    }
    else
    {
        pT2T->wMaxNdefLength = (pT2T->bTms) * 8;
        if(pT2T->bLockTlvCount)
        {
            for(wIndex = 0; wIndex < pT2T->bLockTlvCount; wIndex++)
            {
                pT2T->wMaxNdefLength -= 5;
                pT2T->wMaxNdefLength -=
                    (pT2T->asLockCtrlTvl[wIndex].bSizeInBits / 8);
            }
        }

        if(pT2T->bMemoryTlvCount)
        {
            for(wIndex = 0; wIndex < pT2T->bMemoryTlvCount; wIndex++)
            {
                pT2T->wMaxNdefLength -= 5;
                pT2T->wMaxNdefLength -=
                    pT2T->asMemCtrlTvl[wIndex].bSizeInBytes;
            }
        }
    }
}

phStatus_t phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
    phalTop_T2T_t * pT2T
    )
{
    uint16_t PH_MEMLOC_REM wOffset;
    uint16_t PH_MEMLOC_REM wIndex;
    uint16_t PH_MEMLOC_REM wSectorOffset;
    uint16_t PH_MEMLOC_REM wBlockOffset;

    if(pT2T->sSector.bAddress == 0)
    {
        /* First 3 blocks are internal/lock for sector 0 */
        pT2T->sSector.bLockReservedOtp[0x00] = 0x07;

    }

    if(pT2T->bTagMemoryType == PHAL_TOP_T2T_TAG_MEMORY_TYPE_DYNAMIC)
    {
        /* Update Lock Bytes */
        for(wIndex = 0; wIndex < pT2T->bLockTlvCount; wIndex++)
        {
            if((pT2T->asLockCtrlTvl[wIndex].wByteAddr /
                PHAL_TOP_T2T_SECTOR_SIZE) ==
                pT2T->sSector.bAddress)
            {
                wSectorOffset =
                    (pT2T->asLockCtrlTvl[wIndex].wByteAddr %
                    PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK;

                if((pT2T->asLockCtrlTvl[wIndex].bSizeInBits / 8) % PHAL_TOP_T2T_BYTES_PER_BLOCK)
                {
                    wBlockOffset = 1;
                }
                else
                {
                    wBlockOffset = 0;
                }

                for(wOffset = wSectorOffset;
                    wOffset < (wSectorOffset +
                    ((pT2T->asLockCtrlTvl[wIndex].bSizeInBits / 8) / PHAL_TOP_T2T_BYTES_PER_BLOCK) +
                    wBlockOffset);
                wOffset++)
                {
                    pT2T->sSector.bLockReservedOtp[wOffset / 8] |=
                        (uint8_t)(1 << (wOffset % 8));
                }
            }
        }
        /* Update Memory Bytes */
        for(wIndex = 0; wIndex < pT2T->bMemoryTlvCount; wIndex++)
        {
            if(pT2T->asMemCtrlTvl[wIndex].wByteAddr /
                PHAL_TOP_T2T_SECTOR_SIZE ==
                pT2T->sSector.bAddress)
            {
                wSectorOffset =
                    (pT2T->asMemCtrlTvl[wIndex].wByteAddr %
                    PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK;

                if((pT2T->asMemCtrlTvl[wIndex].bSizeInBytes) / PHAL_TOP_T2T_BYTES_PER_BLOCK)
                {
                    wBlockOffset = 1;
                }
                else
                {
                    wBlockOffset = 0;
                }

                for(wOffset = wSectorOffset;
                    wOffset < (wSectorOffset +
                    ((pT2T->asMemCtrlTvl[wIndex].bSizeInBytes) / PHAL_TOP_T2T_BYTES_PER_BLOCK) +
                    wBlockOffset);
                wOffset++)
                {
                    pT2T->sSector.bLockReservedOtp[wOffset / 8] |=
                        (uint8_t)(1 << (wOffset % 8));
                }
            }
        }
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

#endif /* NXPBUILD__PHAL_TOP_SW */
