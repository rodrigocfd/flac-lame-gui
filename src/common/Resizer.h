/**
* Takes care of positioning child windows when the parent
* is being resized, keeping the layout consistent.
*/

#include <Windows.h>

#define RENONE 0 // control doesn't move or resize
#define REPOS  1 // control size is fixed; control moves around anchored
#define RESIZE 2 // control size stretches; control doesn't move


typedef struct ResizerCtrl_ ResizerCtrl;

typedef struct {
	HWND hParent;
	int  lastInserted;
	SIZE szOrig;
	struct {
		int n;
		ResizerCtrl *ptr;
	} ctrls;
} Resizer;


Resizer Resizer_new            (HWND hParent, int numChildren);
void    Resizer_free           (Resizer *r);
void    Resizer_doIt           (Resizer *r, WPARAM wp, LPARAM lp);
void    Resizer_addByHandle    (Resizer *r, BYTE modeHorz, BYTE modeVert, HWND hCtrl);
void    Resizer_addById        (Resizer *r, BYTE modeHorz, BYTE modeVert, int id);
void    Resizer_addManyByHandle(Resizer *r, BYTE modeHorz, BYTE modeVert, int howMany, ...);
void    Resizer_addManyById    (Resizer *r, BYTE modeHorz, BYTE modeVert, int howMany, ...);
