/*
 *  Copyright STREAMIT BV, 2010.
 *
 *  Project             : SIR
 *  Module              : Session
 *  File name  $Workfile: Session.c  $
 *       Last Save $Date: 2003/08/16  $
 *             $Revision: 0.1  $
 *  Creation Date       : 2003/08/16
 *
 *  Description         : Handles the connection to the Internet via
 *                        ethernet or modem/ppp
 *
 */

#define LOG_MODULE  LOG_SESSION_MODULE

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include <io.h>
#include <fcntl.h>

#include <dev/nicrtl.h>
#include <dev/uartavr.h>
#include <dev/ppp.h>

#include <sys/heap.h>
#include <sys/thread.h>
#include <sys/timer.h>
#include <sys/confnet.h>

#include <netdb.h>
#include <net/route.h>

#include <arpa/inet.h>

#include <pro/httpd.h>
#include <pro/dhcp.h>

#ifdef NUTDEBUG
    #include <sys/osdebug.h>
    #include <net/netdebug.h>
#endif

#include <sys/confos.h>

//#pragma text:appcode

#include "system.h"
#include "session.h"
#include "log.h"
//#include "settings.h"
#include "display.h"
#include "version.h"

/*!
 * \addtogroup Session
 */

/*@{*/

/*--------------------------------------------------------------------------*/
/*  Constant definitions                                                    */
/*--------------------------------------------------------------------------*/

/*!\brief Ethernet chip definitions */
#define ETH0_BASE   0xC300
#define ETH0_IRQ    5

/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/
/*!\brief State of this module */
typedef enum T_SESSION_STATE
{
    STATE_IDLE = 0,                 /* We are idle */
    STATE_STARTING,                 /* We're setting up the session */
    STATE_OPEN,                     /* We have a session */
    STATE_STOPPING                  /* We're stopping */
} TStreamerState;

/*--------------------------------------------------------------------------*/
/*  Local variables                                                         */
/*--------------------------------------------------------------------------*/
/*!\brief Uart device */
static FILE *g_pUart;

/*!\brief Name of the ethernet device */
static char szEthernetIfName[sizeof(devEth0.dev_name)];

/*!\brief Global error code. */
static TError           g_tError;


/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Local functions                                                         */
/*--------------------------------------------------------------------------*/
static void SetDhcpDnsServers(u_long dwDns1, u_long dwDns2);
static void SetFixedDnsServers(void);
static INLINE TError NetConfig(CONST char *szIfName);


/*!
 * \brief Set and save DNS server settings
 *
 * If no servers are specified, NutOS is asked
 * for the DNS servers. If NutOS doesn't have any,
 * we retrieve the previously saved servers
 * Finally, the servers that are current, are stored
 * in the settings and NutOS itself
 *
 * \param dwDns1 IP address of first DNS server
 * \param dwDns2 IP address of second DNS server
 *
 * \return -
 */
static void SetDhcpDnsServers(u_long dwDns1, u_long dwDns2)
{
    // //char szDns[sizeof(SETTINGS_POINTER->Isp.szDns1)];

    // /*
     // * If not specified get current DNS' from NutOs
     // */
    // if ((dwDns1 == 0) && (dwDns2 == 0))
    // {
        // NutGetDnsServers(&dwDns1, &dwDns2);
    // }

    // /*
     // * If still no DNS servers, get previously saved config
     // */
    // if ((dwDns1 == 0) && (dwDns2 == 0))
    // {
        // //SettingsGet(szDns, &SETTINGS_POINTER->Isp.szDns1, sizeof(szDns));
        // if ((dwDns1 = inet_addr(szDns)) == (u_long)-1)
        // {
            // dwDns1 = 0;
        // }
        // //SettingsGet(szDns, &SETTINGS_POINTER->Isp.szDns2, sizeof(szDns));
        // if ((dwDns2 = inet_addr(szDns)) == (u_long)-1)
        // {
            // dwDns2 = 0;
        // }
    // }

    // /*
     // * Save DNS servers and let NutOs use them
     // */
    // if ((dwDns1 != 0) || (dwDns2 != 0))
    // {
        // strcpy(szDns, inet_ntoa(dwDns1));
        // //SettingsSet(szDns, &SETTINGS_POINTER->Isp.szDns1, sizeof(szDns));
        // strcpy(szDns, inet_ntoa(dwDns2));
        // //SettingsSet(szDns, &SETTINGS_POINTER->Isp.szDns2, sizeof(szDns));

        // NutDnsConfig2(0, 0, dwDns1, dwDns2);
    // }
}

static void SetFixedDnsServers(void)
{
    // u_long dwDns1, dwDns2;
    // char szDns[sizeof(SETTINGS_POINTER->Isp.szDns1)];

    // //SettingsGet(szDns, &SETTINGS_POINTER->Isp.szDns1, sizeof(szDns));
    // if ((dwDns1 = inet_addr(szDns)) == (u_long)-1)
    // {
        // dwDns1 = 0;
    // }

    // //SettingsGet(szDns, &SETTINGS_POINTER->Isp.szDns2, sizeof(szDns));
    // if ((dwDns2 = inet_addr(szDns)) == (u_long)-1)
    // {
        // dwDns2 = 0;
    // }

    // NutDnsConfig2(0, 0, dwDns1, dwDns2);
}


static void TryGetDhcp(u_long timeout)
{
    /*
     * Make sure DHCP is started by setting the
     * *CONFIGURED* IP address to zero.
     * AND erase the previous IP address, just in case
     * our MAC address changed.
     */
    confnet.cdn_cip_addr = 0;
    confnet.cdn_ip_addr = 0;
    confnet.cdn_ip_mask = 0;

#ifdef NUTDEBUG
    NutTraceTcp(stdout, 1);
    NutTraceHeap(stdout, 0); // doesn't function !!
    NutTraceOs(stdout, 0); // doesn't function !!
#endif

    /*
     * Start DHCP, and wait for the answer (or timeout)
     */
    LogMsg_P(LOG_DEBUG, PSTR("DHCP client started"));
    if (NutDhcpIfConfig(szEthernetIfName, confnet.cdn_mac, timeout))
    {
        LogMsg_P(LOG_DEBUG, PSTR("No DHCP address retrieved"));
    }
    else
    {
        LogMsg_P(LOG_INFO, PSTR("Ethernet interface %s ready"), inet_ntoa(confnet.cdn_ip_addr));

#ifdef NUTDEBUG
        NutTraceTcp(stdout, 0);
        NutTraceHeap(stdout, 0);
        NutTraceOs(stdout, 0);
#endif
    }
}

/*!
 * \brief Configures the ethernet interface
 *
 * Send out a DHCP request.
 * An error is returned if no response from a DHCP server
 * was received
 *
 * \param szIfName Name of the device.
 *
 * \return OK if success, TError otherwise
 */
static INLINE TError NetConfig(CONST char *szIfName)
{
    // u_long ulMac;
    // u_long ulSerialNumber;
    // u_long ulIpAddress;
    // u_char byTempValue;
    // char szIp[sizeof(SETTINGS_POINTER->Isp.szIp)];

    // LogMsg_P(LOG_DEBUG, PSTR("Configuring ethernet %s"), szIfName);

    // /*
     // * LAN configuration using EEPROM values or DHCP/ARP method.
     // * If it fails, use fixed values.
     // */
    // if (NutNetLoadConfig(szIfName) != 0)
    // {
        // /*
         // * No previous config, ignore
         // */
    // }

    // /*
     // * Override any previously used MAC address by
     // * the one from our own setup
     // *
     // * The MAC address is 00:xx:xx:0y:yy:yy
     // * where x = 4 digits from the IEEE assigned adres
     // *       y = 5 digits from our serial number
     // */
    // ulMac = SettingsGetMacIeee();
    // ulMac = __byte_swap4(ulMac) >> 8;
    // memcpy(confnet.cdn_mac, &ulMac, sizeof(confnet.cdn_mac)/2);

    // //ulSerialNumber = SettingsGetSerialnumber();
    // ulSerialNumber = __byte_swap4(ulSerialNumber) >> 8;
    // memcpy(&confnet.cdn_mac[sizeof(confnet.cdn_mac)/2], &ulSerialNumber, sizeof(confnet.cdn_mac)/2);

    // LogMsg_P(LOG_INFO, PSTR("MAC address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"),
             // confnet.cdn_mac[0],
             // confnet.cdn_mac[1],
             // confnet.cdn_mac[2],
             // confnet.cdn_mac[3],
             // confnet.cdn_mac[4],
             // confnet.cdn_mac[5]);

    // /*
     // * Save the new MAC address
     // */
    // NutNetSaveConfig();

    // /*
     // * Bring up the network.
     // * Use fixed settings if DHCP is disabled.
     // * If the fixed IP address is invalid use DHCP anyway.
     // */
    // //SettingsGet(szIp, &SETTINGS_POINTER->Isp.szIp, sizeof(szIp));
    // ulIpAddress = inet_addr(szIp);
    // if (ulIpAddress == -1)
    // {
        // ulIpAddress = 0;
    // }

    // //SettingsGet(&byTempValue, &SETTINGS_POINTER->Isp.bDhcp, sizeof(byTempValue));
    // if ((byTempValue == 0) &&
        // (ulIpAddress != 0))
    // {
        // /*
         // * Use fixed settings.
         // */
        // LogMsg_P(LOG_INFO, PSTR("Fixed IP address used"));
        // confnet.cdn_cip_addr = inet_addr(szIp);

        // //SettingsGet(szIp, &SETTINGS_POINTER->Isp.szGateway, sizeof(szIp));
        // confnet.cdn_gateway = inet_addr(szIp);
        // if (confnet.cdn_gateway == -1)
        // {
            // confnet.cdn_gateway = 0;
        // }
        // //SettingsGet(szIp, &SETTINGS_POINTER->Isp.szNetmask, sizeof(szIp));
        // confnet.cdn_ip_mask = inet_addr(szIp);
        // if (confnet.cdn_ip_mask == -1)
        // {
            // confnet.cdn_ip_mask = 0;
        // }

        // if (NutNetIfConfig(szIfName, confnet.cdn_mac, confnet.cdn_cip_addr, confnet.cdn_ip_mask) == 0)
        // {
            // NUTDEVICE *dev;

            // /*
             // * Add the default route
             // */
            // if ((dev = NutDeviceLookup(szIfName)) != 0 && dev->dev_type == IFTYP_NET)
            // {
                // NutIpRouteAdd(0, 0, confnet.cdn_gateway, dev);
            // }

            // LogMsg_P(LOG_INFO, PSTR("Ethernet interface %s ready"), inet_ntoa(confnet.cdn_ip_addr));
        // }
        // else
        // {
            // LogMsg_P(LOG_ERR, PSTR("Incorrect static Ip settings"));
        // }
        // SetFixedDnsServers();
    // }
    // else
    // {
        // /*
         // * Use DHCP.
         // */
        // TryGetDhcp(20000L);
        // SetDhcpDnsServers(0,0);
    // }
    return(OK);
}

/*!
 * \brief Starts the ethernet interface
 *
 * Waits for DHCP to finish configuring the interface
 * If that doesn't happen the interface is configured
 * using the previous settings.
 *
 * \return OK if success, TError otherwise
 */
static INLINE TError StartNet(void)
{
    TError tError = OK;
    u_char byTempValue = 0;

    /*
     * Check if we use DHCP
     */
    //SettingsGet(&byTempValue, &SETTINGS_POINTER->Isp.bDhcp, sizeof(byTempValue));
    if (byTempValue != 0)
    {
        if (NutDhcpIsConfigured() == 0)
        {
            if (NutNetLoadConfig(szEthernetIfName) ||
                NutNetIfConfig(szEthernetIfName, confnet.cdn_mac, confnet.cdn_ip_addr, confnet.cdn_ip_mask))
            {
                LogMsg_P(LOG_ERR, PSTR("No usable network"));
                tError = SESSION_NODHCP_NOEEPROM;
            }
            else
            {
                NUTDEVICE *dev;

                /*
                 * Add the default route
                 */
                if ((dev = NutDeviceLookup(szEthernetIfName)) != 0 && dev->dev_type == IFTYP_NET)
                {
                    NutIpRouteAdd(0, 0, confnet.cdn_gateway, dev);
                }

                LogMsg_P(LOG_WARNING, PSTR("No DHCP response, trying previous network settings..."));
            }
            /*
             * Set current DNS servers
             */
            SetDhcpDnsServers(0, 0);
        }
    }


    /*
     * Stop all debugging
     */
#ifdef NUTDEBUG
    NutTraceTcp(stdout, 0);
    NutTraceHeap(stdout, 0);
    NutTraceOs(stdout, 0);
#endif

    return(tError);
}

/*--------------------------------------------------------------------------*/
/*  Global functions                                                        */
/*--------------------------------------------------------------------------*/

/*!
 * \brief Initialises this module
 *
 * \note With NutOS 3.2.1 it is not possible
 *       to have both a ppp and eth0 device
 *       registered at the same time and still
 *       have DHCP working.
 *       So switching interface is currently
 *       a reason for reboot!
 *
 * \return OK if success, TError otherwise
 */
TError SessionInit(void)
{
    TError tError = OK;

    /*
     * Initialise globals
     */
    g_pUart = 0;
    g_tError = OK;

    strncpy_P(szEthernetIfName, PSTR("eth0"), sizeof(szEthernetIfName));

    /*
     * Create a unique hostname from our serial number
     * Save the hostname in NutOS
     */
    //sprintf_P(confos.hostname, PSTR("%.10s%5.5lX"), VersionGetAppProductName(), SettingsGetSerialnumber());
    NutSaveConfig();

    /*
     * Try to bring up the selected
     * interface. Error if none configured
     */
    /*
     * Register Realtek controller
     */
    if (NutRegisterDevice(&devEth0, ETH0_BASE, ETH0_IRQ))
    {
        LogMsg_P(LOG_EMERG, PSTR("Registering ethernet failed"));
        tError = SESSION_NODEVICE;
    }
    else
    {
        /* Let the chip settle down from init */
        NutSleep(1000);
    }

    /*
     * If we have an ethernet interface start it on init
     */
    if (tError == OK)
    {
        tError = NetConfig(szEthernetIfName);
    }

    g_tError = tError;

    return(tError);
}

/*!
 * \brief Opens a session
 *
 * If this succeeds you can start communicating
 * to the Internet
 *
 * \return OK if success, TError otherwise
 */
TError SessionOpen(void)
{
    TError tError = OK;

    g_tError = OK;

    /*
     * Try to bring up the selected
     * interface. Error if none configured
     */
    tError = StartNet();

    /*
     * Display our network settings
     */
    if (tError == OK)
    {
        u_long ulPrimaryDNS;
        u_long ulSecondaryDNS;

        /*
         * Display our IP settings.
         */
        LogMsg_P(LOG_INFO, PSTR("  Local IP: %s"), inet_ntoa(confnet.cdn_ip_addr));
        LogMsg_P(LOG_INFO, PSTR("Gateway IP: %s"), inet_ntoa(confnet.cdn_gateway));

        NutGetDnsServers(&ulPrimaryDNS, &ulSecondaryDNS);
        LogMsg_P(LOG_INFO, PSTR("  Pri. DNS: %s"), inet_ntoa(ulPrimaryDNS));
        LogMsg_P(LOG_INFO, PSTR("  Sec. DNS: %s"), inet_ntoa(ulSecondaryDNS));
    }

    g_tError = tError;

    return(tError);
}

/*!
 * \brief Return the session status
 *
 * Call to check the status of a session.
 *
 * \return see above
 */
TError SessionStatus(void)
{
    return(g_tError);
}

/*!
 * \brief Stop the session
 *
 * \return OK if success, TError otherwise
 */
TError SessionClose(void)
{
    g_tError = USER_ABORT;

    return(OK);
}

/*@}*/
