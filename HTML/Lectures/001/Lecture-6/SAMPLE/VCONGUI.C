//
// Virtual Console on MiniGUI.
// Copyright (c) 1999, Wei Yongming (ymwei@263.net).
//
// Some idea and source come from CCE (Console Chinese Environment) 
// 

/*
**  This source is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public
**  License as published by the Free Software Foundation; either
**  version 2 of the License, or (at your option) any later version.
**
**  This software is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public
**  License along with this library; if not, write to the Free
**  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
**  MA 02111-1307, USA
*/

// Create date: 1999.09.23
//
// Modify records:
//
//  Who             When        Where       For What                Status
//-----------------------------------------------------------------------------
//
// TODO:
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <semaphore.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>

#include "resource.h"
#include "defs.h"
#include "vcongui.h"
#include "vc.h"
#include "child.h"
#include "terminal.h"
#include "vt.h"

static HMENU createpmenuabout ()
{
    HMENU hmnu;
    MENUITEMINFO mii;
    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = 0;
    mii.typedata    = (DWORD)"About...";
    hmnu = CreatePopupMenu (&mii);
    
    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING ;
    mii.state       = 0;
    mii.id          = IDM_ABOUT_THIS;
    mii.typedata    = (DWORD)"About VC on MiniGUI...";
    InsertMenuItem(hmnu, 0, TRUE, &mii);

    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING ;
    mii.state       = 0;
    mii.id          = IDM_ABOUT;
    mii.typedata    = (DWORD)"About MiniGUI...";
    InsertMenuItem(hmnu, 1, TRUE, &mii);

    return hmnu;
}

static HMENU createpmenufile ()
{
    HMENU hmnu;
    MENUITEMINFO mii;
    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = 0;
    mii.typedata    = (DWORD)"File";
    hmnu = CreatePopupMenu (&mii);
    
    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_NEW;
    mii.typedata    = (DWORD)"New";
    InsertMenuItem(hmnu, 0, TRUE, &mii);
    
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_OPEN;
    mii.typedata    = (DWORD)"Open...";
    InsertMenuItem(hmnu, 1, TRUE, &mii);
    
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_SAVE;
    mii.typedata    = (DWORD)"Save";
    InsertMenuItem(hmnu, 2, TRUE, &mii);
    
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_SAVEAS;
    mii.typedata    = (DWORD)"Save As...";
    InsertMenuItem(hmnu, 3, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_CLOSE;
    mii.typedata    = (DWORD)"Close";
    InsertMenuItem(hmnu, 4, TRUE, &mii);
    
    mii.type        = MFT_SEPARATOR;
    mii.state       = 0;
    mii.id          = 0;
    mii.typedata    = 0;
    InsertMenuItem(hmnu, 5, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_EXIT;
    mii.typedata    = (DWORD)"Exit";
    InsertMenuItem(hmnu, 6, TRUE, &mii);

    return hmnu;
}

static HMENU createpmenuedit ()
{
    HMENU hmnu;
    MENUITEMINFO mii;
    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = 0;
    mii.typedata    = (DWORD)"Edit";
    hmnu = CreatePopupMenu (&mii);
    
    mii.type        = MFT_STRING ;
    mii.state       = 0;
    mii.id          = IDM_COPY;
    mii.typedata    = (DWORD)"Copy Screen";
    InsertMenuItem(hmnu, 0, TRUE, &mii);
     
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_PASTE;
    mii.typedata    = (DWORD)"Paste";
    InsertMenuItem(hmnu, 1, TRUE, &mii);  
    
    return hmnu;
}

static HMENU createpmenuterminal ()
{
    HMENU hmnu;
    MENUITEMINFO mii;
    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = 0;
    mii.typedata    = (DWORD)"Terminal";
    hmnu = CreatePopupMenu (&mii);
    
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_40X15;
    mii.typedata    = (DWORD)"40x15 (small)";
    InsertMenuItem(hmnu, 0, TRUE, &mii);
     
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_80X24;
    mii.typedata    = (DWORD)"80x24 (vt100)";
    InsertMenuItem(hmnu, 1, TRUE, &mii);
     
    mii.type        = MFT_STRING;
    mii.state       = MF_CHECKED;
    mii.id          = IDM_80X25;
    mii.typedata    = (DWORD)"80x25 (ibmpc)";
    InsertMenuItem(hmnu, 2, TRUE, &mii);
     
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_80X40;
    mii.typedata    = (DWORD)"80x40 (xterm)";
    InsertMenuItem(hmnu, 3, TRUE, &mii);
     
    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_80X52;
    mii.typedata    = (DWORD)"80x52 (ibmvga)";
    InsertMenuItem(hmnu, 4, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_96X25;
    mii.typedata    = (DWORD)"96x25 (wide)";
    InsertMenuItem(hmnu, 5, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_96X40;
    mii.typedata    = (DWORD)"96x40 (My favorite)";
    InsertMenuItem(hmnu, 6, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_96X52;
    mii.typedata    = (DWORD)"96x52 (large)";
    InsertMenuItem(hmnu, 7, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.state       = 0;
    mii.id          = IDM_CUSTOMIZE;
    mii.typedata    = (DWORD)"Customize...";
    InsertMenuItem(hmnu, 8, TRUE, &mii);
    
    return hmnu;
}

static HMENU createmenu ()
{
    HMENU hmnu;
    MENUITEMINFO mii;

    hmnu = CreateMenu();

    memset (&mii, 0, sizeof(MENUITEMINFO));
    mii.type        = MFT_STRING;
    mii.id          = 100;
    mii.typedata    = (DWORD)"File";
    mii.hsubmenu    = createpmenufile ();

    InsertMenuItem(hmnu, 0, TRUE, &mii);

    mii.type        = MFT_STRING;
    mii.id          = 110;
    mii.typedata    = (DWORD)"Edit";
    mii.hsubmenu    = createpmenuedit ();
    InsertMenuItem(hmnu, 1, TRUE, &mii);
    
    mii.type        = MFT_STRING;
    mii.id          = 120;
    mii.typedata    = (DWORD)"Terminal";
    mii.hsubmenu    = createpmenuterminal ();
    InsertMenuItem(hmnu, 2, TRUE, &mii);
    
    mii.type        = MFT_STRING;
    mii.id          = 130;
    mii.typedata    = (DWORD)"About";
    mii.hsubmenu    = createpmenuabout ();
    InsertMenuItem(hmnu, 3, TRUE, &mii);
                   
    return hmnu;
}

void SetTerminalWindowSize (PCONINFO pConInfo, WPARAM cmd_id)
{
    int col, row;
    RECT new_win_rc;
    struct winsize twinsz;
    
    switch (cmd_id) {
    case IDM_40X15:
        col = 40;
        row = 15;
        break;
    case IDM_80X24:
        col = 80;
        row = 24;
        break;
    case IDM_80X25:
        col = 80;
        row = 25;
        break;
    case IDM_80X40:
        col = 80;
        row = 40;
        break;
    case IDM_80X52:
        col = 80;
        row = 52;
        break;
    case IDM_96X25:
        col = 96;
        row = 25;
        break;
    case IDM_96X40:
        col = 96;
        row = 40;
        break;
    case IDM_96X52:
        col = 96;
        row = 52;
        break;
    case IDM_CUSTOMIZE:
    {
        char cols [10];
        char rows [10];
        char* newcols = cols;
        char* newrows = rows;
        myWINENTRY entries [] = {
            { "New number of columns:", &newcols, 0, 0 },
            { "New number of rows   :", &newrows, 0, 0 },
            { NULL, NULL, 0, 0 }
        };
        myWINBUTTON buttons[] = {
            { "OK", IDOK, BS_DEFPUSHBUTTON },
            { "Cancel", IDCANCEL, 0 },
            { NULL, 0, 0}
        };
        int result;

        sprintf (cols, "%d", pConInfo->cols);
        sprintf (rows, "%d", pConInfo->rows);
        result = myWinEntries (HWND_DESKTOP,
                "New Terminal Window Size",
                "Please specify the new terminal window size",
                340, 100, FALSE, entries, buttons);
        col = atoi (newcols);
        row = atoi (newrows);
        free (newcols);
        free (newrows);

        if (result != IDOK)
            return;

        if (col < MIN_COLS || col > MAX_COLS 
                || row < MIN_ROWS || row > MAX_ROWS) {
            MessageBox (pConInfo->hWnd, 
                    "Please specify a valid terminal window size.",
                    "Virtual Console on MiniGUI",
                    MB_OK | MB_ICONINFORMATION);
            return;
        }
        break;
    }
    default:
        return;
    }

    pConInfo->termType = cmd_id;

    if (!VtChangeWindowSize (pConInfo, row, col))
        return;

    GetWindowRect (pConInfo->hWnd, &new_win_rc);
    MoveWindow (pConInfo->hWnd, new_win_rc.left, new_win_rc.top,
                ClientWidthToWindowWidth (WS_CAPTION | WS_BORDER, 
                    col * GetCharWidth ()),
                ClientHeightToWindowHeight (WS_CAPTION | WS_BORDER,
                    row * GetCharHeight (), 
                    GetMenu (pConInfo->hWnd)), TRUE);

    // Set new terminal window size
    twinsz.ws_row = row;
    twinsz.ws_col = col;
    ioctl (pConInfo->masterPty, TIOCSWINSZ, &twinsz);
}

int VCOnGUIMainWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    PCONINFO pConInfo;

    pConInfo = (PCONINFO)GetWindowAdditionalData (hWnd);
    switch (message) {
        case MSG_CREATE:
        break;
        
        break;
        
        case MSG_SETFOCUS:
            ActiveCaret (hWnd);
            ShowCaret (hWnd);

            init_key_info (&pConInfo->kinfo);
        break;

        case MSG_KILLFOCUS:
            HideCaret (hWnd);

            init_key_info (&pConInfo->kinfo);
        break;
        
        case MSG_KEYUP:
            HandleInputKeyUp (pConInfo, wParam, lParam);
        break;

        case MSG_KEYDOWN:
            HandleInputKeyDown (pConInfo, wParam, lParam);
        break;

        case MSG_CHAR:
            HandleInputChar (pConInfo, wParam, lParam);
        break;

        case MSG_PAINT:
        {
            HDC hdc;
            
            hdc = BeginPaint (hWnd);
            TextRepaintAll (pConInfo);
            EndPaint (hWnd, hdc);
        }
        break;

        case MSG_ERASEBKGND:
        return 0;
        
        case MSG_LBUTTONDOWN:
            if (GetShiftKeyStatus() & 0x00000200)
                HandleMouseBothDown (pConInfo, 
                    LOWORD (lParam), HIWORD (lParam), wParam);
            else {
                SetCapture (hWnd);
                HandleMouseLeftDownWhenCaptured (pConInfo, 
                        LOWORD (lParam), HIWORD (lParam), wParam);
                pConInfo->m_captured = true;
           }
        break;

        case MSG_MOUSEMOVE:
            if (pConInfo->m_captured == true) {
                int x = LOWORD (lParam);
                int y = HIWORD (lParam);
                
                if (wParam & KS_CAPTURED)
                    ScreenToClient (hWnd, &x, &y);
                HandleMouseMoveWhenCaptured (pConInfo, x, y, wParam);
            }
        break;

        case MSG_LBUTTONUP:
            ReleaseCapture ();
            if (pConInfo->m_captured == true) {
                int x = LOWORD (lParam);
                int y = HIWORD (lParam);
                
                if (wParam & KS_CAPTURED)
                    ScreenToClient (hWnd, &x, &y);
                HandleMouseLeftUpWhenCaptured (pConInfo, x, y, wParam);

                pConInfo->m_captured = false;
            }
        break;

        case MSG_RBUTTONDOWN:
            if (GetShiftKeyStatus() & 0x00000100)
                HandleMouseBothDown (pConInfo, 
                    LOWORD (lParam), HIWORD (lParam), wParam);
        break;
        
        case MSG_RBUTTONUP:
            HandleMouseRightUp (pConInfo, 
                    LOWORD (lParam), HIWORD (lParam), wParam);
        break;

        case MSG_ACTIVEMENU:
            if (wParam == 2) {
                CheckMenuRadioItem ((HMENU)lParam, 
                    IDM_40X15, IDM_CUSTOMIZE, 
                    pConInfo->termType, MF_BYCOMMAND);
            }
        break;
        
        case MSG_COMMAND:
        switch (wParam) 
        {
            case IDM_NEW:
            case IDM_OPEN:
            case IDM_SAVE:
            case IDM_SAVEAS:
            case IDM_CLOSE:
            break;

            case IDM_COPY:
                TextCopy (pConInfo, 0, 0, pConInfo->dispxmax - 1, 
                                          pConInfo->dispymax - 1);
            break;

            case IDM_PASTE:
                TextPaste (pConInfo);
            break;
            
            case IDM_EXIT:
                SendMessage (hWnd, MSG_CLOSE, 0, 0L);
            break;

            case IDM_40X15 ... IDM_CUSTOMIZE:
                SetTerminalWindowSize (pConInfo, wParam);
            break;
            
            case IDM_ABOUT_THIS:
                MessageBox (hWnd, 
                    "VCOnGUI - Virtual Console On MiniGUI " VCONGUI_VERSION "\n"
                    "Copyright (C) 1999-2000 Yongming Wei (ymwei@263.net).\n"
                    "Some idea comes from CCE by Rui He and others.\n"
                    "    CCE: Copyright (C) 1998-1999 Rui He and others.\n",
                    "About VC On MiniGUI",
                    MB_OK | MB_ICONINFORMATION);
            break;
            
            case IDM_ABOUT:
                OpenAboutDialog ();
            break;
        }
        break;

        case MSG_CLOSE:
            MessageBox (hWnd, 
                    "Please type \"exit\" at shell prompt "
                    "or terminate the current program to quit.",
                    "Virtual Console on MiniGUI", 
                    MB_OK | MB_ICONQUESTION);
        return 0;
    }
    
    if (pConInfo->DefWinProc)
        return (*pConInfo->DefWinProc)(hWnd, message, wParam, lParam);
    else
        return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

static void InitCreateInfo (PMAINWINCREATE pCreateInfo, 
                PCONINFO pConInfo, int x, int y, bool fMenu)
{
    pCreateInfo->dwStyle = WS_CAPTION | WS_BORDER;
    pCreateInfo->dwExStyle = WS_EX_IMECOMPOSE | WS_EX_USEPRIVATECDC;
    pCreateInfo->spCaption = "Virtual Console on MiniGUI";
    pCreateInfo->hMenu = fMenu?createmenu ():0;
    pCreateInfo->hCursor = GetSystemCursor (IDC_IBEAM);
    pCreateInfo->hIcon = GetSmallSystemIcon (IDI_APPLICATION);
    pCreateInfo->MainWindowProc = VCOnGUIMainWinProc;
    pCreateInfo->lx = x; 
    pCreateInfo->ty = y;
    pCreateInfo->rx 
        = x + ClientWidthToWindowWidth (WS_CAPTION | WS_BORDER, 
                pConInfo->cols * GetCharWidth ());
    pCreateInfo->by
        = y + ClientHeightToWindowHeight (WS_CAPTION | WS_BORDER,
                pConInfo->rows * GetCharHeight (), fMenu);
    pCreateInfo->iBkColor = COLOR_transparent;
    pCreateInfo->dwAddData = (DWORD)pConInfo;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

void* VCOnMiniGUI (void* data)
{
    MSG Msg;
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;
    PCONINFO pConInfo;
    CHILDINFO ChildInfo;

    if (!data) {
        ChildInfo.startupMessage = true;
        ChildInfo.startupStr = NULL;
        ChildInfo.execProg = NULL;
        
        ChildInfo.DefWinProc = NULL;
        ChildInfo.fMenu = true;
        ChildInfo.left = 0;
        ChildInfo.top  = 0;
        ChildInfo.rows = 0;
        ChildInfo.cols = 0;
    }
    else
        ChildInfo = *((PCHILDINFO)data);

    if ( !(pConInfo = AllocConInfo ()) ) {
        MessageBox (HWND_DESKTOP, "Init Virtual Console information error!", 
                                  "Error", MB_OK | MB_ICONSTOP);
        return NULL;
    }
    if (ChildInfo.rows > 0) {
        pConInfo->rows = ChildInfo.rows;
        pConInfo->termType = IDM_CUSTOMIZE;
    }
    if (ChildInfo.cols > 0) {
        pConInfo->cols = ChildInfo.cols;
        pConInfo->termType = IDM_CUSTOMIZE;
    }

    init_key_info (&pConInfo->kinfo);
    
    InitCreateInfo (&CreateInfo, pConInfo, 
        ChildInfo.left, ChildInfo.top, ChildInfo.fMenu);
    pConInfo->DefWinProc = ChildInfo.DefWinProc;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    if (hMainWnd == HWND_INVALID) {
        FreeConInfo (pConInfo);
        MessageBox (HWND_DESKTOP, "Create Virtual Console window error!", 
                                  "Error", MB_OK | MB_ICONSTOP);
        return NULL;
    }

    if (!CreateCaret (hMainWnd, NULL, GetCCharWidth (), GetCharHeight ()))
        fprintf (stderr, "Create caret error!\n");
    
    ShowWindow (hMainWnd, SW_SHOWNORMAL);
    ShowCaret (hMainWnd);

    pConInfo->hWnd = hMainWnd;

    if (!TerminalStart (pConInfo, &ChildInfo)) {
        MessageBox (hMainWnd, "Create Terminal error!", 
                                  "Error", MB_OK | MB_ICONSTOP);
        goto error;
    }

    // Enter message loop.
    do {
        // It is time to read from master pty, and output.
        ReadMasterPty (pConInfo);

        if (pConInfo->terminate)
            break;

        while (HavePendingMessage (hMainWnd)) {
            if (!GetMessage (&Msg, hMainWnd))
                break;
            DispatchMessage (&Msg);
        }

    } while (TRUE);

    TerminalCleanup (pConInfo);

error:
    FreeConInfo (pConInfo);
    DestroyCaret (hMainWnd);
    DestroyMainWindow (hMainWnd);
    MainWindowThreadCleanup (hMainWnd);

    return NULL;
}

void* NewVirtualConsole (PCHILDINFO pChildInfo)
{
    pthread_t th;
    
#if 0    
    pthread_attr_t th_attr;

    pthread_attr_init (&th_attr);
    pthread_attr_setschedpolicy (&th_attr, SCHED_OTHER);
    CreateThreadForMainWindow (&th, &th_attr, VCOnMiniGUI, pChildInfo);
    pthread_attr_destroy (&th_attr);
#else
    CreateThreadForMainWindow (&th, NULL, VCOnMiniGUI, pChildInfo);
#endif

    return NULL;
}

