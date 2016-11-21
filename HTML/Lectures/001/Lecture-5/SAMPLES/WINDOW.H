//
// $Id: window.h,v 1.2 2000/04/20 03:18:24 weiym Exp $
//
// window.h: the head file of window management module.
//
// Copyright (c) 1999, Mr. Wei Yongming.
//
// Create date: 1999.01.14

#ifndef GUI_WINDOW_H
    #define GUI_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef int (* WNDPROC)(HWND, int, WPARAM, LPARAM);
typedef int (* KEYMSGHOOK)(HWND, int, WPARAM, LPARAM);
typedef int (* MOUSEMSGHOOK)(HWND, int, WPARAM, LPARAM);

/******************************** Message support ****************************/
// definitions of common messages.
#define MSG_SYNCMSG         0x0000
#define MSG_NULLMSG         0x0000

    // Group 1 from 0x0001 to 0x000F, the mouse messages.
#define MSG_FIRSTMOUSEMSG   0x0001

#define MSG_LBUTTONDOWN     0x0001
#define MSG_LBUTTONUP       0x0002
#define MSG_LBUTTONDBLCLK   0x0003
#define MSG_MOUSEMOVE       0x0004
#define MSG_RBUTTONDOWN     0x0005
#define MSG_RBUTTONUP       0x0006
#define MSG_RBUTTONDBLCLK   0x0007
#define MSG_NCMOUSEOFF      0x0007
#define MSG_NCLBUTTONDOWN   0x0008
#define MSG_NCLBUTTONUP     0x0009
#define MSG_NCLBUTTONDBLCLK 0x000A
#define MSG_NCMOUSEMOVE     0x000B
#define MSG_NCRBUTTONDOWN   0x000C
#define MSG_NCRBUTTONUP     0x000D
#define MSG_NCRBUTTONDBLCLK 0x000E
#define MSG_MOUSEMOVEIN     0x000F

#define MSG_LASTMOUSEMSG    0x000F

    // Group 2 from 0x0010 to 0x001F, the key messages.
#define MSG_FIRSTKEYMSG     0x0010

#define MSG_KEYDOWN         0x0010
#define MSG_CHAR            0x0011
#define MSG_KEYUP           0x0012
#define MSG_SYSKEYDOWN      0x0013
#define MSG_SYSCHAR         0x0014
#define MSG_SYSKEYUP        0x0015

#define MSG_LASTKEYMSG      0x001F

    // Group 3 from 0x0020 to 0x005F, the post-mousekey messages.
#define MSG_FIRSTPOSTMSG    0x0020

#define MSG_SETCURSOR       0x0020

    #define HT_UNKNOWN          0x00
    #define HT_OUT              0x01
    #define HT_MENUBAR          0x02
    #define HT_TRANSPARENT      0x03
    #define HT_BORDER_TOP       0x04
    #define HT_BORDER_BOTTOM    0x05
    #define HT_BORDER_LEFT      0x06
    #define HT_BORDER_RIGHT     0x07
    #define HT_CORNER_TL        0x08
    #define HT_CORNER_TR        0x09
    #define HT_CORNER_BL        0x0A
    #define HT_CORNER_BR        0x0B
    #define HT_CLIENT           0x0C
    
    #define SBPOS_LEFTARROW     0x81
    #define SBPOS_RIGHTARROW    0x82
    #define SBPOS_LEFTSPACE     0x83
    #define SBPOS_RIGHTSPACE    0x84
    #define SBPOS_UPARROW       0x85
    #define SBPOS_DOWNARROW     0x86
    #define SBPOS_UPSPACE       0x87
    #define SBPOS_DOWNSPACE     0x88
    #define SBPOS_THUMB         0x89
    #define SBPOS_UNKNOWN       0x80
    #define SBPOS_MASK          0x80

    #define HT_BORDER           0x11
    #define HT_NCLIENT          0x12
    #define HT_CAPTION          0x13
    #define HT_ICON             0x14
    #define HT_CLOSEBUTTON      0x15
    #define HT_MAXBUTTON        0x16
    #define HT_MINBUTTON        0x17
    #define HT_HSCROLL          0x18
    #define HT_VSCROLL          0x19

    #define HT_NEEDCAPTURE      0x10

#define MSG_NCHITTEST       0x0021      // this is an async message.
#define MSG_HITTEST         MSG_NCHITTEST
#define MSG_CHANGESIZE      0x0022
#define MSG_QUERYNCRECT     0x0023
#define MSG_QUERYCLIENTAREA 0x0024
#define MSG_SIZECHANGING    0x0025
#define MSG_SIZECHANGED     0x0026

#define MSG_SETFOCUS        0x0030
#define MSG_KILLFOCUS       0x0031
#define MSG_MOUSEACTIVE     0x0032
#define MSG_ACTIVE          0x0033

#define MSG_ACTIVEMENU      0x0040
#define MSG_DEACTIVEMENU    0x0041

    // Scroll bar notifying code
    #define SB_LINEUP           0x01
    #define SB_LINEDOWN         0x02
    #define SB_LINELEFT         0x03
    #define SB_LINERIGHT        0x04
    #define SB_PAGEUP           0x05
    #define SB_PAGEDOWN         0x06
    #define SB_PAGELEFT         0x07
    #define SB_PAGERIGHT        0x08
    #define SB_THUMBPOSITION    0x09
    #define SB_THUMBTRACK       0x0A
    #define SB_ENDSCROLL        0x0B
    
#define MSG_HSCROLL         0x0042
#define MSG_VSCROLL         0x0043

#define MSG_NCSETCURSOR     0x0044

#define MSG_LASTPOSTMSG     0x005F

    // Group 4 from 0x0060 to 0x009F, the creation messages.
#define MSG_FIRSTCREATEMSG  0x0060

#define MSG_CREATE          0x0060
#define MSG_NCCREATE        0x0061
#define MSG_INITPANES       0x0062
#define MSG_DESTROYPANES    0x0063
#define MSG_DESTROY         0x0064
#define MSG_NCDESTROY       0x0065
#define MSG_CLOSE           0x0066
#define MSG_NCCALCSIZE      0x0067

#define MSG_LASTCREATEMSG   0x009F

    // Group 5 from 0x00A0 to 0x00CF, the paint messages.
#define MSG_FIRSTPAINTMSG   0x00A0

#define MSG_SHOWWINDOW      0x00A0

#define MSG_ERASEBKGND      0x00B0      // this is an async message.
#define MSG_PAINT           0x00B1
#define MSG_NCPAINT         0x00B2
#define MSG_NCACTIVATE      0x00B3
#define MSG_SYNCPAINT       0x00B4

#define MSG_ENABLE          0x00C0

#define MSG_LASTPAINTMSG    0x00CF

    // Group 6 from 0x00D0 to 0x00EF, the desktop messages.
#define MSG_FIRSTSESSIONMSG 0x00D0

#define MSG_STARTSESSION    0x00D0
#define MSG_QUERYENDSESSION 0x00D1
#define MSG_ENDSESSION      0x00D2

#define MSG_ERASEDESKTOP    0x00E0
#define MSG_PAINTDESKTOP    0x00E1

#define MSG_DT_MOUSEOFF     0x00E1
#define MSG_DT_LBUTTONDOWN  0x00E2
#define MSG_DT_LBUTTONUP    0x00E3
#define MSG_DT_LBUTTONDBLCLK    0x00E4
#define MSG_DT_MOUSEMOVE    0x00E5
#define MSG_DT_RBUTTONDOWN  0x00E6
#define MSG_DT_RBUTTONUP    0x00E7
#define MSG_DT_RBUTTONDBLCLK    0x00E8

#define MSG_DT_KEYOFF       0x00DA 
#define MSG_DT_KEYDOWN      0x00EA
#define MSG_DT_CHAR         0x00EB
#define MSG_DT_KEYUP        0x00EC
#define MSG_DT_SYSKEYDOWN   0x00ED
#define MSG_DT_SYSCHAR      0x00EE
#define MSG_DT_SYSKEYUP     0x00EF

#define MSG_LASTSESSIONMSG  0x00EF

    // Group 7 from 0x00F0 to 0x010F, the window messages.
#define MSG_FIRSTWINDOWMSG  0x00F0

#define MSG_ADDNEWMAINWIN   0x00F0
#define MSG_REMOVEMAINWIN   0x00F1
#define MSG_MOVETOTOPMOST   0x00F2 
#define MSG_SETACTIVEMAIN   0x00F3
#define MSG_GETACTIVEMAIN   0x00F4
#define MSG_SHOWMAINWIN     0x00F5
#define MSG_HIDEMAINWIN     0x00F6
#define MSG_MOVEMAINWIN     0x00F7
#define MSG_SETCAPTURE      0x00F8
#define MSG_GETCAPTURE      0x00F9

#define MSG_ENDTRACKMENU    0x00FA
#define MSG_TRACKPOPUPMENU  0x00FB
#define MSG_CLOSEMENU       0x00FC
#define MSG_SCROLLMAINWIN   0x00FD
#define MSG_CARET_CREATE    0x00FE
#define MSG_CARET_DESTROY   0x00FF

#define MSG_ENABLEMAINWIN   0x0100
#define MSG_ISENABLED       0x0101

#define MSG_SETWINCURSOR    0x0102

#define MSG_LASTWINDOWMSG   0x010F

    // Group 8 from 0x0120 to 0x013F, the dialog and control messages.
#define MSG_FIRSTCONTROLMSG 0x0120

#define MSG_COMMAND         0x0120
#define MSG_SYSCOMMAND      0x0121

#define MSG_GETDLGCODE      0x0122
#define MSG_INITDIALOG      0x0123
#define MSG_NEXTDLGCTRL     0x0124
#define MSG_ENTERIDLE       0x0125
#define MSG_DLG_GETDEFID    0x0126
#define MSG_DLG_SETDEFID    0x0127
#define MSG_DLG_REPOSITION  0x0128

#define MSG_GETFONT         0x0130
#define MSG_SETFONT         0x0131

#define MSG_GETTEXTLENGTH   0x0132
#define MSG_GETTEXT         0x0133
#define MSG_SETTEXT         0x0134

#define MSG_LASTCONTROLMSG  0x013F

    // Group 9 from 0x0140 to 0x016F, the system messages.
#define MSG_FIRSTSYSTEMMSG  0x0140

#define MSG_QUIT            0x0140
#define MSG_IDLE            0x0142
#define MSG_TIMEOUT         0x0143
#define MSG_TIMER           0x0144
#define MSG_CARETBLINK      0x0145

#define MSG_IME_REGISTER    0x0151
#define MSG_IME_UNREGISTER  0x0152
#define MSG_IME_OPEN        0x0153
#define MSG_IME_CLOSE       0x0154
#define MSG_IME_SETSTATUS   0x0156
#define MSG_IME_GETSTATUS   0x0157
    #define IS_ENABLE       1
    #define IS_FULLCHAR     2
    #define IS_FULLPUNC     3
    #define IS_METHOD       4
#define MSG_IME_SETTARGET   0x0158
#define MSG_IME_GETTARGET   0x0159

#define MSG_SHOWMENU        0x0160
#define MSG_HIDEMENU        0x0161

#define MSG_ADDTIMER        0x0162
#define MSG_REMOVETIMER     0x0163
#define MSG_RESETTIMER      0x0164

#define MSG_WINDOWCHANGED   0x0165

#define MSG_BROADCASTMSG    0x0166

#define MSG_REGISTERWNDCLASS    0x0167
#define MSG_UNREGISTERWNDCLASS  0x0168
#define MSG_NEWCTRLINSTANCE     0x0169
#define MSG_REMOVECTRLINSTANCE  0x016A
#define MSG_GETCTRLCLASSINFO    0x016B
#define MSG_CTRLCLASSDATAOP     0x016C
    #define CCDOP_GETCCI        0x01
    #define CCDOP_SETCCI        0x02

#define MSG_REGISTERKEYHOOK     0x016D
#define MSG_REGISTERMOUSEHOOK   0x016E
#define MSG_UNREGISTERHOOK      0x016F

#define MSG_LASTSYSTEMMSG   0x016F

    // Group 10 from 0x0170 to 0x018F, the menu messages
#define MSG_FIRSTMENUMSG    0x0170

#define MSG_INITMENU        0x0170
#define MSG_INITMENUPOPUP   0x0171
#define MSG_MENUSELECT      0x0172
#define MSG_MENUCHAR        0x0173
#define MSG_ENTERMENULOOP   0x0174
#define MSG_EXITMENULOOP    0x0175
#define MSG_CONTEXTMENU     0x0176
#define MSG_NEXTMENU        0x0177

#define MSG_LASTMENUMSG     0x018F

#define MSG_FIRSTUSERMSG    0x0800

#define MSG_USER            0x0800

#define MSG_LASTUSERMSG     0xEFFF

// NOTE:
// Control messages start from 0xF000

#define MSG_FIRSTCTRLMSG    0xF000

// Static Control messages
#define STM_SETICON         0xF170
#define STM_GETICON         0xF171
#define STM_SETIMAGE        0xF172
#define STM_GETIMAGE        0xF173
#define STM_MSGMAX          0xF174

// Static Control notification code
#define STN_CLICKED         0
#define STN_DBLCLK          1
#define STN_ENABLE          2
#define STN_DISABLE         3

// Edit Control messages
#define EM_GETSEL               0xF0B0
#define EM_SETSEL               0xF0B1
#define EM_GETRECT              0xF0B2
#define EM_SETRECT              0xF0B3
#define EM_SETRECTNP            0xF0B4
#define EM_SCROLL               0xF0B5
#define EM_LINESCROLL           0xF0B6
#define EM_SCROLLCARET          0xF0B7
#define EM_GETMODIFY            0xF0B8
#define EM_SETMODIFY            0xF0B9
#define EM_GETLINECOUNT         0xF0BA
#define EM_LINEINDEX            0xF0BB
#define EM_SETHANDLE            0xF0BC
#define EM_GETHANDLE            0xF0BD
#define EM_GETTHUMB             0xF0BE
#define EM_LINELENGTH           0xF0C1
#define EM_REPLACESEL           0xF0C2
#define EM_GETLINE              0xF0C4
#define EM_LIMITTEXT            0xF0C5
#define EM_CANUNDO              0xF0C6
#define EM_UNDO                 0xF0C7
#define EM_FMTLINES             0xF0C8
#define EM_LINEFROMCHAR         0xF0C9
#define EM_SETTABSTOPS          0xF0CB
#define EM_SETPASSWORDCHAR      0xF0CC
#define EM_EMPTYUNDOBUFFER      0xF0CD
#define EM_GETFIRSTVISIBLELINE  0xF0CE
#define EM_SETREADONLY          0xF0CF
#define EM_SETWORDBREAKPROC     0xF0D0
#define EM_GETWORDBREAKPROC     0xF0D1
#define EM_GETPASSWORDCHAR      0xF0D2
#define EM_SETMARGINS           0xF0D3
#define EM_GETMARGINS           0xF0D4
#define EM_SETLIMITTEXT         EM_LIMITTEXT
#define EM_GETLIMITTEXT         0xF0D5
#define EM_POSFROMCHAR          0xF0D6
#define EM_CHARFROMPOS          0xF0D7
#define EM_SETIMESTATUS         0xF0D8
#define EM_GETIMESTATUS         0xF0D9

// Button Control messages
#define BM_GETCHECK             0xF0F0
#define BM_SETCHECK             0xF0F1
#define BM_GETSTATE             0xF0F2
#define BM_SETSTATE             0xF0F3
#define BM_SETSTYLE             0xF0F4
#define BM_CLICK                0xF0F5
#define BM_GETIMAGE             0xF0F6
#define BM_SETIMAGE             0xF0F7
    #define BM_IMAGE_BITMAP         1
    #define BM_IMAGE_ICON           2

// Progress Bar messages
#define PBM_SETRANGE            0xF0A0
#define PBM_SETSTEP             0xF0A1
#define PBM_SETPOS              0xF0A2
#define PBM_DELTAPOS            0xF0A3
#define PBM_STEPIT              0xF0A4

// Listbox messages
#define LB_ADDSTRING            0xF180
#define LB_INSERTSTRING         0xF181
#define LB_DELETESTRING         0xF182
#define LB_SELITEMRANGEEX       0xF183
#define LB_RESETCONTENT         0xF184
#define LB_SETSEL               0xF185
#define LB_SETCURSEL            0xF186
#define LB_GETSEL               0xF187
#define LB_GETCURSEL            0xF188
#define LB_GETTEXT              0xF189
#define LB_GETTEXTLEN           0xF18A
#define LB_GETCOUNT             0xF18B
#define LB_SELECTSTRING         0xF18C
#define LB_DIR                  0xF18D
#define LB_GETTOPINDEX          0xF18E
#define LB_FINDSTRING           0xF18F
#define LB_GETSELCOUNT          0xF190
#define LB_GETSELITEMS          0xF191
#define LB_SETTABSTOPS          0xF192
#define LB_GETHORIZONTALEXTENT  0xF193
#define LB_SETHORIZONTALEXTENT  0xF194
#define LB_SETCOLUMNWIDTH       0xF195
#define LB_ADDFILE              0xF196
#define LB_SETTOPINDEX          0xF197
#define LB_GETITEMRECT          0xF198
#define LB_GETITEMDATA          0xF199
#define LB_SETITEMDATA          0xF19A
#define LB_SELITEMRANGE         0xF19B
#define LB_SETANCHORINDEX       0xF19C
#define LB_GETANCHORINDEX       0xF19D
#define LB_SETCARETINDEX        0xF19E
#define LB_GETCARETINDEX        0xF19F
#define LB_SETITEMHEIGHT        0xF1A0
#define LB_GETITEMHEIGHT        0xF1A1
#define LB_FINDSTRINGEXACT      0xF1A2
#define LB_SETLOCALE            0xF1A5
#define LB_GETLOCALE            0xF1A6
#define LB_SETCOUNT             0xF1A7
#define LB_INITSTORAGE          0xF1A8
#define LB_ITEMFROMPOINT        0xF1A9
#define LB_SETTEXT              0xF1AA
#define LB_GETCHECKMARK         0xF1AB
#define LB_SETCHECKMARK         0xF1AC
#define LB_GETITEMADDDATA       0xF1AD
#define LB_SETITEMADDDATA       0xF1AE
#define LB_MSGMAX               0xF1B0

// Combo Box messages
#define CB_GETEDITSEL               0xF140
#define CB_LIMITTEXT                0xF141
#define CB_SETEDITSEL               0xF142
#define CB_ADDSTRING                0xF143
#define CB_DELETESTRING             0xF144
#define CB_DIR                      0xF145
#define CB_GETCOUNT                 0xF146
#define CB_GETCURSEL                0xF147
#define CB_GETLBTEXT                0xF148
#define CB_GETLBTEXTLEN             0xF149
#define CB_INSERTSTRING             0xF14A
#define CB_RESETCONTENT             0xF14B
#define CB_FINDSTRING               0xF14C
#define CB_SELECTSTRING             0xF14D
#define CB_SETCURSEL                0xF14E
#define CB_SHOWDROPDOWN             0xF14F
#define CB_GETITEMDATA              0xF150
#define CB_SETITEMDATA              0xF151
#define CB_GETDROPPEDCONTROLRECT    0xF152
#define CB_SETITEMHEIGHT            0xF153
#define CB_GETITEMHEIGHT            0xF154
#define CB_SETEXTENDEDUI            0xF155
#define CB_GETEXTENDEDUI            0xF156
#define CB_GETDROPPEDSTATE          0xF157
#define CB_FINDSTRINGEXACT          0xF158
#define CB_SETLOCALE                0xF159
#define CB_GETLOCALE                0xF15A
#define CB_GETTOPINDEX              0xF15b
#define CB_SETTOPINDEX              0xF15c
#define CB_GETHORIZONTALEXTENT      0xF15d
#define CB_SETHORIZONTALEXTENT      0xF15e
#define CB_GETDROPPEDWIDTH          0xF15f
#define CB_SETDROPPEDWIDTH          0xF160
#define CB_INITSTORAGE              0xF161
#define CB_MSGMAX                   0xF162

// Scroll bar messages
#define SBM_SETPOS                  0xF0E0 /*not in win3.1 */
#define SBM_GETPOS                  0xF0E1 /*not in win3.1 */
#define SBM_SETRANGE                0xF0E2 /*not in win3.1 */
#define SBM_SETRANGEREDRAW          0xF0E6 /*not in win3.1 */
#define SBM_GETRANGE                0xF0E3 /*not in win3.1 */
#define SBM_ENABLE_ARROWS           0xF0E4 /*not in win3.1 */
#define SBM_SETSCROLLINFO           0xF0E9
#define SBM_GETSCROLLINFO           0xF0EA

#define MSG_LASTCTRLMSG             0xFFFF

typedef struct _MSG
{
    HWND             hwnd;
    int              message;
    WPARAM           wParam;
    LPARAM           lParam;
    struct timeval   time;
    POINT            pt;
    void*            pAdd;
}MSG;
typedef MSG* PMSG;

typedef struct _QMSG
{
    MSG                 Msg;
    struct _QMSG*       next;
    BOOL                fromheap;
}QMSG;
typedef QMSG* PQMSG;

typedef struct _SYNCMSG
{
    MSG              Msg;
    int              retval;
    sem_t            sem_handle;
    struct _SYNCMSG* pNext;
}SYNCMSG;
typedef SYNCMSG* PSYNCMSG;

#define DEF_MSGQUEUE_LEN	16
// the MSGQUEUE struct is a internal struct.
// using semaphores to implement message queue.
#define QS_NOTIFYMSG        0x00000100
#define QS_SYNCMSG          0x00000200
#define QS_POSTMSG          0x00000400
#define QS_QUIT             0x00000800
#define QS_INPUT            0x00001000
#define QS_PAINT            0x00002000
#define QS_TIMER            0x000000FF
#define QS_EMPTY            0x00000000

typedef struct _MSGQUEUE
{
    DWORD dwState;              // message queue states

    pthread_mutex_t lock;       // lock
    sem_t wait;                 // wait semaphores

    PQMSG  pFirstNotifyMsg;     // head of the notify message queue
    PQMSG  pLastNotifyMsg;      // tail of the notify message queue

    PSYNCMSG pFirstSyncMsg;     // head of the sync message queue
    PSYNCMSG pLastSyncMsg;      // tail of the sync message queue

    MSG* msg;                   // post message buffer
    int len;                    // buffer len
    int readpos, writepos;      // positions for reading and writing

    /*
     * One thread can only support eight timers.
     * And number of all timers in a MiniGUI applicatoin is 16.
     */
    HWND TimerOwner[8];         // Timer owner
    int  TimerID[8];            // timer identifiers.
    BYTE TimerMask;             // used timer slots mask
}MSGQUEUE;
typedef MSGQUEUE* PMSGQUEUE;

// Free QMSG list
#define SIZE_QMSG_HEAP   64
typedef struct _FREEQMSGLIST
{
    pthread_mutex_t lock;
    PQMSG           head;
    PQMSG           tail;
    int             nr;
    PQMSG           heap;
    int             free;
}FREEQMSGLIST;
typedef FREEQMSGLIST* PFREEQMSGLIST;

BOOL GUIAPI InitMsgQueue(PMSGQUEUE pMsgQueue, int iBufferLen);
void GUIAPI DestroyMsgQueue(PMSGQUEUE pMsgQueue);

int GUIAPI GetMessage(PMSG pMsg, HWND hMainWnd);
BOOL GUIAPI HavePendingMessage(HWND hMainWnd);

#define PM_NOREMOVE     0x0000
#define PM_REMOVE       0x0001
#define PM_NOYIELD      0x0002
BOOL GUIAPI PeekPostMessage (PMSG pMsg, HWND hWnd, int iMsgFilterMin, 
                        int iMsgFilterMax, UINT uRemoveMsg);
                        
int GUIAPI PostMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam);
int GUIAPI SendMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam);
int GUIAPI PostSyncMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam);
int GUIAPI SendAsyncMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam);
int GUIAPI SendNotifyMessage(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam);

int GUIAPI BroadcastMessage(int iMsg, WPARAM wParam, LPARAM lParam);
int GUIAPI PostQuitMessage(HWND hWnd);
int GUIAPI TranslateMessage(PMSG pMsg);
int GUIAPI DispatchMessage(PMSG pMsg);

int GUIAPI ThrowAwayMessages (HWND pMainWnd);

int GUIAPI TranslateAccelerator(HWND hWnd, HACCEL hAccel, PMSG pMsg);

/******************************* Hooks support *******************************/
#define HOOK_OK         0
#define HHOOK_INVALID   -1

#define HOOK_STOP       1
#define HOOK_GOON       2

typedef struct _KEYHOOK
{
    HWND hWnd;
    KEYMSGHOOK hook;
}KEYHOOK;

typedef struct _MOUSEHOOK
{
    HWND hWnd;
    MOUSEMSGHOOK hook;
}MOUSEHOOK;

HHOOK GUIAPI RegisterKeyMsgHook (HWND hWnd, KEYMSGHOOK hook);
HHOOK GUIAPI RegisterMouseMsgHook (HWND hWnd, MOUSEMSGHOOK hook);

int GUIAPI UnregisterHook (HHOOK hHook);

/******************************* Style definitions ***************************/
/* Class styles */
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002

#define CS_OWNDC            0x0020
#define CS_CLASSDC          0x0040
#define CS_PARENTDC         0x0080

#define CS_SAVEBITS         0x0800

#define CS_DBLCLKS          0x0008

#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000

#define CS_NOCLOSE          0x0200

#define CS_KEYCVTWINDOW     0x0004
#define CS_NOKEYCVT         0x0100

#define CS_GLOBALCLASS      0x4000

/* Window Styles */

/* Basic window types */
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L

/* Clipping styles */       // not supported

/* Generic window states */
#define WS_VISIBLE          0x08000000L
#define WS_DISABLED         0x04000000L

/* Main window states */
#define WS_MINIMIZE         0x02000000L
#define WS_MAXIMIZE         0x01000000L

/* Main window styles */
#define WS_CAPTION          0x20000000L
#define WS_SYSMENU          0x10000000L
#define WS_DLGFRAME         0x00800000L
#define WS_BORDER           0x00400000L
#define WS_THICKFRAME       0x00200000L
#define WS_THINFRAME        0x00100000L
#define WS_VSCROLL          0x00080000L
#define WS_HSCROLL          0x00040000L
#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L

/* Control window styles */
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L

/* Common Window Styles */
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED  | \
                             WS_CAPTION     | \
                             WS_SYSMENU     | \
                             WS_THICKFRAME  | \
                             WS_MINIMIZEBOX | \
                             WS_MAXIMIZEBOX )
                             
#define WS_POPUPWINDOW      (WS_POPUP       | \
                             WS_BORDER      | \
                             WS_SYSMENU)

#define WS_CHILDWINDOW      (WS_CHILD)

/* Extended Window Styles */
#define WS_EX_NONE              0x00000000L     // supported
#define WS_EX_DLGMODALFRAME     0x00000001L
#define WS_EX_USEPRIVATECDC     0x00000002L     // supported
#define WS_EX_NOPARENTNOTIFY    0x00000004L
#define WS_EX_TOPMOST           0x00000008L     // supported
#define WS_EX_ACCEPTFILES       0x00000010L
#define WS_EX_TRANSPARENT       0x00000020L
#define WS_EX_MDICHILD          0x00000040L
#define WS_EX_TOOLWINDOW        0x00000080L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_CONTEXTHELP       0x00000400L

#define WS_EX_RIGHT             0x00001000L
#define WS_EX_LEFT              0x00000000L
#define WS_EX_RTLREADING        0x00002000L
#define WS_EX_LTRREADING        0x00000000L
#define WS_EX_LEFTSCROLLBAR     0x00004000L
#define WS_EX_RIGHTSCROLLBAR    0x00000000L

#define WS_EX_CONTROLPARENT     0x00010000L
#define WS_EX_STATICEDGE        0x00020000L
#define WS_EX_APPWINDOW         0x00040000L

#define WS_EX_IMECOMPOSE        0x10000000L     // supported

#define WS_EX_OVERLAPPEDWINDOW  (WS_EX_WINDOWEDGE | \
                                 WS_EX_CLIENTEDGE)

#define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | \
                                 WS_EX_TOOLWINDOW | \
                                 WS_EX_TOPMOST)

/****** control styles ******************************************************/

/* Static Control Styles */
#define SS_LEFT             0x00000000L
#define SS_CENTER           0x00000001L
#define SS_RIGHT            0x00000002L
#define SS_ICON             0x00000003L
#define SS_BLACKRECT        0x00000004L
#define SS_GRAYRECT         0x00000005L
#define SS_WHITERECT        0x00000006L
#define SS_BLACKFRAME       0x00000007L
#define SS_GRAYFRAME        0x00000008L
#define SS_WHITEFRAME       0x00000009L
#define SS_GROUPBOX         0x0000000AL
#define SS_SIMPLE           0x0000000BL
#define SS_LEFTNOWORDWRAP   0x0000000CL
#define SS_OWNERDRAW        0x0000000DL
#define SS_BITMAP           0x0000000EL
#define SS_ENHMETAFILE      0x0000000FL
#define SS_TYPEMASK         0x0000000FL

#define SS_NOPREFIX         0x00000080L

#define SS_ETCHEDHORZ       0x00000010L
#define SS_ETCHEDVERT       0x00000011L
#define SS_ETCHEDFRAME      0x00000012L
#define SS_ETCTYPEMAKS      0x0000001FL

#define SS_NOTIFY           0x00000100L
#define SS_CENTERIMAGE      0x00000200L
#define SS_RIGHTJUST        0x00000400L
#define SS_REALSIZEIMAGE    0x00000800L

    /* not supported styles */
    // #define SS_SUNKEN           0x00001000L
    // #define SS_ENDELLIPSIS      0x00004000L
    // #define SS_PATHELLIPSIS     0x00008000L
    // #define SS_WORDELLIPSIS     0x0000C000L
    // #define SS_ELLIPSISMASK     0x0000C000L


/* Button Styles */
#define BS_PUSHBUTTON       0x00000000L
#define BS_DEFPUSHBUTTON    0x00000001L
#define BS_CHECKBOX         0x00000002L
#define BS_AUTOCHECKBOX     0x00000003L
#define BS_RADIOBUTTON      0x00000004L
#define BS_3STATE           0x00000005L
#define BS_AUTO3STATE       0x00000006L
#define BS_GROUPBOX         0x00000007L
#define BS_USERBUTTON       0x00000008L
#define BS_AUTORADIOBUTTON  0x00000009L
#define BS_OWNERDRAW        0x0000000BL
#define BS_TYPEMASK         0x0000000FL

#define BS_TEXT             0x00000000L
#define BS_LEFTTEXT         0x00000020L
#define BS_ICON             0x00000040L
#define BS_BITMAP           0x00000080L
#define BS_CONTENTMASK      0x000000F0L

#define BS_LEFT             0x00000100L
#define BS_RIGHT            0x00000200L
#define BS_CENTER           0x00000300L
#define BS_TOP              0x00000400L
#define BS_BOTTOM           0x00000800L
#define BS_VCENTER          0x00000C00L
#define BS_ALIGNMASK        0x00000F00L

#define BS_PUSHLIKE         0x00001000L
#define BS_MULTLINE         0x00002000L
#define BS_NOTIFY           0x00004000L
#define BS_FLAT             0x00008000L
#define BS_RIGHTBUTTON      BS_LEFTTEXT

#define BST_UNCHECKED       0x0000
#define BST_CHECKED         0x0001
#define BST_INDETERMINATE   0x0002
#define BST_PUSHED          0x0004
#define BST_FOCUS           0x0008

/* User Button notification codes */
#define BN_CLICKED          0
#define BN_PAINT            1
#define BN_HILITE           2
#define BN_UNHILITE         3
#define BN_DISABLE          4
#define BN_DOUBLECLICKED    5
#define BN_PUSHED           BN_HILITE
#define BN_UNPUSHED         BN_UNHILITE
#define BN_DBLCLK           BN_DOUBLECLICKED
#define BN_SETFOCUS         6
#define BN_KILLFOCUS        7

/* Edit control styles */
#define ES_LEFT             0x00000000L
#define ES_CENTER           0x00000001L
#define ES_RIGHT            0x00000002L
#define ES_MULTILINE        0x00000004L
#define ES_UPPERCASE        0x00000008L
#define ES_LOWERCASE        0x00000010L
#define ES_PASSWORD         0x00000020L
#define ES_AUTOVSCROLL      0x00000040L
#define ES_AUTOHSCROLL      0x00000080L
#define ES_NOHIDESEL        0x00000100L
#define ES_OEMCONVERT       0x00000400L
#define ES_READONLY         0x00000800L

/* Edit control notification codes */
#define EN_SETFOCUS         0x0100
#define EN_KILLFOCUS        0x0200
#define EN_CHANGE           0x0300
#define EN_UPDATE           0x0400
#define EN_ERRSPACE         0x0500
#define EN_MAXTEXT          0x0501
#define EN_HSCROLL          0x0601
#define EN_VSCROLL          0x0602
#define EN_ENTER            0x0700

/* Edit control EM_SETMARGIN parameters */
#define EC_LEFTMARGIN       0x0001
#define EC_RIGHTMARGIN      0x0002
#define EC_USEFONTINFO      0xffff

/* wParam of EM_GET/SETIMESTATUS  */
#define EMSIS_COMPOSITIONSTRING        0x0001

/* lParam for EMSIS_COMPOSITIONSTRING  */
#define EIMES_GETCOMPSTRATONCE         0x0001
#define EIMES_CANCELCOMPSTRINFOCUS     0x0002
#define EIMES_COMPLETECOMPSTRKILLFOCUS 0x0004

/* Progress Bar styles */
#define PBS_NOTIFY              0x0001L
#define PBS_VERTICAL            0x0002L

/* Progress Bar notification code */
#define PBN_REACHMAX            1
#define PBN_REACHMIN            2

/* Listbox styles */
#define LBS_NOTIFY              0x0001L
#define LBS_SORT                0x0002L
#define LBS_MULTIPLESEL         0x0008L
#define LBS_CHECKBOX            0x1000L
#define LBS_USEICON             0x2000L
#define LBS_AUTOCHECK           0x4000L
#define LBS_AUTOCHECKBOX        0x5000L

#define LBS_OWNERDRAWFIXED      0x0010L     // not supported currently
#define LBS_OWNERDRAWVARIABLE   0x0020L     // not supported currently
#define LBS_USETABSTOPS         0x0080L     // not supported currently
#define LBS_MULTICOLUMN         0x0200L     // not supported currently

#define LBS_WANTKEYBOARDINPUT   0x0400L     // not intend to support
#define LBS_NOREDRAW            0x0004L     // not intend to support
#define LBS_HASSTRINGS          0x0040L     // not intend to support
#define LBS_NOINTEGRALHEIGHT    0x0100L     // not intend to support
#define LBS_EXTENDEDSEL         0x0800L     // not intend to support

/* Listbox Notification Codes */
#define LBN_ERRSPACE        (-2)
#define LBN_SELCHANGE       1
#define LBN_DBLCLK          2
#define LBN_SELCANCEL       3
#define LBN_SETFOCUS        4
#define LBN_KILLFOCUS       5
#define LBN_CLICKCHECKMARK  6

/* Listbox return value */
#define LB_OKAY             0
#define LB_ERR              (-1)
#define LB_ERRSPACE         (-2)

/* Combo Box styles */
#define CBS_SIMPLE            0x0001L
#define CBS_DROPDOWN          0x0002L
#define CBS_DROPDOWNLIST      0x0003L
#define CBS_OWNERDRAWFIXED    0x0010L
#define CBS_OWNERDRAWVARIABLE 0x0020L
#define CBS_AUTOHSCROLL       0x0040L
#define CBS_OEMCONVERT        0x0080L
#define CBS_SORT              0x0100L
#define CBS_HASSTRINGS        0x0200L
#define CBS_NOINTEGRALHEIGHT  0x0400L
#define CBS_DISABLENOSCROLL   0x0800L
#define CBS_UPPERCASE           0x2000L
#define CBS_LOWERCASE           0x4000L

/* Combo Box return value */
#define CB_OKAY             0
#define CB_ERR              (-1)
#define CB_ERRSPACE         (-2)

/* Combo Box Notification Codes */
#define CBN_ERRSPACE        (-1)
#define CBN_SELCHANGE       1
#define CBN_DBLCLK          2
#define CBN_SETFOCUS        3
#define CBN_KILLFOCUS       4
#define CBN_EDITCHANGE      5
#define CBN_EDITUPDATE      6
#define CBN_DROPDOWN        7
#define CBN_CLOSEUP         8
#define CBN_SELENDOK        9
#define CBN_SELENDCANCEL    10
  
/******************************** Main window support ************************/
#define HWND_DESKTOP		0
#define HWND_INVALID	    0xFFFFFFFF

typedef struct _MAINWINCREATE
{
    DWORD dwStyle;
    DWORD dwExStyle;    // the extended styles of main window.

    const char* spCaption;
    HMENU hMenu;
    HCURSOR hCursor;
    HICON hIcon;

    HWND  hHosting;     // the hosting main window.
    
    int (*MainWindowProc)(HWND, int, WPARAM, LPARAM);

    int lx, ty;
    int rx, by;

    int iBkColor;

    DWORD dwAddData;
}MAINWINCREATE;
typedef MAINWINCREATE* PMAINWINCREATE;

HWND GUIAPI CreateMainWindow(PMAINWINCREATE pCreateStruct);
BOOL GUIAPI DestroyMainWindow(HWND hWnd);
int DefaultMainWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam);
int DefaultControlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

#define SW_HIDE              0x0000
#define SW_SHOW              0x0010
#define SW_SHOWNORMAL        0x0100

void GUIAPI UpdateWindow (HWND hWnd, BOOL bErase);
BOOL GUIAPI ShowWindow(HWND hWnd, int iCmdShow);

BOOL GUIAPI EnableWindow (HWND hWnd, int fEnable);
BOOL GUIAPI IsWindowEnabled (HWND hWnd);

BOOL GUIAPI GetClientRect(HWND hWnd, PRECT prc);

int GUIAPI GetWindowBkColor (HWND hWnd);
HCURSOR GUIAPI GetWindowCursor (HWND hWnd);
HCURSOR GUIAPI SetWindowCursor (HWND hWnd, HCURSOR hNewCursor);
DWORD GUIAPI GetWindowStyle (HWND hWnd);
BOOL GUIAPI ExcludeWindowStyle (HWND hWnd, DWORD dwStyle);
BOOL GUIAPI IncludeWindowStyle (HWND hWnd, DWORD dwStyle);
DWORD GUIAPI GetWindowExStyle (HWND hWnd);
DWORD GUIAPI GetWindowAdditionalData (HWND hWnd);
DWORD GUIAPI SetWindowAdditionalData (HWND hWnd, DWORD newData);
DWORD GUIAPI GetWindowAdditionalData2 (HWND hWnd);
DWORD GUIAPI SetWindowAdditionalData2 (HWND hWnd, DWORD newData);
DWORD GUIAPI GetWindowClassAdditionalData (HWND hWnd);
DWORD GUIAPI SetWindowClassAdditionalData (HWND hWnd, DWORD newData);
char* GUIAPI GetWindowCaption (HWND hWnd);
BOOL GUIAPI SetWindowCaption (HWND hWnd, char* spCaption);

BOOL GUIAPI InvalidateRect(HWND hWnd, const RECT* prc, BOOL bEraseBkgnd);
HDC GUIAPI BeginPaint(HWND hWnd);
void GUIAPI EndPaint(HWND hWnd, HDC hdc);

void GUIAPI ClientToScreen(HWND hWnd, int* x, int* y);
void GUIAPI ScreenToClient(HWND hWnd, int* x, int* y);

void GUIAPI WindowToScreen(HWND hWnd, int* x, int* y);
void GUIAPI ScreenToWindow(HWND hWnd, int* x, int* y);

int ClientWidthToWindowWidth (DWORD dwStyle, int cw);
int ClientHeightToWindowHeight (DWORD dwStyle, int ch, BOOL hasMenu);

BOOL GUIAPI IsMainWindow (HWND hWnd);
BOOL GUIAPI IsControl (HWND hWnd);
BOOL GUIAPI IsWindow (HWND hWnd);
HWND GUIAPI GetParent (HWND hWnd);
BOOL GUIAPI IsWindowVisible (HWND hWnd);
BOOL GUIAPI GetWindowRect (HWND hWnd, PRECT prc);

HWND GUIAPI GetHosting (HWND hWnd);
HWND GUIAPI GetFirstHosted (HWND hWnd);
HWND GUIAPI GetNextHosted (HWND hHosting, HWND hHosted);

int GUIAPI GetWindowTextLength (HWND hWnd);
int GUIAPI GetWindowText (HWND hWnd, char* spString, int nMaxLen);
BOOL GUIAPI SetWindowText (HWND hWnd, char* spString);

HWND GUIAPI SetFocus(HWND hWnd);
HWND GUIAPI GetFocus(void);
HWND GUIAPI GetFocusChild (HWND hMainWnd);

HWND GUIAPI SetActiveWindow(HWND hMainWnd);
HWND GUIAPI GetActiveWindow(void);
HWND GUIAPI SetCapture(HWND hMainWnd);
HWND GUIAPI GetCapture(void);
void GUIAPI ReleaseCapture(void);

#define GetForegroundWindow GetActiveWindow
#define SetForegroundWindow SetActiveWindow

BOOL GUIAPI MoveWindow (HWND hWnd, int x, int y, int w, int h, BOOL fPaint);

typedef struct _SCROLLWINDOWINFO
{
    int iOffx;
    int iOffy;
    const RECT* rc1;
    const RECT* rc2;
}SCROLLWINDOWINFO;
typedef SCROLLWINDOWINFO* PSCROLLWINDOWINFO;
void GUIAPI ScrollWindow (HWND hWnd, int iOffx, int iOffy, 
                            const RECT* rc1, const RECT* rc2);

#define SB_HORZ     1
#define SB_VERT     2

BOOL GUIAPI EnableScrollBar (HWND hWnd, int iSBar, BOOL bEnable);
BOOL GUIAPI GetScrollPos (HWND hWnd, int iSBar, int* pPos);
BOOL GUIAPI GetScrollRange (HWND hWnd, int iSBar, int* pMinPos, int* pMaxPos);
BOOL GUIAPI SetScrollPos (HWND hWnd, int iSBar, int iNewPos);
BOOL GUIAPI SetScrollRange (HWND hWnd, int iSBar, int iMinPos, int iMaxPos);
BOOL GUIAPI ShowScrollBar (HWND hWnd, int iSBar, BOOL bShow);

#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#define SIF_TRACKPOS        0x0010
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)

typedef struct tagSCROLLINFO
{
    UINT    cbSize;
    UINT    fMask;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
}SCROLLINFO, *LPSCROLLINFO;
typedef SCROLLINFO const *LPCSCROLLINFO;

BOOL GUIAPI SetScrollInfo(HWND, int, LPCSCROLLINFO, BOOL);
BOOL GUIAPI GetScrollInfo(HWND, int, LPSCROLLINFO);

HWND GUIAPI GetMainWindowHandle(HWND hWnd);
int GUIAPI CreateThreadForMainWindow(pthread_t* thread, 
                                     pthread_attr_t* attr, 
                                     void * (*start_routine)(void *), 
                                     void * arg);
pthread_t GUIAPI GetMainWinThread(HWND hMainWnd);
void GUIAPI MainWindowThreadCleanup(HWND hMainWnd);

int GUIAPI WaitMainWindowClose(HWND hWnd, void** returnval);

#define MWM_MINWIDTH            0
#define MWM_MINHEIGHT           1
#define MWM_BORDER              2
#define MWM_THICKFRAME          3
#define MWM_CAPTIONY            4
#define MWM_ICONX               5
#define MWM_ICONY               6
#define MWM_CAPTIONOFFY         7
#define MWM_MENUBARY            8
#define MWM_MENUBAROFFX         9
#define MWM_MENUBAROFFY         10
#define MWM_MENUITEMY           11 
#define MWM_INTERMENUITEMX      12
#define MWM_INTERMENUITEMY      13
#define MWM_MENUITEMOFFX        14
#define MWM_MENUTOPMARGIN       15
#define MWM_MENUBOTTOMMARGIN    16
#define MWM_MENULEFTMARGIN      17
#define MWM_MENURIGHTMARGIN     18
#define MWM_MENUITEMMINX        19
#define MWM_MENUSEPARATORY      20
#define MWM_MENUSEPARATORX      21
#define MWM_SB_HEIGHT           22
#define MWM_SB_WIDTH            23
#define MWM_SB_INTERX           24
#define MWM_THINFRAME           25
#define MWM_CXVSCROLL           26
#define MWM_CYVSCROLL           27
#define MWM_CXHSCROLL           28
#define MWM_CYHSCROLL           29
#define MWM_MINBARLEN           30
#define MWM_DEFBARLEN           31

#define MWM_ITEM_NUMBER         32

extern int WinMainMetrics[];
#define GetMainWinMetrics(iItem)    (WinMainMetrics[iItem])

#define SYSBMP_MINIMIZE         0
#define SYSBMP_MAXIMIZE         1
#define SYSBMP_RESTORE          2
#define SYSBMP_CLOSE            3
#define SYSBMP_ARROWUP          4
#define SYSBMP_ARROWUPD         5
#define SYSBMP_ARROWDOWN        6
#define SYSBMP_ARROWDOWND       7
#define SYSBMP_ARROWLEFT        8
#define SYSBMP_ARROWLEFTD       9
#define SYSBMP_ARROWRIGHT       10
#define SYSBMP_ARROWRIGHTD      11

#define SYSBMP_ITEM_NUMBER      12
extern BITMAP SystemBitmap[];
#define GetSystemBitmap(iItem)      (SystemBitmap + iItem)

#define IDI_APPLICATION         0
#define IDI_HAND                1
#define IDI_STOP                IDI_HAND
#define IDI_QUESTION            2
#define IDI_EXCLAMATION         3
#define IDI_ASTERISK            4
#define IDI_INFORMATION         IDI_ASTERISK

#define SYSICO_ITEM_NUMBER      5
extern HICON LargeSystemIcon[];
#define GetLargeSystemIcon(iItem)   (LargeSystemIcon[iItem])

extern HICON SmallSystemIcon[];
#define GetSmallSystemIcon(iItem)   (SmallSystemIcon[iItem])

/********************** Non-Main Window Class Support ************************/
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002
#define CS_DBLCLKS          0x0008
#define CS_OWNDC            0x0020
#define CS_CLASSDC          0x0040
#define CS_PARENTDC         0x0080
#define CS_NOCLOSE          0x0200
#define CS_SAVEBITS         0x0800
#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000
#define CS_GLOBALCLASS      0x4000

#define CS_IME              0x00010000

#define COP_STYLE           0x0001
#define COP_HCURSOR         0x0002
#define COP_BKCOLOR         0x0004
#define COP_WINPROC         0x0008
#define COP_ADDDATA         0x0010

typedef struct _WNDCLASS
{
    char*   spClassName;

    DWORD   opMask;
    
    DWORD   dwStyle;

    HCURSOR hCursor;
    int     iBkColor;
    int (*WinProc)(HWND, int, WPARAM, LPARAM);
       				// the address of window procedure
    DWORD dwAddData;
                    // the additional data.
}WNDCLASS;
typedef WNDCLASS* PWNDCLASS;

#define MAINWINCLASSNAME    ("MAINWINDOW")

BOOL GUIAPI RegisterWindowClass (PWNDCLASS pWndClass);
BOOL GUIAPI UnregisterWindowClass (const char* szClassName);
char* GUIAPI GetClassName (HWND hWnd);
BOOL GUIAPI GetWindowClassInfo (PWNDCLASS pWndClass);
BOOL GUIAPI SetWindowClassInfo (const WNDCLASS* pWndClass);

/*************************** Non-Main Window support *************************/
HWND GUIAPI CreateWindow(const char* spClassName, const char* spCaption,
                  DWORD dwStyle, int id, 
                  int x, int y, int w, int h,
                  HWND hParentWnd, DWORD dwAddData);
BOOL GUIAPI DestroyWindow(HWND hWnd);

/**************************** IME support ************************************/
int GUIAPI RegisterIMEWindow (HWND hWnd);
int GUIAPI UnregisterIMEWindow (HWND hWnd);
int GUIAPI GetIMEStatus (int StatusCode);
int GUIAPI SetIMEStatus (int StatusCode, int Value);

/**************************** Caret support **********************************/
BOOL GUIAPI CreateCaret (HWND hWnd, PBITMAP pBitmap, int nWidth, int nHeight);
BOOL GUIAPI ChangeCaretSize (HWND hWnd, int newWidth, int newHeight);
BOOL GUIAPI ActiveCaret (HWND hWnd);
UINT GUIAPI GetCaretBlinkTime (HWND hWnd);
BOOL GUIAPI SetCaretBlinkTime (HWND hWnd, UINT uTime);
BOOL GUIAPI DestroyCaret (HWND hWnd);
BOOL GUIAPI HideCaret (HWND hWnd);
BOOL GUIAPI ShowCaret (HWND hWnd);
BOOL GUIAPI SetCaretPos (HWND hWnd, int x, int y);
BOOL GUIAPI GetCaretPos (HWND hWnd, PPOINT pPt);

/**************************** Control support ********************************/
/* Standard control IDs */
#define IDC_STATIC    0
#define IDOK          1
#define IDCANCEL      2
#define IDABORT       3
#define IDRETRY       4
#define IDIGNORE      5
#define IDYES         6
#define IDNO          7

#define MINID_RESERVED      0xF001
#define MAXID_RESERVED      0xFFFF

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // GUI_WINDOW_H

