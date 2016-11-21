//
// desktop.c: The Desktop module.
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

// Create date: 1999.04.19
//
// Modify records:
//
//  Who             When        Where       For What                Status
//-----------------------------------------------------------------------------
//  Wei Yongming    1999.9.11   Tsinghua    WS_DISABLED             done
//  Wei Yongming    1999.9.28   Tsinghua    Remove collector thread done
//  Wei Yongming    1999.9.28   Tsinghua    Timer module to desktop done
//  Wei Yongming    1999.10.28  Tsinghua    Desktop Menu            done
//
// TODO:
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <vgakeyboard.h>
#include <unistd.h>
#include <termios.h>

#include "common.h"
#include "cliprect.h"
#include "gdi.h"
#include "cursor.h"
#include "event.h"
#include "window.h"
#include "internals.h"
#include "misc.h"
#include "menu.h"
#include "fixstr.h"
#include "timer.h"
#include "ctrlclass.h"
#include "accelkey.h"
#include "main.h"
#include "vcswitch.h"

#ifdef _MULFONT
#include "cfont.h"
#endif

#ifdef _DEBUG
#include "msgstr.h"
#endif

#ifndef lint
static char fileid[] = "$Id: desktop.c,v 1.2 2000/04/20 03:18:23 weiym Exp $";
#endif

/******************************* extern data *********************************/
extern int timer_counter;

/******************************* static data *********************************/
static pthread_t desktop, parsor, timer;
static int DesktopProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

/********************* Window management support *****************************/
MSGQUEUE DskMsgs;
RECT sg_rcScr;
FREECLIPRECTLIST sg_FreeInvRectList;

static FREECLIPRECTLIST sg_FreeClipRectList;
static PMAINWIN pActiveMainWnd;
static HWND hCaptureWnd;
static ZORDERINFO MainWinZOrder;
static ZORDERINFO TopMostWinZOrder;

static PTRACKMENUINFO sg_ptmi;

static HWND sg_hIMEWnd;

static HWND sg_hCaretWnd;
static UINT sg_uCaretBTime;

static GCRINFO sg_ScrGCRInfo;

static void InitWndManagementInfo(void)
{
    InitMainWinMetrics();

    hCaptureWnd = HWND_DESKTOP;
    pActiveMainWnd = NULL;

    sg_ptmi = NULL;

    sg_hIMEWnd = HWND_DESKTOP;
    sg_hCaretWnd = HWND_DESKTOP;

    sg_rcScr.left = sg_rcScr.top = 0;
    sg_rcScr.right = GetGDCapability (HDC_SCREEN, GDCAP_MAXX) + 1;
    sg_rcScr.bottom = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) + 1;

    InitClipRgn (&sg_ScrGCRInfo.crgn, &sg_FreeClipRectList);
    SetClipRgn (&sg_ScrGCRInfo.crgn, &sg_rcScr);
    pthread_mutex_init (&sg_ScrGCRInfo.lock, NULL);
    sg_ScrGCRInfo.age = 0;
}

BITMAP SystemBitmap[SYSBMP_ITEM_NUMBER];
HICON  LargeSystemIcon [SYSICO_ITEM_NUMBER] = {0};
HICON  SmallSystemIcon [SYSICO_ITEM_NUMBER] = {0};
BOOL InitDesktop ()
{
    int i;
    int nIconNr;
    char szValue [12];
    
    /*
     * Load system bitmaps here.
     */
    if (!LoadSystemBitmap (SystemBitmap,     "minimize")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 1, "maximize")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 2, "restore")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 3, "close")) return FALSE;

    if (!LoadSystemBitmap (SystemBitmap + 4,  "arrowup")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 5,  "arrowupd")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 6,  "arrowdown")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 7,  "arrowdownd")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 8,  "arrowleft")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 9,  "arrowleftd")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 10, "arrowright")) return FALSE;
    if (!LoadSystemBitmap (SystemBitmap + 11, "arrowrightd")) return FALSE;

    /*
     * Load system icons here.
     */
    if( GetValueFromEtcFile(ETCFILEPATH, "iconinfo", "iconnumber", 
                            szValue, 10) < 0 )
        return FALSE;
    nIconNr = atoi(szValue);
    if (nIconNr <= 0)
        return FALSE;
    nIconNr = nIconNr < SYSICO_ITEM_NUMBER ? nIconNr : SYSICO_ITEM_NUMBER;

    for (i = 0; i < nIconNr; i++) {
        sprintf(szValue, "icon%d", i);
        
        SmallSystemIcon [i] = LoadSystemIcon (szValue, 1);
        LargeSystemIcon [i] = LoadSystemIcon (szValue, 0);

        if (SmallSystemIcon [i] == 0
                || LargeSystemIcon [i] == 0)
            return FALSE;
    }
    
    // Init Window Management information.
    InitWndManagementInfo();

    return TRUE;
}

void TerminateDesktop ()
{
    int i;
    
    for (i=0; i<SYSBMP_ITEM_NUMBER; i++)
        UnloadBitmap (SystemBitmap + i);

    for (i=0; i<SYSICO_ITEM_NUMBER; i++) {
        if (SmallSystemIcon [i])
            DestroyIcon (SmallSystemIcon [i]);

        if (LargeSystemIcon [i])
            DestroyIcon (LargeSystemIcon [i]);
    }
}

PGCRINFO GetGCRgnInfo(HWND hWnd)
{
    PMAINWIN pWin;

    if (hWnd == HWND_DESKTOP)
        return &sg_ScrGCRInfo;

    pWin = GetMainWindow (hWnd);

    return &pWin->GCRInfo;
}

inline void DesktopSetActiveWindow(PMAINWIN pWin)
{
    if (sg_hIMEWnd != HWND_DESKTOP && pWin) {
        if (pWin->dwExStyle & WS_EX_IMECOMPOSE)
            SendNotifyMessage (sg_hIMEWnd, MSG_IME_OPEN, 0, 0);
        else
            SendNotifyMessage (sg_hIMEWnd, MSG_IME_CLOSE, 0, 0);
            
        SendNotifyMessage (sg_hIMEWnd, MSG_IME_SETTARGET, (WPARAM)pWin, 0);
    }

    pActiveMainWnd = pWin;
}

static HWND DesktopSetCapture(HWND hwnd)
{
    HWND hTemp;

    hTemp = hCaptureWnd;
    hCaptureWnd = hwnd;

    return hTemp;
}

/************************* ZOrder operation **********************************/
static void InitZOrderInfo(PZORDERINFO pZOrderInfo, HWND hHost)
{
    pZOrderInfo->nNumber = 0;
    pZOrderInfo->hWnd = hHost;
    pZOrderInfo->pTopMost = NULL;
}

// When show an invisible new main window, 
// call this function to update all other windows' GCRInfo.
//
// Window functions which lead to calling this function:
// ShowWindow: show an invisible main window with a SW_SHOW parameter.
// 
// this main window is a normal main window.
static void dskUpdateGCRInfoOnShowNewMainWin(MAINWIN* pWin)
{
    PZORDERNODE pNode;
    PGCRINFO pGCRInfo;
    PMAINWIN pTemp;
    
    // this main window's GCR Info
    pGCRInfo = &pWin->GCRInfo;
    pthread_mutex_lock (&pGCRInfo->lock);

    // clip by all top most windows
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    pGCRInfo->age ++;
    pthread_mutex_unlock (&pGCRInfo->lock);

    // update all main windows' global clip region under this window.
    // pWin is the top most window.
    pNode = MainWinZOrder.pTopMost->pNext;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
        pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pWin->left));
    pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }
    
    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pWin->left));
    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);
}

// this main window is a top most window.
static void dskUpdateGCRInfoOnShowNewMainWinEx(MAINWIN* pWin)
{
    PZORDERNODE pNode;
    PGCRINFO pGCRInfo;
    
    // update all top most windows' global clip region under this window.
    // pWin is the top most window.
    pNode = TopMostWinZOrder.pTopMost->pNext;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
            pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pWin->left));
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }
    
    // update all normal main windows' global clip region
    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
            pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pWin->left));
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }

    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pWin->left));
    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);
}

// When show a main window, all main windows which are covered by
// this showing main window will be updated.
// 
// Window functions which lead to calling this function:
// ShowWindow: show an invisible main window with a SW_SHOW parameter.
//
// this is a normal main window.
static void dskUpdateGCRInfoOnShowMainWin(MAINWIN* pWin)
{
    PMAINWIN pTemp;
    PZORDERNODE pNode;
    PGCRINFO pGCRInfo;
    RECT rcWin, rcTemp;
    
    // update this showing window's clip region info.

    pGCRInfo = &pWin->GCRInfo;
    IntersectRect (&rcTemp, (PRECT)(&pWin->left), &sg_rcScr);
    pthread_mutex_lock (&pGCRInfo->lock);
    SetClipRgn (&pGCRInfo->crgn, &rcTemp);

    // clip by all top most windows
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    
    // clip by all normal windows which cover this showing window.
    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if (pNode->hWnd == (HWND)pWin)
            break;
            
        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }

    pGCRInfo->age ++;
    pthread_mutex_unlock (&pGCRInfo->lock);
    
    // where is the showing window?
    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if (pNode->hWnd == (HWND)pWin)
            break;
        pNode = pNode->pNext;
    }
    
    // the showing main window's rect
    memcpy (&rcWin, &pWin->left, sizeof(RECT));

    // clip all main windows covered by this showing window.
    pNode = pNode->pNext;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
            pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, &rcWin);
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }
    
    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SubtractClipRect (&sg_ScrGCRInfo.crgn, &rcWin);
    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);
}

// this window is a main window with WS_EX style
static void dskUpdateGCRInfoOnShowMainWinEx(MAINWIN* pWin)
{
    PMAINWIN pTemp;
    PZORDERNODE pNode;
    PGCRINFO pGCRInfo;
    RECT rcWin, rcTemp;
    
    // update this showing window's clip region info.

    pGCRInfo = &pWin->GCRInfo;
    IntersectRect (&rcTemp, (PRECT)(&pWin->left), &sg_rcScr);
    pthread_mutex_lock (&pGCRInfo->lock);
    SetClipRgn (&pGCRInfo->crgn, &rcTemp);

    // clip by all top most windows which cover this showing window.
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        if (pNode->hWnd == (HWND)pWin)
            break;
            
        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    
    pGCRInfo->age ++;
    pthread_mutex_unlock (&pGCRInfo->lock);
    
    // where is the showing window?
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        if (pNode->hWnd == (HWND)pWin)
            break;
        pNode = pNode->pNext;
    }
    
    // the showing main window's rect
    memcpy (&rcWin, &pWin->left, sizeof(RECT));

    // update all top most windows which covered by this showing window.
    pNode = pNode->pNext;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
            pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, &rcWin);
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }
    
    // update all normal main windows' global clip region
    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
            pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, &rcWin); 
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }

    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SubtractClipRect (&sg_ScrGCRInfo.crgn, &rcWin);
    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);
}

static BOOL dskDoesCoverOther (PMAINWIN pWin1, PMAINWIN pWin2)
{
    PZORDERNODE pNode;

    if (pWin1 == NULL) return FALSE;

    if (pWin2 == NULL) return TRUE;

    pNode = MainWinZOrder.pTopMost;
    while(pNode)
    {
        if (pNode->hWnd == (HWND)pWin1) return TRUE;
        if (pNode->hWnd == (HWND)pWin2) return FALSE;
        
        pNode = pNode->pNext;
    }

    return FALSE;
}

static BOOL dskIsTopMost (PMAINWIN pWin)
{
    if (pWin->dwExStyle & WS_EX_TOPMOST)
        return ((HWND)pWin == TopMostWinZOrder.pTopMost->hWnd);
    else
        return ((HWND)pWin == MainWinZOrder.pTopMost->hWnd);
}

static PMAINWIN dskChangActiveWindow (PMAINWIN pWin)
{
    PMAINWIN pOldActive;

    if (sg_hIMEWnd && (sg_hIMEWnd == (HWND)pWin)) return pActiveMainWnd;

    if (pWin == pActiveMainWnd) return pActiveMainWnd;

    pOldActive = pActiveMainWnd;
    DesktopSetActiveWindow (pWin);

    if (pWin) {
        SendAsyncMessage ((HWND)pWin, MSG_NCACTIVATE, TRUE, 0);
        SendNotifyMessage ((HWND)pWin, MSG_ACTIVE, TRUE, 0);
        SendNotifyMessage ((HWND)pWin, MSG_SETFOCUS, 0, 0);
    }

    if (pOldActive && (pOldActive->dwStyle & WS_VISIBLE)) {
        SendAsyncMessage ((HWND)pOldActive, MSG_NCACTIVATE, FALSE, 0);
        SendNotifyMessage ((HWND)pWin, MSG_ACTIVE, FALSE, 0);
        SendNotifyMessage ((HWND)pOldActive, MSG_KILLFOCUS, 0, 0);
    }

    return pOldActive;
}

// This function called when a window was shown.
// return: the new active window.
// when return value is NULL, the active window not changed.
//
static PMAINWIN dskShowMainWindow(PMAINWIN pWin)
{
    if (pWin->dwStyle & WS_VISIBLE)
        return NULL;

    if (pWin->dwExStyle & WS_EX_TOPMOST)
        dskUpdateGCRInfoOnShowMainWinEx (pWin);
    else
        dskUpdateGCRInfoOnShowMainWin (pWin);

    pWin->dwStyle |= WS_VISIBLE;

    SendAsyncMessage ((HWND)pWin, MSG_NCPAINT, 0, 0);

    InvalidateRect ((HWND)pWin, NULL, TRUE);

    if (pWin->dwExStyle & WS_EX_TOPMOST)
        return dskChangActiveWindow (pWin);

    // if the showing window cover the current active window
    // set this window as the active one. 
    if (dskDoesCoverOther (pWin, pActiveMainWnd))
        return dskChangActiveWindow (pWin); 

    return NULL; 
}

// Main window hosting.
//
// Add new hosted main window.
//
void dskAddNewHostedMainWindow (PMAINWIN pHosting, PMAINWIN pHosted)
{
    PMAINWIN head, prev;
    
    pHosted->pNextHosted = NULL;

    head = pHosting->pFirstHosted;
    if (head)
    {
        while (head) {
            prev = head;
            head = head->pNextHosted;
        }

        prev->pNextHosted = pHosted;
    }
    else
        pHosting->pFirstHosted = pHosted;

    return;
}

// Main Window hosting.
//
// Remove a hosted main window.
//
void dskRemoveHostedMainWindow (PMAINWIN pHosting, PMAINWIN pHosted)
{
    PMAINWIN head, prev;
    
    head = pHosting->pFirstHosted;
    if (head == pHosted)
    {
        pHosting->pFirstHosted = head->pNextHosted;

        return;
    }

    while (head) {
        prev = head;
        head = head->pNextHosted;
            
        if (head == pHosted) {
            prev->pNextHosted = head->pNextHosted;
            return;
        }
    }

    return;
}

// this funciton add the new main window to the main window list.
// if new main window is a visible window,
// this new main window becomes the active window and this function
// return the old main window.
// otherwise, return NULL.
static PMAINWIN dskAddNewMainWindow(PMAINWIN pWin, PZORDERNODE pNode)
{
    RECT rcTemp;

    // Handle main window hosting.
    if (pWin->pHosting)
        dskAddNewHostedMainWindow (pWin->pHosting, pWin);
    
    // Init Global Clip Region info.
    pthread_mutex_init (&pWin->GCRInfo.lock, NULL);
    pWin->GCRInfo.age = 0;
    InitClipRgn (&pWin->GCRInfo.crgn, &sg_FreeClipRectList);
    IntersectRect (&rcTemp, (PRECT)(&pWin->left), &sg_rcScr);
    SetClipRgn (&pWin->GCRInfo.crgn, &rcTemp);

    // Init Invalid Region info.
    pthread_mutex_init (&pWin->InvRgn.lock, NULL);
    InitClipRgn (&pWin->InvRgn.rgn, &sg_FreeInvRectList);
    EmptyClipRgn (&pWin->InvRgn.rgn);

    // Update Z Order info.
    pNode->hWnd = (HWND)pWin;
    if (pWin->dwExStyle & WS_EX_TOPMOST) {
        pNode->pNext = TopMostWinZOrder.pTopMost;
        TopMostWinZOrder.pTopMost = pNode;
        TopMostWinZOrder.nNumber++;
    }
    else {
        pNode->pNext = MainWinZOrder.pTopMost;
        MainWinZOrder.pTopMost = pNode;
        MainWinZOrder.nNumber++;
    }

    // show and active this main window.
    if ( pWin->dwStyle & WS_VISIBLE ) {

        if (pWin->dwExStyle & WS_EX_TOPMOST)
            dskUpdateGCRInfoOnShowNewMainWinEx (pWin);
        else
            dskUpdateGCRInfoOnShowNewMainWin (pWin);
        
        SendAsyncMessage ((HWND)pWin, MSG_NCPAINT, 0, 0);

        SendNotifyMessage ((HWND)pWin, MSG_SHOWWINDOW, SW_SHOWNORMAL, 0);

        InvalidateRect ((HWND)pWin, NULL, TRUE);

        return dskChangActiveWindow (pWin);
    }

    return NULL;
}

BOOL wndInvalidateRect (HWND hWnd, const RECT* prc, BOOL bEraseBkgnd);

// this function called when a visible main win becomes as a top most window.
// this is a normal main window.
static void dskUpdateGCRInfoOnChangeTopMost(PMAINWIN pNew)
{
    PGCRINFO pGCRInfo;
    PZORDERNODE pNode;
    RECT rcInter;
    RECT rcNew;
    RECT rcOther;
    BOOL inved = FALSE;

    memcpy (&rcNew, &pNew->left, sizeof(RECT));

    // clip all main windows which will covered by this new top most window
    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if ( pNode->hWnd == (HWND)pNew )
            break;

        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
        
            memcpy(&rcOther, &(((PMAINWIN)(pNode->hWnd))->left), sizeof(RECT));

            if(IntersectRect(&rcInter, &rcNew, &rcOther))
            {
                pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
                pthread_mutex_lock (&pGCRInfo->lock);
                SubtractClipRect (&pGCRInfo->crgn, &rcNew);
                pGCRInfo->age ++;
                pthread_mutex_unlock (&pGCRInfo->lock);

                // update this window's client area.
                rcInter.left -= pNew->cl;
                rcInter.top -= pNew->ct;
                rcInter.right -= pNew->cl;
                rcInter.bottom -= pNew->ct;
                wndInvalidateRect ((HWND)pNew, &rcInter, TRUE);
                inved = TRUE;
            }
        }

        pNode = pNode->pNext;
    }

    if (inved)
        PostMessage ((HWND)pNew, MSG_PAINT, 0, 0);
}

// this is a top most window
static void dskUpdateGCRInfoOnChangeTopMostEx (PMAINWIN pNew)
{
    PGCRINFO pGCRInfo;
    PZORDERNODE pNode;
    RECT rcInter;
    RECT rcNew;
    RECT rcOther;
    BOOL inved = FALSE;

    memcpy (&rcNew, &pNew->left, sizeof(RECT));

    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        if ( pNode->hWnd == (HWND)pNew )
            break;

        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
        
            memcpy(&rcOther, &(((PMAINWIN)(pNode->hWnd))->left), sizeof(RECT));

            if(IntersectRect(&rcInter, &rcNew, &rcOther))
            {
                pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
                pthread_mutex_lock (&pGCRInfo->lock);
                SubtractClipRect (&pGCRInfo->crgn, &rcNew);
                pGCRInfo->age ++;
                pthread_mutex_unlock (&pGCRInfo->lock);

                // update this window's client area.
                rcInter.left -= pNew->cl;
                rcInter.top -= pNew->ct;
                rcInter.right -= pNew->cl;
                rcInter.bottom -= pNew->ct;
                wndInvalidateRect ((HWND)pNew, &rcInter, TRUE);
                inved = TRUE;
            }
        }

        pNode = pNode->pNext;
    }

    if (inved)
        PostMessage ((HWND)pNew, MSG_PAINT, 0, 0);
}

// this function called when a main window becomes a visible top most window.
// Functions which lead to call this function:
//  ShowWindow: show a invisible window with SW_SHOWNORMAL parameters.
//
// return: old active window.
//
static PMAINWIN dskMoveToTopMost(PMAINWIN pWin)
{
    PZORDERNODE pNode, pTemp;
    PMAINWIN pTempWin;
    RECT rcTemp;

    if (!pWin) return pActiveMainWnd;

    if (dskIsTopMost (pWin) && (pWin->dwStyle & WS_VISIBLE))
        return pActiveMainWnd;

    // update this main window's global clip region info.
    pthread_mutex_lock (&pWin->GCRInfo.lock);
    IntersectRect (&rcTemp, (PRECT)(&pWin->left), &sg_rcScr);
    SetClipRgn (&pWin->GCRInfo.crgn, &rcTemp);
    if (!(pWin->dwExStyle & WS_EX_TOPMOST)) {
        // clip by all top most windows
        pNode = TopMostWinZOrder.pTopMost;
        while (pNode)
        {
            pTempWin = (PMAINWIN)(pNode->hWnd);
            if (pTempWin->dwStyle & WS_VISIBLE)
                SubtractClipRect (&pWin->GCRInfo.crgn, 
                                    (PRECT)(&pTempWin->left));

            pNode = pNode->pNext;
        }
    }
    pWin->GCRInfo.age ++;
    pthread_mutex_unlock (&pWin->GCRInfo.lock);

    // activate this main window.
    if ( !(pWin->dwStyle & WS_VISIBLE) )
    {
        // Update Z Order first.
        if (pWin->dwExStyle & WS_EX_TOPMOST) {
            pNode = TopMostWinZOrder.pTopMost;
            if (pNode->hWnd != (HWND)pWin) {
                while(pNode->pNext)
                {
                    if( pNode->pNext->hWnd == (HWND)pWin )
                    {
                       pTemp = pNode->pNext;
                       pNode->pNext = pNode->pNext->pNext;

                       pTemp->pNext = TopMostWinZOrder.pTopMost;
                       TopMostWinZOrder.pTopMost = pTemp;

                       break;
                    }
                    pNode = pNode->pNext;
                }
            }
        }
        else {
            pNode = MainWinZOrder.pTopMost;
            if (pNode->hWnd != (HWND)pWin) {
                while(pNode->pNext)
                {
                    if( pNode->pNext->hWnd == (HWND)pWin )
                    {
                       pTemp = pNode->pNext;
                       pNode->pNext = pNode->pNext->pNext;

                       pTemp->pNext = MainWinZOrder.pTopMost;
                       MainWinZOrder.pTopMost = pTemp;

                       break;
                    }
                    pNode = pNode->pNext;
                }
            }
        }

        if (pWin->dwExStyle & WS_EX_TOPMOST)
            dskUpdateGCRInfoOnShowNewMainWinEx (pWin);
        else
            dskUpdateGCRInfoOnShowNewMainWin (pWin);
        
        pWin->dwStyle |= WS_VISIBLE;

        SendAsyncMessage ((HWND)pWin, MSG_NCPAINT, 0, 0);
    
        InvalidateRect ((HWND)pWin, NULL, TRUE);
    }
    else {
        SendAsyncMessage ((HWND)pWin, MSG_NCPAINT, 0, 0);

        if (pWin->dwExStyle & WS_EX_TOPMOST)
            dskUpdateGCRInfoOnChangeTopMostEx (pWin);
        else
            dskUpdateGCRInfoOnChangeTopMost (pWin);

        // then update Z Order.
        if (pWin->dwExStyle & WS_EX_TOPMOST) {
            pNode = TopMostWinZOrder.pTopMost;
            if (pNode->hWnd != (HWND)pWin) {
                while(pNode->pNext)
                {
                    if( pNode->pNext->hWnd == (HWND)pWin )
                    {
                       pTemp = pNode->pNext;
                       pNode->pNext = pNode->pNext->pNext;

                       pTemp->pNext = TopMostWinZOrder.pTopMost;
                       TopMostWinZOrder.pTopMost = pTemp;

                       break;
                    }
                    pNode = pNode->pNext;
                }
            }
        }
        else {
            pNode = MainWinZOrder.pTopMost;
            if (pNode->hWnd != (HWND)pWin) {
                while (pNode->pNext)
                {
                    if (pNode->pNext->hWnd == (HWND)pWin)
                    {
                       pTemp = pNode->pNext;
                       pNode->pNext = pNode->pNext->pNext;
    
                       pTemp->pNext = MainWinZOrder.pTopMost;
                       MainWinZOrder.pTopMost = pTemp;

                       break;
                    }
                    pNode = pNode->pNext;
                }
            }
        }
    }
    
    return dskChangActiveWindow (pWin);
}

// When hide a main win, all main win which is covered by
// this hidding main win will be regenerated.
//
// Functions which lead to call this function:
//  ShowWindow: hide a visible window with SW_HIDE parameters.
//  DestroyWindow: destroy a visible window.
//
static void dskUpdateGCRInfoOnHideMainWin(MAINWIN* pWin)
{
    PZORDERNODE pNode, pAffected;
    PGCRINFO pGCRInfo;
    RECT     rcWin, rcTemp;
    PMAINWIN pTemp;
    
    // where is the hidding main window?
    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if (pNode->hWnd == (HWND)pWin)
            break;
        pNode = pNode->pNext;
    }
    
    // update this main window's status.
    pWin->dwStyle &= ~WS_VISIBLE;

    // this main window's rect.
    memcpy (&rcWin, &pWin->left, sizeof(RECT));

    // the first affected main window
    pAffected = pNode->pNext;
    while (pAffected){
        pTemp = (PMAINWIN)(pAffected->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE) {

            pGCRInfo = &pTemp->GCRInfo;
            pthread_mutex_lock (&pGCRInfo->lock);
            IntersectRect (&rcTemp, (PRECT)(&pTemp->left), &sg_rcScr);
            SetClipRgn (&pGCRInfo->crgn, &rcTemp);

            // clip by all top most windows.
            pNode = TopMostWinZOrder.pTopMost;
            while (pNode) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }
           
            pNode = MainWinZOrder.pTopMost;
            while (pNode != pAffected) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);

            pTemp = (PMAINWIN)(pAffected->hWnd);

            IntersectRect (&rcTemp, &rcWin, &rcTemp);
            SendAsyncMessage ((HWND)pTemp,
                MSG_NCPAINT, 0, 0);

            rcTemp.left -= pTemp->cl;
            rcTemp.top -= pTemp->ct;
            rcTemp.right -= pTemp->cl;
            rcTemp.bottom -= pTemp->ct;
            InvalidateRect ((HWND)pTemp, &rcTemp, TRUE);
        }

           pAffected = pAffected->pNext;
    }

    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SetClipRgn (&sg_ScrGCRInfo.crgn, &sg_rcScr);
    
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    
    pNode = MainWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }

    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);

    // Erase invalid rect.
    IntersectRect (&rcTemp, &rcWin, &sg_rcScr);
    DesktopProc (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, (LPARAM)(&rcTemp));
}

// this is a top most window
static void dskUpdateGCRInfoOnHideMainWinEx(MAINWIN* pWin)
{
    PZORDERNODE pNode, pAffected;
    PGCRINFO pGCRInfo;
    RECT     rcWin, rcTemp;
    PMAINWIN pTemp;
    
    // where is the hidding main window?
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        if (pNode->hWnd == (HWND)pWin)
            break;
        pNode = pNode->pNext;
    }
    
    // update this main window's status.
    pWin->dwStyle &= ~WS_VISIBLE;

    // this main window's rect.
    memcpy (&rcWin, &pWin->left, sizeof(RECT));

    // the first affected main window
    pAffected = pNode->pNext;
    while (pAffected){
        pTemp = (PMAINWIN)(pAffected->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE) {

            pGCRInfo = &pTemp->GCRInfo;
            pthread_mutex_lock (&pGCRInfo->lock);
            IntersectRect (&rcTemp, (PRECT)(&pTemp->left), &sg_rcScr);
            SetClipRgn (&pGCRInfo->crgn, &rcTemp);

            pNode = TopMostWinZOrder.pTopMost;
            while (pNode != pAffected) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);

            pTemp = (PMAINWIN)(pAffected->hWnd);

            IntersectRect (&rcTemp, &rcWin, &rcTemp);
            SendAsyncMessage ((HWND)pTemp,
                MSG_NCPAINT, 0, 0);

            rcTemp.left -= pTemp->cl;
            rcTemp.top -= pTemp->ct;
            rcTemp.right -= pTemp->cl;
            rcTemp.bottom -= pTemp->ct;
            InvalidateRect ((HWND)pTemp, &rcTemp, TRUE);
        }

           pAffected = pAffected->pNext;
    }

    // update all main windows global clip region.
    // the first affected main window
    pAffected = MainWinZOrder.pTopMost;
    while (pAffected){
        pTemp = (PMAINWIN)(pAffected->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE) {

            pGCRInfo = &pTemp->GCRInfo;
            pthread_mutex_lock (&pGCRInfo->lock);
            IntersectRect (&rcTemp, (PRECT)(&pTemp->left), &sg_rcScr);
            SetClipRgn (&pGCRInfo->crgn, &rcTemp);

            // clip by all top most windows.
            pNode = TopMostWinZOrder.pTopMost;
            while (pNode) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }
           
            pNode = MainWinZOrder.pTopMost;
            while (pNode != pAffected) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);

            pTemp = (PMAINWIN)(pAffected->hWnd);

            IntersectRect (&rcTemp, &rcWin, &rcTemp);
            SendAsyncMessage ((HWND)pTemp,
                MSG_NCPAINT, 0, 0);

            rcTemp.left -= pTemp->cl;
            rcTemp.top -= pTemp->ct;
            rcTemp.right -= pTemp->cl;
            rcTemp.bottom -= pTemp->ct;
            InvalidateRect ((HWND)pTemp, &rcTemp, TRUE);
        }

           pAffected = pAffected->pNext;
    }

    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SetClipRgn (&sg_ScrGCRInfo.crgn, &sg_rcScr);
    
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    
    pNode = MainWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);

    // Erase invalid rect.
    IntersectRect (&rcTemp, &rcWin, &sg_rcScr);
    DesktopProc (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, (LPARAM)(&rcTemp));
}

static PMAINWIN dskGetFirstVisibleWindow ()
{
    PZORDERNODE pNode;
    PMAINWIN pTemp;

    pNode = MainWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE)
            return pTemp;

        pNode = pNode->pNext;
    }
    
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE)
            return pTemp;

        pNode = pNode->pNext;
    }

    return NULL;
    
}

static PMAINWIN dskGetNextVisibleWindow (PMAINWIN pWin)
{
    PZORDERNODE pNode;
    PMAINWIN pTemp;

    if (pWin)
        pNode = pWin->pZOrderNode->pNext;
    else
        pNode = MainWinZOrder.pTopMost;

    while (pNode) {

        pTemp = (PMAINWIN)(pNode->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE && !(pTemp->dwStyle & WS_DISABLED))
            return pTemp;

        pNode = pNode->pNext;
    }
    
    return NULL;
}

static PMAINWIN dskHideMainWindow(PMAINWIN pWin)
{
    if (!(pWin->dwStyle & WS_VISIBLE))
        return NULL;

    // Update all affected windows' GCR info.
    if (pWin->dwExStyle & WS_EX_TOPMOST)
        dskUpdateGCRInfoOnHideMainWinEx (pWin);
    else
        dskUpdateGCRInfoOnHideMainWin (pWin);

    // if this is the avtive window,
    // must choose another window as the active one. 
    if (pActiveMainWnd == pWin)
        dskChangActiveWindow (dskGetFirstVisibleWindow ());

    return pActiveMainWnd;
}

// When destroy a main win, all main win which is covered by
// this destroying main win will be redraw.
//
// Functions which lead to call this function:
//  DestroyWindow: destroy a visible window.
//
// return: the new active window.
static PMAINWIN dskRemoveMainWindow(PMAINWIN pWin)
{
    PZORDERNODE pNode, pTemp;

    // Handle main window hosting.
    if (pWin->pHosting)
        dskRemoveHostedMainWindow (pWin->pHosting, pWin);
        
    // Update all affected windows' GCR info.
    if (pWin->dwStyle & WS_VISIBLE) {
        if (pWin->dwExStyle & WS_EX_TOPMOST)
            dskUpdateGCRInfoOnHideMainWinEx (pWin);
        else
            dskUpdateGCRInfoOnHideMainWin (pWin);

        if (pActiveMainWnd == pWin)
            dskChangActiveWindow (dskGetFirstVisibleWindow ());

        if (pActiveMainWnd == pWin)
            pActiveMainWnd = NULL;
    }

    // Update window Z order list.

    if (pWin->dwExStyle & WS_EX_TOPMOST) {
        pNode = TopMostWinZOrder.pTopMost;
        if (pNode->hWnd == (HWND)pWin) {
            TopMostWinZOrder.pTopMost = pNode->pNext;
            free (pNode);
        }
        else {
            while (pNode->pNext) {
                if (pNode->pNext->hWnd == (HWND)pWin) {
                   pTemp = pNode->pNext->pNext;
                   free (pNode->pNext);
                   pNode->pNext = pTemp;
           
                   break;
                }
                pNode = pNode->pNext;
            }
        }
        TopMostWinZOrder.nNumber--;
    }
    else {
        pNode = MainWinZOrder.pTopMost;
        if (pNode->hWnd == (HWND)pWin) {
            MainWinZOrder.pTopMost = pNode->pNext;
            free (pNode);
        }
        else {
            while (pNode->pNext) {
                if (pNode->pNext->hWnd == (HWND)pWin) {
                   pTemp = pNode->pNext->pNext;
                   free (pNode->pNext);
                   pNode->pNext = pTemp;
           
                   break;
                }
                pNode = pNode->pNext;
            }
        }
        MainWinZOrder.nNumber--;
    }

    return pActiveMainWnd;
}

static BOOL dskUpdateWindow (PMAINWIN pWin, 
        const RECT* prcInv, const RECT* prcWin, const RECT* prcClient)
{
    RECT rcInter;

    if (IntersectRect (&rcInter, prcInv, prcWin)) {
        rcInter.left -= prcWin->left;
        rcInter.top  -= prcWin->top;
        rcInter.right -= prcWin->left;
        rcInter.bottom -= prcWin->top;

        SendAsyncMessage ((HWND)pWin, MSG_NCPAINT, 0, (LPARAM)(&rcInter));

        if (IntersectRect (&rcInter, prcInv, prcClient)) {
            rcInter.left -= prcClient->left;
            rcInter.top  -= prcClient->top;
            rcInter.right -= prcClient->left;
            rcInter.bottom -= prcClient->top;
            InvalidateRect ((HWND)pWin, &rcInter, TRUE);

            return TRUE;
        }
    }

    return FALSE;
}

static void dskUpdateAllGCRInfoOnHideMenu ();
static int dskScrollMainWindow (PMAINWIN pWin, PSCROLLWINDOWINFO pswi)
{
    HDC hdc;
    RECT rcClient, rcInter, rcInvalid, rcScroll;
    PZORDERNODE pNode;
    PMAINWIN pTemp;
    BOOL inved = FALSE;

    rcClient.left = 0;
    rcClient.top  = 0;
    rcClient.right = pWin->cr - pWin->cl;
    rcClient.bottom = pWin->cb - pWin->ct;
    
    if (pswi->rc1)
        IntersectRect (&rcScroll, pswi->rc1, &rcClient);
    else
        rcScroll = rcClient;

    if (pswi->rc2)
        IntersectRect (&rcScroll, pswi->rc2, &rcScroll);
        
    hdc = GetClientDC ((HWND)pWin);
    SelectClipRect (hdc, &rcScroll);
    BitBlt (hdc, 0, 0, rcClient.right, rcClient.bottom,
            hdc, pswi->iOffx, pswi->iOffy, 0);
    ReleaseDC (hdc);

    if (!(pWin->dwExStyle & WS_EX_TOPMOST)) {
        WindowToScreen ((HWND)pWin, &rcClient.left, &rcClient.top);
        WindowToScreen ((HWND)pWin, &rcClient.right, &rcClient.bottom);

        pNode = TopMostWinZOrder.pTopMost;
        while (pNode) {

            pTemp = (PMAINWIN)(pNode->hWnd);
            if (pTemp != pWin && pTemp->dwStyle & WS_VISIBLE) {

                if (IntersectRect (&rcInter, 
                            &rcClient, (PRECT)(&pTemp->left))) {
                            
                    ScreenToClient ((HWND)pWin, 
                        &rcInter.left, &rcInter.top);
                    ScreenToClient ((HWND)pWin, 
                        &rcInter.right, &rcInter.bottom);
                    
                    if (pswi->iOffx < 0) {
                        rcInter.right = rcInter.left;
                        rcInter.left += pswi->iOffx;
                    }
                    else if (pswi->iOffx > 0) {
                        rcInter.left = rcInter.right;
                        rcInter.right += pswi->iOffx;
                    }

                    if (pswi->iOffy < 0) {
                        rcInter.bottom = rcInter.top;
                        rcInter.top += pswi->iOffy;
                    }
                    else if (pswi->iOffy > 0) {
                        rcInter.top = rcInter.bottom;
                        rcInter.bottom += pswi->iOffy;
                    }

                    rcInter.right += 2;
                    rcInter.bottom += 2;

                    wndInvalidateRect ((HWND)pWin, &rcInter, TRUE);
                    inved = TRUE;
                }
            }

            pNode = pNode->pNext;
        }
    }

    if (pswi->iOffx < 0) {
        rcInvalid.top = 0;
        rcInvalid.bottom = pWin->cb - pWin->ct;
        rcInvalid.right = pWin->cr - pWin->cl;
        rcInvalid.left = rcInvalid.right + pswi->iOffx;

        wndInvalidateRect ((HWND)pWin, &rcInvalid, TRUE);
        inved = TRUE;
    }
    else if (pswi->iOffx > 0) {
        rcInvalid.top = 0;
        rcInvalid.bottom = pWin->cb - pWin->ct;
        rcInvalid.left = 0;
        rcInvalid.right = pswi->iOffx;

        wndInvalidateRect ((HWND)pWin, &rcInvalid, TRUE);
        inved = TRUE;
    }
    
    if (pswi->iOffy < 0) {
        rcInvalid.left = 0;
        rcInvalid.right = pWin->cr - pWin->cl;
        rcInvalid.bottom = pWin->cb - pWin->ct;
        rcInvalid.top = rcInvalid.bottom + pswi->iOffy;

        wndInvalidateRect ((HWND)pWin, &rcInvalid, TRUE);
        inved = TRUE;
    }
    else if (pswi->iOffy > 0) {
        rcInvalid.left = 0;
        rcInvalid.right = pWin->cr - pWin->cl;
        rcInvalid.top = 0;
        rcInvalid.bottom = pswi->iOffy;

        wndInvalidateRect ((HWND)pWin, &rcInvalid, TRUE);
        inved = TRUE;
    }

    if (inved)
        PostMessage ((HWND)pWin, MSG_PAINT, 0, 0);

    return 0;
}

static void dskMoveMainWindow (PMAINWIN pWin, const RECT* prcExpect)
{
    RECT oldWinRect, oldClientRect, src;       // old window rect.
    RECT rc[4], rcInv[4];       // invalid rects.
    RECT rcInter;
    int nCount, nInvCount; // number of invalid rects.
    int i;
    PZORDERNODE pNode;
    PMAINWIN pTemp;
    HDC hdc, hMemDC;

    memcpy (&src, &pWin->left, sizeof (RECT));
    memcpy (&oldClientRect, &pWin->cl, sizeof (RECT));
    oldWinRect = src;

    // calculate invalid rect of other windows
    nCount = SubtractRect (rc, &src, prcExpect);

    // calculate invalid rect of moving window
    nInvCount = SubtractRect (rcInv, &src, &sg_rcScr);

    // save window rect.
    hdc = GetDC ((HWND)pWin);
    hMemDC = CreateCompatibleDC (hdc);
    BitBlt (hdc, 0, 0, RECTW(oldWinRect), RECTH(oldWinRect), hMemDC, 0, 0, 0);
    ReleaseDC (hdc);
    
    // Update all affected windows' GCR info.
    SendAsyncMessage ((HWND)pWin, MSG_CHANGESIZE, (WPARAM)(prcExpect), 0);
    dskUpdateAllGCRInfoOnHideMenu ();
   
    // put window rect.
    hdc = GetDC ((HWND)pWin);
//    ScreenCopy (oldWinRect.left, oldWinRect.top, hdc, 0, 0);

    BitBlt (hMemDC, 0, 0, 
                    RECTW (oldWinRect), RECTH (oldWinRect),
                    hdc, 0, 0, 0);
    DeleteCompatibleDC (hMemDC);
    ReleaseDC (hdc);

    // when old window rect out of date.
    for (i=0; i<nInvCount; i++)
        if (dskUpdateWindow (pWin, rcInv + i, &oldWinRect, &oldClientRect))
            SendAsyncMessage ((HWND)pWin, MSG_PAINT, 0, 0);

    
    if (!(pWin->dwExStyle & WS_EX_TOPMOST)) {
        pNode = TopMostWinZOrder.pTopMost;
        while (pNode) {

            pTemp = (PMAINWIN)(pNode->hWnd);
            if (pTemp != pWin && pTemp->dwStyle & WS_VISIBLE) {
                dskUpdateWindow (pWin, (PRECT)(&pTemp->left), 
                        &oldWinRect, &oldClientRect);
//                if (dskUpdateWindow (pWin, (PRECT)(&pTemp->left), 
//                        &oldWinRect, &oldClientRect))
//                    SendAsyncMessage ((HWND)pWin, MSG_PAINT, 0, 0);
            }

            pNode = pNode->pNext;
        }

    }
   

    // when other windows' rect out of date. 
    for (i=0; i<nCount; i++) {
        if (pWin->dwExStyle & WS_EX_TOPMOST) {
            pNode = TopMostWinZOrder.pTopMost;
            while (pNode) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp != pWin && pTemp->dwStyle & WS_VISIBLE)
                    dskUpdateWindow (pTemp, rc + i, 
                            (PRECT)(&pTemp->left), (PRECT)(&pTemp->cl));
//                    if (dskUpdateWindow (pTemp, rc + i, 
//                            (PRECT)(&pTemp->left), (PRECT)(&pTemp->cl)))
//                        SendAsyncMessage ((HWND)pTemp, MSG_PAINT, 0, 0);

                pNode = pNode->pNext;
            }
        }
        
        pNode = MainWinZOrder.pTopMost;
        while (pNode) {
    
            pTemp = (PMAINWIN)(pNode->hWnd);
            if (pTemp != pWin && pTemp->dwStyle & WS_VISIBLE)
                dskUpdateWindow (pTemp, rc + i, 
                        (PRECT)(&pTemp->left), (PRECT)(&pTemp->cl));

            pNode = pNode->pNext;
        }

        IntersectRect (&rcInter, rc + i, &sg_rcScr);
        DesktopProc (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, (LPARAM)(&rcInter));
    }
    
    {
        MSG msg;
        // remove any mouse moving messages in mesasge queue.
        while (PeekPostMessage (&msg, HWND_DESKTOP, 
                    MSG_MOUSEMOVE, MSG_MOUSEMOVE, PM_REMOVE))
        { };
    }
}

static void dskUpdateAllGCRInfoOnShowMenu (RECT* prc)
{
    PZORDERNODE pNode;
    PGCRINFO pGCRInfo;
    
    // update all main windows' global clip region
    
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
            pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, prc);
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }

    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if (((PMAINWIN)(pNode->hWnd))->dwStyle & WS_VISIBLE) {
            pGCRInfo = &((PMAINWIN)(pNode->hWnd))->GCRInfo;
        
            pthread_mutex_lock (&pGCRInfo->lock);
            SubtractClipRect (&pGCRInfo->crgn, prc);
            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

        pNode = pNode->pNext;
    }
    
    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SubtractClipRect (&sg_ScrGCRInfo.crgn, prc);
    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);
}

static void dskUpdateAllGCRInfoOnHideMenu ()
{
    PZORDERNODE pNode, pAffected;
    PGCRINFO pGCRInfo;
    RECT     rcTemp;
    PMAINWIN pTemp;
    PTRACKMENUINFO ptmi;
    
    pAffected = TopMostWinZOrder.pTopMost;
    while (pAffected){

        pTemp = (PMAINWIN)(pAffected->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE) {

            pGCRInfo = &pTemp->GCRInfo;
            pthread_mutex_lock (&pGCRInfo->lock);
            IntersectRect (&rcTemp, (PRECT)(&pTemp->left), &sg_rcScr);
            SetClipRgn (&pGCRInfo->crgn, &rcTemp);

            pNode = TopMostWinZOrder.pTopMost;
            while (pNode != pAffected) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }

            ptmi = sg_ptmi;
            while (ptmi) {
                SubtractClipRect (&pGCRInfo->crgn, &ptmi->rc);
                ptmi = ptmi->next;
            }

            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

           pAffected = pAffected->pNext;
    }
    
    pAffected = MainWinZOrder.pTopMost;
    while (pAffected){

        pTemp = (PMAINWIN)(pAffected->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE) {

            pGCRInfo = &pTemp->GCRInfo;
            pthread_mutex_lock (&pGCRInfo->lock);
            IntersectRect (&rcTemp, (PRECT)(&pTemp->left), &sg_rcScr);
            SetClipRgn (&pGCRInfo->crgn, &rcTemp);

            pNode = TopMostWinZOrder.pTopMost;
            while (pNode) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }
            
            pNode = MainWinZOrder.pTopMost;
            while (pNode != pAffected) {

                pTemp = (PMAINWIN)(pNode->hWnd);
                if (pTemp->dwStyle & WS_VISIBLE)
                    SubtractClipRect (&pGCRInfo->crgn, (PRECT)(&pTemp->left));

                pNode = pNode->pNext;
            }

            ptmi = sg_ptmi;
            while (ptmi) {
                SubtractClipRect (&pGCRInfo->crgn, &ptmi->rc);
                ptmi = ptmi->next;
            }

            pGCRInfo->age ++;
            pthread_mutex_unlock (&pGCRInfo->lock);
        }

           pAffected = pAffected->pNext;
    }

    // update desktop's global clip region.
    pthread_mutex_lock (&sg_ScrGCRInfo.lock);
    SetClipRgn (&sg_ScrGCRInfo.crgn, &sg_rcScr);
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode) {

        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    pNode = MainWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);

        if (pTemp->dwStyle & WS_VISIBLE)
            SubtractClipRect (&sg_ScrGCRInfo.crgn, (PRECT)(&pTemp->left));

        pNode = pNode->pNext;
    }
    ptmi = sg_ptmi;
    while (ptmi) {
        SubtractClipRect (&sg_ScrGCRInfo.crgn, &ptmi->rc);
        ptmi = ptmi->next;
    }
    sg_ScrGCRInfo.age ++;
    pthread_mutex_unlock (&sg_ScrGCRInfo.lock);
}

// call back proc of tracking menu.
// defined in Menu module.
int PopupMenuTrackProc (PTRACKMENUINFO ptmi,
    int message, WPARAM wParam, LPARAM lParam);

static int dskTrackPopupMenu(PTRACKMENUINFO ptmi)
{
    PTRACKMENUINFO plast;
    
    if (sg_ptmi) {
        plast = sg_ptmi;
        while (plast->next) {
            plast = plast->next;
        }

        plast->next = ptmi;
        ptmi->prev = plast;
        ptmi->next = NULL;
    }
    else {
        sg_ptmi = ptmi;
        ptmi->next = NULL;
        ptmi->prev = NULL;
    }

    PopupMenuTrackProc (ptmi, MSG_INITMENU, 0, 0);

    // Update all main windows' GCR info.
    dskUpdateAllGCRInfoOnShowMenu (&ptmi->rc);

    PopupMenuTrackProc (ptmi, MSG_SHOWMENU, 0, 0);

    return 0;
}

static int dskEndTrackMenu(PTRACKMENUINFO ptmi)
{
    PZORDERNODE pNode;
    PMAINWIN pTemp;
    PTRACKMENUINFO plast;
    RECT rcInvalid;
    
    if (sg_ptmi == ptmi)
        sg_ptmi = NULL;
    else {
        plast = sg_ptmi;
        while (plast->next) {
            plast = plast->next;
        }
        plast->prev->next = NULL;
    }

    // Update all main windows' GCR info.
    dskUpdateAllGCRInfoOnHideMenu ();

    PopupMenuTrackProc (ptmi, MSG_HIDEMENU, 0, 0);

    // Invalidate rect of affected main window.
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode) {

        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE) {
            if (IntersectRect (&rcInvalid, &ptmi->rc, (PRECT)(&pTemp->cl))) {
                rcInvalid.left -= pTemp->cl;
                rcInvalid.top  -= pTemp->ct;
                rcInvalid.right -= pTemp->cl;
                rcInvalid.bottom -= pTemp->ct;
                InvalidateRect ((HWND)pTemp, &rcInvalid, FALSE);
            }
        }

        pNode = pNode->pNext;
    }
    pNode = MainWinZOrder.pTopMost;
    while (pNode) {
    
        pTemp = (PMAINWIN)(pNode->hWnd);

        pTemp = (PMAINWIN)(pNode->hWnd);
        if (pTemp->dwStyle & WS_VISIBLE) {
            if (IntersectRect (&rcInvalid, &ptmi->rc, (PRECT)(&pTemp->cl))) {
                rcInvalid.left -= pTemp->cl;
                rcInvalid.top  -= pTemp->ct;
                rcInvalid.right -= pTemp->cl;
                rcInvalid.bottom -= pTemp->ct;
                InvalidateRect ((HWND)pTemp, &rcInvalid, FALSE);
            }
        }

        pNode = pNode->pNext;
    }


    PopupMenuTrackProc (ptmi, MSG_ENDTRACKMENU, 0, 0);

    return sg_ptmi == ptmi;
}

static BOOL dskForceCloseMenu ()
{
    if (sg_ptmi == NULL) return FALSE;

    SendNotifyMessage (sg_ptmi->hwnd, MSG_DEACTIVEMENU, 
                       (WPARAM)sg_ptmi->pmb, (LPARAM)sg_ptmi->pmi);
                       
    PopupMenuTrackProc (sg_ptmi, MSG_CLOSEMENU, 0, 0);

    sg_ptmi = NULL;

    // Update all main windows' GCR info.
    dskUpdateAllGCRInfoOnHideMenu ();

    return TRUE;
}

/*********************** Hook support ****************************************/
#define NR_KEYHOOK      5
#define NR_MOUSEHOOK    5
#define MIN_MHHOOK      NR_KEYHOOK
#define MAX_MHHOOK      (NR_KEYHOOK + NR_MOUSEHOOK - 1)

static KEYHOOK keyhook [NR_KEYHOOK];
static MOUSEHOOK mousehook[NR_MOUSEHOOK];

static HHOOK dskRegisterKeyHook (HWND hWnd, KEYMSGHOOK hook)
{
    int i;

    for (i = 0; i<NR_KEYHOOK; i++) {
        if (keyhook [i].hWnd == HWND_DESKTOP) {
            keyhook [i].hWnd = hWnd;
            keyhook [i].hook = hook;
            return (HHOOK)i;
        }
    }
    
    return HHOOK_INVALID;
}

static HHOOK dskRegisterMouseHook (HWND hWnd, MOUSEMSGHOOK hook)
{
    int i;

    for (i = 0; i<NR_MOUSEHOOK; i++) {
        if (mousehook [i].hWnd == HWND_DESKTOP) {
            mousehook [i].hWnd = hWnd;
            mousehook [i].hook = hook;
            return (HHOOK)(i + MIN_MHHOOK);
        }
    }
    
    return HHOOK_INVALID;
}

static int dskUnregisterHook (HHOOK hHook)
{
    int i;

    if (hHook < 0 || hHook > MAX_MHHOOK)
        return HHOOK_INVALID;
    
    if (hHook >= MIN_MHHOOK) {
        i = hHook - MIN_MHHOOK;
        if (mousehook [i].hWnd == HWND_DESKTOP)
            return HHOOK_INVALID;
        
        mousehook [i].hWnd = HWND_DESKTOP;
        mousehook [i].hook = NULL;
    }
    else {
        i = hHook;
        if (keyhook [i].hWnd == HWND_DESKTOP)
            return HHOOK_INVALID;
        
        keyhook [i].hWnd = HWND_DESKTOP;
        keyhook [i].hook = NULL;
    }

    return HOOK_OK;
}

static void dskHandleKeyHooks (HWND hWnd, 
        int message, WPARAM wParam, LPARAM lParam)
{
    int i;

    for (i = 0; i < NR_KEYHOOK; i++) {
        if (keyhook [i].hWnd != HWND_DESKTOP) {
            (*(keyhook [i].hook)) (keyhook [i].hWnd, 
                message, wParam, lParam);
        }
    }
}

static void dskHandleMouseHooks (HWND hWnd, 
        int message, WPARAM wParam, LPARAM lParam)
{
    int i;

    for (i = 0; i < NR_MOUSEHOOK; i++) {
        if (mousehook [i].hWnd != HWND_DESKTOP) {
            (*(mousehook [i].hook)) (mousehook [i].hWnd, 
                    message, wParam, lParam);
        }
    }
}

/*********************** Desktop window support ******************************/
inline PMAINWIN GetMainWinUnderPointer(int x, int y)
{
    PZORDERNODE pNode;
    PMAINWIN pTemp;

    pNode = TopMostWinZOrder.pTopMost;
    while(pNode)
    {
        pTemp = (PMAINWIN)pNode->hWnd;
        if( PtInRect((PRECT)(&(pTemp->left)), x, y) 
            && (pTemp->dwStyle & WS_VISIBLE))
            return pTemp;

        pNode = pNode->pNext; 
    }

    pNode = MainWinZOrder.pTopMost;
    while(pNode)
    {
        pTemp = (PMAINWIN)pNode->hWnd;
        if( PtInRect((PRECT)(&(pTemp->left)), x, y) 
            && (pTemp->dwStyle & WS_VISIBLE))
            return pTemp;
            
        pNode = pNode->pNext; 
    }
    return NULL;
}

static int HandleSpecialKey (scancode)
{
    switch (scancode) {
    case SCANCODE_BACKSPACE:
        TerminateLWEvent();
        TerminateGDI();
        exit (0);
        break;
    }

    return 0;
}

void GUIAPI ExitGUISafely (int exitcode)
{
    pthread_kill_other_threads_np ();

    TerminateLWEvent();
    TerminateGDI();

    exit (exitcode);
}

static int KeyMessageHandler (int message, int scancode, DWORD status)
{
    static int altdown = 0;
    static int modal = 0;
    PMAINWIN pNextWin;

    if ((message == MSG_KEYDOWN) && (status & KS_ALT) && (status & KS_CTRL))
        return HandleSpecialKey (scancode);
        
    if (scancode == SCANCODE_LEFTALT ||
        scancode == SCANCODE_RIGHTALT) {
        if (message == MSG_KEYDOWN) {
            altdown = 1;
            return 0;
        }
        else {
            altdown = 0;
            if (modal == 1) {
                modal = 0;
                return 0;
            }
        }
    }

    if (altdown) {
        if (message == MSG_KEYDOWN) {
            if( scancode == SCANCODE_TAB) {
            
                modal = 1;
                
                if (sg_ptmi)
                    dskForceCloseMenu ();

                pNextWin = dskGetNextVisibleWindow (pActiveMainWnd);
                dskMoveToTopMost (pNextWin);
                return 0;
            }
            else if (scancode == SCANCODE_ESCAPE) {

                modal = 1;

                if (pActiveMainWnd) {
#ifdef  _DEBUG
                    fprintf (stderr, "Will be Closed: %p.\n", pActiveMainWnd);
#endif
                    PostMessage ((HWND)pActiveMainWnd, MSG_CLOSE, 0, 0);
                    return 0;
                }
            }
        }
        else if (modal == 1)
            return 0;
    }
    
    if (scancode == SCANCODE_F10 || 
        scancode == SCANCODE_LEFTALT ||
        scancode == SCANCODE_RIGHTALT ||
        altdown)
        message = (message == MSG_KEYDOWN)?MSG_SYSKEYDOWN:MSG_SYSKEYUP;
    else if (sg_hIMEWnd != HWND_DESKTOP) {
        if (pActiveMainWnd && (pActiveMainWnd->dwExStyle & WS_EX_IMECOMPOSE)) {
            dskHandleKeyHooks (sg_hIMEWnd, message,
                    (WPARAM)scancode, (LPARAM)status);
            PostMessage (sg_hIMEWnd, message,
                    (WPARAM)scancode, (LPARAM)status);

            return 0;
        }
    }
    
    if (pActiveMainWnd) {
        dskHandleKeyHooks ((HWND)pActiveMainWnd, message, 
                (WPARAM)scancode, (LPARAM)status);
        PostMessage ((HWND)pActiveMainWnd, message, 
                (WPARAM)scancode, (LPARAM)status);
    }
    else {
        dskHandleKeyHooks (HWND_DESKTOP, message, 
                (WPARAM)scancode, (LPARAM)status);
        SendMessage (HWND_DESKTOP, MSG_DT_KEYOFF + message,
                (WPARAM)scancode, (LPARAM)status);
    }

    return 0;
}

static int MouseMessageHandler(int message, WPARAM flags, int x, int y)
{
    static PMAINWIN pOldUnderPointer = NULL;
    static PMAINWIN pCaptured = (void*)HWND_INVALID;
    PMAINWIN pUnderPointer;
    int CapHitCode = HT_UNKNOWN;
    int UndHitCode = HT_UNKNOWN;
    int cx = 0, cy = 0;

    if (message == MSG_WINDOWCHANGED) {
        pCaptured = (void*) HWND_INVALID;
        CapHitCode = HT_UNKNOWN;

        pOldUnderPointer = NULL;

        PostMessage (HWND_DESKTOP, MSG_MOUSEMOVE, 0, MAKELONG (x, y));
        return 0;
    }
    
    dskHandleMouseHooks (HWND_DESKTOP, message, flags, MAKELONG (x, y));
    
    if (hCaptureWnd)  {
        PostMessage (hCaptureWnd, message, flags|KS_CAPTURED, MAKELONG (x, y));
        return 0;
    }

    if (pCaptured != (void*)HWND_INVALID && pCaptured != NULL) {
        CapHitCode = SendAsyncMessage((HWND)pCaptured, MSG_HITTEST,
                                        (WPARAM)x, (LPARAM)y);
    }
    
    pUnderPointer = GetMainWinUnderPointer(x, y);

    if (pUnderPointer) {
        UndHitCode = SendAsyncMessage((HWND)pUnderPointer, MSG_HITTEST, 
                                        (WPARAM)x, (LPARAM)y);
        cx = x - pUnderPointer->cl;
        cy = y - pUnderPointer->ct;
    }
    
    switch (message)
    {
        case MSG_MOUSEMOVE:
            if (pCaptured != (void *)HWND_INVALID) {
                if (pCaptured)
                    PostMessage((HWND)pCaptured, MSG_NCMOUSEMOVE, 
                                CapHitCode, MAKELONG (x, y));
                else
                    PostMessage(HWND_DESKTOP, MSG_DT_MOUSEMOVE,
                                pUnderPointer == NULL, MAKELONG (x, y));
                break;
            }

            if(pUnderPointer == NULL) {
                if (pOldUnderPointer) {
                    PostMessage ((HWND)pOldUnderPointer,
                        MSG_MOUSEMOVEIN, FALSE, 0);
                    PostMessage ((HWND)pOldUnderPointer,
                        MSG_NCMOUSEMOVE, HT_OUT, MAKELONG (x, y));
                }
                SetCursor(GetSystemCursor(IDC_ARROW));
            }
            else {
                if(pOldUnderPointer && (pOldUnderPointer != pUnderPointer))
                {
                    PostMessage ((HWND)pOldUnderPointer,
                        MSG_MOUSEMOVEIN, FALSE, 0);
                    PostMessage ((HWND)pOldUnderPointer,
                        MSG_NCMOUSEMOVE, HT_OUT, MAKELONG (x, y));
                    PostMessage ((HWND)pUnderPointer,
                        MSG_MOUSEMOVEIN, TRUE, 0);
                }

                if (pUnderPointer->dwStyle & WS_DISABLED) {
                    SetCursor(GetSystemCursor(IDC_ARROW));
                    break;
                }

                if(UndHitCode == HT_CLIENT)
                {
                    SetCursor (pUnderPointer->hCursor);
                    PostMessage ((HWND)pUnderPointer, MSG_SETCURSOR, 
                            UndHitCode, MAKELONG (cx, cy));
                    PostMessage((HWND)pUnderPointer, MSG_NCMOUSEMOVE, 
                            UndHitCode, MAKELONG (x, y));
                    PostMessage((HWND)pUnderPointer, MSG_MOUSEMOVE, 
                            flags, MAKELONG (cx, cy));
                }
                else
                {
                    SetCursor (pUnderPointer->hCursor);
                    PostMessage ((HWND)pUnderPointer, MSG_NCSETCURSOR, 
                            UndHitCode, MAKELONG (x, y));
                    PostMessage((HWND)pUnderPointer, MSG_NCMOUSEMOVE, 
                            UndHitCode, MAKELONG (x, y));
                }
            }
        break;

        case MSG_LBUTTONDOWN:
        case MSG_RBUTTONDOWN:
            if (pUnderPointer) {
                if (sg_ptmi)
                    dskForceCloseMenu ();

                if (pUnderPointer->dwStyle & WS_DISABLED) {
                    Ping ();
                    break;
                }
                
                if (!dskIsTopMost (pUnderPointer))
                    dskMoveToTopMost (pUnderPointer);
                
                if (pUnderPointer != dskChangActiveWindow (pUnderPointer))
                    PostMessage ((HWND) pUnderPointer,
                        MSG_MOUSEACTIVE, UndHitCode, 0);
                
                if (UndHitCode != HT_CLIENT) {
                    if (UndHitCode & HT_NEEDCAPTURE)
                        pCaptured = pUnderPointer;
                    else
                        pCaptured = (void*)HWND_INVALID;

                    PostMessage ((HWND)pUnderPointer, message + MSG_NCMOUSEOFF,
                            UndHitCode, MAKELONG (x, y));
                }
                else {
                    PostMessage((HWND)pUnderPointer, message, 
                        flags, MAKELONG(cx, cy));
                    pCaptured = (void*)HWND_INVALID;
                }
            }
            else {
                dskChangActiveWindow (NULL);
                pCaptured = NULL;
                PostMessage (HWND_DESKTOP, message + MSG_DT_MOUSEOFF,
                            pUnderPointer == NULL, MAKELONG (x, y));
            }
        break;

        case MSG_LBUTTONUP:
        case MSG_RBUTTONUP:
            if (pCaptured != (void*)HWND_INVALID) {
                if (pCaptured)
                    PostMessage ((HWND)pCaptured, message + MSG_NCMOUSEOFF,
                        CapHitCode, MAKELONG (x, y));
                else
                    PostMessage (HWND_DESKTOP, message + MSG_DT_MOUSEOFF,
                        pUnderPointer == NULL, MAKELONG (x, y));
                
                pCaptured = (void*)HWND_INVALID;
                break;
            }
            else {
                if (pUnderPointer) {
                    if (pUnderPointer->dwStyle & WS_DISABLED) {
                        break;
                    }
                    
                    if(UndHitCode == HT_CLIENT)
                    {
                        PostMessage((HWND)pUnderPointer, message, 
                            flags, MAKELONG (cx, cy));
                    }
                    else
                    {
                        PostMessage((HWND)pUnderPointer, 
                            message + MSG_NCMOUSEOFF, 
                            UndHitCode, MAKELONG (x, y));
                    }
                }
                else
                    PostMessage (HWND_DESKTOP, message + MSG_DT_MOUSEOFF,
                        pUnderPointer == NULL, MAKELONG (x, y));
            }
        break;
        
        case MSG_LBUTTONDBLCLK:
        case MSG_RBUTTONDBLCLK:
            if (pUnderPointer)
            {
                if (pUnderPointer->dwStyle & WS_DISABLED) {
                    Ping ();
                    break;
                }

                if(UndHitCode == HT_CLIENT)
                    PostMessage((HWND)pUnderPointer, message, 
                        flags, MAKELONG(cx, cy));
                else
                    PostMessage((HWND)pUnderPointer, message + MSG_NCMOUSEOFF, 
                        UndHitCode, MAKELONG (x, y));
            }
            else {
                SetCursor(GetSystemCursor(IDC_ARROW));

                PostMessage(HWND_DESKTOP, message + MSG_DT_MOUSEOFF, 
                        TRUE, MAKELONG (x, y));
            }
        break;

    }

    pOldUnderPointer = pUnderPointer;

    return 0;
}

static int WindowMessageHandler(int message, PMAINWIN pWin, LPARAM lParam)
{
    PMAINWIN pTemp;
    POINT mousePos;
    int iRet = 0;

    switch (message)
    {
        case MSG_ADDNEWMAINWIN:
            if (sg_ptmi)
                dskForceCloseMenu ();
            iRet = (int)dskAddNewMainWindow(pWin, (PZORDERNODE)lParam);
        break;

        case MSG_REMOVEMAINWIN:
            if (sg_ptmi)
                dskForceCloseMenu ();
            iRet = (int)dskRemoveMainWindow(pWin);
        break;

        case MSG_MOVETOTOPMOST:
            if (sg_ptmi)
                dskForceCloseMenu ();
            pTemp = dskMoveToTopMost(pWin);
        break;

        case MSG_SHOWMAINWIN:
            if (sg_ptmi)
                dskForceCloseMenu ();
            iRet = (int)dskShowMainWindow(pWin);

            if ((HWND)pWin == sg_hIMEWnd)
                return iRet;
        break;

        case MSG_HIDEMAINWIN:
            if (sg_ptmi)
                dskForceCloseMenu ();
            iRet = (int)dskHideMainWindow(pWin);

            if ((HWND)pWin == sg_hIMEWnd)
                return iRet;
        break;

        case MSG_MOVEMAINWIN:
            dskMoveToTopMost(pWin);
            dskMoveMainWindow (pWin, (const RECT*)lParam);
        return 0;

        case MSG_GETACTIVEMAIN:
            return (int)pActiveMainWnd;
        break;
        
        case MSG_SETACTIVEMAIN:
            return (int)dskChangActiveWindow (pWin);
        break;

        case MSG_GETCAPTURE:
            return (int)hCaptureWnd;
        break;
        
        case MSG_SETCAPTURE:
            return (int)DesktopSetCapture ((HWND)pWin);
        break;
        
        case MSG_TRACKPOPUPMENU:
            return dskTrackPopupMenu((PTRACKMENUINFO)lParam);
        break;

        case MSG_ENDTRACKMENU:
            return dskEndTrackMenu((PTRACKMENUINFO)lParam);
        break;

        case MSG_CLOSEMENU:
            if (!dskForceCloseMenu ()) return 0;

            if (!lParam) return 0;
        break;

        case MSG_SCROLLMAINWIN:
            return dskScrollMainWindow (pWin, (PSCROLLWINDOWINFO)lParam);
        break;

        case MSG_CARET_CREATE:
            sg_hCaretWnd = (HWND)pWin;
            sg_uCaretBTime = pWin->pCaretInfo->uTime;
            return 0;

        case MSG_CARET_DESTROY:
            sg_hCaretWnd = HWND_DESKTOP;
            return 0;

        case MSG_ENABLEMAINWIN:
            iRet = !(pWin->dwStyle & WS_DISABLED);

            if ( (!(pWin->dwStyle & WS_DISABLED) && !lParam)
                    || ((pWin->dwStyle & WS_DISABLED) && lParam) ) {
                if (lParam)
                    pWin->dwStyle &= ~WS_DISABLED;
                else
                    pWin->dwStyle |=  WS_DISABLED;

                if (pWin->dwStyle & WS_DISABLED) {
                
                    if (hCaptureWnd && GetMainWindow (hCaptureWnd) == pWin) 
                        hCaptureWnd = HWND_DESKTOP;

                    if (pActiveMainWnd == pWin) {
                        dskChangActiveWindow (NULL);
                        break;
                    }
                }

                SendAsyncMessage ((HWND)pWin, MSG_NCPAINT, 0, 0);
            }
            break;
        
        case MSG_ISENABLED:
            return !(pWin->dwStyle & WS_DISABLED);
        
        case MSG_SETWINCURSOR:
        {
            HCURSOR old = pWin->hCursor;

            pWin->hCursor = (HCURSOR)lParam;
            return old;
        }
            break;
   }

   GetCursorPos (&mousePos);
   MouseMessageHandler (MSG_WINDOWCHANGED, 0, mousePos.x, mousePos.y);

   return iRet;
}

#define IDM_REDRAWBG    MINID_RESERVED
#define IDM_CLOSEALLWIN (MINID_RESERVED + 1)
#define IDM_ENDSESSION  (MINID_RESERVED + 2)

#define IDM_FIRSTWINDOW (MINID_RESERVED + 101)

static HMENU dskCreateWindowSubMenu ()
{
    HMENU hmnu;
    MENUITEMINFO mii;

    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = 0;
    mii.typedata    = (DWORD)"Windows...";

    hmnu = CreatePopupMenu (&mii);
    return hmnu;
}

static HMENU dskCreateDesktopMenu ()
{
    HMENU hmnu;
    MENUITEMINFO mii;

    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = 0;
    mii.typedata    = (DWORD)"开始...";

    hmnu = CreatePopupMenu (&mii);

    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = IDM_REDRAWBG;
    mii.typedata    = (DWORD)"刷新背景"; 
    InsertMenuItem (hmnu, 0, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.id          = IDM_CLOSEALLWIN;
    mii.typedata    = (DWORD)"关闭所有窗口"; 
    InsertMenuItem (hmnu, 1, TRUE, &mii);
    
    mii.type        = MFT_STRING;
    mii.id          = IDM_ENDSESSION;
    mii.typedata    = (DWORD)"终止会话";
    InsertMenuItem (hmnu, 2, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.id          = 0;
    mii.typedata    = (DWORD)"窗口...";
    mii.hsubmenu     = dskCreateWindowSubMenu();
    InsertMenuItem (hmnu, 3, TRUE, &mii);
                        
    mii.type        = MFT_SEPARATOR;
    mii.id          = 0;
    mii.typedata    = 0;
    mii.hsubmenu    = 0;
    InsertMenuItem(hmnu, 4, TRUE, &mii);

    return hmnu;
}

static void dskUpdateDesktopMenu (HMENU hDesktopMenu)
{
    MENUITEMINFO mii;
    HMENU hWinMenu;
    int nCount, iPos;
    PZORDERNODE pNode;
    PMAINWIN    pWin;
    int id;

    hWinMenu = GetSubMenu (hDesktopMenu, 3);

    nCount = GetMenuItemCount (hWinMenu);

    for (iPos = nCount; iPos > 0; iPos --)
        DeleteMenu (hWinMenu, iPos - 1, MF_BYPOSITION);
    
    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type = MFT_STRING;

    id = IDM_FIRSTWINDOW;
    iPos = 0;
    
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        pWin = (PMAINWIN)(pNode->hWnd);
        if (pWin->dwStyle & WS_VISIBLE) {
            mii.state       = MFS_ENABLED;
        }
        else
            mii.state       = MFS_DISABLED;
            
        mii.id              = id;
        mii.typedata        = (DWORD)pWin->spCaption; 
        InsertMenuItem(hWinMenu, iPos, TRUE, &mii);

        id++;
        iPos++;
        pNode = pNode->pNext;
    }
    
    if (iPos != 0) {
        mii.type            = MFT_SEPARATOR;
        mii.state           = 0;
        mii.id              = 0;
        mii.typedata        = 0;
        InsertMenuItem(hWinMenu, iPos, TRUE, &mii);
        iPos ++;
        mii.type            = MFT_STRING;
    }

    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        pWin = (PMAINWIN)(pNode->hWnd);
        if (pWin->dwStyle & WS_VISIBLE) {
            mii.state       = MFS_ENABLED;
        }
        else
            mii.state       = MFS_DISABLED;
            
        mii.id              = id;
        mii.typedata        = (DWORD)pWin->spCaption; 
        InsertMenuItem(hWinMenu, iPos, TRUE, &mii);

        id++;
        iPos++;
        pNode = pNode->pNext;
    }
    
    nCount = GetMenuItemCount (hDesktopMenu);
    for (iPos = nCount; iPos > 5; iPos --)
        DeleteMenu (hDesktopMenu, iPos - 1, MF_BYPOSITION);
        
    CustomizeDesktopMenu (hDesktopMenu, 5);
}

static PMAINWIN dskGetWindowFromZOrder (int iZOrder)
{
    int i = 0;
    PZORDERNODE pNode;
    
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode)
    {
        if (iZOrder == i)
            return (PMAINWIN)(pNode->hWnd);

        i ++;
        pNode = pNode->pNext;
    }

    pNode = MainWinZOrder.pTopMost;
    while (pNode)
    {
        if (iZOrder == i)
            return (PMAINWIN)(pNode->hWnd);

        i ++;
        pNode = pNode->pNext;
    }

    return NULL;
}

static int dskDesktopCommand (int id)
{
    if (id == IDM_REDRAWBG)
        SendMessage (HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);
    else if (id == IDM_CLOSEALLWIN) {
        PZORDERNODE pNode;
        HWND hwnd;
    
        pNode = TopMostWinZOrder.pTopMost;
        while (pNode) {
            hwnd = pNode->hWnd;
            if (((PMAINWIN)hwnd)->pHosting == NULL) // skip hosted main window.
                PostMessage (hwnd, MSG_CLOSE, 0, 0);
            pNode = pNode->pNext;
        }

        pNode = MainWinZOrder.pTopMost;
        while (pNode) {
            hwnd = pNode->hWnd;
            if (((PMAINWIN)hwnd)->pHosting == NULL) // skip hosted main window.
                PostMessage (hwnd, MSG_CLOSE, 0, 0);
            pNode = pNode->pNext;
        }
    }
    else if (id == IDM_ENDSESSION)
        PostMessage (HWND_DESKTOP, MSG_ENDSESSION, 0, 0);
    else if (id >= IDM_FIRSTWINDOW) {
        PMAINWIN pSelectedWin;

        pSelectedWin = dskGetWindowFromZOrder (id - IDM_FIRSTWINDOW);

        if (pSelectedWin && !(pSelectedWin->dwStyle & WS_DISABLED)) {
            if (sg_ptmi)
               dskForceCloseMenu ();

            dskMoveToTopMost (pSelectedWin);
        }
    }

    return 0;
}

static int dskBroadcastMessage (PMSG pMsg)
{
    PZORDERNODE pNode;
    HWND hwnd;
    int count = 0;
    
    pNode = TopMostWinZOrder.pTopMost;
    while (pNode) {
        hwnd = pNode->hWnd;
        PostMessage (hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
        count ++;
        pNode = pNode->pNext;
    }

    pNode = MainWinZOrder.pTopMost;
    while (pNode) {
        hwnd = pNode->hWnd;
        PostMessage (hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
        count ++;
        pNode = pNode->pNext;
    }

    return count;
}

static void dskOnNewCtrlInstance (PMAINWIN pWin, PCONTROL pNewCtrl)
{
    PCONTROL pFirstCtrl, pLastCtrl;
        
    pFirstCtrl = (PCONTROL)(pWin->hFirstChild);

    pNewCtrl->next = NULL;
    
    if (!pFirstCtrl) {
        pWin->hFirstChild = (HWND) pNewCtrl;
        pNewCtrl->prev = NULL;
    }
    else {
        pLastCtrl = pFirstCtrl;
        
        while (pLastCtrl->next)
            pLastCtrl = pLastCtrl->next;
            
        pLastCtrl->next = pNewCtrl;
        pNewCtrl->prev = pLastCtrl;
    }

    pthread_mutex_init (&pNewCtrl->InvRgn.lock, NULL);
    InitClipRgn (&pNewCtrl->InvRgn.rgn, &sg_FreeInvRectList);
    EmptyClipRgn (&pNewCtrl->InvRgn.rgn);

    pNewCtrl->pcci->nUseCount ++;
}

static BOOL dskOnRemoveCtrlInstance (PMAINWIN pWin, PCONTROL pCtrl)
{
    PCONTROL pFirstCtrl;
    BOOL fFound = FALSE;

    pFirstCtrl = (PCONTROL)(pWin->hFirstChild);

    if (!pFirstCtrl)
        return FALSE;
    else {
        if (pFirstCtrl == pCtrl) {
            pWin->hFirstChild = (HWND)(pCtrl->next);
            if (pCtrl->next)
                pCtrl->next->prev = NULL;
            fFound = TRUE;
        }
        else {
            while (pFirstCtrl->next) {
                if (pFirstCtrl->next == pCtrl) {
                    pFirstCtrl->next = pCtrl->next;
                    if (pFirstCtrl->next)
                        pFirstCtrl->next->prev = pCtrl->prev;
                    fFound = TRUE;
                    break;
                }

                pFirstCtrl = pFirstCtrl->next;
            }
        }
    }

    if (fFound)
        pCtrl->pcci->nUseCount --;

    return fFound;
}

static int dskRegisterIMEWnd (HWND hwnd)
{
    if (sg_hIMEWnd != HWND_DESKTOP)
        return ERR_IME_TOOMUCHIMEWND;

    if (!MainWindow (hwnd))
        return ERR_INV_HWND;

    sg_hIMEWnd = hwnd;

    if (pActiveMainWnd && (pActiveMainWnd->dwExStyle & WS_EX_IMECOMPOSE))
        SendNotifyMessage (sg_hIMEWnd, MSG_IME_OPEN, 0, 0);
    else
        SendNotifyMessage (sg_hIMEWnd, MSG_IME_CLOSE, 0, 0);
        
    SendNotifyMessage (sg_hIMEWnd, MSG_IME_SETTARGET, 
            (WPARAM)pActiveMainWnd, 0);

    return ERR_OK;
}

static int dskUnregisterIMEWnd (HWND hwnd)
{
    if (sg_hIMEWnd != hwnd)
        return ERR_IME_NOSUCHIMEWND;

    sg_hIMEWnd = HWND_DESKTOP;

    return ERR_OK;
}

static int dskSetIMEStatus (int iIMEStatusCode, int Value)
{
    if (sg_hIMEWnd == HWND_DESKTOP)
        return ERR_IME_NOIMEWND;

    SendMessage (sg_hIMEWnd, 
        MSG_IME_SETSTATUS, (WPARAM)iIMEStatusCode, (LPARAM)Value);

    return ERR_OK;
}

static int dskGetIMEStatus (int iIMEStatusCode)
{
    if (sg_hIMEWnd == HWND_DESKTOP)
        return ERR_IME_NOIMEWND;

    return SendMessage (sg_hIMEWnd, MSG_IME_GETSTATUS, iIMEStatusCode, 0);
}

static int dskGetBgPicturePos ()
{
    char szValue [20];

    if( GetValueFromEtcFile(ETCFILEPATH, "bgpicture", "position",
            szValue, 10) < 0 ) {
        fprintf (stderr, "Get bgpicture position error!\n");
        return 0;
    }

    if (!strcmp (szValue, "center"))
        return 0;
    if (!strcmp (szValue, "upleft"))
        return 1;
    if (!strcmp (szValue, "downleft"))
        return 2;
    if (!strcmp (szValue, "upright"))
        return 3;
    if (!strcmp (szValue, "downright"))
        return 4;
    if (!strcmp (szValue, "upcenter"))
        return 5;
    if (!strcmp (szValue, "downcenter"))
        return 6;
    if (!strcmp (szValue, "vcenterleft"))
        return 7;
    if (!strcmp (szValue, "vcenterright"))
        return 8;

    return 0;
}

static void dskGetBgPictureXY (int pos, int w, int h, int* x, int* y)
{
    switch (pos) {
    case 0: // center
        *x = (sg_rcScr.right - w) >> 1;
        *y = (sg_rcScr.bottom - h) >> 1;
        break;
    case 1: // upleft
        *x = 0;
        *y = 0;
        break;
    case 2: // downleft
        *x = 0;
        *y = sg_rcScr.bottom - h;
        break;
    case 3: // upright
        *x = sg_rcScr.right - w;
        *y = 0;
        break;
    case 4: // downright
        *x = sg_rcScr.right - w;
        *y = sg_rcScr.bottom - h;
        break;
    case 5: // upcenter
        *x = (sg_rcScr.right - w) >> 1;
        *y = 0;
        break;
    case 6: // downcenter
        *x = (sg_rcScr.right - w) >> 1;
        *y = sg_rcScr.bottom - h;
        break;
    case 7: // vcenterleft
        *x = 0;
        *y = (sg_rcScr.bottom - h) >> 1;
        break;
    case 8: // vcenterright
        *x = sg_rcScr.right - w;
        *y = (sg_rcScr.bottom - h) >> 1;
        break;
    default:
        *x = 0;
        *y = 0;
        break;
    }
}

static int DesktopProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static BITMAP Bitmap;
    static BOOL fValid = TRUE;
    static int pic_x, pic_y;
    static HDC hDesktopDC;
    static HMENU hDesktopMenu;
    RECT* pInvalidRect;
    int flags, x, y;

    if (message >= MSG_FIRSTWINDOWMSG && message <= MSG_LASTWINDOWMSG)
        return WindowMessageHandler (message, (PMAINWIN)wParam, lParam);

    if (message >= MSG_FIRSTKEYMSG && message <= MSG_LASTKEYMSG) {
        if (wParam == SCANCODE_PRINTSCREEN && message == MSG_KEYDOWN)
        {
            static int n = 1;
            char buffer[20];

            sprintf (buffer, "%x-%d.bmp", (lParam & KS_CTRL)?
                                    (HWND)pActiveMainWnd:
                                    HWND_DESKTOP, n);
            if (SaveMainWindowContent ((lParam & KS_CTRL)?
                                    (HWND)pActiveMainWnd:
                                    HWND_DESKTOP,
                                    buffer)) {
                Ping ();
                n ++;
            }
        }
        else if (wParam == SCANCODE_ESCAPE && lParam & KS_CTRL) {
            dskUpdateDesktopMenu (hDesktopMenu);
            TrackPopupMenu (hDesktopMenu, TPM_DEFAULT, 
                0, sg_rcScr.bottom, HWND_DESKTOP);
        }

        if (sg_ptmi) {
            if (PopupMenuTrackProc (sg_ptmi, message, wParam, lParam))
                return KeyMessageHandler (message, (int)wParam, (DWORD)lParam);
        }
        else
            return KeyMessageHandler (message, (int)wParam, (DWORD)lParam);
    }
        
    if (message >= MSG_FIRSTMOUSEMSG && message <= MSG_LASTMOUSEMSG) {
    
        flags = (int)wParam;

        x = LOWORD (lParam);
        y = HIWORD (lParam);

        if (sg_ptmi) {
            if (PopupMenuTrackProc (sg_ptmi, message, x, y))
                return MouseMessageHandler (message, flags, x, y);
        }
        else
            return MouseMessageHandler (message, flags, x, y);
    }
    
    if (message == MSG_COMMAND) {
        if (wParam <= MAXID_RESERVED && wParam >= MINID_RESERVED)
            return dskDesktopCommand ((int)wParam);
        else
            return CustomDesktopCommand ((int)wParam);
    }

    switch(message)
    {
        case MSG_STARTSESSION:
            // set the global clip region of desktop
            hDesktopDC = CreatePrivateDC (HWND_DESKTOP);
            if (!LoadSystemBitmap (&Bitmap, "bgpicture")) {
                fprintf (stderr, "Loading bitmap failure!\n");
                fValid = FALSE;
            }
            else
                dskGetBgPictureXY (dskGetBgPicturePos (), 
                                    Bitmap.bmWidth, Bitmap.bmHeight,
                                    &pic_x, &pic_y);
            hDesktopMenu = dskCreateDesktopMenu ();
            SetCursorPos (sg_rcScr.right>>1, sg_rcScr.bottom>>1);
        break;

        case MSG_ENDSESSION:
            if ((MainWinZOrder.nNumber  + TopMostWinZOrder.nNumber) == 0) {
                UnloadBitmap(&Bitmap);
                DeletePrivateDC (hDesktopDC);
                DestroyMenu (hDesktopMenu);

                PostQuitMessage (HWND_DESKTOP);
                return 1;
            }
        break;

        case MSG_ERASEDESKTOP:
            SetBrushColor(hDesktopDC, COLOR_darkcyan);
            pInvalidRect = (PRECT)lParam;
            if (pInvalidRect) {
                SelectClipRect (hDesktopDC, pInvalidRect);
                FillBox(hDesktopDC, pInvalidRect->left, 
                       pInvalidRect->top, 
                       pInvalidRect->right, 
                       pInvalidRect->bottom);
            }
            else {
                SelectClipRect (hDesktopDC, &sg_rcScr);
                FillBox(hDesktopDC, sg_rcScr.left, sg_rcScr.top, 
                       sg_rcScr.right + 1, 
                       sg_rcScr.bottom + 1);
            }

            if( fValid )
                FillBoxWithBitmap(hDesktopDC, 
                                  pic_x, pic_y,
                                  Bitmap.bmWidth,
                                  Bitmap.bmHeight,
                                  &Bitmap);
        break;

        case MSG_TIMEOUT:
            BroadcastMessage (MSG_IDLE, wParam, 0);
        break;

        case MSG_ADDTIMER:
            return AddTimer (((PTIMER)lParam)->hWnd,
                      ((PTIMER)lParam)->id,
                      ((PTIMER)lParam)->speed);
        break;

        case MSG_REMOVETIMER:
            return RemoveTimer (((PTIMER)lParam)->hWnd,
                      ((PTIMER)lParam)->id);
        break;

        case MSG_RESETTIMER:
            return SetTimerSpeed (((PTIMER)lParam)->hWnd,
                      ((PTIMER)lParam)->id,
                      ((PTIMER)lParam)->speed);
        break;

        case MSG_BROADCASTMSG:
            return dskBroadcastMessage ((PMSG)lParam);

        case MSG_REGISTERWNDCLASS:
            return AddNewControlClass ((PWNDCLASS)lParam);

        case MSG_UNREGISTERWNDCLASS:
            return DeleteControlClass ((const char*)lParam);

        case MSG_NEWCTRLINSTANCE:
            dskOnNewCtrlInstance ((PMAINWIN)wParam, (PCONTROL)lParam);
            break;
            
        case MSG_REMOVECTRLINSTANCE:
            if (!dskOnRemoveCtrlInstance ((PMAINWIN)wParam, (PCONTROL)lParam))
                return -1;
            break;
        
        case MSG_GETCTRLCLASSINFO:
            return (int)GetControlClassInfo ((const char*)lParam);

        case MSG_CTRLCLASSDATAOP:
            return (int)ControlClassDataOp (wParam, (WNDCLASS*)lParam);
            
        case MSG_REGISTERKEYHOOK:
            return (int)dskRegisterKeyHook ((HWND)wParam, 
                            (KEYMSGHOOK)lParam);

        case MSG_REGISTERMOUSEHOOK:
            return (int)dskRegisterMouseHook ((HWND)wParam, 
                            (MOUSEMSGHOOK)lParam);

        case MSG_UNREGISTERHOOK:
            return dskUnregisterHook ((HHOOK)wParam);

        case MSG_IME_REGISTER:
            return dskRegisterIMEWnd ((HWND)wParam);
            
        case MSG_IME_UNREGISTER:
            return dskUnregisterIMEWnd ((HWND)wParam);
            
        case MSG_IME_SETSTATUS:
            return dskSetIMEStatus ((int)wParam, (int)lParam);

        case MSG_IME_GETSTATUS:
            return dskGetIMEStatus ((int)wParam);
            
        case MSG_DT_LBUTTONDOWN:
        case MSG_DT_LBUTTONUP:
        case MSG_DT_LBUTTONDBLCLK:
        case MSG_DT_MOUSEMOVE:
        case MSG_DT_RBUTTONDOWN:
        break;
        
        case MSG_DT_RBUTTONUP:
            if (!wParam)
                break;

            x = LOWORD (lParam);
            y = HIWORD (lParam);
            dskUpdateDesktopMenu (hDesktopMenu);
            TrackPopupMenu (hDesktopMenu, TPM_DEFAULT, x, y, HWND_DESKTOP);
        break;
            
        case MSG_DT_RBUTTONDBLCLK:
        break;

        case MSG_TIMER:      // per 0.01s
        {
            static UINT uCounter = 0;

            DispatchTimerMessage ();

            if (timer_counter % 10 != 0)
                break;

            uCounter += 100;
            if ((sg_hCaretWnd != HWND_DESKTOP)
                && (uCounter >= sg_uCaretBTime)) {
                PostMessage (sg_hCaretWnd, MSG_CARETBLINK, 0, 0);
                uCounter = 0;
            }
        }
        break;
        
    }

    return 0;
}

#ifdef _DEBUG

void* DesktopMain(void* data)
{
    MSG Msg;
    PSYNCMSG pSyncMsg;
    int iRet = 0;

    DesktopProc(HWND_DESKTOP, MSG_STARTSESSION, 0, 0);

    PostMessage(HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);

    sem_post ((sem_t*)data);

    while (GetMessage(&Msg, HWND_DESKTOP)) {
        fprintf (stderr, "Message, %s: hWnd: %x, wP: %x, lP: %lx. %s\n",
            Message2Str (Msg.message),
            Msg.hwnd,
            Msg.wParam,
            Msg.lParam,
            Msg.pAdd?"Sync":"Normal");

        if ( Msg.pAdd ) // this is a sync message.
        {
            pSyncMsg = (PSYNCMSG)(Msg.pAdd);
            pSyncMsg->retval = DesktopProc(HWND_DESKTOP, 
                   Msg.message, Msg.wParam, Msg.lParam);
            sem_post(&pSyncMsg->sem_handle);
            iRet = pSyncMsg->retval;
        }
        else
            iRet = DesktopProc(HWND_DESKTOP, 
                    Msg.message, Msg.wParam, Msg.lParam);

        fprintf (stderr, "Message, %s done, return value: %x\n",
            Message2Str (Msg.message), iRet);
    }

    return NULL;
}

#else

void* DesktopMain(void* data)
{
    MSG Msg;
    PSYNCMSG pSyncMsg;

    DesktopProc(HWND_DESKTOP, MSG_STARTSESSION, 0, 0);

    PostMessage(HWND_DESKTOP, MSG_ERASEDESKTOP, 0, 0);

    sem_post ((sem_t*)data);

    while (GetMessage(&Msg, HWND_DESKTOP)) {

        if ( Msg.pAdd ) // this is a sync message.
        {
            pSyncMsg = (PSYNCMSG)(Msg.pAdd);
            pSyncMsg->retval = DesktopProc(HWND_DESKTOP, 
                   Msg.message, Msg.wParam, Msg.lParam);
            sem_post(&pSyncMsg->sem_handle);
        }
        else
            DesktopProc(HWND_DESKTOP, Msg.message, Msg.wParam, Msg.lParam);
    }

    return NULL;
}

#endif

pthread_t GUIAPI GetMainWinThread(HWND hMainWnd)
{
    if(hMainWnd == HWND_DESKTOP) return desktop;

    return ((PMAINWIN)hMainWnd)->th;
}

/****************************** Message support ******************************/
static FREEQMSGLIST FreeQMSGList;

/* QMSG allocation */
static BOOL InitFreeQMSGList()
{
    if( !(FreeQMSGList.heap = malloc (sizeof(QMSG)*SIZE_QMSG_HEAP))) 
        return FALSE;
        
    FreeQMSGList.free = 0;

    pthread_mutex_init (&FreeQMSGList.lock, NULL);
    FreeQMSGList.head = FreeQMSGList.tail = NULL;
    FreeQMSGList.nr = 0;

    return TRUE;
}

static PQMSG QMSGAlloc()
{
    PQMSG pqmsg;

    pthread_mutex_lock (&FreeQMSGList.lock);
    if (FreeQMSGList.head) {
        pqmsg = FreeQMSGList.head;
        FreeQMSGList.head = pqmsg->next;
        FreeQMSGList.nr --;
    }
    else {
        if (FreeQMSGList.free < SIZE_QMSG_HEAP) {
            pqmsg = FreeQMSGList.heap + FreeQMSGList.free;
            pqmsg->fromheap = TRUE;
            FreeQMSGList.free ++;
        }
        else {
            pqmsg = malloc (sizeof(QMSG));
            if (pqmsg == NULL)
                fprintf (stderr, "DESKTOP error: alloc qmessage failure!\n");
            else
                pqmsg->fromheap = FALSE;
        }
    }

    pthread_mutex_unlock (&FreeQMSGList.lock);

    return pqmsg;
}

static void FreeQMSG(PQMSG pqmsg)
{
    pthread_mutex_lock (&FreeQMSGList.lock);

    pqmsg->next = NULL;
    if (FreeQMSGList.head) {
        FreeQMSGList.tail->next = pqmsg;
        FreeQMSGList.tail = pqmsg;
    }
    else {
        FreeQMSGList.head = FreeQMSGList.tail = pqmsg; 
    }

    FreeQMSGList.nr++;
    pthread_mutex_unlock (&FreeQMSGList.lock);
}

static void EmptyFreeQMSGList()
{
    PQMSG pqmsg, pTemp;

    pthread_mutex_lock (&FreeQMSGList.lock);

    pqmsg = FreeQMSGList.head;

    while (pqmsg) {
        pTemp = pqmsg->next;
        
        if (!pqmsg->fromheap)
            free (pqmsg);

        pqmsg = pTemp;
    }

    FreeQMSGList.head = FreeQMSGList.tail = NULL;
    FreeQMSGList.nr = 0;
    FreeQMSGList.free = 0;

    pthread_mutex_unlock (&FreeQMSGList.lock);
}

static void DestroyFreeQMSGList()
{
    EmptyFreeQMSGList ();
    free (FreeQMSGList.heap);
}

BOOL GUIAPI InitMsgQueue (PMSGQUEUE pMsgQueue, int iBufferLen)
{
    int i;
    
    pMsgQueue->dwState = QS_EMPTY;

    pthread_mutex_init (&pMsgQueue->lock, NULL);

    sem_init (&pMsgQueue->wait, 0, 0);

    pMsgQueue->pFirstNotifyMsg = NULL;
    pMsgQueue->pLastNotifyMsg = NULL;

    pMsgQueue->readpos = 0;
    pMsgQueue->writepos = 0;
    pMsgQueue->pFirstSyncMsg = NULL;
    pMsgQueue->pLastSyncMsg = NULL;

    if (iBufferLen <= 0)
        iBufferLen = DEF_MSGQUEUE_LEN;
        
    pMsgQueue->msg = malloc (sizeof (MSG) * iBufferLen);
    pMsgQueue->len = iBufferLen;

    pMsgQueue->TimerMask = 0xFF;
    for (i = 0; i < 8; i++) {
        pMsgQueue->TimerOwner [i] = HWND_DESKTOP;
        pMsgQueue->TimerID [i] = 0;
    }

    return pMsgQueue->msg != NULL;
}

void GUIAPI DestroyMsgQueue (PMSGQUEUE pMsgQueue)
{
    free (pMsgQueue->msg);
    pMsgQueue->msg = NULL;
}

PMAINWIN MainWindow(HWND hWnd)
{
    PMAINWIN pWin;

    pWin = (PMAINWIN)hWnd;
    if(pWin->DataType != TYPE_HWND)
        return NULL;

    if (pWin->WinType == TYPE_MAINWIN)
        return pWin;

    return NULL; 
}

PMAINWIN GetMainWindow(HWND hWnd)
{
    PMAINWIN pWin;

    if (hWnd == HWND_DESKTOP)
        return NULL;

    pWin = (PMAINWIN)hWnd;
    if(pWin->DataType != TYPE_HWND)
        return NULL;

    if (pWin->WinType == TYPE_MAINWIN)
        return pWin;

    return ((PCONTROL)hWnd)->pMainWin;
}

PMSGQUEUE GetMsgQueue(HWND hWnd)
{
    if(hWnd == HWND_DESKTOP) return &DskMsgs;

    return GetMainWindow(hWnd)->pMessages;
}

HWND GUIAPI GetMainWindowHandle(HWND hWnd)
{
    PCONTROL pChildWin;

    pChildWin = (PCONTROL)hWnd;
    if(pChildWin->DataType != TYPE_HWND)
        return HWND_INVALID;

    if (pChildWin->WinType == TYPE_CONTROL)
        return (HWND)(pChildWin->pMainWin);

    return hWnd;
}

static inline WNDPROC GetWndProc(HWND hWnd)
{
     PMAINWIN  pMainWin = (PMAINWIN)hWnd;
     PCONTROL pChildWin = (PCONTROL)hWnd;

     if( hWnd == HWND_DESKTOP )
         return DesktopProc;

     if( pMainWin->WinType == TYPE_MAINWIN)
         return pMainWin->MainWindowProc;

     return pChildWin->pcci->ControlProc;
}

static HWND msgCheckInvalidRegion (PMAINWIN pWin)
{
    PCONTROL pCtrl;

    if (pWin->InvRgn.rgn.head)
        return (HWND)pWin;

    pCtrl = (PCONTROL)(pWin->hFirstChild);
    if (pCtrl) {
            
        while (pCtrl) {
            if (pCtrl->InvRgn.rgn.head)
                return (HWND)pCtrl;

            pCtrl = pCtrl->next;
        }
    }

    return 0;
}

static PMAINWIN msgGetHostingRoot (PMAINWIN pHosted)
{
    PMAINWIN pHosting;
    
    pHosting = pHosted->pHosting;
    if (pHosting)
        return msgGetHostingRoot (pHosting);

    return pHosted;
}

static HWND msgCheckHostedTree (PMAINWIN pHosting)
{
    HWND hNeedPaint;
    PMAINWIN pHosted;

    if ( (hNeedPaint = msgCheckInvalidRegion (pHosting)) )
        return hNeedPaint;

    pHosted = pHosting->pFirstHosted;
    while (pHosted) {
        if ( (hNeedPaint = msgCheckHostedTree (pHosted)) )
            return hNeedPaint;

        pHosted = pHosted->pNextHosted;
    }

    return 0;
}

int GUIAPI GetMessage(PMSG pMsg, HWND hWnd)
{
    PMAINWIN pWin;
    PMSGQUEUE pMsgQueue;
    PQMSG phead;
    int slot;

    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return ERR_INV_HWND;

    memset (pMsg, 0, sizeof(MSG));

checkagain:

    if (pMsgQueue->dwState & QS_NOTIFYMSG) {
       
        pthread_mutex_lock (&pMsgQueue->lock);
        if (pMsgQueue->pFirstNotifyMsg) {
            phead = pMsgQueue->pFirstNotifyMsg;
            pMsgQueue->pFirstNotifyMsg = phead->next;
            
            *pMsg = phead->Msg;
            pMsg->pAdd = NULL;

            FreeQMSG (phead);

            pthread_mutex_unlock (&pMsgQueue->lock);
            return 1;
        }
        else
            pMsgQueue->dwState &= ~QS_NOTIFYMSG;
        
        pthread_mutex_unlock (&pMsgQueue->lock);
    }

    if (pMsgQueue->dwState & QS_SYNCMSG) {
        pthread_mutex_lock (&pMsgQueue->lock);
        if (pMsgQueue->pFirstSyncMsg) {
            *pMsg = pMsgQueue->pFirstSyncMsg->Msg;
            pMsg->pAdd = pMsgQueue->pFirstSyncMsg;
            pMsgQueue->pFirstSyncMsg = pMsgQueue->pFirstSyncMsg->pNext;

            pthread_mutex_unlock (&pMsgQueue->lock);
            return 1;
        }
        else
            pMsgQueue->dwState &= ~QS_SYNCMSG;
            
        pthread_mutex_unlock (&pMsgQueue->lock);
    }

    if (pMsgQueue->dwState & QS_POSTMSG) {
    
        pthread_mutex_lock (&pMsgQueue->lock);
        if (pMsgQueue->readpos != pMsgQueue->writepos) {

            *pMsg = pMsgQueue->msg[pMsgQueue->readpos];
            pMsg->pAdd = NULL;

            pMsgQueue->readpos++;
            if (pMsgQueue->readpos >= pMsgQueue->len) pMsgQueue->readpos = 0;

            pthread_mutex_unlock (&pMsgQueue->lock);
            return 1;
        }
        else
            pMsgQueue->dwState &= ~QS_POSTMSG;

        pthread_mutex_unlock (&pMsgQueue->lock);
    }

    if (pMsgQueue->dwState & QS_QUIT) {
        pMsg->hwnd = hWnd;
        pMsg->message = MSG_QUIT;
        pMsg->wParam = 0;
        pMsg->lParam = 0;
        pMsg->pAdd = NULL;

        pMsgQueue->dwState &= ~QS_QUIT;
        
        return 0;
    }

    if (pMsgQueue->dwState & QS_PAINT) {
        PMAINWIN pHostingRoot;
        HWND hNeedPaint;
        
        pWin = GetMainWindow (hWnd);

        pMsg->message = MSG_PAINT;
        pMsg->wParam = 0;
        pMsg->lParam = 0;
        pMsg->pAdd = NULL;

        pHostingRoot = msgGetHostingRoot (pWin);

        if ( (hNeedPaint = msgCheckHostedTree (pHostingRoot)) ) {
            pMsg->hwnd = hNeedPaint;
            return 1;
        }
        
        pMsgQueue->dwState &= ~QS_PAINT;
    }
    
    if (pMsgQueue->dwState & QS_TIMER) {
        if (hWnd == HWND_DESKTOP) {
            pMsg->hwnd = hWnd;
            pMsg->message = MSG_TIMER;
            pMsg->wParam = 0;
            pMsg->lParam = 0;
            pMsg->pAdd = NULL;

            pMsgQueue->dwState &= ~0x01;

            return 1;
        }
        
        for (slot=0; slot<8; slot++) {
            if (pMsgQueue->dwState & (0x01 << slot))
                break;
        }
        if (slot == 8)
            pMsgQueue->dwState &= ~QS_TIMER;
        else {
            pMsg->hwnd = pMsgQueue->TimerOwner[slot];
            pMsg->message = MSG_TIMER;
            pMsg->wParam = pMsgQueue->TimerID[slot];
            pMsg->lParam = 0;
            pMsg->pAdd = NULL;

            pMsgQueue->dwState &= ~(0x01 << slot);

            return 1;
        }
    }

    // no message, wait again.
    sem_wait (&pMsgQueue->wait);
    goto checkagain;

    return 1;
}

BOOL GUIAPI HavePendingMessage (HWND hWnd)
{
    PMSGQUEUE pMsgQueue;

    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return FALSE;
    
    if (pMsgQueue->dwState & QS_NOTIFYMSG) {
        if (pMsgQueue->pFirstNotifyMsg)
            return TRUE;
    }

    if (pMsgQueue->dwState & QS_SYNCMSG) {
        if (pMsgQueue->pFirstSyncMsg)
            return TRUE;
    }

    if (pMsgQueue->dwState & QS_POSTMSG) {
        if (pMsgQueue->readpos != pMsgQueue->writepos)
            return TRUE;
    }

    if (pMsgQueue->dwState & (QS_QUIT | QS_PAINT | QS_TIMER))
        return TRUE;

    return FALSE;
}

int GUIAPI ThrowAwayMessages (HWND hWnd)
{
    PMSG        pMsg;
    PMSGQUEUE   pMsgQueue;
    PQMSG       pQMsg;
    PSYNCMSG    pSyncMsg;
    int         nCount = 0;

    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return ERR_INV_HWND;

    pthread_mutex_lock (&pMsgQueue->lock);

    if (pMsgQueue->pFirstNotifyMsg) {
        pQMsg = pMsgQueue->pFirstNotifyMsg;
        
        while (pQMsg) {
            pMsg = &pQMsg->Msg;

            if (GetMainWindow (pMsg->hwnd) == (PMAINWIN)hWnd) {
                pMsg->hwnd = HWND_INVALID;
                nCount ++;
            }

            pQMsg = pQMsg->next;
        }
    }

    if (pMsgQueue->pFirstSyncMsg) {
        pSyncMsg = pMsgQueue->pFirstSyncMsg;
        
        while (pSyncMsg) {
            pMsg = &pSyncMsg->Msg;

            if (GetMainWindow (pMsg->hwnd) == (PMAINWIN)hWnd) {
                pMsg->hwnd = HWND_INVALID;
                nCount ++;
            }

            pSyncMsg = pSyncMsg->pNext;
        }
    }

    while (pMsgQueue->readpos != pMsgQueue->writepos) {
        pMsg = pMsgQueue->msg + pMsgQueue->readpos;

        if (GetMainWindow (pMsg->hwnd) == (PMAINWIN)hWnd) {
            pMsg->hwnd = HWND_INVALID;
            nCount ++;
        }
        
        pMsgQueue->readpos++;
        if (pMsgQueue->readpos >= pMsgQueue->len) 
            pMsgQueue->readpos = 0;
    }

    pthread_mutex_unlock (&pMsgQueue->lock);

    return nCount;
}

BOOL GUIAPI PeekPostMessage (PMSG pMsg, HWND hWnd, int iMsgFilterMin, 
                        int iMsgFilterMax, UINT uRemoveMsg)
{
    PMSGQUEUE pMsgQueue;
    PMSG pPostMsg;
    
    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return FALSE;

    memset (pMsg, 0, sizeof(MSG));

    if (pMsgQueue->dwState & QS_POSTMSG) {
    
        pthread_mutex_lock (&pMsgQueue->lock);
        if (pMsgQueue->readpos != pMsgQueue->writepos) {

            pPostMsg = pMsgQueue->msg + pMsgQueue->readpos;

            if (iMsgFilterMin == 0 && iMsgFilterMax == 0)
                *pMsg = *pPostMsg;
            else if (pPostMsg->message <= iMsgFilterMax &&
                    pPostMsg->message >= iMsgFilterMin)
                *pMsg = *pPostMsg;
            else {
                pthread_mutex_unlock (&pMsgQueue->lock);
                return FALSE;
            }
            

            pMsg->pAdd = NULL;

            if (uRemoveMsg == PM_REMOVE) {
                pMsgQueue->readpos++;
                if (pMsgQueue->readpos >= pMsgQueue->len) 
                    pMsgQueue->readpos = 0;
            }

            pthread_mutex_unlock (&pMsgQueue->lock);
            return TRUE;
        }

        pthread_mutex_unlock (&pMsgQueue->lock);
    }

    return FALSE;
}
                        
int GUIAPI PostMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam)
{
    PMSGQUEUE pMsgQueue;
    PMSG pMsg;
    int sem_value;

    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return ERR_INV_HWND;

    pthread_mutex_lock (&pMsgQueue->lock);

    if (iMsg == MSG_PAINT) {
        pMsgQueue->dwState |= QS_PAINT;
        goto goodret;
    }
    
    if ((pMsgQueue->writepos + 1) % pMsgQueue->len == pMsgQueue->readpos) {
        pthread_mutex_unlock (&pMsgQueue->lock);
        return ERR_QUEUE_FULL;
    }
    
    // Write the data and advance write pointer */
    pMsg = &(pMsgQueue->msg[pMsgQueue->writepos]);
    pMsg->hwnd = hWnd;
    pMsg->message = iMsg;
    pMsg->wParam = wParam;
    pMsg->lParam = lParam;

    pMsgQueue->writepos++;
    if (pMsgQueue->writepos >= pMsgQueue->len) pMsgQueue->writepos = 0;

    pMsgQueue->dwState |= QS_POSTMSG;

goodret:

    pthread_mutex_unlock (&pMsgQueue->lock);
    // Signal that the msg queue contains one more element for reading
    sem_getvalue (&pMsgQueue->wait, &sem_value);
    if (sem_value == 0)
        sem_post(&pMsgQueue->wait);

    return ERR_OK;
}


int GUIAPI PostQuitMessage(HWND hWnd)
{
    PMSGQUEUE pMsgQueue;
    int sem_value;

    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return ERR_INV_HWND;

    pMsgQueue->dwState |= QS_QUIT;

    // Signal that the msg queue contains one more element for reading
    sem_getvalue (&pMsgQueue->wait, &sem_value);
    if (sem_value == 0)
        sem_post(&pMsgQueue->wait);

    return ERR_OK;
}

int GUIAPI PostSyncMessage(HWND hWnd, int msg, WPARAM wParam, LPARAM lParam)
{
    PMSGQUEUE pMsgQueue;
    SYNCMSG SyncMsg;
    int sem_value;

    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return ERR_INV_HWND;

    pthread_mutex_lock (&pMsgQueue->lock);

    // queue the sync message.
    SyncMsg.Msg.hwnd = hWnd;
    SyncMsg.Msg.message = msg;
    SyncMsg.Msg.wParam = wParam;
    SyncMsg.Msg.lParam = lParam;
    SyncMsg.pNext = NULL;
    sem_init(&SyncMsg.sem_handle, 0, 0);

    if (pMsgQueue->pFirstSyncMsg == NULL) {
        pMsgQueue->pFirstSyncMsg = pMsgQueue->pLastSyncMsg = &SyncMsg;
    }
    else {
        pMsgQueue->pLastSyncMsg->pNext = &SyncMsg;
        pMsgQueue->pLastSyncMsg = &SyncMsg;
    }

    pMsgQueue->dwState |= QS_SYNCMSG;

    pthread_mutex_unlock (&pMsgQueue->lock);

    // Signal that the msg queue contains one more element for reading
    sem_getvalue (&pMsgQueue->wait, &sem_value);
    if (sem_value == 0)
        sem_post(&pMsgQueue->wait);

    // suspend until the message been handled.
    sem_wait(&SyncMsg.sem_handle);

    sem_destroy(&SyncMsg.sem_handle);

    return SyncMsg.retval;
}

int GUIAPI SendMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC WndProc;
    PMAINWIN pMainWin;

    if (hWnd == HWND_INVALID) return -1;

    if (hWnd == HWND_DESKTOP) {
        if (pthread_self() != desktop)
            return PostSyncMessage (HWND_DESKTOP, iMsg, wParam, lParam);
        else
            return DesktopProc (hWnd, iMsg, wParam, lParam);
    }

    pMainWin = GetMainWindow (hWnd);

    if (pMainWin->th != pthread_self())
        return PostSyncMessage (hWnd, iMsg, wParam, lParam);
    
    WndProc = GetWndProc(hWnd);

    return (*WndProc)(hWnd, iMsg, wParam, lParam);

}

int GUIAPI SendNotifyMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam)
{
    PMAINWIN pMainWin;
    PMSGQUEUE pMsgQueue;
    PQMSG pqmsg;
    int sem_value;

    if (hWnd == HWND_INVALID) return -1;

    if( !(pMsgQueue = GetMsgQueue(hWnd)) ) return ERR_INV_HWND;

    if (hWnd == HWND_DESKTOP) {
        if (pthread_self() == desktop)
            return DesktopProc (hWnd, iMsg, wParam, lParam);
    }
    else {
        pMainWin = GetMainWindow (hWnd);
        if (pMainWin->th == pthread_self()) {
            WNDPROC WndProc;
    
            WndProc = GetWndProc(hWnd);

            return (*WndProc)(hWnd, iMsg, wParam, lParam);
        }
    }
    
    pqmsg = QMSGAlloc();

    pthread_mutex_lock (&pMsgQueue->lock);

    // queue the sync message.
    pqmsg->Msg.hwnd = hWnd;
    pqmsg->Msg.message = iMsg;
    pqmsg->Msg.wParam = wParam;
    pqmsg->Msg.lParam = lParam;
    pqmsg->next = NULL;

    if (pMsgQueue->pFirstNotifyMsg == NULL) {
        pMsgQueue->pFirstNotifyMsg = pMsgQueue->pLastNotifyMsg = pqmsg;
    }
    else {
        pMsgQueue->pLastNotifyMsg->next = pqmsg;
        pMsgQueue->pLastNotifyMsg = pqmsg;
    }

    pMsgQueue->dwState |= QS_NOTIFYMSG;

    pthread_mutex_unlock (&pMsgQueue->lock);

    // Signal that the msg queue contains one more element for reading
    sem_getvalue (&pMsgQueue->wait, &sem_value);
    if (sem_value == 0)
        sem_post(&pMsgQueue->wait);

    return ERR_OK;
}

int GUIAPI SendAsyncMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC WndProc;
    
    if (hWnd == HWND_INVALID) return -1;

    WndProc = GetWndProc(hWnd);

    return (*WndProc)(hWnd, iMsg, wParam, lParam);

}

int GUIAPI BroadcastMessage (int iMsg, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    
    msg.message = iMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;

    return SendMessage (HWND_DESKTOP, MSG_BROADCASTMSG, 0, (LPARAM)(&msg));
}

#ifdef _DEBUG

int GUIAPI DispatchMessage(PMSG pMsg)
{
    WNDPROC WndProc;
    PSYNCMSG pSyncMsg;
    int iRet;

    fprintf (stderr, "Message, %s: hWnd: %x, wP: %x, lP: %lx. %s\n",
        Message2Str (pMsg->message),
        pMsg->hwnd,
        pMsg->wParam,
        pMsg->lParam,
        pMsg->pAdd?"Sync":"Normal");

    if (pMsg->hwnd == HWND_INVALID) {
        if (pMsg->pAdd) {
            pSyncMsg = (PSYNCMSG)pMsg->pAdd;
            pSyncMsg->retval = HWND_INVALID;
            sem_post(&pSyncMsg->sem_handle);
        }
        
        fprintf (stderr, "Message have been thrown away: %s\n",
            Message2Str (pMsg->message));

        return HWND_INVALID;
    }

    WndProc = GetWndProc(pMsg->hwnd);

    if ( pMsg->pAdd ) // this is a sync message.
    {
         pSyncMsg = (PSYNCMSG)pMsg->pAdd;
         pSyncMsg->retval = (*WndProc)
                   (pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
         sem_post(&pSyncMsg->sem_handle);
         iRet = pSyncMsg->retval;
    }

    iRet = (*WndProc)(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);

    fprintf (stderr, "Message, %s done, return value: %x\n",
        Message2Str (pMsg->message),
        iRet);

    return iRet;
}

#else

int GUIAPI DispatchMessage(PMSG pMsg)
{
    WNDPROC WndProc;
    PSYNCMSG pSyncMsg;

    if (pMsg->hwnd == HWND_INVALID) {
        if (pMsg->pAdd) {
            pSyncMsg = (PSYNCMSG)pMsg->pAdd;
            pSyncMsg->retval = HWND_INVALID;
            sem_post(&pSyncMsg->sem_handle);
        }
        return HWND_INVALID;
    }

    WndProc = GetWndProc(pMsg->hwnd);

    if ( pMsg->pAdd ) // this is a sync message.
    {
        pSyncMsg = (PSYNCMSG)pMsg->pAdd;
        pSyncMsg->retval = (*WndProc)
                   (pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
        sem_post(&pSyncMsg->sem_handle);
        return pSyncMsg->retval;
    }

    return (*WndProc)(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
}

#endif

void* ParseEvent(void* data)
{
    LWEVENT lwe;
    PMOUSEEVENT me;
    PKEYEVENT ke;
    MSG Msg;
    PMSGQUEUE pMsgQueue;
    HWND hWnd = HWND_DESKTOP;
    int sem_value;

    ke = &(lwe.data.ke);
    me = &(lwe.data.me);
    me->x = 0; me->y = 0;
    Msg.wParam = 0;
    Msg.lParam = 0;

    sem_post ((sem_t*)data);

    while (TRUE) {
        if (!GetLWEvent(TRUE, &lwe))
            continue;

        Msg.pt.x = me->x;
        Msg.pt.y = me->y;
        gettimeofday(&Msg.time, NULL);
        if(lwe.type == LWETYPE_TIMEOUT) {
            Msg.hwnd = hWnd = HWND_DESKTOP;
            Msg.message = MSG_TIMEOUT;
            Msg.wParam = (WPARAM)lwe.count;
            Msg.lParam = 0;
        }
        else if(lwe.type == LWETYPE_KEY) {
            Msg.hwnd = hWnd = HWND_DESKTOP;
            Msg.wParam = ke->scancode;
            Msg.lParam = ke->status;
            if(ke->event == KE_KEYDOWN){
                Msg.message = MSG_KEYDOWN;
            }
            else if(ke->event == KE_KEYUP) {
                Msg.message = MSG_KEYUP;
            }
        }
        else if(lwe.type == LWETYPE_MOUSE) {
            Msg.hwnd = hWnd = HWND_DESKTOP;
            
            // the wParam is the key status.
            Msg.wParam = me->status;

            Msg.lParam = MAKELONG (me->x, me->y);

            switch(me->event) {
            case ME_MOVED:
                Msg.message = MSG_MOUSEMOVE;
                break;
            case ME_LEFTDOWN:
                Msg.message = MSG_LBUTTONDOWN;
                break;
            case ME_LEFTUP:
                Msg.message = MSG_LBUTTONUP;
                break;
            case ME_LEFTDBLCLICK:
                Msg.message = MSG_LBUTTONDBLCLK;
                break;
            case ME_RIGHTDOWN:
                Msg.message = MSG_RBUTTONDOWN;
                break;
            case ME_RIGHTUP:
                Msg.message = MSG_RBUTTONUP;
                break;
            case ME_RIGHTDBLCLICK:
                Msg.message = MSG_RBUTTONDBLCLK;
                break;
            }
        }

        pMsgQueue = GetMsgQueue(hWnd);

        pthread_mutex_lock (&pMsgQueue->lock);

        if ((pMsgQueue->writepos + 1) % pMsgQueue->len == pMsgQueue->readpos) {
            pthread_mutex_unlock (&pMsgQueue->lock);
            continue;
        }

        // Write the data and advance write pointer */
        pMsgQueue->msg[pMsgQueue->writepos] = Msg;

        pMsgQueue->writepos++;
        if (pMsgQueue->writepos >= pMsgQueue->len) pMsgQueue->writepos = 0;

        pMsgQueue->dwState |= QS_POSTMSG;

        pthread_mutex_unlock (&pMsgQueue->lock);
   
        // Signal that the msg queue contains one more element for reading
        sem_getvalue (&pMsgQueue->wait, &sem_value);
        if (sem_value == 0)
            sem_post(&pMsgQueue->wait);
    }

    return NULL;
}

void* TimerEntry (void* data)
{
    if (!InitTimer ()) {
        fprintf (stderr, "Init Timer failure!\n");
        TerminateLWEvent();
        TerminateGDI();
        exit (-1);
        return NULL;
    }

    sem_post ((sem_t*)data);

    pthread_join (desktop, NULL);

    TerminateTimer();

    return NULL;
}

/************************** System Initialization ****************************/
static BOOL SystemThreads()
{
    sem_t wait;

    InitZOrderInfo(&MainWinZOrder, HWND_DESKTOP);
    InitZOrderInfo(&TopMostWinZOrder, HWND_DESKTOP);
    
    if (!InitDesktop ()) {
        fprintf (stderr, "Init Desktop error!\n");
        return FALSE;
    }
   
    if (!InitFreeQMSGList ()) {
        TerminateDesktop ();
        fprintf (stderr, "Init free QMSG list error!\n");
        return FALSE;
    }

    if (!InitMsgQueue(&DskMsgs, 0)) {
        TerminateDesktop ();
        DestroyFreeQMSGList ();
        fprintf (stderr, "Init MSG queue error!\n");
        return FALSE;
    }

    sem_init (&wait, 0, 0);
    // this is the thread for desktop window.
    // this thread should have a normal priority same as
    // other main window threads. 
    // if there is no main window can receive the low level events,
    // this desktop window is the only one can receive the events.
    // to close a MiniGUI application, we should close the desktop 
    // window.
    pthread_create(&desktop, NULL, DesktopMain, &wait);
    sem_wait (&wait);

    // this is the thread of timer.
    // when this thread start, it init timer data and install
    // a signal handler of SIGALARM, and call setitimer system call
    // to install a interval timer.
    // after initialization, this thread wait desktop to terminate,
    // and then remove the interval timer and free data structs.
    // the signal handler will alert desktop a MSG_TIMER message.
    pthread_create(&timer, NULL, TimerEntry, &wait);
    sem_wait (&wait);
    
    // this thread collect low level event from SVGALib,
    // if there is no event, this thread will suspend to wait a event.
    // the events maybe mouse event, keyboard event, or timeout event.
    //
    // this thread also parse low level event and translate it to message,
    // then post the message to the approriate message queue.
    // this thread also translate SVGALib events to MiniGUI message, 
    // for example, it translate mouse button event to mouse button
    // down and mouse button up, as well as mouse double click event.
    // this thread works as a mouse and keyboard driver.
    pthread_create(&parsor, NULL, ParseEvent, &wait);
    sem_wait (&wait);

    sem_destroy (&wait);

    return TRUE;
}

static struct termios savedtermio;

void* GUIAPI GetOriginalTermIO ()
{
    return &savedtermio;
}

BOOL GUIAPI InitGUI()
{
    // Save original termio
	tcgetattr (0, &savedtermio);
    
    if (!InitFixStr ()) {
        fprintf (stderr, "Init Fixed String module failure!\n");
        return FALSE;
    }
    
    if (!InitFreeClipRectList (&sg_FreeClipRectList, SIZE_CLIPRECTHEAP)) {
        fprintf (stderr, "Allocate free clip rect heap failure!\n");
        return FALSE;
    }

    if (!InitFreeClipRectList (&sg_FreeInvRectList, SIZE_INVRECTHEAP)) {
        fprintf (stderr, "Allocate free invalid rect heap failure!\n");
        return FALSE;
    }

    // Init IOPerm
    if (!InitIOPerm ()) {
        fprintf (stderr, "Initialization of Beep failure!\n");
        return FALSE;
    }

    // Init GDI.
    if(!InitGDI()) {
        fprintf (stderr, "Initialization of GDI failure!\n");
        return FALSE;
    }

    // Init low level event
    if(!InitLWEvent()) {
        fprintf(stderr, "Low level event initialization failure!\n");
        return FALSE;
    }

    if (!InitMenu ()) {
        fprintf (stderr, "Init Menu module failure!\n");
        return FALSE;
    }

    // Init mouse cursor.
    if( !InitCursor() ) {
        fprintf (stderr, "Count not initialize mouse cursor support.\n");
        return FALSE;
    }

    // Init control class
    if(!InitControlClass()) {
        fprintf(stderr, "Init Control Class failure!\n");
        return FALSE;
    }

    // Init accelerator
    if(!InitAccel()) {
        fprintf(stderr, "Init Accelerator failure!\n");
        return FALSE;
    }

#ifdef _MULFONT
    if (!InitFont ()) {
        fprintf(stderr, "Init Font failure!\n");
        return FALSE;
    }
#endif

    SetCursor(GetSystemCursor(0));

    return SystemThreads();
}

void GUIAPI TerminateGUI (int rcByGUI)
{
    if (rcByGUI >= 0)
        pthread_join (timer, NULL);
    
    pthread_kill_other_threads_np ();

    TerminateDesktop ();

    DestroyMsgQueue (&DskMsgs);

    DestroyFreeQMSGList ();

#ifdef _MULFONT
    TerminateFont ();
#endif

    TerminateAccel ();

    TerminateControlClass();

    TerminateCursor();

    TerminateMenu();

    TerminateLWEvent();
    
    TerminateGDI();

    TerminateVCSwitch ();

    TerminateIOPerm ();

    DestroyFreeClipRectList (&sg_FreeClipRectList);

    DestroyFreeClipRectList (&sg_FreeInvRectList);

    TerminateFixStr();

    TerminateTimer ();
}

BOOL GUIAPI ResumeGUI (void)
{
    if (!InitFixStr ()) {
        fprintf (stderr, "Init Fixed String module failure!\n");
        return FALSE;
    }
    
    if (!InitFreeClipRectList (&sg_FreeClipRectList, SIZE_CLIPRECTHEAP)) {
        fprintf (stderr, "Allocate free clip rect heap failure!\n");
        return FALSE;
    }

    if (!InitFreeClipRectList (&sg_FreeInvRectList, SIZE_INVRECTHEAP)) {
        fprintf (stderr, "Allocate free invalid rect heap failure!\n");
        return FALSE;
    }

    if(!InitGDI()) {
        fprintf (stderr, "Initialization of GDI failure!\n");
        return FALSE;
    }

    if(!InitLWEvent()) {
        fprintf(stderr, "Low level event initialization failure!\n");
        return FALSE;
    }

    if (!InitMenu ()) {
        fprintf (stderr, "Init Menu module failure!\n");
        return FALSE;
    }

    if( !InitCursor() ) {
        fprintf (stderr, "Count not initialize mouse cursor support.\n");
        return FALSE;
    }

    if(!InitControlClass()) {
        fprintf(stderr, "Init Control Class failure!\n");
        return FALSE;
    }

    if(!InitAccel()) {
        fprintf(stderr, "Init Accelerator failure!\n");
        return FALSE;
    }

#ifdef _MULFONT
    if (!InitFont ()) {
        fprintf(stderr, "Init Font failure!\n");
        return FALSE;
    }
#endif

    SetCursor(GetSystemCursor(0));

    return SystemThreads();
}

// Note that the calling thread of this function 
// MUST be the Main Thread of MiniGUI Application,
// and there is no other user defined thread running.
void GUIAPI SuspendGUI (void)
{
    pthread_kill_other_threads_np ();

    TerminateDesktop ();

    DestroyMsgQueue (&DskMsgs);

    DestroyFreeQMSGList ();

#ifdef _MULFONT
    TerminateFont ();
#endif

    TerminateAccel ();

    TerminateControlClass ();

    TerminateCursor ();

    TerminateMenu ();

    TerminateLWEvent ();
    
    TerminateGDI ();

    TerminateVCSwitch ();

    DestroyFreeClipRectList (&sg_FreeClipRectList);

    DestroyFreeClipRectList (&sg_FreeInvRectList);

    TerminateFixStr ();

    TerminateTimer ();
}

