/**
* Program entry-point.
*/

#include <crtdbg.h>
#include <Windows.h>
#include <CommCtrl.h>
#include "MainEvents.h"
#include "common/ListView.h"
#include "common/util.h"
#include "../res/resource.h"


static INT_PTR CALLBACK Main_dialogProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wp))
		{
		case IDCANCEL: SendMessage(hDlg, WM_CLOSE, 0, 0); return TRUE; // close on ESC
		case RAD_MP3:
		case RAD_FLAC:
		case RAD_WAV:  Main_onSelectFormat(); return TRUE;
		case RAD_CBR:
		case RAD_VBR:  Main_onSelectRate(); return TRUE;
		case BTN_RUN:  Main_onRun(); return TRUE;
		case MNU_ABOUT:
			msgBoxFmt(hDlg, MB_ICONINFORMATION, L"About",
				L"FLAC and LAME frontend v1.0.8\nRodrigo César de Freitas Dias\n"
				L"\x72\x63\x65\x73\x61\x72\x40\x67\x6D\x61\x69\x6C\x2E\x63\x6F\x6D\n\n"
				L"Usage:\nDrag MP3, FLAC or WAV files into the window,\n"
				L"then choose the conversion to perform.");
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		switch(((NMHDR*)lp)->idFrom)
		{
		case LST_FILES:
			switch(((NMHDR*)lp)->code)
			{
			case LVN_INSERTITEM:
			case LVN_DELETEALLITEMS: Main_onListChanged(ListView_count(GetDlgItem(hDlg, LST_FILES))); return TRUE;
			case LVN_DELETEITEM:     Main_onListChanged(ListView_count(GetDlgItem(hDlg, LST_FILES)) - 1); return TRUE;
			case LVN_KEYDOWN:
				switch(((NMLVKEYDOWN*)lp)->wVKey)
				{
				case 'A':       if(hasCtrl()) ListView_selAllItems(GetDlgItem(hDlg, LST_FILES)); return TRUE; // Ctrl+A
				case VK_DELETE: ListView_delSelItems(GetDlgItem(hDlg, LST_FILES)); return TRUE; // Del
				}
				break;
			}
			break;
		}
		break;

	case WM_INITDIALOG: Main_onInitDialog(hDlg); return TRUE;
	case WM_SIZE:       Main_onSize(wp, lp); return TRUE;
	case WM_DROPFILES:  Main_onDropFiles(wp); return TRUE;
	case WM_CLOSE:      Main_onClose(); return TRUE;
	case WM_DESTROY:    PostQuitMessage(0); return TRUE;
	}
	return FALSE;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE h0, LPWSTR cmdLine, int cmdShow)
{
	int ret = runDialog(hInst, cmdShow, DLG_MAIN, ICO_DOG, ACC_MAIN, Main_dialogProc, 0);
	_ASSERT(!_CrtDumpMemoryLeaks());
	return ret;
}
