
#include <Windows.h>
#include "Strings.h"


/* String-related stuff. */
#define same(a, b)       (!lstrcmp(a, b))
#define endswith(s, end) same((s) + lstrlen(s) - lstrlen(end), end)
#define within(x, a, b)  ((x) >= (a) && (x) <= (b))
#define between(x, a, b) ((x) > (a) && (x) < (b))

wchar_t* allocfmtv       (const wchar_t *fmt, va_list args);
wchar_t* allocfmt        (const wchar_t *fmt, ...);
void     appendfmt       (wchar_t **pStr, const wchar_t *fmt, ...);
wchar_t* trim            (wchar_t *s);
void     explodeMultiStr (const wchar_t *multiStr, Strings *pBuf);
void     explodeQuotedStr(const wchar_t *quotedStr, Strings *pBuf);


/* Win32 shorthand routines. */
#define hasCtrl()                               ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
#define isDir(path)                             ((GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0)
#define fileExists(path)                        (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
#define readIni(path, section, key, buf, bufsz) GetPrivateProfileString(section, key, L"", buf, bufsz, path)
#define enableMenu(hmenu, id, enable)           EnableMenuItem(hmenu, id, MF_BYCOMMAND | ((enable) ? MF_ENABLED : MF_GRAYED))
#define setEnable(hparent, id, enable)          EnableWindow(GetDlgItem(hparent, id), enable)
#define getEnable(hparent, id)                  IsWindowEnabled(GetDlgItem(hparent, id))
#define getCheck(hparent, id)                   (SendDlgItemMessage(hparent, id, BM_GETCHECK, 0, 0) == BST_CHECKED)

void     debugfmt      (const wchar_t *fmt, ...);
DWORD    exec          (const wchar_t *cmdLine);
wchar_t* exeDir        (wchar_t *buf, int bufsz);
void     setTextFmt    (HWND hWnd, int id, const wchar_t *fmt, ...);
void     setCheck      (HWND hParent, int id, BOOL check, BOOL emulateClick);
int      msgBoxFmt     (HWND hParent, UINT uType, const wchar_t *caption, const wchar_t *msg, ...);
int      runDialog     (HINSTANCE hInst, int cmdShow, int dialogId, int iconId, int accelTableId, DLGPROC dialogProc, LPARAM lp);
void     centerOnParent(HWND hDlg);
void     popMenu       (HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo);
HICON    explorerIcon  (const wchar_t *fileExtension);
BOOL     openFile      (HWND hWnd, const wchar_t *filter, wchar_t *buf, int szBuf);
int      openFiles     (HWND hWnd, const wchar_t *filter, Strings *pBuf);


/* Global system font, handled by runDialog(). */
extern HFONT g_hSysFont;
