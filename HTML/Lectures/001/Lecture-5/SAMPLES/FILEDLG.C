//
// Written by You Huayun.
//   2000/2/26
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "../include/vacs.h"
#include "filedlg.h"
#include "filemanage.h"

#define IDC_DIRLST1         100
#define IDC_FILELST1        110
#define IDC_FILENAME        120
#define IDC_CLFILE1         130
#define IDC_NCFILE1         140


#define IDC_DIRLST2         200
#define IDC_FILELST2        210
#define IDC_FILETODEL       220
#define IDC_CLFILE2         230
#define IDC_NCFILE2         240
#define IDC_CHKDEL          250

#define IDC_DIRLST3         300
#define IDC_FILELST3        310
#define IDC_FILESRCE        320
#define IDC_FILEDEST        330
#define IDC_CLFILE3         340
#define IDC_NCFILE3         350
#define IDC_CHKOVR          360

#define IDC_DIRLST4         400
#define IDC_FILELST4        410
#define IDC_CLFILE4         420
#define IDC_NCFILE4         430

static void ListDir ( HWND hWnd, int dirListID, int fileListID, char* path)
{
    struct dirent* pDirEnt;
    DIR* dir;
    struct stat ftype;
    char fullpath [PATH_MAX + 1];

    dir = opendir (path);
    while ( (pDirEnt = readdir ( dir )) != NULL ) {

        // Assemble full path name.
        strcpy (fullpath, path);
        strcat (fullpath, "/");
        strcat (fullpath, pDirEnt->d_name);
        
        if (lstat (fullpath, &ftype) < 0 ){
           Ping();
           continue;
        }
        if (S_ISDIR (ftype.st_mode))
            SendDlgItemMessage( hWnd, dirListID, LB_ADDSTRING,
                             0, (LPARAM)pDirEnt->d_name);
        else if (S_ISREG(ftype.st_mode))   
            SendDlgItemMessage( hWnd, fileListID, LB_ADDSTRING,
                             0, (LPARAM)pDirEnt->d_name); 
    }
    closedir(dir);
}

static void ChangeDirList (HWND hWnd, int dirListID, int fileListID, 
                int editID, char* path)
{
    char msg [PATH_MAX + 30];
    
    SendDlgItemMessage (hWnd, dirListID, LB_RESETCONTENT, 0, (LPARAM)0);
    SendDlgItemMessage (hWnd, fileListID, LB_RESETCONTENT, 0, (LPARAM)0);
    if (editID)
        SetWindowText (GetDlgItem (hWnd, editID), "");

    sprintf (msg, "对不起，未找到指定的目录：\n\n%s\n", path);
    if( access( path, F_OK ) == -1){
         MessageBox( hWnd, msg, "提示信息", MB_OK | MB_ICONSTOP);
         return;
    }

    ListDir ( hWnd, dirListID, fileListID, path);
}

DLGTEMPLATE DlgFileOpenInfo = 
{
    WS_BORDER | WS_CAPTION, WS_EX_NONE,
    180, 80, 430, 315, "打开文件", 0, 0, 11, NULL
};

CTRLDATA CtrlFileOpenInfo [] = 
{
    { "static", WS_VISIBLE | SS_SIMPLE,
        10, 10, 40, 40, IDC_STATIC, "LOGO", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 10, 90, 24, IDC_STATIC, "目录", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        142, 10, 260, 24, IDC_STATIC, "文件", 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        50, 35, 90, 195, IDC_DIRLST1, NULL, 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        142, 35, 138, 195, IDC_FILELST1, NULL, 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 250, 60, 24, IDC_STATIC, "文件名:", 0 },
    { "edit", WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        110, 245, 170, 24, IDC_FILENAME, NULL, 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 35, 100, 26, IDC_CLFILE1, "刀位文件", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 70, 100, 26, IDC_NCFILE1, "数控文件", 0 },
    { "button", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
        305, 210, 100, 26, IDOK, "打开", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 245, 100, 26, IDCANCEL, "取消", 0 }
};

static int
DialogFileOpenProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
    {
        struct FileOpenInfo* fi = (struct FileOpenInfo*) lParam;
        
        ListDir (hDlg, IDC_DIRLST1, IDC_FILELST1, fi->dir);

        SetWindowText (GetDlgItem (hDlg, IDC_FILENAME), fi->filename);
        SetWindowAdditionalData2 (hDlg, (DWORD)lParam);
        return 1;
    }
        
    case MSG_COMMAND:
    {
        struct FileOpenInfo* fi = 
            (struct FileOpenInfo*) GetWindowAdditionalData2 (hDlg);
        int nSelect;
        int id = LOWORD (wParam);
        int code = HIWORD (wParam);
 
        switch (id) {
        case IDC_DIRLST1:
        {/*
            char dir[NAME_MAX + 1];
            
            if (code == LBN_DBLCLK){
                nSelect = 
                    SendDlgItemMessage(hDlg, IDC_DIRLST1, LB_GETCURSEL, 0, 0);
                if (nSelect != -1)
                    SendDlgItemMessage(hDlg, IDC_DIRLST1, LB_GETTEXT, 
                                nSelect, (LPARAM)dir);
                if (strcmp (fi->dir, ".") ==0 )
                    break;
                else ;    
                strcat (fi->dir, "/");
                strcat (fi->dir, dir);
                MessageBox (hDlg, fi->dir, "Error", MB_OK);
                //GetAbsolutePathName (dir, fi->dir);
                ChangeDirList (hDlg, IDC_DIRLST1, IDC_FILELST1, 
                                IDC_FILENAME, fi->dir);
                                
            }*/
            break;
        }
        
        case IDC_FILELST1:
        {
            if (code == LBN_SELCHANGE) {
                nSelect = 
                    SendDlgItemMessage(hDlg, IDC_FILELST1, LB_GETCURSEL, 0, 0);
                if(nSelect != -1)
                    SendDlgItemMessage(hDlg, IDC_FILELST1, LB_GETTEXT, 
                                nSelect, (LPARAM)fi->filename);
                SetWindowText(GetDlgItem(hDlg, IDC_FILENAME), fi->filename);
            }
            break;
        }
        
        case IDC_CLFILE1:
            if (code != 0) break;

            GetAbsolutePathName (DIR_DATA_CL, fi->dir);
            if( access( fi->dir, F_OK ) == -1){
               MessageBox( hDlg, "对不起，未找到刀位文件所在目录",
                            "提示信息", MB_OK | MB_ICONSTOP);
                break;
            }
            ChangeDirList (hDlg, IDC_DIRLST1, IDC_FILELST1, 
                            IDC_FILENAME, fi->dir);
            break;
        
        case IDC_NCFILE1:
            if (code != 0) break;

            GetAbsolutePathName (DIR_DATA_NC, fi->dir);
            if( access( fi->dir, F_OK ) == -1){
                MessageBox( hDlg, "对不起，未找到数控文件所在目录",
                            "提示信息", MB_OK | MB_ICONSTOP);
                break;
            }
            ChangeDirList (hDlg, IDC_DIRLST1, IDC_FILELST1, 
                            IDC_FILENAME, fi->dir);
            break;        
        
        case IDOK:
        {
            char msg[PATH_MAX + 16];

            if( code != 0) break;

            GetWindowText (GetDlgItem(hDlg, IDC_FILENAME), 
                           fi->filename, NAME_MAX);
            if (fi->filename[0] == '\0') {
                MessageBox (hDlg, 
                    "请注意文件名：\n\n输入的文件名中不要"
                    "夹杂空格，\n尤其头尾不要留有空格，\n文件名中不能使"
                    "用 / 字符．\n\n请输入正确的文件名．", 
                    "打开文件出错", 
                    MB_OK | MB_ICONINFORMATION);
                break;
            }
            
            strcpy(fi->fullname, fi->dir);
            strcat(fi->fullname, "/");
            strcat(fi->fullname, fi->filename);
            if (access (fi->fullname, F_OK) == -1) {
                if (!fi->IsEdit) {
                    sprintf (msg, "文件 %s 不存在！", fi->filename);
                    MessageBox (hDlg, msg, "打开文件出错", MB_OK | MB_ICONSTOP);
                    break;
                }
                else {
                    sprintf(msg, 
                            "文件：\n\n  %s\n\n 不存在，是否创建该文件？",
                            fi->filename);
                    if (MessageBox (hDlg, 
                                msg,
                                "确认信息", 
                                MB_YESNO | MB_ICONQUESTION) != IDYES)
                       break;
                }   
            }    
            EndDialog (hDlg, IDOK);
            break;
	    }

        case IDCANCEL:
            if( code != 0) break;

            EndDialog (hDlg, wParam);
            break;
        }
	
        break;
    }
    }
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int FileOpenDialogBox (HWND hWnd, struct FileOpenInfo* fi)
{
    DlgFileOpenInfo.controls = CtrlFileOpenInfo;

    if (DialogBoxIndirectParam (&DlgFileOpenInfo, hWnd, 
          DialogFileOpenProc, (LPARAM)fi) == IDOK)
        return VACS_OK;
    else
        return VACS_ERROR_GUI;
}


DLGTEMPLATE DlgFileDeleteInfo = 
{
    WS_BORDER | WS_CAPTION, WS_EX_NONE,
    180, 80, 430, 315, "删除文件", 0, 0, 12, NULL
};

CTRLDATA CtrlFileDeleteInfo [] = 
{
    { "static", WS_VISIBLE | SS_SIMPLE,
        10, 10, 40, 40, IDC_STATIC, "LOGO", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 10, 90, 24, IDC_STATIC, "目录", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        142, 10, 260, 24, IDC_STATIC, "文件", 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        50, 35, 90, 164, IDC_DIRLST2, NULL, 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        142, 35, 138, 164, IDC_FILELST2, NULL, 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 215, 60, 24, IDC_STATIC, "文件名:", 0 },
    { "edit", WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        110, 210, 170, 24, IDC_FILETODEL, NULL, 0 },
    { "button", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 
        50, 250, 200, 26, IDC_CHKDEL, "删除文件，不经确认", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 35, 100, 26, IDC_CLFILE2, "刀位文件", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 70, 100, 26, IDC_NCFILE2, "数控文件", 0 },
    { "button", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
        305, 210, 100, 26, IDOK, "删除", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 245, 100, 26, IDCANCEL, "取消", 0 }
};


static int
DialogFileDeleteProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
    {
        struct FileDeleteInfo* fi = (struct FileDeleteInfo*) lParam;
    
        ListDir (hDlg, IDC_DIRLST2, IDC_FILELST2, fi->dir);
        SetWindowAdditionalData2 (hDlg, (DWORD)lParam);
        return 1;
    }
        
    case MSG_COMMAND:
    {
    	struct FileDeleteInfo *fi = (struct FileDeleteInfo*)
				                    GetWindowAdditionalData2 (hDlg);
        int id = LOWORD( wParam );
        int code = HIWORD ( wParam );
        int nSelect;

        switch (id) {
        case IDC_FILELST2:
            if (code == LBN_SELCHANGE) {
                    nSelect = SendDlgItemMessage( hDlg, IDC_FILELST2,
                                                 LB_GETCURSEL, 0, 0);
                if (nSelect == -1)
                    break;
                SendDlgItemMessage(hDlg, IDC_FILELST2, LB_GETTEXT, 
                                  nSelect, (LPARAM)fi->filename);
                SetWindowText(GetDlgItem(hDlg, IDC_FILETODEL), 
                                  fi->filename);
            }
            break;
            
        case IDC_CLFILE2:
            if (code != 0) break;

            GetAbsolutePathName (DIR_DATA_CL, fi->dir);
            if (access( fi->dir, F_OK ) == -1) {
                MessageBox (hDlg, 
                        "对不起，未找到刀位文件所在目录",
                        "提示信息", 
                        MB_OK | MB_ICONSTOP);
                break;
            }
            ChangeDirList (hDlg, IDC_DIRLST2, IDC_FILELST2, 
                                    IDC_FILETODEL, fi->dir);
            break;
            
        case IDC_NCFILE2:
            if (code != 0) break;

            GetAbsolutePathName (DIR_DATA_NC, fi->dir);
            if (access (fi->dir, F_OK ) == -1) {
                MessageBox( hDlg, "对不起，未找到数控文件所在目录",
                        "提示信息", MB_OK | MB_ICONSTOP);
                break;
            }
            ChangeDirList (hDlg, IDC_DIRLST2, IDC_FILELST2, 
                                    IDC_FILETODEL, fi->dir);
            break;
            
        case IDOK:
        {
            char msg [ PATH_MAX + 16];

            if (code != 0) break;

            // get the file name
            GetWindowText (GetDlgItem(hDlg, IDC_FILETODEL), 
                                    fi->filename, NAME_MAX);

            // make sure the file being there
            if (fi->filename [0] == '\0' ){
                MessageBox( hDlg, 
                        "请注意文件名：\n\n输入的文件名中不要"
                        "夹杂空格，\n尤其头尾不要留有空格，\n文件名中不能使"
                        "用 / 字符．\n\n请输入正确的文件名．", 
                        "删除文件出错", 
                        MB_OK | MB_ICONINFORMATION);
                break;
            } 
            strcpy (fi->fullname, fi->dir);
            strcat (fi->fullname, "/" );
            strcat(fi->fullname, fi->filename);
            if (access (fi->fullname, F_OK) == -1) {
                 sprintf(msg, "文件 %s 不存在！", fi->filename);
                 MessageBox( hDlg, msg, "删除文件出错", MB_OK | MB_ICONSTOP);
                 break;
            }
                
            // Delete the file now
            sprintf(msg, "您真的要删除文件：\n\n   %s\n\n吗？", fi->filename);
            if (!IsDlgButtonChecked (hDlg, IDC_CHKDEL)) {
                if (MessageBox (hDlg, msg,"确认信息", 
                                    MB_OKCANCEL | MB_ICONQUESTION)  == IDOK)
                    DelFile(fi->fullname);
                else 
                    break;
            } else
                DelFile(fi->fullname);
                EndDialog (hDlg, IDOK);
                break;
	        }
            
        case IDCANCEL:
            if (code != 0) break;

            EndDialog (hDlg, wParam);
            break;
        }
        break;
    }
    }
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int FileDeleteDialogBox (HWND hWnd,struct FileDeleteInfo *fi)
{
    if (access (fi->dir, F_OK) == -1) 
        return VACS_ERROR_PATHNOTEXIST;
    
    DlgFileDeleteInfo.controls = CtrlFileDeleteInfo;
    if (DialogBoxIndirectParam (&DlgFileDeleteInfo, hWnd,  
          DialogFileDeleteProc, (LPARAM)fi) == IDOK)
        return VACS_OK;
    else
        return VACS_ERROR_GUI;
}

////////////////////////////////////
DLGTEMPLATE DlgFileCopyInfo = 
{
    WS_BORDER | WS_CAPTION, WS_EX_NONE,
    180, 80, 430, 315, "复制文件", 0, 0, 14, NULL
};

CTRLDATA CtrlFileCopyInfo [] = 
{
    { "static", WS_VISIBLE | SS_SIMPLE,
        10, 10, 40, 40, IDC_STATIC, "LOGO", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 10, 90, 24, IDC_STATIC, "目录", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        142, 10, 260, 24, IDC_STATIC, "文件", 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        50, 35, 90, 130, IDC_DIRLST3, NULL, 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        142, 35, 138, 130, IDC_FILELST3, NULL, 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 180, 130, 24, IDC_STATIC, "源  文  件:", 0 },
    { "edit", WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        140, 175, 140, 24, IDC_FILESRCE, NULL, 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 215, 130, 24, IDC_STATIC, "复制到文件:", 0 },
    { "edit", WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        140, 210, 140, 24, IDC_FILEDEST, NULL, 0 },
    { "button", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 
        50, 250, 200, 26, IDC_CHKOVR, "覆盖已有文件", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 35, 100, 26, IDC_CLFILE3, "刀位文件", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 70, 100, 26, IDC_NCFILE3, "数控文件", 0 },
    { "button", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
        305, 210, 100, 26, IDOK, "复制", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 245, 100, 26, IDCANCEL, "取消", 0 }
};


static int
DialogFileCopyProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
    {
        struct FileCopyInfo* fi = (struct FileCopyInfo*) lParam;
        
        fi->dfilename [0] = '\0';
        fi->sfilename [0] = '\0';

        ListDir (hDlg, IDC_DIRLST3, IDC_FILELST3, fi->dir);
        SetWindowAdditionalData2 (hDlg, (DWORD)lParam);
        return 1;
    }
        
    case MSG_COMMAND:
    {
    	struct FileCopyInfo *fi 
                = (struct FileCopyInfo*) GetWindowAdditionalData2 (hDlg);
        int nSelect;
        int id = LOWORD( wParam );
        int code = HIWORD ( wParam );

        switch ( id ) {
        case IDC_FILELST3:
            if (code == LBN_SELCHANGE) {
                nSelect = SendDlgItemMessage( hDlg, IDC_FILELST3, 
                                            LB_GETCURSEL, 0, 0);
                if (nSelect == -1)
                    break;
                SendDlgItemMessage (hDlg, IDC_FILELST3, LB_GETTEXT, 
                                nSelect, (LPARAM)fi->sfilename);
                SetWindowText ( GetDlgItem (hDlg, IDC_FILESRCE), fi->sfilename);
            }
            break;
            
        case IDC_CLFILE3:
            if (code != 0)
                break;

            GetAbsolutePathName (DIR_DATA_CL, fi->dir);
            if (access( fi->dir, F_OK ) == -1) {
                MessageBox (hDlg, "对不起，未找到刀位文件所在目录",
                        "提示信息", MB_OK | MB_ICONSTOP);
                break;
            }
            
            ChangeDirList (hDlg, IDC_DIRLST3, IDC_FILELST3, 
                                IDC_FILESRCE, fi->dir);
            break;
            
        case IDC_NCFILE3:
            if (code != 0)
                break;

            GetAbsolutePathName (DIR_DATA_NC, fi->dir);
            if (access( fi->dir, F_OK ) == -1) {
                MessageBox (hDlg, "对不起，未找到数控文件所在目录",
                        "提示信息", MB_OK | MB_ICONSTOP);
                break;
            }
            ChangeDirList (hDlg, IDC_DIRLST3, IDC_FILELST3, 
                            IDC_FILESRCE, fi->dir);
            break;
            
       case IDOK:
       {
            char msg [ PATH_MAX + 1];

            if (code != 0) break;
                
            GetWindowText (GetDlgItem(hDlg, IDC_FILESRCE), 
                                    fi->sfilename, NAME_MAX);
            if (fi->sfilename [0] == '\0') {
                MessageBox (hDlg, 
                    "请注意源文件名：\n\n输入的文件名中不要"
                    "夹杂空格，\n尤其头尾不要留有空格，\n文件名中不能使"
                    "用 / 字符．\n\n请输入正确的文件名．", 
                    "复制文件出错", 
                    MB_OK | MB_ICONINFORMATION);
                break;
            }
            
            strcpy (fi->sfullname, fi->dir);
            strcat (fi->sfullname, "/");
            strcat (fi->sfullname, fi->sfilename);
 
            if (access ( fi->sfullname, F_OK) == -1) {
                sprintf (msg, "文件 %s 不存在！", fi->sfilename);
                MessageBox (hDlg, msg, "复制文件出错", MB_OK | MB_ICONSTOP);
                break;
            }

            GetWindowText (GetDlgItem(hDlg, IDC_FILEDEST), fi->dfilename,
                     GetWindowTextLength(GetDlgItem(hDlg, IDC_FILEDEST)));
                     
            if (fi->dfilename[0] == '\0' ){
                MessageBox (hDlg, 
                        "请注意目标文件名：\n\n输入的文件名中不要"
                        "夹杂空格，\n尤其头尾不要留有空格，\n文件名中不能使用 /"
                        " 字符．\n\n请输入正确的目标文件名．", 
                        "复制文件出错", 
                        MB_OK | MB_ICONINFORMATION);
                break;
           }
           
           strcpy (fi->dfullname, fi->dir);
           strcat (fi->dfullname, "/");
           strcat (fi->dfullname, fi->dfilename);
           if (access (fi->dfullname, F_OK) != -1) {
           ///////// Copy the file now
               sprintf (msg, 
                    "目标文件：\n\n　　　　%s\n\n已经存在，是否覆盖原文件？", 
                    fi->dfilename);
               if (!IsDlgButtonChecked (hDlg, IDC_CHKOVR)) {
                    if (MessageBox (hDlg, msg, "确认信息", 
                                    MB_YESNO | MB_ICONQUESTION)  == IDYES)
                        CopyFile (fi->sfullname, fi->dfullname);
                    else 
                        break;
                }
            }
            else
                CopyFile (fi->sfullname, fi->dfullname);
                
            EndDialog (hDlg, IDOK);
            break;
	    }
        
        case IDCANCEL:
            if (code != 0)
                break;

            EndDialog (hDlg, wParam);
            break;
        }
	
        break;
    }
    }
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int FileCopyDialogBox ( HWND hWnd, struct FileCopyInfo *fi )
{
    if ( access (fi->dir, F_OK) == -1){ 
       return VACS_ERROR_PATHNOTEXIST;             
    }
  
    DlgFileCopyInfo.controls = CtrlFileCopyInfo;

    if (DialogBoxIndirectParam (&DlgFileCopyInfo, hWnd, 
          DialogFileCopyProc, (LPARAM)fi) == IDOK)
        return VACS_OK;
    else
        return VACS_ERROR_GUI;
}

/////////////////////////////////////////
DLGTEMPLATE DlgFileBrowseInfo = 
{
    WS_BORDER | WS_CAPTION, WS_EX_NONE,
    180, 80, 430, 295, "浏览文件", 0, 0, 8, NULL
};

CTRLDATA CtrlFileBrowseInfo [] = 
{
    { "static", WS_VISIBLE | SS_SIMPLE,
        10, 10, 40, 40, IDC_STATIC, "LOGO", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        50, 10, 90, 24, IDC_STATIC, "目录", 0 },
    { "static", WS_VISIBLE | SS_LEFT,
        142, 10, 260, 24, IDC_STATIC, "文件", 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        50, 35, 90, 195, IDC_DIRLST4, NULL, 0 },
    { "listbox", WS_VISIBLE | WS_TABSTOP | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        142, 35, 138, 195, IDC_FILELST4, NULL, 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 35, 100, 26, IDC_CLFILE4, "刀位文件", 0 },
    { "button", WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        305, 70, 100, 26, IDC_NCFILE4, "数控文件", 0 },
    { "button", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
        305, 210, 100, 26, IDOK, "返回", 0 }
};

static int
DialogFileBrowseProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_INITDIALOG:
    {
        struct FileBrowseInfo* fi = (struct FileBrowseInfo*) lParam;
        
        ListDir (hDlg, IDC_DIRLST4, IDC_FILELST4, fi->dir);
        
        SetWindowAdditionalData2 (hDlg, (DWORD)lParam);
        return 1;
    }
        
    case MSG_COMMAND:
    {
        struct FileBrowseInfo* fi = 
            (struct FileBrowseInfo*) GetWindowAdditionalData2 (hDlg);
 
        switch (wParam) {
        case IDC_CLFILE4:
            GetAbsolutePathName (DIR_DATA_CL, fi->dir);
            if (access( fi->dir, F_OK ) == -1) {
                MessageBox( hDlg, "对不起，未找到刀位文件所在目录",
                        "提示信息", MB_OK | MB_ICONSTOP);
                break;
            }
            ChangeDirList (hDlg, IDC_DIRLST4, IDC_FILELST4, 0, fi->dir);
            break;
            
        case IDC_NCFILE4:
             GetAbsolutePathName (DIR_DATA_NC, fi->dir);
             if( access( fi->dir, F_OK ) == -1){
               MessageBox( hDlg, "对不起，未找到数控文件所在目录",
                        "提示信息", MB_OK | MB_ICONSTOP);
                break;
             }
             ChangeDirList (hDlg, IDC_DIRLST4, IDC_FILELST4, 0, fi->dir);
             break;
             
        case IDOK:
        {
            EndDialog (hDlg, IDOK);
            break;
	    }

        }
        break;
    }
    }
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int FileBrowseDialogBox (HWND hWnd,struct FileBrowseInfo* fi )
{
    if ( access (fi->dir, F_OK) == -1) 
       return VACS_ERROR_PATHNOTEXIST;             

    DlgFileBrowseInfo.controls = CtrlFileBrowseInfo;
    DialogBoxIndirectParam (&DlgFileBrowseInfo, hWnd, 
                            DialogFileBrowseProc, (LPARAM)fi);
   return VACS_OK;                         
}

