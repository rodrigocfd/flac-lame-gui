
#include "Resizer.h"
#include "util.h"


Resizer Resizer_new(HWND hParent, int numChildren)
{
	Resizer obj = { 0 };
	obj.hParent = hParent;
	obj.ctrls.n = numChildren;
	obj.ctrls.ptr = malloc(sizeof(ResizerCtrl) * numChildren);
	obj.lastInserted = -1;
	return obj;
}

void Resizer_free(Resizer *r)
{
	if(r->ctrls.ptr)
		free(r->ctrls.ptr);
	SecureZeroMemory(r, sizeof(Resizer));
}

void Resizer_doIt(Resizer *r, WPARAM wp, LPARAM lp)
{
	// Call this within WM_SIZE processing.

	int  i, state = (int)wp;
	int  cx = LOWORD(lp), cy = HIWORD(lp);
	HDWP hdwp;

	if(!r->ctrls.ptr || state == SIZE_MINIMIZED)
		return; // if minimized, no need to resize

	hdwp = BeginDeferWindowPos(r->ctrls.n);
	for(i = 0; i < r->ctrls.n; ++i)
	{
		ResizerCtrl *pCtrl = &r->ctrls.ptr[i]; // current child control being worked with
		UINT         uFlags = SWP_NOZORDER;

		if(pCtrl->modeHorz == REPOS && pCtrl->modeVert == REPOS) // reposition both vert & horz
			uFlags |= SWP_NOSIZE;
		else if(pCtrl->modeHorz == RESIZE && pCtrl->modeVert == RESIZE) // resize both vert & horz
			uFlags |= SWP_NOMOVE;

		DeferWindowPos(hdwp, pCtrl->hWnd, 0,
			pCtrl->modeHorz == REPOS ?
				cx - r->szOrig.cx + pCtrl->rcOrig.left :
				pCtrl->rcOrig.left, // keep original pos
			pCtrl->modeVert == REPOS ?
				cy - r->szOrig.cy + pCtrl->rcOrig.top :
				pCtrl->rcOrig.top, // keep original pos
			pCtrl->modeHorz == RESIZE ?
				cx - r->szOrig.cx + pCtrl->rcOrig.right - pCtrl->rcOrig.left :
				pCtrl->rcOrig.right - pCtrl->rcOrig.left, // keep original width
			pCtrl->modeVert == RESIZE ?
				cy - r->szOrig.cy + pCtrl->rcOrig.bottom - pCtrl->rcOrig.top :
				pCtrl->rcOrig.bottom - pCtrl->rcOrig.top, // keep original height
			uFlags);
	}
	EndDeferWindowPos(hdwp);	
}

void Resizer_addByHandle(Resizer *r, BYTE modeHorz, BYTE modeVert, HWND hCtrl)
{
	ResizerCtrl *pCtrl;

	if(r->lastInserted >= r->ctrls.n - 1) { // protection against buffer overflow
		MessageBox(0, L"OH NO: Resizer_addByHandle() called with index beyond limit!", L"Fail", MB_ICONERROR); // shout unpolitely!
		return;
	}
	else if(r->lastInserted == -1) { // first control being added
		RECT rcP;
		GetClientRect(r->hParent, &rcP);
		r->szOrig.cx = rcP.right;
		r->szOrig.cy = rcP.bottom; // keep original size of parent
	}

	pCtrl = &r->ctrls.ptr[++r->lastInserted]; // current child control being added
	pCtrl->hWnd = hCtrl;
	pCtrl->modeHorz = modeHorz;
	pCtrl->modeVert = modeVert;

	GetWindowRect(pCtrl->hWnd, &pCtrl->rcOrig);
	ScreenToClient(r->hParent, (POINT*)&pCtrl->rcOrig);
	ScreenToClient(r->hParent, (POINT*)&pCtrl->rcOrig.right); // client coordinates relative to parent
}

void Resizer_addById(Resizer *r, BYTE modeHorz, BYTE modeVert, int id)
{
	Resizer_addByHandle(r, modeHorz, modeVert, GetDlgItem(r->hParent, id));
}

void Resizer_addManyByHandle(Resizer *r, BYTE modeHorz, BYTE modeVert, int howMany, ...)
{
	int     i;
	va_list marker;

	va_start(marker, howMany);
	for(i = 0; i < howMany; ++i) // user should pass HWND of each child to be added
		Resizer_addByHandle(r, modeHorz, modeVert, va_arg(marker, HWND));
	va_end(marker);
}

void Resizer_addManyById(Resizer *r, BYTE modeHorz, BYTE modeVert, int howMany, ...)
{
	int     i;
	va_list marker;

	va_start(marker, howMany);
	for(i = 0; i < howMany; ++i) // user should pass ID of each child to be added
		Resizer_addByHandle(r, modeHorz, modeVert, GetDlgItem(r->hParent, va_arg(marker, int)));
	va_end(marker);
}
