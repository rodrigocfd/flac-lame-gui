
#include "Combo.h"


void Combo_addMany(HWND hParent, int id, int howMany, ...)
{
	int     i;
	va_list marker;

	va_start(marker, howMany);
	for(i = 0; i < howMany; ++i) // user should pass pointer to each string to be added
		Combo_add(hParent, id, va_arg(marker, const wchar_t*));
	va_end(marker);
}
