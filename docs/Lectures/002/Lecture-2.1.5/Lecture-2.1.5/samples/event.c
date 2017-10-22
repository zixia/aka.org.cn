//
// event.c: Low level event handling module.
//
// Copyright (C) 1999, Wei Yongming.
//
// Current maintainer: Wei Yongming.

/*
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Library General Public
**  License as published by the Free Software Foundation; either
**  version 2 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Library General Public License for more details.
**
**  You should have received a copy of the GNU Library General Public
**  License along with this library; if not, write to the Free
**  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
**  MA 02111-1307, USA
*/

// Create date: 1999.01.11
//
// Modify records:
//
//  Who             When        Where       For What                Status
//-----------------------------------------------------------------------------
//  Wei Yongming    1999.7      Tsinghua    Shift Key Status        Finished
//
// TODO:
// 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <linux/keyboard.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <vga.h>
#include <vgagl.h>
#include <vgamouse.h>
#include <vgakeyboard.h>

#include "common.h"
#include "cursor.h"
#include "event.h"
#include "cliprect.h"
#include "gdi.h"
#include "misc.h"

#ifndef lint
static char fileid[] = "$Id: event.c,v 1.2 2000/04/20 03:18:23 weiym Exp $";
#endif

// Mouse event parameter.
// The variable, dblclicktime, is a shared varible.

static int dblclicktime;
static int timeoutsec;
static int timeoutusec;
static void GetDblclickTime()
{
    char szValue[11];
    int ms;

    dblclicktime = DEF_MSEC_DBLCLICK;
    if( GetValueFromEtcFile(ETCFILEPATH, MOUSEPARA,
                            MOUSEPARA_DBLCLICKTIME, szValue, 10) < 0 )
        return;

    ms = atoi(szValue);
        
    if(ms > 0 && ms < 1000)
    {
        dblclicktime = ms;
    }
}

static void GetTimeout()
{
    char szValue[11];
    int secvalue, usecvalue;

    timeoutsec = DEF_SEC_TIMEOUT;
    timeoutusec = DEF_SEC_TIMEOUT;

    if( GetValueFromEtcFile(ETCFILEPATH, EVENTPARA,
                            EVENTPARA_TIMEOUTSEC, szValue, 10) < 0 )
        return;
    secvalue = atoi(szValue);
        
    if( GetValueFromEtcFile(ETCFILEPATH, EVENTPARA,
                            EVENTPARA_TIMEOUTUSEC, szValue, 10) < 0 )
        return;
    usecvalue = atoi(szValue);

    if(secvalue >= 0 && usecvalue > 0)
    {
        timeoutsec = secvalue;
        timeoutusec = usecvalue;
    }
}

// Mouse event parameters.
static int oldbutton = 0;
static struct timeval time1 = {0, 0};
static struct timeval time2 = {0, 0};

// Key event parameters.

/********* the source bellow comes from MINIX PC/AT keyboard driver. ********/

/* Standard and AT keyboard.  (PS/2 MCA implies AT throughout.) */
#define KEYBD           0x60    /* I/O port for keyboard data */

/* AT keyboard. */
#define KB_COMMAND      0x64    /* I/O port for commands on AT */
#define KB_GATE_A20     0x02    /* bit in output port to enable A20 line */
#define KB_PULSE_OUTPUT 0xF0    /* base for commands to pulse output port */
#define KB_RESET        0x01    /* bit in output port to reset CPU */
#define KB_STATUS       0x64    /* I/O port for status on AT */
#define KB_ACK          0xFA    /* keyboard ack response */
#define KB_BUSY         0x02    /* status bit set when KEYBD port ready */
#define LED_CODE        0xED    /* command to keyboard to set LEDs */
#define MAX_KB_ACK_RETRIES 0x1000       /* max #times to wait for kb ack */
#define MAX_KB_BUSY_RETRIES 0x1000      /* max #times to loop while kb busy */
#define KBIT            0x80    /* bit used to ack characters to keyboar */


static unsigned char oldkeystate[NR_KEYS];
static unsigned char olddownkey = 0;
static DWORD status;
static int alt1 = 0;               /* left alt key state */
static int alt2 = 0;               /* right alt key state */
static int capslock = 0;           /* caps lock key state */
static int esc = 0;                /* escape scan code detected? */
static int caps_off = 1;           /* 1 = normal position, 0 = depressed */
static int numlock = 0;            /* number lock key state */
static int num_off = 1;            /* 1 = normal position, 0 = depressed */
static int slock = 0;              /* scroll lock key state */
static int slock_off = 1;          /* 1 = normal position, 0 = depressed */
static int control1 = 0;           /* left control key state */
static int control2 = 0;           /* right control key state */
static int shift1 = 0;             /* left shift key state */
static int shift2 = 0;             /* left shift key state */

/*==========================================================================*
 *                              kb_wait                                     *
 *==========================================================================*/
int kb_wait()
{
/* Wait until the controller is ready; return zero if this times out. */

    int retries;

    retries = MAX_KB_BUSY_RETRIES + 1;
    while (--retries != 0 && inb(KB_STATUS) & KB_BUSY)
        ;                         /* wait until not busy */
    return(retries);              /* nonzero if ready */
}


/*==========================================================================*
 *                              kb_ack                                      *
 *==========================================================================*/
static int kb_ack()
{
/* Wait until kbd acknowledges last command; return zero if this times out. */

    int retries;

    retries = MAX_KB_ACK_RETRIES + 1;
    while (--retries != 0 && inb(KEYBD) != KB_ACK)
        ;                         /* wait for ack */
    return(retries);              /* nonzero if ack received */
}


/*===========================================================================*
 *                              set_leds                                     *
 *===========================================================================*/
static void set_leds()
{
/* Set the LEDs on the caps lock and num lock keys */

    unsigned leds;

    /* encode LED bits */
    leds = (slock << 0) | (numlock << 1) | (capslock << 2);
    status = (DWORD)leds << 16;

    kb_wait();                    /* wait for buffer empty  */
    outb(LED_CODE, KEYBD);        /* prepare keyboard to accept LED values */
    kb_ack();                     /* wait for ack response  */

    kb_wait();                    /* wait for buffer empty  */
    outb(leds, KEYBD);            /* give keyboard LED values */
    kb_ack();                     /* wait for ack response  */
}


/********* the source above comes from MINIX PC/AT keyboard driver. ********/

static void ResetMouseEvent(void)
{
    mouse_update();
    oldbutton = mouse_getbutton();
    timerclear(&time1);
    timerclear(&time2);
}

static void ResetKeyEvent(void)
{
    keyboard_update();
    memcpy(oldkeystate, keyboard_getstate(), NR_KEYS);
    olddownkey  = 0;
    status      = 0;
    alt1        = 0;
    alt2        = 0;
    esc         = 0;
    control1    = 0;
    control2    = 0;
    shift1      = 0;
    shift2      = 0;
    capslock    = 0;
    caps_off    = 1;
    numlock     = 0;
    num_off     = 1;
    slock       = 0;
    slock_off   = 1;
    set_leds();
}

BOOL GUIAPI GetLWEvent(BOOL wait, PLWEVENT lwe)
{
    static int timeout_count = 0;

    int event = 0;
    struct timeval temp = {0, 0};
    struct timeval timeout;
    int secinter, usecinter;
    int button;
    PMOUSEEVENT me = &(lwe->data.me);
    PKEYEVENT ke = &(lwe->data.ke);
    unsigned char* keystate;
    int i;
    int make;       /* 0 = release, 1 = presse */

    if(wait) {
        timeout.tv_sec = timeoutsec;
        timeout.tv_usec = timeoutusec;
        event = vga_waitevent(VGA_MOUSEEVENT | VGA_KEYEVENT,
                              NULL, NULL, NULL, &timeout);
    }
    else {
        if(mouse_update()) event |= VGA_MOUSEEVENT;
        if(keyboard_update()) event |= VGA_KEYEVENT;
    }
    
    if( event == 0 && wait) {
        
        if(timeout.tv_sec == 0 && timeout.tv_usec == 0) {
            lwe->type = LWETYPE_TIMEOUT;
            lwe->count = ++timeout_count;
            return 1;
        }
        return 0;
    }

    timeout_count = 0;
    // There was a event occurred.
    if (event & VGA_MOUSEEVENT) {
        lwe->type = LWETYPE_MOUSE;
        if (RefreshCursor(&me->x, &me->y, &button))
        {
            me->event = ME_MOVED;
            timerclear(&time1);
            timerclear(&time2);

            return 1;
        }
        
        if (oldbutton == button)
            return 0;

        if ( !(oldbutton & MOUSE_LEFTBUTTON) && 
              (button & MOUSE_LEFTBUTTON) )
        {
            if( timerisset(&time1) )	// check double click
            {
                gettimeofday(&temp, NULL);
                secinter = temp.tv_sec - time1.tv_sec;
                usecinter = (temp.tv_usec - time1.tv_usec)/1000;
                if(usecinter < 0)
                    usecinter += 1000;
                if( secinter < 2 && usecinter <= dblclicktime )
                    me->event = ME_LEFTDBLCLICK;
                else
                    me->event = ME_LEFTDOWN;

                timerclear(&time1);
            }
            else
            {
                gettimeofday(&time1, NULL);
                me->event = ME_LEFTDOWN;
            }
            goto mouseret;
        }

        if ( (oldbutton & MOUSE_LEFTBUTTON) && 
             !(button & MOUSE_LEFTBUTTON) )
        {
            me->event = ME_LEFTUP;
            goto mouseret;
        }

        if ( !(oldbutton & MOUSE_RIGHTBUTTON) && 
              (button & MOUSE_RIGHTBUTTON) )
        {
            if( timerisset(&time2) )	// check double click
            {
                gettimeofday(&temp, NULL);
                secinter = temp.tv_sec - time2.tv_sec;
                usecinter = (temp.tv_usec - time2.tv_usec)/1000;
                if(usecinter < 0)
                    usecinter += 1000;
                if( secinter < 2 && usecinter <= dblclicktime )
                    me->event = ME_RIGHTDBLCLICK;
                else
                    me->event = ME_RIGHTDOWN;

                timerclear(&time2);
            }
            else
            {
                gettimeofday(&time2, NULL);
                me->event = ME_RIGHTDOWN;
            }
            goto mouseret;
        }

        if ( (oldbutton & MOUSE_RIGHTBUTTON) && 
            !(button & MOUSE_RIGHTBUTTON) )
        {
            me->event = ME_RIGHTUP;
            goto mouseret;
        }
    }

    if(event & VGA_KEYEVENT) {
        lwe->type = LWETYPE_KEY;
        keystate = keyboard_getstate();
        for(i = 0; i < NR_KEYS; i++) {
            if(!oldkeystate[i] && keystate[i]) {
                 ke->event = KE_KEYDOWN;
                 ke->scancode = i;
                 olddownkey = i;
                 break;
            }
            if(oldkeystate[i] && !keystate[i]) {
                 ke->event = KE_KEYUP;
                 ke->scancode = i;
                 break;
            }
        }
        if (i == NR_KEYS) {
            ke->event = KE_KEYDOWN;
            ke->scancode = olddownkey;
        }
        
        make = (ke->event == KE_KEYDOWN)?1:0;

        if (i != NR_KEYS) {
            switch (ke->scancode) {
                case SCANCODE_CAPSLOCK:
                    if (make && caps_off) {
                        capslock = 1 - capslock;
                        set_leds();
                    }
                    caps_off = 1 - make;
                break;
                    
                case SCANCODE_NUMLOCK:
                    if (make && num_off) {
                        numlock = 1 - numlock;
                        set_leds();
                    }
                    num_off = 1 - make;
                break;
                
                case SCANCODE_SCROLLLOCK:
                    if (make & slock_off) {
                        slock = 1 - slock;
                        set_leds();
                    }
                    slock_off = 1 - make;
                    break;

                case SCANCODE_LEFTCONTROL:
                    control1 = make;
                    break;
                    
                case SCANCODE_RIGHTCONTROL:
                    control2 = make;
                    break;
                    
                case SCANCODE_LEFTSHIFT:
                    shift1 = make;
                    break;
                    
                case SCANCODE_RIGHTSHIFT:
                    shift2 = make;
                    break;
                    
                case SCANCODE_LEFTALT:
                    alt1 = make;
                    break;

                case SCANCODE_RIGHTALT:
                    alt2 = make;
                    break;
                    
            }

            status &= 0xFFFFF0C0;

            status |= (DWORD)((capslock << 8) |
                             (numlock << 7)   |
                             (slock << 6)     |
                             (control1 << 5)  |
                             (control2 << 4)  |
                             (alt1 << 3)      |
                             (alt2 << 2)      |
                             (shift1 << 1)    |
                             (shift2));
                             
            // Mouse button status
            if (oldbutton & MOUSE_LEFTBUTTON)
                status |= 0x00000100;
            else if (oldbutton & MOUSE_RIGHTBUTTON)
                status |= 0x00000200;
        }
        ke->status = status;
        memcpy(oldkeystate, keystate, NR_KEYS);
        return 1;
    } 

    return 0;

mouseret:
    status &= 0xFFFFF0FF;
    oldbutton = button;
    if (oldbutton & MOUSE_LEFTBUTTON)
        status |= 0x00000100;
    if (oldbutton & MOUSE_RIGHTBUTTON)
        status |= 0x00000200;
    me->status = status;
    return 1;
}

BOOL GUIAPI GetKeyStatus (UINT uKey)
{
    if (uKey & 0x0F00)      // this is a mouse key
        return oldbutton & (uKey >> 8);
    else if (uKey < NR_KEYS)
        return oldkeystate [uKey];

    return FALSE;
}

DWORD GUIAPI GetShiftKeyStatus (void)
{
    return status;
}

BOOL InitLWEvent(void)
{
    // Use raw keyboard.
    keyboard_translatekeys (TRANSLATE_KEYPADENTER 
                            | DONT_CATCH_CTRLC
                            | DONT_SWITCH_VT);

    if( keyboard_init() ) {
        return FALSE;
    }

    GetDblclickTime();
    GetTimeout();

    ResetMouseEvent();
    ResetKeyEvent();

    return TRUE;
}

void TerminateLWEvent(void)
{
    ResetMouseEvent ();
    ResetKeyEvent ();

    keyboard_close ();
    mouse_close ();
}

