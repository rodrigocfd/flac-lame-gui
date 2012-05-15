
#include "Converter.h"
#include "common/Thread.h"
#include "common/util.h"


typedef struct {
	wchar_t src[MAX_PATH];
	wchar_t quality[32];
	BOOL delSrc, isVbr;
} Converter_ThreadData;


void Converter_GetLamePath(wchar_t *buf, int bufsz)
{
	wchar_t ini[MAX_PATH];
	exeDir(ini, ARRAYSIZE(ini));
	lstrcat(ini, L"FlacLameGui.ini");
	readIni(ini, L"Tools", L"lame", buf, bufsz);
}

void Converter_GetFlacPath(wchar_t *buf, int bufsz)
{
	wchar_t ini[MAX_PATH];
	exeDir(ini, ARRAYSIZE(ini));
	lstrcat(ini, L"FlacLameGui.ini");
	readIni(ini, L"Tools", L"flac", buf, bufsz);
}

static void _Converter_ToWavRun(void *pArg)
{
	Converter_ThreadData *ctd = (Converter_ThreadData*)pArg;
	wchar_t  flacpath[MAX_PATH];
	wchar_t *cmdLine = NULL;

	if(!endswith(ctd->src, L".flac")) {
		debugfmt(L"Invalid file extension: %s.", wcsrchr(ctd->src, L'.'));
		goto bye;
	}

	Converter_GetFlacPath(flacpath, ARRAYSIZE(flacpath)); // retrieve FLAC tool path
	cmdLine = allocfmt(L"\"%s\" -d \"%s\"", flacpath, ctd->src); // will be consumed in thread

#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	debugfmt(L"Run %s\n", cmdLine);
	if(ctd->delSrc) debugfmt(L"Del %s\n", ctd->src);
#endif

	exec(cmdLine); // execute tool
	free(cmdLine);
	if(ctd->delSrc) DeleteFile(ctd->src); // delete source file

bye:
	free(ctd); // consume
}

static void _Converter_ToFlacRun(void *pArg)
{
	Converter_ThreadData *ctd = (Converter_ThreadData*)pArg;
	wchar_t  realSrc[MAX_PATH];
	BOOL     hasIntermediary;
	wchar_t *cmdLine = NULL;
	wchar_t  flacpath[MAX_PATH];

	if(!endswith(ctd->src, L".wav") && !endswith(ctd->src, L".flac")) {
		debugfmt(L"Invalid file extension: %s.", wcsrchr(ctd->src, L'.'));
		goto bye;
	}

	lstrcpy(realSrc, ctd->src);
	hasIntermediary = endswith(realSrc, L".flac"); // FLAC can be reencoded

	if(hasIntermediary) // FLAC is converted to WAV first
	{
		Converter_ThreadData *data = calloc(1, sizeof(Converter_ThreadData)); // will be consumed
		lstrcpy(data->src, realSrc);
		data->delSrc = FALSE;
		_Converter_ToWavRun(data); // not another thread, will block until returns

		{
			wchar_t newWav[MAX_PATH];
			lstrcpy(newWav, ctd->src);
			lstrcpy(wcsrchr(newWav, L'.'), L".wav"); // path of newly created WAV
			lstrcpy(wcsrchr(realSrc, L'.'), L"_tmp.wav"); // remove .FLAC, append suffix and .WAV
			MoveFile(newWav, realSrc); // rename new WAV to have suffix
		}
	}

	// Proceed with FLAC command line tool.
	Converter_GetFlacPath(flacpath, ARRAYSIZE(flacpath));
	cmdLine = allocfmt(L"\"%s\" -%s -V --no-seektable \"%s\"",
		flacpath, ctd->quality, realSrc); // will be consumed in thread

#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	debugfmt(L"Run %s\n", cmdLine);
	if(hasIntermediary) debugfmt(L"Del %s\n", realSrc);
	else if(ctd->delSrc) debugfmt(L"Del %s\n", ctd->src);
#endif

	exec(cmdLine); // execute tool
	free(cmdLine);

	if(hasIntermediary) {
		DeleteFile(realSrc); // delete intermediary WAV
		DeleteFile(ctd->src); // delete original FLAC

		lstrcpy(wcsrchr(realSrc, L'.'), L".flac"); // remove suffix, append .FLAC
		MoveFile(realSrc, ctd->src); // rename final file to same original name
	}
	else if(ctd->delSrc)
		DeleteFile(ctd->src); // delete source file

bye:
	free(ctd); // consume
}

static void _Converter_ToMp3Run(void *pArg)
{
	Converter_ThreadData *ctd = (Converter_ThreadData*)pArg;
	wchar_t  realSrc[MAX_PATH];
	BOOL     hasIntermediary;
	wchar_t *cmdLine = NULL;
	wchar_t  lamepath[MAX_PATH];

	if(!endswith(ctd->src, L".wav") && !endswith(ctd->src, L".flac")) {
		debugfmt(L"Invalid file extension: %s.", wcsrchr(ctd->src, L'.'));
		goto bye;
	}

	lstrcpy(realSrc, ctd->src);
	hasIntermediary = endswith(realSrc, L".flac");

	if(hasIntermediary) // FLAC is converted to WAV first
	{
		Converter_ThreadData *data = calloc(1, sizeof(Converter_ThreadData)); // will be consumed
		lstrcpy(data->src, realSrc);
		data->delSrc = FALSE;
		_Converter_ToWavRun(data); // not another thread, will block until returns

		lstrcpy(wcsrchr(realSrc, L'.'), L".wav"); // src will be the newly-converted WAV
	}

	// Proceed with LAME command line tool.
	Converter_GetLamePath(lamepath, ARRAYSIZE(lamepath));
	cmdLine = allocfmt(L"\"%s\" -%s%s --noreplaygain \"%s\" \"%s\"",
		lamepath, (ctd->isVbr ? L"V" : L"b"), ctd->quality, realSrc, realSrc); // will be consumed in thread
	lstrcpy(wcsrchr(cmdLine, L'.'), L".mp3\""); // MP3 extension instead of WAV on destination filepath

#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	debugfmt(L"Run %s\n", cmdLine);
	if(hasIntermediary) debugfmt(L"Del %s\n", realSrc);
	if(ctd->delSrc) debugfmt(L"Del %s\n", ctd->src);
#endif

	exec(cmdLine); // execute tool
	free(cmdLine);
	if(hasIntermediary) DeleteFile(realSrc); // delete intermediary WAV
	if(ctd->delSrc) DeleteFile(ctd->src); // delete source file

bye:
	free(ctd); // consume
}

BOOL Converter_ToWav(const wchar_t *src, BOOL delSrc)
{
	Converter_ThreadData *ctd = calloc(1, sizeof(Converter_ThreadData)); // will be consumed by thread
	lstrcpy(ctd->src, src);
	ctd->delSrc = delSrc;
	return Thread_RunAsync(_Converter_ToWavRun, ctd);
}

BOOL Converter_ToFlac(const wchar_t *src, BOOL delSrc, const wchar_t *quality)
{
	Converter_ThreadData *ctd = calloc(1, sizeof(Converter_ThreadData)); // will be consumed by thread
	lstrcpy(ctd->src, src);
	ctd->delSrc = delSrc;
	lstrcpy(ctd->quality, quality);
	return Thread_RunAsync(_Converter_ToFlacRun, ctd);
}

BOOL Converter_ToMp3(const wchar_t *src, BOOL delSrc, const wchar_t *quality, BOOL isVbr)
{
	Converter_ThreadData *ctd = malloc(sizeof(Converter_ThreadData)); // will be consumed by thread
	lstrcpy(ctd->src, src);
	ctd->delSrc = delSrc;
	lstrcpy(ctd->quality, quality);
	ctd->isVbr = isVbr;
	return Thread_RunAsync(_Converter_ToMp3Run, ctd);
}
