/**
* Combobox shorthand routines and macros.
* The idea is, when dealing with a combo, use only these, and don't
* call directly the system routines/macros.
*/

#include <Windows.h>


#define Combo_add(hparent, id, str)  SendDlgItemMessage(hparent, id, CB_ADDSTRING, 0, (LPARAM)(str))
#define Combo_count(hparent, id)     SendDlgItemMessage(hparent, id, CB_GETCOUNT, 0, 0)
#define Combo_setSel(hparent, id, i) SendDlgItemMessage(hparent, id, CB_SETCURSEL, i, 0)

void Combo_addMany(HWND hParent, int id, int howMany, ...);
