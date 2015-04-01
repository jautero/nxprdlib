/**
 * \file phOsal_GLib.h
 * \author Donatien Garnier
 */

#ifndef PHOSAL_GLIB_H_
#define PHOSAL_GLIB_H_

/** \defgroup phOsal_GLib Component : GLib
* \brief These Components implement the OSAL commands.
* @{
*/

#include <glib.h>
#include "phOsal.h"

//TODO make sure unique
#define PH_OSAL_GLIB_ID                 0x08U      /**< ID for GLIB osal component */

#define OSAL_GLIB_MAX_TIMERS 				4		/**< Maximum number of timers */
#define TIMER_USED                         1U         /**< Indicates that the timer is used */
#define TIMER_FREE                         0U         /**< Indicates that the timer is free */

//#define TIMER_RUNNING					1U
//#define TIMER_STOPPED					0U

typedef struct
{
    uint32_t       dwTimerId;               /**< ID of the timer */
    GSource*	   pGTimeoutSource;
	uint32_t	   dwMillisecs;				/**< Millisecs after which the callback will be called */
    uint8_t        bTimerFree;             /**< Indicates whether the current timer is free or used */
	//uint8_t		   bTimerRunning;			/**< Whether the timer is running */
    ppCallBck_t    pApplicationCallback;   /**< The call back function for this timer */
    void*          pContext;               /**< The argument to the call back function */
} phOsal_GLib_timer_t;

/**
* \brief GLib OSAL parameter stucture
*/
typedef struct
{
    uint16_t           wId;                            /**< ID of this component, do not modify */
    phOsal_GLib_timer_t gTimers[OSAL_GLIB_MAX_TIMERS];		/**< Timer structures */
    GMainContext*		pgGLibMainContext;
} phOsal_GLib_DataParams_t;

//This function MUST be called by user
void phOsal_GLib_Init_GLib( phOsal_GLib_DataParams_t  *pDataParams, GMainContext* pgGLibMainContext );

/**
* Starts the timer with set to expire after \a dwRegTimeCnt and
* calls the callback funnction ppApplicationCallback once timer expires.
*
* \return   Status code
* \retval   #PH_OSAL_ERR_INVALID_TIMER If timer ID supplied was invalid
* \retval   #PH_ERR_SUCCESS            on success
*
*/

phStatus_t phOsal_GLib_Timer_Start( phOsal_GLib_DataParams_t   *pDataParams,     /**<[In] pointer to this layer's data structure */
                                      uint32_t                        dwTimerId,       /**<[In] Valid timer ID as returned by phOsal_Timer_Create() */
                                      uint32_t                        dwRegTimeCnt,    /**<[In] The required amount of time out */
                                      uint16_t                        wOption,         /**<[In] Option for time out value in MS or US */
                                      ppCallBck_t                     pApplicationCallback, /**<[In] Pointer to the call back function that will be called once timer expires */
                                      void                            *pContext        /**<[In] Argument with which the call back function will be called */
                                      );

/**
* Stops the said timer.
* This function does not free the timer. It only disables the timer.
* Use phOsal_Timer_Delete() to free the timer.
*
* \return  status code
* \retval  #PH_OSAL_ERR_INVALID_TIMER if the timer ID supplied was invalid
* \retval  #PH_ERR_SUCCESS            on success
*
*/

phStatus_t phOsal_GLib_Timer_Stop( phOsal_GLib_DataParams_t   *pDataParams,   /**<[In] Pointer to this layer's data structure */
                                     uint32_t                       dwTimerId      /**<[In] Id of the timer to be stopped */
                                     );

/**
* Frees the timer.
* When this function is called, the timer with given ID is returned to the
* free timer pool.
*
* \return  status code
* \retval  #PH_OSAL_ERR_INVALID_TIMER if the timer ID supplied was invalid
* \retval  #PH_ERR_SUCCESS            on success
*
*/
phStatus_t phOsal_GLib_Timer_Delete( phOsal_GLib_DataParams_t   *pDataParams,   /**<[In] Pointer to this layer's data structure */
                                       uint32_t                        dwTimerId      /**<[In] Timer ID */
                                       );

/**
* \brief initialises timers to the application
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/

phStatus_t phOsal_GLib_Timer_Init( phOsal_GLib_DataParams_t  *pDataParams );

/**
* Creates a timer component.
* Internally, an array is maintained which stores timers along with
* information as to whether the timer is available or not. This
* function searches a free timer that is available and returns the
* timer id in \a pTimerId. If there are no free timers,
* then appropriate error code is returned.
*
* \return   status code
* \retval   #PH_ERR_SUCCESS on success
* \retval   #PH_OSAL_ERR_NO_FREE_TIMER if no timers are available
*
*/

phStatus_t phOsal_GLib_Timer_Create( phOsal_GLib_DataParams_t   *pDataParams,  /**<[In]  DataParams representing this component */
                                       uint32_t                      *pTimerId      /**<[Out] Timer ID that was selected in case of success or else -1 */
                                       );

/**
* Delays the execution thread by given amount of delay.
* This function is capable of delaying the execution in terms of Milli
* seconds as well in terms of micro seconds
*
* \return   status code
* \retval   #PH_ERR_SUCCESS
*
*/

phStatus_t phOsal_GLib_Timer_Wait( phOsal_GLib_DataParams_t  *pDataParams,       /**<[In] Data structure representing this component */
                                     uint8_t                        bTimerDelayUnit,   /**<[In] The time unit in which delay is represented */
                                     uint16_t                       wDelay             /**<{in] Amount of delay by which execution is to be delayed */
                                     );

/**
* Timer Reset
* \return   status code
* \retval   #PH_ERR_SUCCESS
*
*/

phStatus_t phOsal_GLib_Timer_Reset( phOsal_GLib_DataParams_t   *pDataParams,   /**<[In]  DataParams representing this component */
                                      uint32_t                        pTimerId       /**<[In] Timer ID that was selected in case of success or else -1 */
                                      );

/**
* Timer ExecCallback
*
*/
phStatus_t phOsal_GLib_Timer_ExecCallback( phOsal_GLib_DataParams_t  *pDataParams,
                                             uint32_t                       dwTimerId
                                             );

/**
* \brief Get memory from the heap segment
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/

phStatus_t phOsal_GLib_GetMemory( phOsal_GLib_DataParams_t   *pDataParams,   /**< [In] Pointer to this layers parameter structure. */
                                    uint32_t                        dwLength,      /**< [In] Required memory length */
                                    void                          **pMem           /**< [Out] Pointer to Memory allocated */
                                    );

/**
* \brief Free the memory allocated
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/

phStatus_t phOsal_GLib_FreeMemory( phOsal_GLib_DataParams_t  *pDataParams,   /**< [In] Pointer to this layers parameter structure. */
                                     void                          *ptr            /**< [In] Pointer to memory to be freed */
                                     );

/**
 * @}
 */

#endif /* PHOSAL_GLIB_H_ */
