/**
 * \file phOsal_GLib.c
 * \author Donatien Garnier
 */

#include <ph_Status.h>
#include <ph_NxpBuild.h>

#ifdef NXPBUILD__PH_OSAL_GLIB

#include <phOsal.h>
#include <phOsal_GLib.h>

#include <glib.h>

//Internal function
static gboolean phOsal_Int_Timer_Callback(gpointer data);

void phOsal_GLib_Init_GLib( phOsal_GLib_DataParams_t  *pDataParams, GMainContext* pgGLibMainContext )
{
	pDataParams->pgGLibMainContext = pgGLibMainContext;
}

phStatus_t phOsal_GLib_Init( phOsal_GLib_DataParams_t  *pDataParams )
{
    pDataParams->wId = PH_COMP_OSAL | PH_OSAL_GLIB_ID;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

phStatus_t phOsal_GLib_Timer_Init( phOsal_GLib_DataParams_t  *pDataParams )
{
    //Init timers
	for(uint32_t i = 0; i < OSAL_GLIB_MAX_TIMERS; i++)
	{
		pDataParams->gTimers[i].dwTimerId = i; //Needed to recover timer id from callback
		pDataParams->gTimers[i].bTimerFree = TIMER_FREE;
		pDataParams->gTimers[i].pGTimeoutSource = NULL;
		pDataParams->gTimers[i].pApplicationCallback = NULL;
		pDataParams->gTimers[i].pContext = NULL;
	}

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

phStatus_t phOsal_GLib_Timer_Create( phOsal_GLib_DataParams_t   *pDataParams,
                                     uint32_t                   *pTimerId
                                   )
{
	//g_debug("New timer...\r\n");
	for(*pTimerId = 0; *pTimerId < OSAL_GLIB_MAX_TIMERS; (*pTimerId)++)
	{
		if(pDataParams->gTimers[*pTimerId].bTimerFree == TIMER_FREE)
		{
			break;
		}
	}

	if( *pTimerId == OSAL_GLIB_MAX_TIMERS )
	{
		//No free timer left
		g_error("Not enough timers!\r\n");
        return PH_ADD_COMPCODE(PH_OSAL_ERR_NO_FREE_TIMER,  PH_COMP_OSAL);
	}

	pDataParams->gTimers[*pTimerId].bTimerFree = TIMER_USED;
	pDataParams->gTimers[*pTimerId].pGTimeoutSource = NULL;

	//g_debug("Timer id %d\r\n", *pTimerId);


    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

phStatus_t phOsal_GLib_Timer_Delete( phOsal_GLib_DataParams_t   *pDataParams,
                                     uint32_t                    dwTimerId
                                   )
{
   //g_debug("Deleting timer %d\r\n", dwTimerId);
   if( (dwTimerId > OSAL_GLIB_MAX_TIMERS) || (pDataParams->gTimers[dwTimerId].bTimerFree == TIMER_FREE) )
   {
	   return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_OSAL);
   }

   //Make sure the timer is stopped
   if( pDataParams->gTimers[dwTimerId].pGTimeoutSource != NULL )
   {
	   phOsal_GLib_Timer_Stop(pDataParams, dwTimerId);
   }

   pDataParams->gTimers[dwTimerId].bTimerFree = TIMER_FREE;
   pDataParams->gTimers[dwTimerId].pApplicationCallback = NULL;
   pDataParams->gTimers[dwTimerId].pContext = NULL;

   return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

phStatus_t phOsal_GLib_Timer_Start( phOsal_GLib_DataParams_t  *pDataParams,
                                    uint32_t                   dwTimerId,
                                    uint32_t                   dwRegTimeCnt,
                                    uint16_t                   wOption,
                                    ppCallBck_t                pApplicationCallback,
                                    void                      *pContext
                                  )
{
   //g_debug("Starting timer %d...\r\n", dwTimerId);
   if( (dwTimerId > OSAL_GLIB_MAX_TIMERS) || (pDataParams->gTimers[dwTimerId].bTimerFree == TIMER_FREE) )
   {
	   /* Can't use a non existent timer */
	   /* Can't start a free timer, first create the timer */

	   return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_OSAL);
   }

   if( pDataParams->gTimers[dwTimerId].pGTimeoutSource != NULL )
   {
	   //Cannot start at timer which is already running
	   g_error("Timer %d already running\n", dwTimerId);
	   return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_OSAL);
   }

   //Convert usecs to millisecs if needed
   if( wOption ==  0 ) //
   {
	   //g_debug("%d us", dwRegTimeCnt);
	   //We're running in user-mode on a (probably) non realtime kernel, so even millisecs are very approximate
	   if( dwRegTimeCnt < 1000 )
	   {
		   pDataParams->gTimers[dwTimerId].dwMillisecs = 1;
	   }
	   else
	   {
		   pDataParams->gTimers[dwTimerId].dwMillisecs = dwRegTimeCnt / 1000;
	   }
   }
   else //These are already millisecs
   {
	   //g_debug("%d ms", dwRegTimeCnt);
	   pDataParams->gTimers[dwTimerId].dwMillisecs = dwRegTimeCnt;
   }

   //Remember callback
   pDataParams->gTimers[dwTimerId].pApplicationCallback = pApplicationCallback;
   pDataParams->gTimers[dwTimerId].pContext = pContext;

   //Create source
   pDataParams->gTimers[dwTimerId].pGTimeoutSource = g_timeout_source_new(pDataParams->gTimers[dwTimerId].dwMillisecs);
   if( pDataParams->gTimers[dwTimerId].pGTimeoutSource == NULL)
   {
	   return PH_ADD_COMPCODE(PH_ERR_INSUFFICIENT_RESOURCES, PH_COMP_OSAL);
   }

   //Set callback
   g_source_set_callback(pDataParams->gTimers[dwTimerId].pGTimeoutSource, phOsal_Int_Timer_Callback, &pDataParams->gTimers[dwTimerId], NULL);

   //Attach source to context
   g_source_attach(pDataParams->gTimers[dwTimerId].pGTimeoutSource, pDataParams->pgGLibMainContext);

   return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

phStatus_t phOsal_GLib_Timer_Reset( phOsal_GLib_DataParams_t   *pDataParams,
                                    uint32_t                    dwTimerId
                                  )
{
	if ((dwTimerId > OSAL_GLIB_MAX_TIMERS) || (pDataParams->gTimers[dwTimerId].bTimerFree == TIMER_FREE))
	{
		return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_OSAL);
	}

	//Destroy the source and recreate it
	if(pDataParams->gTimers[dwTimerId].pGTimeoutSource != NULL)
	{
		g_source_destroy(pDataParams->gTimers[dwTimerId].pGTimeoutSource);

		//Create new one
		pDataParams->gTimers[dwTimerId].pGTimeoutSource = g_timeout_source_new(pDataParams->gTimers[dwTimerId].dwMillisecs);
		if( pDataParams->gTimers[dwTimerId].pGTimeoutSource == NULL)
		{
		   return PH_ADD_COMPCODE(PH_ERR_INSUFFICIENT_RESOURCES, PH_COMP_OSAL);
		}

		//Set callback
		g_source_set_callback(pDataParams->gTimers[dwTimerId].pGTimeoutSource, phOsal_Int_Timer_Callback, &pDataParams->gTimers[dwTimerId], NULL);

		//Attach source to context
		g_source_attach(pDataParams->gTimers[dwTimerId].pGTimeoutSource, pDataParams->pgGLibMainContext);

	}

	return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

phStatus_t phOsal_GLib_Timer_Stop( phOsal_GLib_DataParams_t  *pDataParams,
                                   uint32_t                   dwTimerId
                                 )
{
	if ((dwTimerId > OSAL_GLIB_MAX_TIMERS) || (pDataParams->gTimers[dwTimerId].bTimerFree == TIMER_FREE))
	{
		return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_OSAL);
	}

	//Destroy the source
	if(pDataParams->gTimers[dwTimerId].pGTimeoutSource != NULL)
	{
		//Will block till context can be acquired, so there won't be concurrent access on callback
		g_source_destroy(pDataParams->gTimers[dwTimerId].pGTimeoutSource);
	}

	pDataParams->gTimers[dwTimerId].pGTimeoutSource = NULL;

	return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}


phStatus_t phOsal_GLib_Timer_ExecCallback( phOsal_GLib_DataParams_t   *pDataParams,
                                           uint32_t                    dwTimerId
                                         )
{
	if( pDataParams->gTimers[dwTimerId].pApplicationCallback != NULL )
	{
		pDataParams->gTimers[dwTimerId].pApplicationCallback(dwTimerId, pDataParams->gTimers[dwTimerId].pContext);
	}
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

/* Wait function */
phStatus_t phOsal_GLib_Timer_Wait( phOsal_GLib_DataParams_t   *pDataParams,
                                   uint8_t                     bTimerDelayUnit,
                                   uint16_t                    wDelay
                                 )
{
	//bTimerDelayUnit: 0 -> us, 1 -> ms
	//printf("Sleeping...\r\n");
	if( bTimerDelayUnit == 1 ) //Delay in ms
	{
		g_usleep(wDelay * 1000);
	}
	else //Delays in us
	{
		g_usleep(wDelay);
	}
	//printf("Done\r\n");
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

/* Memory functions */

phStatus_t phOsal_GLib_GetMemory( phOsal_GLib_DataParams_t   *pDataParams,
                                  uint32_t                    dwLength,
                                  void                      **pMem
                                )
{
    phStatus_t status = PH_ERR_SUCCESS;

    PHOSAL_UNUSED_VARIABLE(pDataParams);
    *pMem = (void *) g_malloc(dwLength);

    if (*pMem == NULL)
        status = PH_ERR_RESOURCE_ERROR;

    return status;
}

phStatus_t phOsal_GLib_FreeMemory( phOsal_GLib_DataParams_t   *pDataParams,
                                   void                       *ptr
                                 )
{
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_OSAL);
}

/* Internal */

gboolean phOsal_Int_Timer_Callback(gpointer data)
{
	//Cast data into a phOsal_GLib_timer_t structure
	phOsal_GLib_timer_t* pTimer = (phOsal_GLib_timer_t*) data;
	//g_debug("Callback for timer %d\r\n", pTimer->dwTimerId);
	//Try to call the callback function
	if( pTimer->pApplicationCallback != NULL )
	{
		pTimer->pApplicationCallback(pTimer->dwTimerId ,pTimer->pContext);
	}

	//Return false so that the timer is not repeated
	return FALSE;
}



#endif /* NXPBUILD__PH_OSAL_GLib */
/******************************************************************************
**                            End Of File
******************************************************************************/

