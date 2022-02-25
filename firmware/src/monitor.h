/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    monitor.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "MONITOR_Initialize" and "MONITOR_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "MONITOR_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _MONITOR_H
#define _MONITOR_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"
#include "vt100.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
    // DOM-IGNORE-END

    // *****************************************************************************
    // *****************************************************************************
    // Section: Type Definitions
    // *****************************************************************************
    // *****************************************************************************

#define MONITOR_TIMER_SEC_COUNT      1000
#define MONITOR_DELAY_1_SECOND       (1*(1000/MONITOR_TIMER_SEC_COUNT))
#define MONITOR_DELAY_2_SECONDS      (2*(1000/MONITOR_TIMER_SEC_COUNT)) 

#define WLAN_NET 0
#define ETH_NET  1

void MONITOR_Wifi_Callback(uint32_t event, void * data, void *cookie);
bool MONITOR_Check_For_New_DHCP_Client_Lease(TCPIP_NET_HANDLE net_hdl, IPV4_ADDR *prev_ip_addr);

    // *****************************************************************************

    /* Application states

      Summary:
        Application states enumeration

      Description:
        This enumeration defines the valid application states.  These states
        determine the behavior of the application at various times.
     */

    typedef enum {
        /* Application's state machine's initial state. */
        MONITOR_STATE_INIT = 0,
        MONITOR_STATE_WAIT_FOR_TCP_STACK_READY,
        MONITOR_STATE_WAIT_FOR_DHCP,
        MONITOR_STATE_SERVICE_TASKS,
        /* TODO: Define states used by the application state machine. */

    } MONITOR_STATES;


    // *****************************************************************************

    /* Application Data

      Summary:
        Holds application data

      Description:
        This structure holds the application's data.

      Remarks:
        Application strings and buffers are be defined outside this structure.
     */

    typedef struct {
        /* The application's current state */
        MONITOR_STATES state;

        uint32_t seconds;
        uint32_t minutes;
        uint32_t hours;
        uint32_t seconds_total;
        bool status_display_flag;
        SYS_TIME_HANDLE timer_sec_hdl;
        bool trigger_every_second;
        TCPIP_NET_HANDLE wlan_net_hdl;
        TCPIP_NET_HANDLE eth_net_hdl;
        IPV4_ADDR eth_ip_addr;
        int32_t reset_countdown;
        int32_t dhcp_countdown;        
        TCPIP_EVENT_HANDLE eth_event_hdl;
        TCPIP_EVENT_HANDLE wlan_event_hdl;
        const void* dhcp_eth_hParam;
    } MONITOR_DATA;

    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Callback Routines
    // *****************************************************************************
    // *****************************************************************************
    /* These routines are called by drivers when certain events occur.
     */

    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Initialization and State Machine Functions
    // *****************************************************************************
    // *****************************************************************************

    /*******************************************************************************
      Function:
        void MONITOR_Initialize ( void )

      Summary:
         MPLAB Harmony application initialization routine.

      Description:
        This function initializes the Harmony application.  It places the
        application in its initial state and prepares it to run so that its
        MONITOR_Tasks function can be called.

      Precondition:
        All other system initialization routines should be called before calling
        this routine (in "SYS_Initialize").

      Parameters:
        None.

      Returns:
        None.

      Example:
        <code>
        MONITOR_Initialize();
        </code>

      Remarks:
        This routine must be called from the SYS_Initialize function.
     */

    void MONITOR_Initialize(void);


    /*******************************************************************************
      Function:
        void MONITOR_Tasks ( void )

      Summary:
        MPLAB Harmony Demo application tasks function

      Description:
        This routine is the Harmony Demo application's tasks function.  It
        defines the application's state machine and core logic.

      Precondition:
        The system and application initialization ("SYS_Initialize") should be
        called before calling this.

      Parameters:
        None.

      Returns:
        None.

      Example:
        <code>
        MONITOR_Tasks();
        </code>

      Remarks:
        This routine must be called from SYS_Tasks() routine.
     */

    void MONITOR_Tasks(void);

    void MONITOR_Reset(void);
    
    void MONITOR_Print_Time(void);
    
    void MONITOR_TcpipStack_EventHandler(TCPIP_NET_HANDLE hNet, TCPIP_EVENT event, const void *fParam);
    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _MONITOR_H */

/*******************************************************************************
 End of File
 */

