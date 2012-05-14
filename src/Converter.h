/**
* Responsible for the LAME/FLAC conversion.
* Each conversion is made in a separated thread, and many can be made in parallel.
*/

#include <Windows.h>


void Converter_GetLamePath(wchar_t *buf, int bufsz);
void Converter_GetFlacPath(wchar_t *buf, int bufsz);
BOOL Converter_ToWav      (const wchar_t *src, BOOL delSrc);
BOOL Converter_ToFlac     (const wchar_t *src, BOOL delSrc, const wchar_t *quality);
BOOL Converter_ToMp3      (const wchar_t *src, BOOL delSrc, const wchar_t *quality, BOOL isVbr);
