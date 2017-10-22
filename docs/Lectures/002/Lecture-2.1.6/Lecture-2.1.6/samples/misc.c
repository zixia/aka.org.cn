//
// misc.c
// This file include some miscelleous functions. 
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

// Create date: 1998.12.31
//
// Last modified date: 1999.3.15.
//
// Modify records:
//
//  Who             When        Where       For What                Status
//-----------------------------------------------------------------------------
//
// TODO:
// 

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/io.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <vga.h>
#include <vgagl.h>

#include "common.h"
#include "cliprect.h"
#include "gdi.h"
#include "misc.h"
#include "text.h"

#ifndef lint
static char fileid[] = "$Id: misc.c,v 1.2 2000/04/20 03:18:24 weiym Exp $";
#endif

void GUIAPI Ping(void)
{
    putchar('\a');
    fflush(stdout);
}

#define KEYBD           0x60    /* I/O port for keyboard data */
#define COUNTER_ADDR    0x61
#define DEF_BEEP_DELAYUS    500000
static int beepCount;

BOOL InitIOPerm ()
{
    char szValue[11];
    int us;

    // Get the keyboard port access permission.
    if (ioperm (KEYBD, 5, 1))
    {
        fprintf (stderr, "Can not get keyboard I/O permission!\n");
        return FALSE;
    }

    if (ioperm (COUNTER_ADDR, 1, TRUE)) {
        fprintf (stderr, "Can not get beep I/O permission!\n");
        return FALSE;
    }
    
    beepCount = DEF_BEEP_DELAYUS;
    if( GetValueFromEtcFile(ETCFILEPATH, "beep",
                            "delaytime", szValue, 10) < 0 )
        return TRUE;

    us = atoi(szValue);
    if (us >  0)
        beepCount = us;

    return TRUE;
}

void TerminateIOPerm ()
{
    ioperm (COUNTER_ADDR, 1, FALSE);
    ioperm (KEYBD, 5, FALSE);
}

void GUIAPI Beep (void)
{
    outb (inb (COUNTER_ADDR) | 3, COUNTER_ADDR);
    usleep (beepCount);
    outb (inb (COUNTER_ADDR) & 0xFC, COUNTER_ADDR);
}

// This function locate the specified section in the etc file.
static int etc_LocateSection(FILE* fp, const char* pSection, FILE* bak_fp)
{
    char szBuff[ETC_MAXLINE + 1];
    char* current;
    char* tail;

    while (TRUE) {
        if (!fgets(szBuff, ETC_MAXLINE, fp))
            return ETC_FILEIOFAILED;
        else if (bak_fp && fputs (szBuff, bak_fp) == EOF)
            return ETC_FILEIOFAILED;
        
        current = szBuff;

        while (*current == ' ' ||  *current == '\t') current++; 

        if (*current == ';' || *current == '#') continue;

        if (*current++ == '[') 
            while (*current == ' ' ||  *current == '\t') current ++;
        else
            continue;

        // Locate the tail.
        tail = current;
        while (*tail != ']' && *tail != '\n' &&
              *tail != ';' && *tail != '#' && *tail != '\0')
              tail++;
        *tail = '\0';
        while (*tail == ' ' || *tail == '\t') {
            *tail = '\0';
            tail--; 
        }
            
        if (strcmp (current, pSection) == 0)
            return ETC_OK; 
    }

    return ETC_SECTIONNOTFOUND;
}

// This function locate the specified key in the etc file.
static int etc_LocateKeyValue(FILE* fp, const char* pKey, 
                               BOOL bCurSection, char* pValue, int iLen,
                               FILE* bak_fp)
{
    char szBuff[ETC_MAXLINE + 1 + 1];
    char* current;
    char* tail;
    char* value;

    while (TRUE) {
        if (!fgets(szBuff, ETC_MAXLINE, fp))
            return ETC_FILEIOFAILED;
        if (szBuff [strlen (szBuff) - 1] == '\n')
            szBuff [strlen (szBuff) - 1] = '\0';
        current = szBuff;

        while (*current == ' ' ||  *current == '\t') current++; 

        if (*current == ';' || *current == '#') continue;

        if (*current == '[') {
            if (bCurSection) {
                if (bak_fp && fputs (szBuff, bak_fp) == EOF)
                    return ETC_FILEIOFAILED;
                return ETC_KEYNOTFOUND; 
            }
            else
                continue;
        }

        tail = current;
        while (*tail != '=' && *tail != '\n' &&
              *tail != ';' && *tail != '#' && *tail != '\0')
              tail++;

        value = tail + 1;
        if (*tail != '=')
            *value = '\0'; 

        *tail-- = '\0';
        while (*tail == ' ' || *tail == '\t') {
            *tail = '\0';
            tail--; 
        }
            
        if (strcmp (current, pKey) == 0) {
            tail = value;
            while (*tail != '\n' && *tail != '\0') tail++;
            *tail = '\0'; 

            if (pValue)
                strncpy (pValue, value, iLen);

            return ETC_OK; 
        }
        else if (bak_fp && *current != '\0')
            fprintf (bak_fp, "%s=%s\n", current, value);
    }

    return ETC_KEYNOTFOUND;
}

/* Function: GetValueFromEtcFile(const char* pEtcFile, const char* pSection,
 *                               const char* pKey, char* pValue, int iLen);
 * Parameter:
 *     pEtcFile: etc file path name.
 *     pSection: Section name.
 *     pKey:     Key name.
 *     pValue:   The buffer will store the value of the key.
 *     iLen:     The max length of value string.
 * Return:
 *     int               meaning
 *     ETC_FILENOTFOUND           The etc file not found. 
 *     ETC_SECTIONNOTFOUND        The section is not found. 
 *     ETC_EKYNOTFOUND        The Key is not found.
 *     ETC_OK            OK.
 */
int GUIAPI GetValueFromEtcFile(const char* pEtcFile, const char* pSection,
                               const char* pKey, char* pValue, int iLen)
{
    FILE* fp;

    if (!(fp = fopen(pEtcFile, "r")))
         return ETC_FILENOTFOUND;

    if (pSection)
         if (etc_LocateSection (fp, pSection, NULL) != ETC_OK) {
             fclose (fp);
             return ETC_SECTIONNOTFOUND;
         }

    if (etc_LocateKeyValue (fp, pKey, pSection != NULL, 
                pValue, iLen, NULL) != ETC_OK) {
         fclose (fp);
         return ETC_KEYNOTFOUND;
    }

    fclose (fp);
    return ETC_OK;
}

static int etc_CopyAndLocate (FILE* etc_fp, FILE* tmp_fp, 
                const char* pSection, const char* pKey)
{
    if (pSection && etc_LocateSection (etc_fp, pSection, tmp_fp) != ETC_OK)
        return ETC_SECTIONNOTFOUND;

    if (etc_LocateKeyValue (etc_fp, pKey, pSection != NULL, 
                NULL, 0, tmp_fp) != ETC_OK)
        return ETC_KEYNOTFOUND;

    return ETC_OK;
}

static int etc_FileCopy (FILE* sf, FILE* df)
{
    char line [ETC_MAXLINE + 1];
    
    while (fgets (line, ETC_MAXLINE + 1, sf) != NULL)
        if (fputs (line, df) == EOF) {
            return ETC_FILEIOFAILED;
        }

    return ETC_OK;
}

/* Function: SetValueToEtcFile(const char* pEtcFile, const char* pSection,
 *                               const char* pKey, char* pValue);
 * Parameter:
 *     pEtcFile: etc file path name.
 *     pSection: Section name.
 *     pKey:     Key name.
 *     pValue:   Value.
 * Return:
 *     int                      meaning
 *     ETC_FILENOTFOUND         The etc file not found.
 *     ETC_TMPFILEFAILED        Create tmp file failure.
 *     ETC_OK                   OK.
 */
int GUIAPI SetValueToEtcFile (const char* pEtcFile, const char* pSection,
                               const char* pKey, char* pValue)
{
    FILE* etc_fp;
    FILE* tmp_fp;
    int rc;

    if ((tmp_fp = tmpfile ()) == NULL)
        return ETC_TMPFILEFAILED;

    if (!(etc_fp = fopen (pEtcFile, "r+"))) {
        fclose (tmp_fp);
        if (!(etc_fp = fopen (pEtcFile, "w")))
            return ETC_FILEIOFAILED;
        fprintf (etc_fp, "[%s]\n", pSection);
        fprintf (etc_fp, "%s=%s\n", pKey, pValue);
        fclose (etc_fp);
        return ETC_OK;
    }

    switch (etc_CopyAndLocate (etc_fp, tmp_fp, pSection, pKey)) {
    case ETC_SECTIONNOTFOUND:
        fprintf (tmp_fp, "\n[%s]\n", pSection);
        fprintf (tmp_fp, "%s=%s\n", pKey, pValue);
        break;
    
    default:
        fprintf (tmp_fp, "%s=%s\n", pKey, pValue);
        break;
    }

    if ((rc = etc_FileCopy (etc_fp, tmp_fp)) != ETC_OK)
        goto error;
    
    // replace etc content with tmp file content
    // truncate etc content first
    fclose (etc_fp);
    if (!(etc_fp = fopen (pEtcFile, "w")))
        return ETC_FILEIOFAILED;
    
    rewind (tmp_fp);
    rc = etc_FileCopy (tmp_fp, etc_fp);

error:
    fclose (etc_fp);
    fclose (tmp_fp);
    return rc;
}

void GUIAPI Draw3DUpFrame(HDC hDC, int l, int t, int r, int b, int fillc)
{
     r--;
     b--;

     SetPenColor(hDC, COLOR_lightgray);
     Rectangle(hDC, l, t, r, b);
     SetPenColor(hDC, COLOR_darkgray);
     Rectangle(hDC, l, t, r - 1, b - 1);
     SetPenColor(hDC, COLOR_black);
     MoveTo(hDC, l, b);
     LineTo(hDC, r, b);
     LineTo(hDC, r, t);
     SetPenColor(hDC, COLOR_lightwhite);
     MoveTo(hDC, l + 1, b - 1);
     LineTo(hDC, l + 1, t + 1);
     LineTo(hDC, r - 1, t + 1); 

     if( fillc > 0)
     {
         SetBrushColor(hDC, fillc);
         FillBox(hDC, l + 2, t + 2, r - l - 3, b - t - 3);
     }
}

void GUIAPI Draw3DDownFrame(HDC hDC, int l, int t, int r, int b, int fillc)
{
     SetPenColor(hDC, COLOR_black);
     MoveTo(hDC, l, b);
     LineTo(hDC, l, t);
     LineTo(hDC, r, t);
     SetPenColor(hDC, COLOR_darkgray);
     MoveTo(hDC, l + 1, b - 1);
     LineTo(hDC, l + 1, t + 1);
     LineTo(hDC, r - 1, t + 1);
     SetPenColor(hDC, COLOR_darkgray);
     MoveTo(hDC, l + 1, b - 1);
     LineTo(hDC, r - 1, b - 1);
     LineTo(hDC, r - 1, t + 1);
     SetPenColor(hDC, COLOR_lightwhite);
     MoveTo(hDC, l, b);
     LineTo(hDC, r, b);
     LineTo(hDC, r, t);

     if( fillc > 0)
     {
         SetBrushColor(hDC, fillc);
         FillBox(hDC, l + 2, t + 2, r - l - 3, b - t - 3);
     }
}

void GUIAPI Draw3DUpThickFrame(HDC hDC, int l, int t, int r, int b, int fillc)
{
     SetPenColor(hDC, COLOR_darkgray);
     MoveTo(hDC, l, b);
     LineTo(hDC, r, b);
     LineTo(hDC, r, t);
     SetPenColor(hDC, COLOR_lightwhite);
     MoveTo(hDC, l, b);
     LineTo(hDC, l, t);
     LineTo(hDC, r, t); 

     if( fillc > 0)
     {
         SetBrushColor(hDC, fillc);
         FillBox(hDC, l + 1, t + 1, r - l - 2, b - t - 2);
     }
}

void GUIAPI Draw3DDownThickFrame(HDC hDC, int l, int t, int r, int b, int fillc)
{
     SetPenColor(hDC, COLOR_lightwhite);
     MoveTo(hDC, l, b);
     LineTo(hDC, r, b);
     LineTo(hDC, r, t);
     SetPenColor(hDC, COLOR_darkgray);
     MoveTo(hDC, l, b);
     LineTo(hDC, l, t);
     LineTo(hDC, r, t);

     if( fillc > 0)
     {
         SetBrushColor(hDC, fillc);
         FillBox(hDC, l + 1, t + 1, r - l - 2, b - t - 2);
     }
}

void GUIAPI Draw3DBorder (HDC hdc, int l, int t, int r, int b)
{
    SetPenColor (hdc, COLOR_lightwhite);
    Rectangle (hdc, l + 1, t + 1, r - 1, b - 1);

    SetPenColor (hdc, COLOR_darkgray);
    Rectangle (hdc, l, t, r - 2, b - 2);
}

void GUIAPI DisabledTextOut (HDC hDC, int x, int y, const char* szText)
{
    SetBkMode (hDC, BM_TRANSPARENT);
    SetTextColor (hDC, COLOR_lightwhite);
    TextOut (hDC, x + 1, y + 1, szText);
    SetTextColor (hDC, COLOR_darkgray);
    TextOut (hDC, x, y, szText);
}

/****************************** System resource support *********************/
BOOL GUIAPI LoadSystemBitmap (PBITMAP pBitmap, const char* szItemName)
{
    char szPathName[MAX_PATH + 1];
    char szFileName[MAX_PATH + 1];
    char szValue[MAX_FILENAME + 1];
    
    if( GetValueFromEtcFile(ETCFILEPATH, "bitmapinfo", "bitmappath",
            szPathName, MAX_PATH) < 0 ) {
        fprintf (stderr, "Get bitmap path error!\n");
        return FALSE;
    }
    if( GetValueFromEtcFile(ETCFILEPATH, "bitmapinfo", szItemName,
            szValue, MAX_FILENAME) < 0 ) {
        fprintf (stderr, "Get bitmap file name error!\n");
        return FALSE;
    }
    
    strcpy(szFileName, szPathName);
    strcat(szFileName, szValue);
    
    if (LoadBitmap (pBitmap, szFileName) < 0) {
        fprintf (stderr, "Load bitmap error: %s!\n", szFileName);
        return FALSE;
    }
    
    return TRUE;
}

HICON GUIAPI LoadSystemIcon (const char* szItemName, int which)
{
    char szPathName[MAX_PATH + 1];
    char szFileName[MAX_PATH + 1];
    char szValue[MAX_FILENAME + 1];
    HICON hIcon;
    
    if( GetValueFromEtcFile(ETCFILEPATH, "iconinfo", "iconpath",
            szPathName, MAX_PATH) < 0 ) {
        fprintf (stderr, "Get icon path error!\n");
        return 0;
    }
    if( GetValueFromEtcFile(ETCFILEPATH, "iconinfo", szItemName,
            szValue, MAX_FILENAME) < 0 ) {
        fprintf (stderr, "Get icon file name error!\n");
        return 0;
    }
    
    strcpy(szFileName, szPathName);
    strcat(szFileName, szValue);
    
    if ((hIcon = LoadIconFromFile (szFileName, which)) == 0) {
        fprintf (stderr, "Load icon error: %s!\n", szFileName);
        return 0;
    }
    
    return hIcon;
}

