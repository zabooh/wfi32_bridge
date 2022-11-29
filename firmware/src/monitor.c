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

extern EXCEPT_MSG last_expt_msg;
extern int RFMAC_count;
extern int ETHERNET_counter;

void MONITOR_DHCP_eth_Handler(TCPIP_NET_HANDLE hNet, TCPIP_DHCP_EVENT_TYPE evType, const void* param);
void MONITOR_Print_State_Change(void);

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

    if (monitorData.dhcp_countdown > 0) {
        if(monitorData.eth_is_connected == true){
            monitorData.dhcp_countdown--;
        }
    }

    if (monitorData.wlan_alone_countdown > 0) {
        monitorData.wlan_alone_countdown--;
        if (monitorData.wlan_alone_countdown == 0) {
            TCPIP_DHCPS_Enable(monitorData.wlan_net_hdl);
            SYS_CONSOLE_PRINT("WLAN: DHCP Server Enabled\n\r");
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
    monitorData.dhcp_countdown = -1;
    monitorData.eth_event_hdl = NULL;
    monitorData.wlan_event_hdl = NULL;
    monitorData.eth_is_connected = false;
    monitorData.wlan_is_connected = false;
    monitorData.wlan_alone_countdown = 0;
            
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */

    monitorData.timer_sec_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(MONITOR_TIMER_SEC_COUNT), &MONITOR_TimerSecCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
    SYS_TIME_TimerStart(monitorData.timer_sec_hdl);
}

void MONITOR_CheckForDHCPLease(void) {
    TCPIP_NET_HANDLE eth_netHdl = TCPIP_STACK_NetHandleGet("PIC32MZWINT");
    TCPIP_DHCPS_LEASE_HANDLE eth_dhcpsLease = 0;
    TCPIP_DHCPS_LEASE_ENTRY eth_dhcpsLeaseEntry;
    static TCPIP_DHCPS_LEASE_ENTRY eth_dhcpsLeaseEntry_old;

    eth_dhcpsLease = TCPIP_DHCPS_LeaseEntryGet(eth_netHdl, &eth_dhcpsLeaseEntry, eth_dhcpsLease);
    if (eth_dhcpsLeaseEntry_old.ipAddress.Val == eth_dhcpsLeaseEntry.ipAddress.Val) {
        return;
    }
    if (0 != eth_dhcpsLease) {
        SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
        SYS_CONSOLE_PRINT("DHCP Server Lease ETH  IP:%d.%d.%d.%d   MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                eth_dhcpsLeaseEntry.ipAddress.v[0], eth_dhcpsLeaseEntry.ipAddress.v[1], eth_dhcpsLeaseEntry.ipAddress.v[2], eth_dhcpsLeaseEntry.ipAddress.v[3],
                eth_dhcpsLeaseEntry.hwAdd.v[0], eth_dhcpsLeaseEntry.hwAdd.v[1], eth_dhcpsLeaseEntry.hwAdd.v[2], eth_dhcpsLeaseEntry.hwAdd.v[3], eth_dhcpsLeaseEntry.hwAdd.v[4], eth_dhcpsLeaseEntry.hwAdd.v[5]);
        eth_dhcpsLeaseEntry_old.ipAddress.Val = eth_dhcpsLeaseEntry.ipAddress.Val;
    }
    
    
    TCPIP_NET_HANDLE wlan_netHdl = TCPIP_STACK_IndexToNet(WLAN_NET);//TCPIP_STACK_NetHandleGet("PIC32MZW1");
    TCPIP_DHCPS_LEASE_HANDLE wlan_dhcpsLease = 0;
    TCPIP_DHCPS_LEASE_ENTRY wlan_dhcpsLeaseEntry;
    static TCPIP_DHCPS_LEASE_ENTRY wlan_dhcpsLeaseEntry_old;

    wlan_dhcpsLease = TCPIP_DHCPS_LeaseEntryGet(wlan_netHdl, &wlan_dhcpsLeaseEntry, wlan_dhcpsLease);
    if (wlan_dhcpsLeaseEntry_old.ipAddress.Val == wlan_dhcpsLeaseEntry.ipAddress.Val) {
        return;
    }
    if (0 != wlan_dhcpsLease) {
         SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
        SYS_CONSOLE_PRINT("DHCP Server Lease WLAN IP:%d.%d.%d.%d   MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                wlan_dhcpsLeaseEntry.ipAddress.v[0], wlan_dhcpsLeaseEntry.ipAddress.v[1], wlan_dhcpsLeaseEntry.ipAddress.v[2], wlan_dhcpsLeaseEntry.ipAddress.v[3],
                wlan_dhcpsLeaseEntry.hwAdd.v[0], wlan_dhcpsLeaseEntry.hwAdd.v[1], wlan_dhcpsLeaseEntry.hwAdd.v[2], wlan_dhcpsLeaseEntry.hwAdd.v[3], wlan_dhcpsLeaseEntry.hwAdd.v[4], wlan_dhcpsLeaseEntry.hwAdd.v[5]);
        wlan_dhcpsLeaseEntry_old.ipAddress.Val = wlan_dhcpsLeaseEntry.ipAddress.Val;
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

    MONITOR_Print_State_Change();
    
    if (monitorData.trigger_every_second) {
        monitorData.trigger_every_second = false;
        MONITOR_Display_Status();
        //MONITOR_CheckForDHCPLease();
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
                SYS_CONSOLE_PRINT("Branch: MAC_Copy tc2\n\r");
                SYS_CONSOLE_PRINT("Device ID: %08x\n\r", DeviceID);
                SYS_CONSOLE_PRINT("Monitor Task State Run\n\r");
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
                SYS_CONSOLE_PRINT("wlan0 net handle:%08x\n\r",monitorData.wlan_net_hdl);
                SYS_CONSOLE_PRINT("eth0 net handle:%08x\n\r",monitorData.eth_net_hdl);
                monitorData.eth_event_hdl = TCPIP_STACK_HandlerRegister(monitorData.eth_net_hdl, TCPIP_EV_CONN_ALL, MONITOR_TcpipStack_EventHandler, NULL);
                monitorData.wlan_event_hdl = TCPIP_STACK_HandlerRegister(monitorData.wlan_net_hdl, TCPIP_EV_CONN_ALL, MONITOR_TcpipStack_EventHandler, NULL);                
                monitorData.dhcp_countdown = 11;                
                TCPIP_DHCP_HandlerRegister(monitorData.eth_net_hdl, MONITOR_DHCP_eth_Handler, &monitorData.dhcp_eth_hParam);
                monitorData.state = MONITOR_STATE_WAIT_FOR_ALL_NETS_UP; 
            }
            break;
        }
        
        case MONITOR_STATE_WAIT_FOR_ALL_NETS_UP:
            if(monitorData.eth_is_connected && monitorData.wlan_is_connected){
                monitorData.state = MONITOR_STATE_WAIT_COPY_MAC;
                monitorData.wlan_alone_countdown = 0;
            }
            break;
                 
        case MONITOR_STATE_WAIT_COPY_MAC:
        {
            TCPIP_MAC_ADDR* pMac;
            TCPIP_MAC_ADDR macAddr;            
            
            /* copy WLAN MAC address to ETH  */
            pMac = (TCPIP_MAC_ADDR*)TCPIP_STACK_NetAddressMac(monitorData.eth_net_hdl);
            TCPIP_Helper_MACAddressToString(pMac, monitorData.EthMACAddrBuff, sizeof(monitorData.EthMACAddrBuff));
            SYS_CONSOLE_PRINT("ETH MAC Address: %s\r\n", monitorData.EthMACAddrBuff);            
            pMac = (TCPIP_MAC_ADDR*)TCPIP_STACK_NetAddressMac(monitorData.wlan_net_hdl);
            TCPIP_Helper_MACAddressToString(pMac, monitorData.WlanMACAddrBuff, sizeof(monitorData.WlanMACAddrBuff));
            SYS_CONSOLE_PRINT("WLAN MAC Address: %s\r\n", monitorData.WlanMACAddrBuff);            
            TCPIP_Helper_StringToMACAddress(monitorData.WlanMACAddrBuff, macAddr.v);            
            TCPIP_STACK_NetAddressMacSet(monitorData.eth_net_hdl, &macAddr);
            
            TCPIP_DHCP_Enable(monitorData.eth_net_hdl);
            monitorData.state = MONITOR_STATE_WAIT_FOR_DHCP;
        }
            break;
        
            
        case MONITOR_STATE_WAIT_FOR_DHCP:
        {
            IPV4_ADDR ipAddr;          
            IPV4_ADDR  ipMask;
            TCPIP_MAC_ADDR macAddr;
            if (TCPIP_DHCP_IsBound(monitorData.eth_net_hdl) == true) {                
                
                /* copy new IP Address for ETH to WLAN*/
                ipAddr.Val = TCPIP_STACK_NetAddress(monitorData.eth_net_hdl);
                TCPIP_Helper_IPAddressToString(&ipAddr, monitorData.EthIPAddrBuff, sizeof (monitorData.EthIPAddrBuff));
                ipMask.Val = TCPIP_STACK_NetMask(monitorData.eth_net_hdl);
                /* copy new Netmask for ETH to WLAN*/
                TCPIP_Helper_StringToIPAddress(monitorData.EthIPAddrBuff,&ipAddr);
                TCPIP_STACK_NetAddressSet(monitorData.wlan_net_hdl, &ipAddr, &ipMask, true);
                /* Restore original MAC Address of ETH*/                
                TCPIP_Helper_StringToMACAddress(monitorData.EthMACAddrBuff, macAddr.v);
                TCPIP_STACK_NetAddressMacSet(monitorData.eth_net_hdl, &macAddr);                            
                /* Restart DHCP Client on ETH */
                TCPIP_DHCP_Disable(monitorData.eth_net_hdl);
                TCPIP_DHCP_Enable(monitorData.eth_net_hdl);
                
                monitorData.state = MONITOR_STATE_SERVICE_TASKS;
            }
            if(monitorData.dhcp_countdown==0){
                SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
                TCPIP_Helper_StringToMACAddress(monitorData.EthMACAddrBuff, macAddr.v);
                TCPIP_STACK_NetAddressMacSet(monitorData.eth_net_hdl, &macAddr);                
                TCPIP_DHCP_Disable(monitorData.eth_net_hdl);
                SYS_CONSOLE_PRINT("ETH: DHCP Client Disabled\n\r");
                TCPIP_DHCPS_Enable(monitorData.eth_net_hdl);                
                SYS_CONSOLE_PRINT("ETH: DHCP Server Enabled\n\r");
                TCPIP_DHCPS_Enable(monitorData.wlan_net_hdl);                
                SYS_CONSOLE_PRINT("WLAN: DHCP Server Enabled\n\r");                
                SYS_CONSOLE_PRINT("Connected to Host\n\r");
                monitorData.state = MONITOR_STATE_SERVICE_TASKS;
            }            
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

void DisableAllDHCPx(void){
    
        TCPIP_DHCPS_Disable(monitorData.wlan_net_hdl);
        TCPIP_DHCPS_Disable(monitorData.eth_net_hdl);
        TCPIP_DHCP_Disable(monitorData.wlan_net_hdl);
        TCPIP_DHCP_Disable(monitorData.eth_net_hdl);
}

void MONITOR_Wifi_Callback(uint32_t event, void * data, void *cookie) {

    switch (event) {
        case SYS_WIFI_DISCONNECT:
            SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
            SYS_CONSOLE_PRINT("WiFi Event DISCONNECT\n\r");     
            monitorData.reset_countdown = 2;
            break;
        case SYS_WIFI_CONNECT:
            SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
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

bool MONITOR_Check_For_New_DHCP_Client_Lease(TCPIP_NET_HANDLE net_hdl, IPV4_ADDR *prev_ip_addr) {
    IPV4_ADDR ip_addr;
    bool ret_val = false;

    ip_addr.Val = TCPIP_STACK_NetAddress(net_hdl);
    if (prev_ip_addr->Val != ip_addr.Val) {
        prev_ip_addr->Val = ip_addr.Val;
        IPV4_ADDR ip_default_addr;
        TCPIP_Helper_StringToIPAddress(TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX1, &ip_default_addr);

        if (ip_addr.Val != ip_default_addr.Val) {
            SYS_CONSOLE_PRINT("%s new IP Address: ", TCPIP_STACK_NetNameGet(net_hdl));
            SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ip_addr.v[0], ip_addr.v[1], ip_addr.v[2], ip_addr.v[3]);
            ret_val = true;
        }
    }

    return ret_val;
}

void MONITOR_Arp_Scan(void) {
    IPV4_ADDR ip_addr;
    TCPIP_MAC_ADDR mac_addr;
    uint32_t ix;
    char MacAddrBuff[20];
    char IPAddrBuff[20];

    SYS_CONSOLE_PRINT("ARP scan started\n\r");

    /* get the current IP address of the interface */
    ip_addr.Val = TCPIP_STACK_NetAddress(monitorData.eth_net_hdl);

    /* scan through all addresses from x.x.x.1 to x.x.x.255 */
    for (ix = 1; ix < 255; ix++) {
        ip_addr.v[3] = ix;
        /* send ARP request */
        TCPIP_ARP_Resolve(monitorData.eth_net_hdl, &ip_addr);
        /* wait 20 miili seconds for a response */
        vTaskDelay(20 / portTICK_PERIOD_MS);
        if (TCPIP_ARP_IsResolved(monitorData.eth_net_hdl, &ip_addr, &mac_addr) == true) {
            /* when host responses, display MAC and IP */
            TCPIP_Helper_MACAddressToString(&mac_addr, MacAddrBuff, sizeof (MacAddrBuff));
            TCPIP_Helper_IPAddressToString(&ip_addr, IPAddrBuff, sizeof (IPAddrBuff));
            SYS_CONSOLE_PRINT("%s  %s\n\r", MacAddrBuff, IPAddrBuff);
        }
        /* in all cases delete the ARP request from the cache 
         * so that no retry or cache overrun could occur 
         */
        TCPIP_ARP_EntryRemove(monitorData.eth_net_hdl, &ip_addr);
    }

}

void MONITOR_TcpipStack_EventHandler(TCPIP_NET_HANDLE hNet, TCPIP_EVENT event, const void *fParam) {
    const char *netName = TCPIP_STACK_NetNameGet(hNet);
    
    SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
    SYS_CONSOLE_PRINT("TCP Stack Event Handler %s - %x - ", netName, event);
    if (event & TCPIP_EV_CONN_ESTABLISHED) {
        SYS_CONSOLE_PRINT("connection established ");
        if (hNet == monitorData.eth_net_hdl) {
             monitorData.eth_is_connected = true;
             SYS_CONSOLE_PRINT(" Ethernet\r\n");
        } else if (hNet == monitorData.wlan_net_hdl) {
            monitorData.wlan_is_connected = true;
            monitorData.wlan_alone_countdown = 10;
            SYS_CONSOLE_PRINT("Wlan \r\n");
        }
    } else if (event & TCPIP_EV_CONN_LOST) {
        SYS_CONSOLE_PRINT("connection lost ");
        if (hNet == monitorData.eth_net_hdl) {
            monitorData.eth_is_connected = false;
            monitorData.reset_countdown = 2;
            SYS_CONSOLE_PRINT(" Ethernet\r\n");
        } else if (hNet == monitorData.wlan_net_hdl) {
            SYS_CONSOLE_PRINT("Wlan \r\n");
        }
    } else {
        SYS_CONSOLE_PRINT("TCP Stack Event Handler %s Unknown event = %d\r\n", netName, event);
    }

}

void MONITOR_DHCP_eth_Handler(TCPIP_NET_HANDLE hNet, TCPIP_DHCP_EVENT_TYPE evType, const void* param) {

    SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
    SYS_CONSOLE_PRINT("%s - ", TCPIP_STACK_NetNameGet(hNet));

    switch (evType) {
        case DHCP_EVENT_NONE: SYS_CONSOLE_PRINT("DHCP_EVENT_NONE\n\r");
            break;
        case DHCP_EVENT_DISCOVER: SYS_CONSOLE_PRINT("DHCP_EVENT_DISCOVER\n\r");
            break;
        case DHCP_EVENT_REQUEST: SYS_CONSOLE_PRINT("DHCP_EVENT_REQUEST\n\r");
            break;
        case DHCP_EVENT_ACK: SYS_CONSOLE_PRINT("DHCP_EVENT_ACK\n\r");
            break;
        case DHCP_EVENT_ACK_INVALID: SYS_CONSOLE_PRINT("DHCP_EVENT_ACK_INVALID\n\r");
            break;
        case DHCP_EVENT_DECLINE: SYS_CONSOLE_PRINT("DHCP_EVENT_DECLINE\n\r");
            break;
        case DHCP_EVENT_NACK: SYS_CONSOLE_PRINT("DHCP_EVENT_NACK\n\r");
            break;
        case DHCP_EVENT_TIMEOUT: SYS_CONSOLE_PRINT("DHCP_EVENT_TIMEOUT\n\r");
            break;
        case DHCP_EVENT_BOUND: SYS_CONSOLE_PRINT("DHCP_EVENT_BOUND\n\r");
            break;
        case DHCP_EVENT_REQUEST_RENEW: SYS_CONSOLE_PRINT("DHCP_EVENT_REQUEST_RENEW\n\r");
            break;
        case DHCP_EVENT_REQUEST_REBIND: SYS_CONSOLE_PRINT("DHCP_EVENT_REQUEST_REBIND\n\r");
            break;
        case DHCP_EVENT_CONN_LOST: SYS_CONSOLE_PRINT("DHCP_EVENT_CONN_LOST\n\r");
            break;
        case DHCP_EVENT_CONN_ESTABLISHED: SYS_CONSOLE_PRINT("DHCP_EVENT_CONN_ESTABLISHED\n\r");
            break;
        case DHCP_EVENT_SERVICE_DISABLED: SYS_CONSOLE_PRINT("DHCP_EVENT_SERVICE_DISABLED\n\r");
            break;

    }
}


char *states_str[] = {
    "MONITOR_STATE_INIT",
    "MONITOR_STATE_WAIT_FOR_TCP_STACK_READY",
    "MONITOR_STATE_WAIT_FOR_DHCP",
    "MONITOR_STATE_WAIT_FOR_ALL_NETS_UP",
    "MONITOR_STATE_WAIT_COPY_MAC",
    "MONITOR_STATE_SERVICE_TASKS"
};

void MONITOR_Print_State_Change(void) {
    static MONITOR_STATES states = MONITOR_STATE_EMPTY;
    if (states != monitorData.state) {
        states = monitorData.state;
        SYS_CONSOLE_PRINT("%02d:%02d:%02d  ", monitorData.hours, monitorData.minutes, monitorData.seconds);
        SYS_CONSOLE_PRINT("New Monitor State: %s\n\r", states_str[states]);
    }
}


                
/*******************************************************************************
 End of File
 */
