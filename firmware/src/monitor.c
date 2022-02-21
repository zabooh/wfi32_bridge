/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    monitor.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "monitor.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the MONITOR_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */



MONITOR_DATA monitorData;

//uint32_t seconds = 0;
//uint32_t minutes = 0;
//uint32_t hours = 0;
//uint32_t seconds_total = 0;
//bool status_display_flag = false;
//SYS_TIME_HANDLE timer_sec_hdl;
//bool trigger_every_second = false;
//TCPIP_NET_HANDLE wlan_net_hdl;
//TCPIP_NET_HANDLE eth_net_hdl;

extern EXCEPT_MSG last_expt_msg;
extern int RFMAC_count;
extern int ETHERNET_counter;

extern TaskHandle_t hdl_Task[];
extern volatile uint32_t StackBotton[];
extern volatile uint32_t StackEnd[12];

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
/*******************************************************************************
  Function:
    void MONITOR_Initialize ( void )

  Remarks:
    See prototype in monitor.h.
 */

void MONITOR_TimerSecCallback(uintptr_t context) {
    monitorData.trigger_every_second = true;

    if (monitorData.reset_countdown > 0) {
        monitorData.reset_countdown--;
        if (monitorData.reset_countdown == 0) {
            MONITOR_Reset();
        }
    }
}

void MONITOR_Initialize(void) {
    /* Place the App state machine in its initial state. */
    monitorData.state = MONITOR_STATE_INIT;

    monitorData.seconds = 0;
    monitorData.minutes = 0;
    monitorData.hours = 0;
    monitorData.seconds_total = 0;
    monitorData.status_display_flag = false;
    monitorData.timer_sec_hdl = 0;
    monitorData.trigger_every_second = false;
    monitorData.wlan_net_hdl = NULL;
    monitorData.eth_net_hdl = NULL;
    monitorData.reset_countdown = -1;
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */

    monitorData.timer_sec_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(MONITOR_TIMER_SEC_COUNT), &MONITOR_TimerSecCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
    SYS_TIME_TimerStart(monitorData.timer_sec_hdl);
}

void MONITOR_CheckForDHCPLease(void) {
    TCPIP_NET_HANDLE netHdl = TCPIP_STACK_NetHandleGet("PIC32MZWINT");
    TCPIP_DHCPS_LEASE_HANDLE dhcpsLease = 0;
    TCPIP_DHCPS_LEASE_ENTRY dhcpsLeaseEntry;
    static TCPIP_DHCPS_LEASE_ENTRY dhcpsLeaseEntry_old;

    dhcpsLease = TCPIP_DHCPS_LeaseEntryGet(netHdl, &dhcpsLeaseEntry, dhcpsLease);
    if (dhcpsLeaseEntry_old.ipAddress.Val == dhcpsLeaseEntry.ipAddress.Val) {
        return;
    }
    if (0 != dhcpsLease) {
        SYS_CONSOLE_PRINT("\r\nConnected ETH   IP:%d.%d.%d.%d   MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                dhcpsLeaseEntry.ipAddress.v[0], dhcpsLeaseEntry.ipAddress.v[1], dhcpsLeaseEntry.ipAddress.v[2], dhcpsLeaseEntry.ipAddress.v[3],
                dhcpsLeaseEntry.hwAdd.v[0], dhcpsLeaseEntry.hwAdd.v[1], dhcpsLeaseEntry.hwAdd.v[2], dhcpsLeaseEntry.hwAdd.v[3], dhcpsLeaseEntry.hwAdd.v[4], dhcpsLeaseEntry.hwAdd.v[5]);
        dhcpsLeaseEntry_old.ipAddress.Val = dhcpsLeaseEntry.ipAddress.Val;
    }
}

void MONITOR_SetDisplayStatus(bool flag) {
    monitorData.status_display_flag = flag;
}

void MONITOR_Display_Status(void) {
    char str[256];
    HeapStats_t xHeapStats;

    if (monitorData.seconds == 60) {
        monitorData.seconds = 0;
        if (monitorData.minutes == 59) {
            monitorData.minutes = 0;
            monitorData.hours++;
        }
        monitorData.minutes++;
    }
    monitorData.seconds++;
    monitorData.seconds_total++;

    if (monitorData.status_display_flag == false) return;

    vPortGetHeapStats(&xHeapStats);

    sprintf(str,
            VT100_CURSOR_SAVE
            VT100_CURSOR_HOME
            VT100_BACKGROUND_WHITE
            VT100_FOREGROUND_BLUE
            VT100_CLR_LINE
            "Time:%02d:%02d:%02d Int:%d(RFMAC)%d(ETHERNET) Heap:%d\n\r"
            , monitorData.hours, monitorData.minutes, monitorData.seconds
            , RFMAC_count, ETHERNET_counter
            , xHeapStats.xAvailableHeapSpaceInBytes
            );

    SYS_CONSOLE_PRINT(str);

    sprintf(str,
            VT100_BACKGROUND_WHITE
            VT100_FOREGROUND_MAGNETA
            VT100_CLR_LINE
            ">>===<<"
            );

    SYS_CONSOLE_PRINT(str);

    SYS_CONSOLE_PRINT(
            VT100_BACKGROUND_BLACK
            VT100_FOREGROUND_GREEN
            VT100_CURSOR_RESTORE);


}

/******************************************************************************
  Function:
    void MONITOR_Tasks ( void )

  Remarks:
    See prototype in monitor.h.
 */

void MONITOR_Tasks(void) {
    uint32_t DeviceID;
    int ix;
    
    if (monitorData.trigger_every_second) {
        monitorData.trigger_every_second = false;
        MONITOR_Display_Status();
        MONITOR_CheckForDHCPLease();
    }

    /* Check the application's current state. */
    switch (monitorData.state) {
            /* Application's initial state. */
        case MONITOR_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized) {

                monitorData.state = MONITOR_STATE_WAIT_FOR_TCP_STACK_READY;
            }
            break;
        }

        case MONITOR_STATE_WAIT_FOR_TCP_STACK_READY:
        {
            if (TCPIP_STACK_Status(sysObj.tcpip) == SYS_STATUS_READY) {
                DeviceID = DEVID;
                SYS_CONSOLE_PRINT(
                        "======================================================\n\r");
                SYS_CONSOLE_PRINT("L2 Bridge Build Time  " __DATE__ " " __TIME__ "\n\r");
                SYS_CONSOLE_PRINT("Build Stamp 202201121109 tc1\n\r");
                SYS_CONSOLE_PRINT("Device ID: %08x\n\r", DeviceID);
                SYS_CONSOLE_PRINT("Monitor Task State Run\n\r");

                for (ix = 0; ix < 12; ix++) {
                    SYS_CONSOLE_PRINT("Task %02d Stack %08x Value %08x\n\r", ix, StackBotton[ix], StackEnd[ix]);
                }
                
                if (last_expt_msg.magic == MAGIC_CODE) {
                    SYS_CONSOLE_PRINT(VT100_TEXT_DEFAULT "\n\r!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
                    SYS_CONSOLE_PRINT(VT100_TEXT_DEFAULT "Last Runtime has ended with the following Message:\n\r");
                    {
                        char ch;
                        int ix = 0;
                        for (ix = 0; ix < 4096; ix++) {
                            ch = last_expt_msg.msg[ix];
                            if (ch == 0)break;
                            SYS_CONSOLE_PRINT("%c", ch);
                        }
                    }
                    SYS_CONSOLE_PRINT("%c", last_expt_msg.msg[0]);
                    SYS_CONSOLE_PRINT(VT100_TEXT_DEFAULT "\n\r!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
                    last_expt_msg.magic = 0;
                }
                monitorData.wlan_net_hdl = TCPIP_STACK_IndexToNet(WLAN_NET);
                monitorData.eth_net_hdl = TCPIP_STACK_IndexToNet(ETH_NET);
//                TCPIP_DHCPS_Disable(monitorData.wlan_net_hdl);
//                TCPIP_DHCPS_Disable(monitorData.eth_net_hdl);
//                TCPIP_DHCP_Disable(monitorData.wlan_net_hdl);
//                TCPIP_DHCP_Disable(monitorData.eth_net_hdl);
                monitorData.state = MONITOR_STATE_SERVICE_TASKS;
            }
            break;
        }

        case MONITOR_STATE_SERVICE_TASKS:
        {

            break;
        }

            /* TODO: implement your application state machine.*/


            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

#include <time.h>

time_t time(time_t * time_r) { /* seconds since 00:00:00 Jan 1 1970 */
    //    if(time_r){
    //        *time_r = seconds_total;
    //    }
    return monitorData.seconds_total;
}

volatile int peng1 = 1;
volatile int peng2 = 1;

void DisableAllDHCPx(void){
    int abc[2048];
    int ix;
    
    for(ix=0;ix<2048;ix++)abc[ix]=0xAFFEAFFE;    
    peng1 = abc[peng1];
    
        TCPIP_DHCPS_Disable(monitorData.wlan_net_hdl);
        TCPIP_DHCPS_Disable(monitorData.eth_net_hdl);
        TCPIP_DHCP_Disable(monitorData.wlan_net_hdl);
        TCPIP_DHCP_Disable(monitorData.eth_net_hdl);
}

void MONITOR_Wifi_Callback(uint32_t event, void * data, void *cookie) {

    switch (event) {
        case SYS_WIFI_DISCONNECT:
            SYS_CONSOLE_PRINT("WiFi Event DISCONNECT\n\r");     
            monitorData.reset_countdown = 2;
            break;
        case SYS_WIFI_CONNECT:
            SYS_CONSOLE_PRINT("WiFi Event CONNECT\n\r");;
            break;
    }

}

void MONITOR_Reset(void){
        while (1) {
        SYSKEY = 0x00000000;
        SYSKEY = 0xAA996655;
        SYSKEY = 0x556699AA;
        RSWRSTSET = _RSWRST_SWRST_MASK;
        RSWRST;
        Nop();
        Nop();
        Nop();
        Nop();
    }
}


/*******************************************************************************
 End of File
 */
