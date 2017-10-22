//
// timer.c: The Timer module.
//
// Copyright (C) 1999, Wei Yongming.
//
// Current maintainer: Wei Yongming.
//
// NOTE:
// Some idea come from MyLib by Indrek Mandre.
// 

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

// Create date: 1999.04.21
//
// Modify records:
//
//  Who             When        Where       For What                Status
//-----------------------------------------------------------------------------
//  Wei Yongming    1999.10.05  Tsinghua    Timer as a thread       Working
//
// TODO:
// 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include "common.h"
#include "inline.h"
#include "cliprect.h"
#include "gdi.h"
#include "window.h"
#include "internals.h"
#include "timer.h"

#ifndef lint
static char fileid[] = "$Id: timer.c,v 1.2 2000/04/20 03:18:24 weiym Exp $";
#endif

extern MSGQUEUE DskMsgs;

int timer_counter = 0;

static struct sigaction old_alarm_handler;
static struct itimerval old_timer;
static TIMER *timerstr[MAX_TIMERS];

PMAINWIN MainWindow (HWND hWnd);
            
void sig_handler (int v)
{
    int sem_value;

    timer_counter++;

    // alert desktop
    DskMsgs.dwState |= 0x01;
    sem_getvalue (&DskMsgs.wait, &sem_value);
    if (sem_value == 0)
        sem_post(&DskMsgs.wait);
}

BOOL InitTimer ()
{
    struct itimerval timerv;
    struct sigaction siga;
    
    siga.sa_handler = sig_handler;
    siga.sa_flags = 0;
    
    __memset (&siga.sa_mask, 0, sizeof (sigset_t));

    sigaction (SIGALRM, &siga, &old_alarm_handler);

    timerv.it_interval.tv_sec = 0;
    timerv.it_interval.tv_usec = 10000;
    timerv.it_value = timerv.it_interval;

    if (setitimer (ITIMER_REAL, &timerv, &old_timer)) {
        fprintf(stderr, "TIMER: setitimer call failed!\n");
        perror("setitimer");
		return FALSE;
    }

    timer_counter = 0;

    return TRUE;
}

BOOL TerminateTimer ()
{
    int i;

    if (setitimer (ITIMER_REAL, &old_timer, 0) == -1) {
        fprintf( stderr, "Timer: setitimer call failed!\n");
        perror("setitimer");
        return 0;
    }

    if (sigaction (SIGALRM, &old_alarm_handler, NULL) == -1) {
        fprintf( stderr, "Timer: sigaction call failed!\n");
        perror("sigaction");
    	return 0;
    }

    for (i=0; i<MAX_TIMERS; i++) {
        if (timerstr[i] != NULL)
            free ( timerstr[i] );
        timerstr[i] = NULL;
    }

    return 1;
}

/************************* Functions run in desktop thread *******************/
void DispatchTimerMessage ()
{
    PMAINWIN pWin;
    PMSGQUEUE pMsgQueue;
    int i, slot;
    int sem_value;

    for ( i=0; i<MAX_TIMERS; i++ ) {
        if ( timerstr[i] ) {
    	    timerstr[i]->count += 1<<7;
            if ( timerstr[i]->count >= timerstr[i]->speed ) {

                pWin = MainWindow (timerstr[i]->hWnd);
                pMsgQueue = pWin->pMessages;

                for (slot=0; slot<8; slot++) {
                    if (pMsgQueue->TimerID[slot] == timerstr[i]->id
                        && pMsgQueue->TimerOwner[slot] == timerstr[i]->hWnd)
                        break;
                }
                if (slot != 8) {
                    pMsgQueue->dwState |= (0x01 << slot);
                    sem_getvalue (&pMsgQueue->wait, &sem_value);
                    if (sem_value == 0)
                        sem_post(&pMsgQueue->wait);
                }
                
                timerstr[i]->count -= timerstr[i]->speed;
            }
        }
    }
}

BOOL AddTimer (HWND hWnd, int id, int speed)
{
#if 0
    sigset_t sa_mask;
#endif

    int i;
    PMAINWIN pWin;
    PMSGQUEUE pMsgQueue;
    int slot;

    // Is this window a main window?
    if (!(pWin = MainWindow (hWnd))) return FALSE;

#if 0
    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif

    pMsgQueue = pWin->pMessages;
    // Is there a empty timer slot?
    for (slot=0; slot<8; slot++) {
        if ((pMsgQueue->TimerMask >> slot) & 0x01)
            break;
    }
    if (slot == 8) goto badret;

    for (i=0; i<MAX_TIMERS; i++)
        if ( timerstr[i] != NULL )
            if ( timerstr[i]->hWnd == hWnd && timerstr[i]->id == id)
                goto badret;

    for (i=0; i<MAX_TIMERS; i++)
        if ( timerstr[i] == NULL )
            break;

    if (i == MAX_TIMERS)
        goto badret ;

    timerstr[i] = malloc (sizeof (TIMER));

    timerstr[i]->speed = (1000<<7)/speed;
    timerstr[i]->hWnd = hWnd;
    timerstr[i]->id = id;
    timerstr[i]->count = 0;

    pMsgQueue->TimerOwner[slot] = hWnd;
    pMsgQueue->TimerID[slot] = id;
    pMsgQueue->TimerMask &= ~(0x01 << slot);

#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return TRUE;
    
badret:

#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

BOOL RemoveTimer (HWND hWnd, int id)
{
#if 0
    sigset_t sa_mask;
#endif

    int i;
    PMAINWIN pWin;
    PMSGQUEUE pMsgQueue;
    int slot;
    void* temp;

    // Is this window a main window?
    if (!(pWin = MainWindow (hWnd))) return FALSE;

#if 0
    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif
    
    pMsgQueue = pWin->pMessages;
    for (slot=0; slot<8; slot++) {
        if (pMsgQueue->TimerID[slot] == id)
            break;
    }
    if (slot == 8) goto badret;

    for (i=0; i<MAX_TIMERS; i++)
        if (timerstr[i] != NULL)
            if (timerstr[i]->hWnd == hWnd && timerstr[i]->id == id)
                break;

    if (i == MAX_TIMERS) goto badret;
    
    temp = timerstr[i];
    timerstr[i] = NULL;
    free (temp);

    pMsgQueue->TimerMask |= (0x01 << slot);

#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif
    
    return TRUE;

badret:
#if 0
    // unblock SIGALRM
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

BOOL GUIAPI IsTimerInstalled (HWND hWnd, int id)
{
    int i;

#if 0
    sigset_t sa_mask;

    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif

    for (i=0; i<MAX_TIMERS; i++) {
        if ( timerstr[i] != NULL ) {
            if ( timerstr[i]->hWnd == hWnd && timerstr[i]->id == id) {
#if 0
                pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif
                return TRUE;
            }
        }
    }

#if 0
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

BOOL SetTimerSpeed (HWND hWnd, int id, int speed)
{
    int i;
    
#if 0
    sigset_t sa_mask;

    // block SIGALRM temporarily
    sigemptyset (&sa_mask);
    sigaddset (&sa_mask, SIGALRM);
    pthread_sigmask (SIG_BLOCK, &sa_mask, NULL);
#endif

    for (i=0; i<MAX_TIMERS; i++)
	if (timerstr[i]->hWnd == hWnd && timerstr[i]->id == id) {
		timerstr[i]->speed = (1000<<7)/speed;
		timerstr[i]->count = 0;
#if 0
        pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif
        return TRUE;
	}

#if 0
    pthread_sigmask (SIG_UNBLOCK, &sa_mask, NULL);
#endif

    return FALSE;
}

/****************** Timer Interfaces for applications ************************/
int GUIAPI GetTickCount ()
{
    return timer_counter;
}

BOOL GUIAPI SetTimer (HWND hWnd, int id, int speed)
{
    TIMER timer;
    
    timer.hWnd = hWnd;
    timer.id = id;
    timer.speed = speed;
    
    return SendMessage (HWND_DESKTOP, MSG_ADDTIMER, 0, (LPARAM)&timer);
}

BOOL GUIAPI KillTimer (HWND hWnd, int id)
{
    TIMER timer;
    
    timer.hWnd = hWnd;
    timer.id = id;
    
    return SendMessage (HWND_DESKTOP, MSG_REMOVETIMER, 0, (LPARAM)&timer);
}

BOOL GUIAPI ResetTimer (HWND hWnd, int id, int speed)
{
    TIMER timer;
    
    timer.hWnd = hWnd;
    timer.id = id;
    timer.speed = speed;
    
    return SendMessage (HWND_DESKTOP, MSG_RESETTIMER, 0, (LPARAM)&timer);
}

