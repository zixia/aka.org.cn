#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "control.h"

#include "mywindows.h"

void errorWindow(HWND hwnd, char* str, char* title)
{
    char* a;

    if (!strstr (str, "%s")) {
        a = alloca (strlen (str) + 5);
        strcpy(a, str);
        strcat(a, ": %s");
        str = a;
    }

    myMessageBox (hwnd, MB_OK | MB_ICONSTOP, 
                title?title:"Error", str, strerror (errno));
}

int myMessageBox (HWND hwnd, DWORD dwStyle, char* title, char* text, ...)
{
    char * buf = NULL;
    int size = 0;
    int i = 0;
    va_list args;
    int rc;

    va_start(args, text);

    do {
        size += 1000;
        if (buf) free(buf);
        buf = malloc(size);
        i = vsnprintf(buf, size, text, args);
    } while (i == size);

    va_end(args);

    rc = MessageBox (hwnd, buf, title, dwStyle);

    if (buf)
        free (buf);

    return rc;
}

DLGTEMPLATE DlgButtons =
{
    WS_BORDER | WS_CAPTION, WS_EX_NONE,
    (640 - 418)/2, (480 - 200)/2, 418, 200,
    NULL, 0, 0, 5, NULL, 0
};

#define IDC_BASE        100
#define IDC_ONE         101
#define IDC_TWO         102
#define IDC_THREE       103

CTRLDATA CtrlButtons [] =
{ 
    { "static", WS_VISIBLE | SS_ICON,
        10, 10, 40, 40, IDC_STATIC, "LOGO", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 10, 340, 120, IDC_STATIC, NULL, 0 },
    { "button", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        100, 140, 80, 24, IDC_ONE, NULL, 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        200, 140, 80, 24, IDC_TWO, NULL, 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        300, 140, 80, 24, IDC_THREE, NULL, 0 }
};

static int 
ButtonsBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
        return 1;
    break;
        
    case MSG_COMMAND:
        if (wParam == IDC_ONE || wParam == IDC_TWO
                || wParam == IDC_THREE)
            EndDialog (hDlg, wParam);
        break;
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int myWinMessage (HWND hwnd, char* title, char* button1, 
                       char* text, ...)
{
    char * buf = NULL;
    int size = 0;
    int i = 0;
    va_list args;
    int rc;

    va_start(args, text);
    do {
        size += 1000;
        if (buf) free(buf);
        buf = malloc(size);
        i = vsnprintf(buf, size, text, args);
    } while (i == size);
    va_end(args);

    DlgButtons.caption   = title;
    DlgButtons.controls  = CtrlButtons;
    DlgButtons.controlnr = 3;

    CtrlButtons [1].caption = buf;
    CtrlButtons [2].caption = button1;
    CtrlButtons [2].x       = 300;
    
    CtrlButtons [0].dwAddData = GetLargeSystemIcon (IDI_APPLICATION);
    rc = DialogBoxIndirectParam (&DlgButtons, hwnd,
            ButtonsBoxProc, 0L);

    if (buf)
        free (buf);

    return rc - IDC_BASE;
}

int myWinChoice (HWND hwnd, char* title, char* button1, char* button2,
                       char* text, ...)
{
    char * buf = NULL;
    int size = 0;
    int i = 0;
    va_list args;
    int rc;

    va_start(args, text);
    do {
        size += 1000;
        if (buf) free(buf);
        buf = malloc(size);
        i = vsnprintf(buf, size, text, args);
    } while (i == size);
    va_end(args);

    DlgButtons.caption   = title;
    DlgButtons.controls  = CtrlButtons;
    DlgButtons.controlnr = 4;

    CtrlButtons [1].caption = buf;
    CtrlButtons [2].caption = button1;
    CtrlButtons [3].caption = button2;
    CtrlButtons [2].x       = 200;
    CtrlButtons [3].x       = 300;
    
    CtrlButtons [0].dwAddData = GetLargeSystemIcon (IDI_APPLICATION);
    rc = DialogBoxIndirectParam (&DlgButtons, hwnd,
            ButtonsBoxProc, 0L);

    if (buf)
        free (buf);

    return rc - IDC_BASE;
}

int myWinTernary (HWND hwnd, char* title, char* button1, char* button2,
                       char* button3, char* text, ...)
{
    char * buf = NULL;
    int size = 0;
    int i = 0;
    va_list args;
    int rc;

    va_start(args, text);
    do {
        size += 1000;
        if (buf) free(buf);
        buf = malloc(size);
        i = vsnprintf(buf, size, text, args);
    } while (i == size);
    va_end(args);

    DlgButtons.caption   = title;
    DlgButtons.controls  = CtrlButtons;
    DlgButtons.controlnr = 5;

    CtrlButtons [1].caption = buf;
    CtrlButtons [2].caption = button1;
    CtrlButtons [3].caption = button2;
    CtrlButtons [4].caption = button3;
    CtrlButtons [2].x       = 100;
    CtrlButtons [3].x       = 200;
    CtrlButtons [4].x       = 300;
    
    CtrlButtons [0].dwAddData = GetLargeSystemIcon (IDI_APPLICATION);
    rc = DialogBoxIndirectParam (&DlgButtons, hwnd,
            ButtonsBoxProc, 0L);

    if (buf)
        free (buf);

    return rc - IDC_BASE;
}

static int StatusWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_PAINT:
        {
            HDC hdc;
            char* text;
            RECT rc;
            
            hdc = BeginPaint (hWnd);
 
            text = (char*) GetWindowAdditionalData (hWnd);
            if (text) {
                GetClientRect (hWnd, &rc);
                SetTextColor (hdc, COLOR_black);
                SetBkColor (hdc, COLOR_lightgray);
                TextOut (hdc, 5, (rc.bottom - GetCharHeight())>>1, text);
            }
 
            EndPaint (hWnd, hdc);
        }
        return 0;
    }
 
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

HWND createStatusWin (HWND hParentWnd, int width, int height, 
            char* title, char* text, ...)
{
    HWND hwnd;
    char* buf = NULL;
    int size = 0;
    int i = 0;
    va_list args;
    MAINWINCREATE CreateInfo;

    va_start (args, text);
    do {
        size += 1000;
        if (buf)
            free(buf);
        buf = malloc(size);
        
        i = vsnprintf(buf, size, text, args);
    } while (i == size);
    va_end(args);
    
    width = ClientWidthToWindowWidth (WS_CAPTION | WS_BORDER, width);
    height= ClientHeightToWindowHeight (WS_CAPTION | WS_BORDER, height, FALSE);
    
    CreateInfo.dwStyle = WS_CAPTION | WS_BORDER | WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = title;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor (IDC_WAIT);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = StatusWinProc;
    CreateInfo.lx = (GetGDCapability (HDC_SCREEN, GDCAP_MAXX) - width) >> 1;
    CreateInfo.ty = (GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - height) >> 1;
    CreateInfo.rx = CreateInfo.lx + width;
    CreateInfo.by = CreateInfo.ty + height;
    CreateInfo.iBkColor = COLOR_lightgray;
    CreateInfo.dwAddData = (DWORD) buf;
    CreateInfo.hHosting = hParentWnd;

    hwnd = CreateMainWindow (&CreateInfo);
    if (hwnd == HWND_INVALID)
        return hwnd;

    UpdateWindow (hwnd, TRUE);

    return hwnd;
}

void destroyStatusWin (HWND hwnd)
{
    char* buff;

    buff = (char*)GetWindowAdditionalData (hwnd);

    DestroyMainWindow (hwnd);
    MainWindowThreadCleanup (hwnd);
    free (buff);
}

HWND createProgressWin (HWND hParentWnd, char * title, char * label,
        int id, int range)
{
    HWND hwnd;
    MAINWINCREATE CreateInfo;
    int ww, wh;
    HWND hStatic, hProgBar;

    ww = ClientWidthToWindowWidth (WS_CAPTION | WS_BORDER, 400);
    wh = ClientHeightToWindowHeight (WS_CAPTION | WS_BORDER, 
            (range > 0) ? 70 : 35, FALSE);
    
    CreateInfo.dwStyle = WS_CAPTION | WS_BORDER | WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = title;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_WAIT);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = DefaultMainWinProc;
    CreateInfo.lx = (GetGDCapability (HDC_SCREEN, GDCAP_MAXX) - ww) >> 1;
    CreateInfo.ty = (GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - wh) >> 1;
    CreateInfo.rx = CreateInfo.lx + ww;
    CreateInfo.by = CreateInfo.ty + wh;
    CreateInfo.iBkColor = COLOR_lightgray;
    CreateInfo.dwAddData = 0L;
    CreateInfo.hHosting = hParentWnd;

    hwnd = CreateMainWindow (&CreateInfo);
    if (hwnd == HWND_INVALID)
        return hwnd;

    hStatic = CreateWindow ("static", 
                  label, 
                  WS_VISIBLE | SS_SIMPLE, 
                  IDC_STATIC, 
                  10, 10, 380, 16, hwnd, 0);
    
    if (range > 0) {
        hProgBar = CreateWindow ("progressbar", 
                  NULL, 
                  WS_VISIBLE,
                  id,
                  10, 30, 380, 30, hwnd, 0);
        SendDlgItemMessage (hwnd, id, PBM_SETRANGE, 0, range);
    }
    else
        hProgBar = HWND_INVALID;

    UpdateWindow (hwnd, TRUE);
    SendMessage (hStatic, MSG_PAINT, 0, 0L);

    if (hProgBar != HWND_INVALID)
        SendMessage (hProgBar, MSG_PAINT, 0, 0L);

    return hwnd;
}

void destroyProgressWin (HWND hwnd)
{
    DestroyAllControls (hwnd);
    DestroyMainWindow (hwnd);
    ThrowAwayMessages (hwnd);
    MainWindowThreadCleanup (hwnd);
}


#define INTERW_CTRLS        5
#define INTERH_CTRLS        5

#define LABEL_HEIGHT        16
#define LABEL_WIDTH         160
#define BUTTON_WIDTH        80
#define BUTTON_HEIGHT       24
#define ENTRY_HEIGHT        22

#define MARGIN_TOP          10
#define MARGIN_BOTTOM       10
#define MARGIN_LEFT         50
#define MARGIN_RIGHT        10

static int
WinMenuBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case MSG_INITDIALOG:
    {
        myWINMENUITEMS* pWinMenuItems = (myWINMENUITEMS*)lParam;
        int i = 0;
        char** items = pWinMenuItems->items;

        while (items[i]) {
            SendDlgItemMessage (hDlg, pWinMenuItems->listboxid, 
                    LB_ADDSTRING, 0, (LPARAM)(items[i]));
            i++;
        }
        
        SendDlgItemMessage (hDlg, pWinMenuItems->listboxid,
                LB_SETCURSEL, (WPARAM)(*(pWinMenuItems->selected)), 0L);

        SetWindowAdditionalData2 (hDlg, (DWORD)lParam);
        return 1;
    }
    break;

    case MSG_COMMAND:
    {
        myWINMENUITEMS* pWinMenuItems 
            = (myWINMENUITEMS*)GetWindowAdditionalData2 (hDlg);

        if ( (wParam >= pWinMenuItems->minbuttonid) 
                && (wParam <= pWinMenuItems->maxbuttonid) ) {
            *(pWinMenuItems->selected)
                = SendDlgItemMessage (hDlg, pWinMenuItems->listboxid,
                        LB_GETCURSEL, 0, 0L);
            EndDialog (hDlg, wParam);
        }
    }
        break;
  }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int myWinMenu (HWND hParentWnd, const char* title, const char* label, 
                int width, int listboxheight, char** items, int* listItem, 
                myWINBUTTON* buttons)
{
    DLGTEMPLATE DlgBoxData;
    CTRLDATA* pCtrlData;
    int buttonCount = 0, ctrlCount;
    int i;
    int cw, ch, ww, wh;
    int maxid = 0, minbuttonid, maxbuttonid;
    int buttonx, buttony;
    myWINMENUITEMS WinMenuItems;
    RECT rcText;
    int listboxy;

    minbuttonid = buttons->id;
    maxbuttonid = buttons->id;
    while ((buttons + buttonCount)->text) {
        int id;

        id = (buttons + buttonCount)->id;
        maxid += id;
        if ( id > maxbuttonid)
            maxbuttonid = id;
        else if (id < minbuttonid)
            minbuttonid = id;

        buttonCount ++;
    }
    maxid ++;

    if (buttonCount == 0)
        return 0;

    ctrlCount = buttonCount + 3;
    if ( !(pCtrlData = alloca (sizeof (CTRLDATA) * ctrlCount)) )
        return 0;
    
    DlgBoxData.dwStyle      = WS_CAPTION | WS_BORDER;
    DlgBoxData.dwExStyle    = WS_EX_NONE;
    DlgBoxData.caption      = title;
    DlgBoxData.hIcon        = 0;
    DlgBoxData.hMenu        = 0;
    DlgBoxData.controlnr    = ctrlCount;
    DlgBoxData.controls     = pCtrlData;
    DlgBoxData.dwAddData    = 0L;

    cw = BUTTON_WIDTH * buttonCount + INTERW_CTRLS * (buttonCount - 1);
    cw = max (cw, width);
    cw += MARGIN_LEFT + MARGIN_RIGHT;

    rcText.left = 0;
    rcText.top  = 0;
    rcText.right = cw - MARGIN_LEFT - MARGIN_RIGHT;
    rcText.bottom = GetCharHeight ();
    DrawText (HDC_SCREEN, label, -1, &rcText, 
                DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);

    ch = listboxheight + BUTTON_HEIGHT + INTERH_CTRLS 
            + MARGIN_TOP + MARGIN_BOTTOM + RECTH (rcText) + INTERH_CTRLS;

    ww = ClientWidthToWindowWidth (DlgBoxData.dwStyle, cw);
    wh = ClientHeightToWindowHeight (DlgBoxData.dwStyle, ch, FALSE);

    DlgBoxData.x = (GetGDCapability (HDC_SCREEN, GDCAP_HPIXEL) - ww)/2;
    DlgBoxData.y = (GetGDCapability (HDC_SCREEN, GDCAP_VPIXEL) - wh)/2;
    DlgBoxData.w = ww;
    DlgBoxData.h = wh;
   
    // LOGO:
    pCtrlData->class    = "static";
    pCtrlData->dwStyle  = WS_CHILD | WS_VISIBLE | SS_ICON;
    pCtrlData->x        = 10;
    pCtrlData->y        = MARGIN_TOP;
    pCtrlData->w        = 40;
    pCtrlData->h        = 40;;
    pCtrlData->id       = IDC_STATIC;
    pCtrlData->caption  = "LOGO";
    pCtrlData->dwAddData = GetLargeSystemIcon (IDI_APPLICATION);
    
    // label:
    (pCtrlData + 1)->class    = "static";
    (pCtrlData + 1)->dwStyle  = WS_CHILD | WS_VISIBLE | SS_LEFT;
    (pCtrlData + 1)->x        = MARGIN_LEFT;
    (pCtrlData + 1)->y        = MARGIN_TOP;
    (pCtrlData + 1)->w        = cw - MARGIN_LEFT - MARGIN_RIGHT;
    (pCtrlData + 1)->h        = RECTH (rcText);
    (pCtrlData + 1)->id       = IDC_STATIC;
    (pCtrlData + 1)->caption  = label;
    (pCtrlData + 1)->dwAddData = 0;
    
    // listbox:
    listboxy = MARGIN_TOP + RECTH (rcText) + INTERH_CTRLS;
    (pCtrlData + 2)->class      = "listbox";
    (pCtrlData + 2)->dwStyle    = WS_CHILD | WS_VISIBLE | WS_TABSTOP
                                    | WS_VSCROLL | WS_BORDER;
    (pCtrlData + 2)->x          = MARGIN_LEFT;
    (pCtrlData + 2)->y          = listboxy;
    (pCtrlData + 2)->w          = cw - MARGIN_LEFT - MARGIN_RIGHT;
    (pCtrlData + 2)->h          = listboxheight;
    (pCtrlData + 2)->id         = maxid;
    (pCtrlData + 2)->caption    = NULL;
    (pCtrlData + 2)->dwAddData  = 0;

    buttony =  (pCtrlData + 2)->y + (pCtrlData + 2)->h + INTERH_CTRLS;
    buttonx =  ((cw - MARGIN_LEFT - MARGIN_RIGHT - 
        BUTTON_WIDTH * buttonCount - INTERW_CTRLS * (buttonCount - 1))>>1)
        + MARGIN_LEFT;

    // buttons
    for (i = 3; i < buttonCount + 3; i++) {
        (pCtrlData + i)->class      = "button";
        (pCtrlData + i)->dwStyle    = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
        (pCtrlData + i)->dwStyle    |= (buttons + i - 3)->flags;
        (pCtrlData + i)->x          = buttonx;
        (pCtrlData + i)->y          = buttony;
        (pCtrlData + i)->w          = BUTTON_WIDTH;
        (pCtrlData + i)->h          = BUTTON_HEIGHT;
        (pCtrlData + i)->id         = (buttons + i - 3)->id;
        (pCtrlData + i)->caption    = (buttons + i - 3)->text;
        (pCtrlData + i)->dwAddData  = 0;

        buttonx += BUTTON_WIDTH + INTERW_CTRLS;
    }
    
    WinMenuItems.items       = items;
    WinMenuItems.listboxid   = maxid;
    WinMenuItems.selected    = listItem;
    WinMenuItems.minbuttonid = minbuttonid;
    WinMenuItems.maxbuttonid = maxbuttonid;
    return DialogBoxIndirectParam (&DlgBoxData, hParentWnd, WinMenuBoxProc, 
                                        (LPARAM)(&WinMenuItems));
}

static int
WinEntryBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case MSG_INITDIALOG:
    {
        int i;
        myWINENTRYITEMS* pWinEntryItems 
            = (myWINENTRYITEMS*)lParam;

        for (i = 0; i < pWinEntryItems->entrycount; i++) {
            if ((pWinEntryItems->entries + i)->maxlen > 0)
                SendDlgItemMessage (hDlg, pWinEntryItems->firstentryid + i,
                        EM_LIMITTEXT,
                        (pWinEntryItems->entries + i)->maxlen,
                        0L);
        }

        SetWindowAdditionalData2 (hDlg, (DWORD)lParam);
        return 1;
    }
    break;

    case MSG_COMMAND:
    {
        int i;
        myWINENTRYITEMS* pWinEntryItems 
            = (myWINENTRYITEMS*)GetWindowAdditionalData2 (hDlg);

        if ( (wParam >= pWinEntryItems->minbuttonid) 
                && (wParam <= pWinEntryItems->maxbuttonid) ) {
            for (i = 0; i < pWinEntryItems->entrycount; i++) {
                char** value;
                value = (pWinEntryItems->entries + i)->value;
                *value = GetDlgItemText2 (hDlg, 
                        pWinEntryItems->firstentryid + i, NULL);
            }
            
            EndDialog (hDlg, wParam);
        }
    }
        break;
  }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}
int myWinEntries (HWND hParentWnd, const char* title, const char* label,
                int width, int editboxwidth, BOOL fIME, myWINENTRY* items, 
                myWINBUTTON* buttons)
{
    DLGTEMPLATE DlgBoxData;
    CTRLDATA* pCtrlData;
    int buttonCount = 0, entryCount = 0, ctrlCount, count;
    int i;
    int cw, ch, ww, wh, labelwidth;
    int maxid = 0, minbuttonid, maxbuttonid;
    int entryx, entryy;
    int buttonx, buttony;
    myWINENTRYITEMS WinEntryItems;
    RECT rcText;

    minbuttonid = buttons->id;
    maxbuttonid = buttons->id;
    while ((buttons + buttonCount)->text) {
        int id;

        id = (buttons + buttonCount)->id;
        maxid += id;
        if ( id > maxbuttonid)
            maxbuttonid = id;
        else if (id < minbuttonid)
            minbuttonid = id;

        buttonCount ++;
    }
    maxid ++;

    while ((items + entryCount)->text) {
        entryCount ++;
    }

    if (buttonCount == 0 || entryCount == 0)
        return 0;

    ctrlCount = buttonCount + (entryCount<<1) + 2;

    if ( !(pCtrlData = alloca (sizeof (CTRLDATA) * ctrlCount)) )
        return 0;
    
    DlgBoxData.dwStyle      = WS_CAPTION | WS_BORDER;
    DlgBoxData.dwExStyle    = fIME?WS_EX_IMECOMPOSE:WS_EX_NONE;
    DlgBoxData.caption      = title;
    DlgBoxData.hIcon        = 0;
    DlgBoxData.hMenu        = 0;
    DlgBoxData.controlnr    = ctrlCount;
    DlgBoxData.controls     = pCtrlData;
    DlgBoxData.dwAddData    = 0L;

    cw = BUTTON_WIDTH * buttonCount + INTERW_CTRLS * (buttonCount - 1);
    if (width < editboxwidth + INTERW_CTRLS + LABEL_WIDTH)
        width = editboxwidth + INTERW_CTRLS + LABEL_WIDTH;
    cw = max (cw, width);
    labelwidth = cw - editboxwidth - INTERW_CTRLS;
    cw += MARGIN_LEFT + MARGIN_RIGHT;

    rcText.left = 0;
    rcText.top  = 0;
    rcText.right = cw - MARGIN_LEFT - MARGIN_RIGHT;
    rcText.bottom = GetCharHeight ();
    DrawText (HDC_SCREEN, label, -1, &rcText, 
                DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);
    
    ch = MARGIN_TOP + RECTH (rcText) + INTERH_CTRLS
                    + (ENTRY_HEIGHT + INTERH_CTRLS) * entryCount 
                    + BUTTON_HEIGHT
                    + MARGIN_BOTTOM;

    ww = ClientWidthToWindowWidth (DlgBoxData.dwStyle, cw);
    wh = ClientHeightToWindowHeight (DlgBoxData.dwStyle, ch, FALSE);

    DlgBoxData.x = (GetGDCapability (HDC_SCREEN, GDCAP_HPIXEL) - ww)/2;
    DlgBoxData.y = (GetGDCapability (HDC_SCREEN, GDCAP_VPIXEL) - wh)/2;
    DlgBoxData.w = ww;
    DlgBoxData.h = wh;
    
    // LOGO:
    pCtrlData->class    = "static";
    pCtrlData->dwStyle  = WS_CHILD | WS_VISIBLE | SS_ICON;
    pCtrlData->x        = 10;
    pCtrlData->y        = MARGIN_TOP;
    pCtrlData->w        = 40;
    pCtrlData->h        = 40;;
    pCtrlData->id       = IDC_STATIC;
    pCtrlData->caption  = "LOGO";
    pCtrlData->dwAddData = GetLargeSystemIcon (IDI_APPLICATION);
    
    // label:
    (pCtrlData + 1)->class    = "static";
    (pCtrlData + 1)->dwStyle  = WS_CHILD | WS_VISIBLE | SS_LEFT;
    (pCtrlData + 1)->x        = MARGIN_LEFT;
    (pCtrlData + 1)->y        = MARGIN_TOP;
    (pCtrlData + 1)->w        = cw - MARGIN_LEFT - MARGIN_RIGHT;
    (pCtrlData + 1)->h        = RECTH (rcText);
    (pCtrlData + 1)->id       = IDC_STATIC;
    (pCtrlData + 1)->caption  = label;
    (pCtrlData + 1)->dwAddData = 0;
    
    // entries:
    entryy = MARGIN_TOP + RECTH (rcText) + INTERH_CTRLS;
    entryx = MARGIN_LEFT + labelwidth + INTERW_CTRLS;
    for (i = 0; i < entryCount; i++) {
        // label
        (pCtrlData + i*2 + 2)->class    = "static";
        (pCtrlData + i*2 + 2)->dwStyle  = WS_CHILD | WS_VISIBLE | SS_RIGHT;
        (pCtrlData + i*2 + 2)->x        = MARGIN_LEFT;
        (pCtrlData + i*2 + 2)->y        = entryy + 3;
        (pCtrlData + i*2 + 2)->w        = labelwidth;
        (pCtrlData + i*2 + 2)->h        = LABEL_HEIGHT;
        (pCtrlData + i*2 + 2)->id       = IDC_STATIC;
        (pCtrlData + i*2 + 2)->caption  = (items + i)->text;
        (pCtrlData + i*2 + 2)->dwAddData = 0;
        
        // editbox
        (pCtrlData + i*2 + 3)->class    = "edit";
        (pCtrlData + i*2 + 3)->dwStyle  = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
        (pCtrlData + i*2 + 3)->dwStyle |= WS_BORDER | (items + i)->flags;
        (pCtrlData + i*2 + 3)->x        = entryx;
        (pCtrlData + i*2 + 3)->y        = entryy;
        (pCtrlData + i*2 + 3)->w        = editboxwidth;
        (pCtrlData + i*2 + 3)->h        = ENTRY_HEIGHT;
        (pCtrlData + i*2 + 3)->id       = maxid + i;
        (pCtrlData + i*2 + 3)->caption  = *(items + i)->value;
        (pCtrlData + i*2 + 3)->dwAddData = 0;

        entryy += ENTRY_HEIGHT + INTERH_CTRLS;
    }

    buttony =  entryy;
    buttonx =  ((cw - MARGIN_LEFT - MARGIN_RIGHT - 
        BUTTON_WIDTH*buttonCount - INTERW_CTRLS * (buttonCount - 1))>>1)
        + MARGIN_LEFT;

    // buttons
    count = (entryCount<<1) + 2;
    for (i = count; i < buttonCount + count; i++) {
        (pCtrlData + i)->class      = "button";
        (pCtrlData + i)->dwStyle    = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
        (pCtrlData + i)->dwStyle   |= (buttons + i - count)->flags;
        (pCtrlData + i)->x          = buttonx;
        (pCtrlData + i)->y          = buttony;
        (pCtrlData + i)->w          = BUTTON_WIDTH;
        (pCtrlData + i)->h          = BUTTON_HEIGHT;
        (pCtrlData + i)->id         = (buttons + i - count)->id;
        (pCtrlData + i)->caption    = (buttons + i - count)->text;
        (pCtrlData + i)->dwAddData  = 0;

        buttonx += BUTTON_WIDTH + INTERW_CTRLS;
    }
    
    WinEntryItems.entries       = items;
    WinEntryItems.entrycount    = entryCount;
    WinEntryItems.firstentryid  = maxid;
    WinEntryItems.minbuttonid   = minbuttonid;
    WinEntryItems.maxbuttonid   = maxbuttonid;
    return DialogBoxIndirectParam (&DlgBoxData, hParentWnd, WinEntryBoxProc, 
                                        (LPARAM)(&WinEntryItems));
}

static void logwinAddTextLine (HWND hLogWin, const char* text, BOOL fUpdate)
{
    PLOGWININFO info;
    PTEXTLINE line;
    PTEXTLINE oldline = NULL;
    int nCharH = GetCharHeight ();

    info = (PLOGWININFO)GetWindowAdditionalData2 (hLogWin);
    line = info->lines;

    info->linenr ++;

    if (info->linenr > info->vislines) {
        oldline = info->lines;
        info->lines = info->lines->next;
    }
    
    if (!info->lines) {
        if (oldline)
            info->lines  = oldline;
        else
            info->lines = malloc (sizeof (TEXTLINE));

        strncpy (info->lines->text, text, MAX_LENGTH_LINE); 
        info->lines->text [MAX_LENGTH_LINE] = '\0';
        info->lines->next = NULL;
        info->lines->lineno = info->linenr;
    }
    else {
        line = info->lines;

        while(line->next) {
            line = line->next;
        }
        
        if (oldline)
            line->next = oldline;
        else
            line->next = malloc (sizeof (TEXTLINE));

        line = line->next;
        strncpy (line->text, text, MAX_LENGTH_LINE);
        line->text [MAX_LENGTH_LINE] = '\0';
        line->next = NULL;
        line->lineno = info->linenr;
    }
    
    if (fUpdate) {
        RECT rcLine;
        RECT rcClient;
        int lastline = info->linenr;
        
        if (lastline > info->vislines) {
            lastline = info->vislines;
            ScrollWindow (hLogWin, 0, -nCharH, NULL, NULL);
        }
        else {
            // Get line output rect
            GetClientRect (hLogWin, &rcClient);
            rcLine.left   = rcClient.left;
            rcLine.right  = rcClient.right;
            rcLine.top    = rcClient.top + nCharH * (lastline - 1);
            rcLine.bottom = rcLine.top + nCharH;
        
            InvalidateRect (hLogWin, &rcLine, FALSE);
        }
    }
}

static void logwinDeleteContent (HWND hLogWin)
{
    PLOGWININFO info;
    PTEXTLINE line;
    PTEXTLINE next;

    info = (PLOGWININFO)GetWindowAdditionalData2 (hLogWin);
    line = info->lines;

    if (!line) return;
    while (line) {
        next = line->next;
        free (line);
        line = next;
    }

    info->lines = NULL;
    info->linenr = 0;
}

static void logwinOutLogWinText(HWND hLogWin, HDC hdc)
{
    PLOGWININFO info;
    PTEXTLINE line;
    RECT rcClient;
    RECT rcLine;
    int iLine = 0;
    int nCharH = GetCharHeight ();

    info = (PLOGWININFO)GetWindowAdditionalData2 (hLogWin);
    line = info->lines;
    
    GetClientRect(hLogWin, &rcClient);
    SetTextColor (hdc, COLOR_lightwhite);
    SetBkColor (hdc, COLOR_black);
    while (line) {
        char szLineNo [10];

        rcLine.left = rcClient.left + 2;
        rcLine.top = rcClient.top + nCharH*iLine;
        rcLine.right = rcClient.right;
        rcLine.bottom = rcLine.top + nCharH;
        
        sprintf (szLineNo, "%06d", line->lineno);
        TextOut (hdc, rcLine.left, rcLine.top, szLineNo);
        TextOut (hdc, rcLine.left + GetCharWidth () * 6 + 4,
                          rcLine.top, line->text);
        
        line = line->next;
        iLine++;
    }
}

extern HWND hLogWin;
static int LogWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_CREATE:
        break;

    case MSG_PAINT:
        {
            HDC hdc;
            hdc = BeginPaint (hWnd);
            logwinOutLogWinText(hWnd, hdc);
            EndPaint (hWnd, hdc);
        }
        return 0;

    case MSG_TIMER:
        break;

    case MSG_LW_ADDTEXT:
        logwinAddTextLine (hWnd, (char*)lParam, TRUE);
        break;
        
    case MSG_LW_CLEANUP:
        logwinDeleteContent (hWnd);
        break;
        
    case MSG_CLOSE:
        {
            ONCLOSE OnClose 
                        = (ONCLOSE) GetWindowAdditionalData (hWnd);
            if (OnClose && (*OnClose) (hWnd)) {
                logwinDeleteContent (hWnd);
                DestroyMainWindow (hWnd);
                PostQuitMessage (hWnd);
            }
            break;
        }
    }
 
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static void logwinInitCreateInfo (PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_THINFRAME;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = "Log Window";
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = LogWinProc;
    pCreateInfo->lx = -1;
    pCreateInfo->ty = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - 141;
    pCreateInfo->rx = GetGDCapability (HDC_SCREEN, GDCAP_MAXX) + 1;
    pCreateInfo->by = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) + 1;
    pCreateInfo->iBkColor = COLOR_black;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
} 

void* LogWindow (void* data)
{
    MSG Msg;
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;
    LOGWINCREATEINFO* LogWinCreateInfo = (LOGWINCREATEINFO*)data;
    LOGWININFO info;
    RECT rcClient;

    logwinInitCreateInfo (&CreateInfo);
    if (LogWinCreateInfo->fVisible)
        CreateInfo.dwStyle |= WS_VISIBLE;
    CreateInfo.dwAddData = (DWORD)(LogWinCreateInfo->OnClose);

    hMainWnd = CreateMainWindow (&CreateInfo);
    LogWinCreateInfo->hLogWin = hMainWnd;
    sem_post (&LogWinCreateInfo->wait);

    if (hMainWnd == HWND_INVALID)
        return NULL;

    GetClientRect (hMainWnd, &rcClient);
    info.vislines  = (RECTH (rcClient) + GetCharHeight () - 1)/GetCharHeight ();
    if (info.vislines <= 0)
        info.vislines = 1;

    info.linenr  = 0;
    info.lines   = NULL;

    SetWindowAdditionalData2 (hMainWnd, (DWORD)(&info));

    while( GetMessage (&Msg, hMainWnd) ) {
        DispatchMessage (&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);
    return NULL;
}

HWND createLogWin (BOOL fVisible, ONCLOSE OnClose)
{
    LOGWINCREATEINFO LogWinCreateInfo;
    pthread_t th;

    sem_init (&LogWinCreateInfo.wait, 0, 0);
    LogWinCreateInfo.fVisible = fVisible;
    LogWinCreateInfo.OnClose  = OnClose;
    CreateThreadForMainWindow (&th, NULL, LogWindow, &LogWinCreateInfo);
    sem_wait (&LogWinCreateInfo.wait);

    sem_destroy (&LogWinCreateInfo.wait);

    return LogWinCreateInfo.hLogWin;
}
 
void destroyLogWin (HWND hLogWin)
{
    SendMessage (hLogWin, MSG_CLOSE, 0, 0);
}

