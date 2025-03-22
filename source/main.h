#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include "3ds.h"
#include "graphics.h"
#include "drawableObject.h"
#include "mysvcs.h"
#include "csvc.h"
#include "curl/curl.h"


#define check_prim(result, err) if ((result) != 0) {g_primary_error = err; \
    goto error; }
#define check_sec(result, err) if ((result) != 0) {g_secondary_error = err; \
    goto error; }
#define check_third(result, err) if ((result) != 0) {g_third_error = err; \
    goto error; }

typedef uint32_t  u32;
typedef uint8_t   u8;
#define CURRENT_PROCESS_HANDLE  (0xffff8001)
#define RESULT_ERROR            (1)
#define TMPBUFFER_SIZE          (0x20000)
#define URL_MAX 1024

#define READREMOTEMEMORY_TIMEOUT    (char *)s_error[0]
#define OPENPROCESS_FAILURE         (char *)s_error[1]
#define PROTECTMEMORY_FAILURE       (char *)s_error[2]
#define REMOTECOPY_FAILURE          (char *)s_error[3]
#define CFGU_INIT_FAILURE           (char *)s_error[4]
#define CFGU_SECURE_FAILURE         (char *)s_error[5]
#define WRONGREGION                 (char *)s_error[6]
#define SMPATCH_FAILURE             (char *)s_error[7]
#define FSPATCH_FAILURE             (char *)s_error[8]
#define FILEOPEN_FAILURE            (char *)s_error[9]
#define NULLSIZE                    (char *)s_error[10]
#define LINEARMEMALIGN_FAILURE      (char *)s_error[11]
#define ACCESSPATCH_FAILURE         (char *)s_error[12]
#define UNKNOWN_FIRM                (char *)s_error[13]
#define UNKNOWN_HOMEMENU            (char *)s_error[14]
#define USER_ABORT                  (char *)s_error[15]
#define FILE_COPY_ERROR             (char *)s_error[16]
#define LOAD_FAILED                 (char *)s_error[17]
#define NTR_ALREADY_LAUNCHED        (char *)s_error[18]

#define DEFAULT_MOD_PATH "/MK7DX"
#define FILE_DOWN_PREFIX "https://github.com/MidyGamy/MK7DX-updates-unofficial/raw/master/updates/data"
#define FULLMOD_DOWNLOAD_URL_CIA "https://ctgp7.page.link/cia_install"
#define FULLMOD_DOWNLOAD_URL_3DSX  "https://ctgp7.page.link/3dsx_install"
// #define FULLMOD_DOWNLOAD_URL_CIA "https://MK7DX.page.link/cia_install"
// #define FULLMOD_DOWNLOAD_URL_3DSX  "https://MK7DX.page.link/3dsx_install"
#define FULLMOD_DOWNLOAD_FINAL_CIA "/MK7DX-tmp/MK7DX_Installer.cia"
#define FULLMOD_DOWNLOAD_FINAL_3DSX  "/MK7DX-tmp/MK7DX_Installer.3dsx"

#define TOINSTALL_CIA_PATH "/MK7DX/cia/tooInstall.cia"
#define TOINSTALL_3DSX_PATH "/MK7DX/cia/tooInstall.3dsx"
#define FINAL_CIA_PATH "/MK7DX/cia/MK7DX.cia"
#define TEMPORAL_3DSX_PATH "/MK7DX/cia/MK7DX.3dsx"
#define FINAL_3DSX_PATH "/3ds/MK7DX/MK7DX.3dsx"
#define FINAL_3DSX_INSTALLER_PATH "/3ds/MK7DX/MK7DX_Installer.3dsx"
#define LAUNCH_OPT_SAVE "/MK7DX/config/launchopt.bin"
#define PLGLDR_TMP "/MK7DX/tempboot.firm"
#define PLGLDR_URL "https://raw.githubusercontent.com/MidyGamy/MK7DX-updates-unofficial/master/luma/boot.firm"
#define PLGLDR_PATH "romfs:/boot.firm"
#define MK7DX_TID (0x000400000138F700ULL)
#define CIA_ALREADY_EXISTS 0xC8E083FC

typedef struct  updateData_s
{
    char            url[URL_MAX];
    u32             responseCode;
}               updateData_t;

typedef void(*funcType)();
typedef struct  s_BLOCK
{
    u32         buffer[4];
}               t_BLOCK;

typedef enum    version_e
{
    V32 = 0,
    V33 = 1,
    V36 = 2,
}               version_t;
/*
* main.c
*/
void getModVersion();
/*
** misc.s
*/
void    kFlushDataCache(void *, u32);
void    flushDataCache(void);
void    FlushAllCache(void);
void    InvalidateEntireInstructionCache(void);
void    InvalidateEntireDataCache(void);
s32     backdoorHandler();

/*
** mainMenu.c
*/
void	initMainMenuResources(void);
void    initMainMenu(void);
void    exitMainMenu(void);
int     mainMenu(void);
void    ntrDumpMode(void);
void    greyExit();

/*
** files.c
*/
int     copy_file(char *old_filename, char  *new_filename);
bool    fileExists(const char *path);
int     createDir(const char *path);
/*
** common_functions.c
*/
bool    abort_and_exit(void);
u32     getCurrentProcessHandle(void);
void    flushDataCache(void);
void    doFlushCache(void);
void    doStallCpu(void);
void    doWait(void);
void    strJoin(char *dst, const char *s1, const char *s2);
void    strInsert(char *dst, char *src, int index);
void    strncpyFromTail(char *dst, char *src, int nb);
bool    inputPathKeyboard(char *dst, char *hintText, char *initialText, int bufSize);
void    waitAllKeysReleased(void);
void    wait(int seconds);
void    debug(char *str, int seconds);
void customBreak(u32 r0, u32 r1, u32 r2, u32 r3);
/*
** ntr_launcher.c
*/
Result      bnPatchAccessCheck(void);
Result      bnLoadAndExecuteNTR(void);
Result      bnBootNTR(void);
void        launchNTRDumpMode(void);

/*
** pathPatcher.c
*/
Result        loadAndPatch(version_t version);

/*
** memory_functions.c
*/
u32     protectRemoteMemory(Handle hProcess, u32 addr, u32 size);
u32     copyRemoteMemory(Handle hDst, u32 ptrDst, Handle hSrc, u32 ptrSrc, u32 size);
u32     patchRemoteProcess(u32 pid, u32 addr, u8 *buf, u32 len);
u32     rtAlignToPageSize(u32 size);
u32     rtGetPageOfAddress(u32 addr);
u32     rtCheckRemoteMemoryRegionSafeForWrite(Handle hProcess, u32 addr, u32 size);
u32     memfind(u8 *startPos, u32 size, const void *pattern, u32 patternSize);
u32     findNearestSTMFD(u32 base, u32 pos);
u32     searchBytes(u32 startAddr, u32 endAddr, u8* pat, int patlen, int step);

/*
** firmware.c   
*/
Result  bnInitParamsByFirmware(void);

/*
** kernel.c
*/
void    kernelCallback(void);

/*
** homemenu.c
*/
Result  bnInitParamsByHomeMenu(void);

/*
** updater.c
*/

extern char	CURL_lastErrorCode[CURL_ERROR_SIZE];
bool    updateAvailable(void);
bool	downloadChangelog(void);
int		downloadFile(char* URL, char* filepath, progressbar_t* progbar);
int		updateLuma3DS(progressbar_t* progbar);
int     downloadString(char* URL, char** out);
char*	getProgText(float prog, int index);
int		performUpdate(progressbar_t* progbar, bool* restartNeeded);
FILE* fopen_mkdir(const char* name, const char* mode);
void setControlsMode(int mode);
void perform3dsxUpdate();
int downloadModFromInternet(progressbar_t* progbar, bool get3dsx);
/*
** updater_UI.c

*/
void UpdatesMenu();
void clearUpdateSprites();
void    InitUpdatesUI(void);

/*
** launcher.c
*/
void launchMod();
/*
** installer.c
*/
u64 installMod(progressbar_t* progbar, u64 zipSize);
/*
**credits.c
*/
void creditsMenu();
void initCreditsResources();
void freeCreditsResources();
#endif