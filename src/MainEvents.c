
#include "MainEvents.h"
#include "Converter.h"
#include "common/Combo.h"
#include "common/Font.h"
#include "common/Glob.h"
#include "common/ListView.h"
#include "common/Resizer.h"
#include "common/util.h"
#include "../res/resource.h"

static HWND hDlg = 0, hList = 0;
static Resizer resizer = { 0 };


static void _Main_fileToList(const wchar_t *file)
{
	int iType = -1;
	if(endswith(file, L".mp3"))       iType = 0;
	else if(endswith(file, L".flac")) iType = 1;
	else if(endswith(file, L".wav"))  iType = 2; // what type of audio file is this?

	if(iType == -1)
		return; // bypass file if unaccepted format

	if(!ListView_itemExists(hList, file))
		ListView_addItem(hList, file, iType); // add only if not present yet
}

static void _Main_convertToMp3()
{
	BOOL    deleteSrc = getCheck(hDlg, CHK_DELSRC); // delete source file?
	BOOL    isVbr = getCheck(hDlg, RAD_VBR);       // CBR or VBR
	int     cmbId = isVbr ? CMB_VBR : CMB_CBR;    // combo with setting of quality
	wchar_t quality[32];
	int     i, numFiles = ListView_count(hList);

	GetDlgItemText(hDlg, cmbId, quality, ARRAYSIZE(quality));
	*wcschr(quality, L' ') = L'\0'; // first characters of chosen option are the quality setting itself

	// Check for non-WAV or non-FLAC file.	
	for(i = 0; i < numFiles; ++i) {
		wchar_t file[MAX_PATH];
		ListView_getText(hList, i, 0, file, ARRAYSIZE(file));

		if(!endswith(file, L".wav") && !endswith(file, L".flac")) {
			msgBoxFmt(hDlg, MB_ICONEXCLAMATION, L"Invalid file type",
				L"Only FLAC or WAV files can be converted to MP3.\nFailed: \"%s\"", file);
			return; // abort
		}
	}
	
	// Process the files.
	for(i = 0 ; i < numFiles; ++i) {
		wchar_t file[MAX_PATH];
		ListView_getText(hList, i, 0, file, ARRAYSIZE(file));
		Converter_ToMp3(file, deleteSrc, quality, isVbr);
	}
}

static void _Main_convertToFlac()
{
	BOOL    deleteSrc = getCheck(hDlg, CHK_DELSRC); // delete source file?
	wchar_t quality[32];
	int     i, numFiles = ListView_count(hList);

	GetDlgItemText(hDlg, CMB_FLAC, quality, ARRAYSIZE(quality)); // text is quality setting itself

	// Check for non-WAV or non-FLAC file.
	for(i = 0; i < numFiles; ++i) {
		wchar_t file[MAX_PATH];
		ListView_getText(hList, i, 0, file, ARRAYSIZE(file));

		if(!endswith(file, L".wav") && !endswith(file, L".flac")) {
			msgBoxFmt(hDlg, MB_ICONEXCLAMATION, L"Invalid file type",
				L"Only FLAC or WAV files can be converted to FLAC.\nFailed: \"%s\"", file);
			return; // abort
		}
	}

	// Process the files.
	for(i = 0; i < numFiles; ++i) {
		wchar_t file[MAX_PATH];
		ListView_getText(hList, i, 0, file, ARRAYSIZE(file));
		Converter_ToFlac(file, deleteSrc, quality);
	}
}

static void _Main_convertToWav()
{
	BOOL deleteSrc = getCheck(hDlg, CHK_DELSRC); // delete source file?
	int  i, numFiles = ListView_count(hList);

	// Check for non-FLAC file.
	for(i = 0; i < numFiles; ++i) {
		wchar_t file[MAX_PATH];
		ListView_getText(hList, i, 0, file, ARRAYSIZE(file));

		if(!endswith(file, L".flac")) {
			msgBoxFmt(hDlg, MB_ICONEXCLAMATION, L"Invalid file type",
				L"Only FLAC files can be converted to WAV.\nFailed: \"%s\"", file);
			return; // abort
		}
	}

	// Process the files.
	for(i = 0; i < numFiles; ++i) {
		wchar_t file[MAX_PATH];
		ListView_getText(hList, i, 0, file, ARRAYSIZE(file));
		Converter_ToWav(file, deleteSrc);
	}
}

static BOOL _Main_checkTools(wchar_t **pErr)
{
	wchar_t path[MAX_PATH];

	// Search for INI file.
	exeDir(path, ARRAYSIZE(path));
	lstrcat(path, L"FlacLameGui.ini");
	if(!fileExists(path)) {
		if(pErr) *pErr = allocfmt(L"Could not find INI file at:\n%s", path);
		return FALSE;
	}

	// Search for FLAC and LAME tools.
	Converter_GetLamePath(path, ARRAYSIZE(path));
	if(!fileExists(path)) {
		if(pErr) *pErr = allocfmt(L"Could not find LAME tool at:\n%s", path);
		return FALSE;
	}

	Converter_GetFlacPath(path, ARRAYSIZE(path));
	if(!fileExists(path)) {
		if(pErr) *pErr = allocfmt(L"Could not find FLAC tool at:\n%s", path);
		return FALSE;
	}

	return TRUE; // all good
}

void Main_onInitDialog(HWND hDialog)
{
	hDlg = hDialog;
	hList = GetDlgItem(hDlg, LST_FILES);
	Font_applyOnChildren(g_hSysFont, hDlg); // apply global system font

	// Validation of tools.
	{
		wchar_t *pErr = NULL;
		if(!_Main_checkTools(&pErr)) {
			msgBoxFmt(hDlg, MB_ICONERROR, L"Fail", pErr);
			free(pErr);
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return;
		}
	}

	// ListView initialization.
	ListView_fullRowSel(hList);
	ListView_addColumn(hList, L"File", 300);
	ListView_fitColumn(hList, 0);
	ListView_pushSysIcon(hList, L"mp3"); // system icons of the 3 filetypes we use
	ListView_pushSysIcon(hList, L"flac");
	ListView_pushSysIcon(hList, L"wav");

	// Resizer initialization.
	resizer = Resizer_new(hDlg, 12);
	Resizer_addByHandle(&resizer, RESIZE, RESIZE, hList);
	Resizer_addManyById(&resizer, RENONE, REPOS, 9,
			RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL, CMB_CBR, CMB_VBR, CMB_FLAC);
	Resizer_addManyById(&resizer, REPOS, REPOS, 2,
			CHK_DELSRC, BTN_RUN);

	// Comboboxes initialization.
	Combo_addMany(hDlg, CMB_CBR, 14,
		L"32 kbps", L"40 kbps", L"48 kbps", L"56 kbps", L"64 kbps", L"80 kbps", L"96 kbps", L"112 kbps",
		L"128 kbps; default", L"160 kbps", L"192 kbps", L"224 kbps", L"256 kbps", L"320 kbps");
	Combo_setSel(hDlg, CMB_CBR, 8);

	Combo_addMany(hDlg, CMB_VBR, 10,
		L"0 (~245 kbps)", L"1 (~225 kbps)", L"2 (~190 kbps)", L"3 (~175 kbps)", L"4 (~165 kbps); default",
		L"5 (~130 kbps)", L"6 (~115 kbps)", L"7 (~100 kbps)", L"8 (~85 kbps)", L"9 (~65 kbps)");
	Combo_setSel(hDlg, CMB_VBR, 4);

	Combo_addMany(hDlg, CMB_FLAC, 8, L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8");
	Combo_setSel(hDlg, CMB_FLAC, 7);

	// Radio buttons initialization.
	setCheck(hDlg, RAD_VBR, TRUE, TRUE);
	setCheck(hDlg, RAD_MP3, TRUE, TRUE);
}

void Main_onSize(WPARAM wp, LPARAM lp)
{
	Resizer_doIt(&resizer, wp, lp);
	ListView_fitColumn(hList, 0);
}

void Main_onClose()
{
	Resizer_free(&resizer);
	DestroyWindow(hDlg);
}

void Main_onDropFiles(WPARAM wp)
{
	HDROP hDrop = (HDROP)wp;
	int i, numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);

	SendMessage(hList, WM_SETREDRAW, (WPARAM)FALSE, 0);

	for(i = 0; i < numFiles; ++i) {
		wchar_t path[MAX_PATH];
		DragQueryFile(hDrop, i, path, ARRAYSIZE(path)); // retrieve filepath

		if(isDir(path)) { // if folder, add all files inside of it
			wchar_t subFileBuf[MAX_PATH];
			Glob globMp3 = Glob_new(path, L"*.mp3");
			Glob globFlac = Glob_new(path, L"*.flac");
			Glob globWav = Glob_new(path, L"*.wav");

			while(Glob_next(&globMp3, subFileBuf)) _Main_fileToList(subFileBuf);
			while(Glob_next(&globFlac, subFileBuf)) _Main_fileToList(subFileBuf);
			while(Glob_next(&globWav, subFileBuf)) _Main_fileToList(subFileBuf);

			Glob_free(&globMp3);
			Glob_free(&globFlac);
			Glob_free(&globWav);
		}
		else // add single file
			_Main_fileToList(path);
	}

	SendMessage(hList, WM_SETREDRAW, (WPARAM)TRUE, 0);
	Main_onListChanged(ListView_count(hList));
	DragFinish(hDrop);
}

void Main_onListChanged(int newCount)
{
	// Update counter on Run button.
	if(newCount) {
		setTextFmt(hDlg, BTN_RUN, L"&Run (%d)", newCount);
		setEnable(hDlg, BTN_RUN, TRUE);
	}
	else {
		SetDlgItemText(hDlg, BTN_RUN, L"&Run");
		setEnable(hDlg, BTN_RUN, FALSE);
	}
}

void Main_onSelectFormat()
{
	setEnable(hDlg, RAD_CBR, getCheck(hDlg, RAD_MP3));
	setEnable(hDlg, CMB_CBR, getCheck(hDlg, RAD_MP3) && getCheck(hDlg, RAD_CBR));

	setEnable(hDlg, RAD_VBR, getCheck(hDlg, RAD_MP3));
	setEnable(hDlg, CMB_VBR, getCheck(hDlg, RAD_MP3) && getCheck(hDlg, RAD_VBR));

	setEnable(hDlg, LBL_LEVEL, getCheck(hDlg, RAD_FLAC));
	setEnable(hDlg, CMB_FLAC, getCheck(hDlg, RAD_FLAC));
}

void Main_onSelectRate()
{
	setEnable(hDlg, CMB_CBR, getCheck(hDlg, RAD_CBR));
	setEnable(hDlg, CMB_VBR, getCheck(hDlg, RAD_VBR));
}

void Main_onRun()
{
	int i, num = ListView_count(hList);

	// Check the existence of each file added to list.
	for(i = 0; i < num; ++i) {
		wchar_t filebuf[MAX_PATH];
		ListView_getText(hList, i, 0, filebuf, ARRAYSIZE(filebuf));
		if(!fileExists(filebuf)) {
			msgBoxFmt(hDlg, MB_ICONERROR, L"Fail", L"File does not exist:\n%s", filebuf);
			return;
		}
	}

	// Proceed the conversion.
	if(getCheck(hDlg, RAD_MP3))       _Main_convertToMp3();
	else if(getCheck(hDlg, RAD_FLAC)) _Main_convertToFlac();
	else if(getCheck(hDlg, RAD_WAV))  _Main_convertToWav();
}
