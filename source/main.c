#include "main.h"
#include "draw.h"
#include "button.h"
#include <time.h>
#include "Unicode.h"
#include "sound.h"

u8                *tmpBuffer;
char              *g_primary_error = NULL;
char              *g_secondary_error = NULL;
char              *g_third_error = NULL;
bool              g_exit = false;
bool			  forceUpdate;
bool			  restartneeded = false;
char			  g_modversion[15] = { '\0' };
u8				  g_launcherversion[3] = { '\0' };
u8				  hmac[0x20];
extern sprite_t*  updaterControlsText;
extern char*      versionList[100];
extern char textVersion[20];
extern u64 launchAppID;
extern FS_MediaType launchAppMedtype;
extern progressbar_t *updateProgBar;
extern sprite_t* topInfoSpriteUpd;
extern sprite_t* topInfoSprite;

extern CWAV* sfx_sound;
extern CWAV* lag_sound;
extern CWAV* cred_sound;

aptHookCookie aptCookie;

static void mainAptHook(APT_HookType hook, void* param) {
	static bool bSleeping = false;
	switch (hook)
	{
	case APTHOOK_ONRESTORE:
	case APTHOOK_ONWAKEUP:
		bSleeping = false;
		break;
	case APTHOOK_ONSUSPEND:
	case APTHOOK_ONSLEEP:
		if (!bSleeping) {
			if (lag_sound) STOPLAG();
			if (cred_sound) STOPCRED();
		}
		bSleeping = true;
		break;

	case APTHOOK_ONEXIT:
		acExit();
		amExit();
		csndExit();
		httpcExit();
		romfsExit();
		gfxExit();
		aptExit();
		APT_CloseApplication(NULL, 0, 0);
		svcExitProcess();
		break;

	default:
		break;
	}
}


void getModVersion() {
	memset(g_modversion, 0, 15);
	FILE* file = NULL;
	file = fopen("/CTGP-7/config/version.bin", "r");
	if (!file) {
		return;
	}
	fread(g_modversion, 1, 15, file);
	fclose(file);
	if (!g_modversion[0]) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x0);
	file = NULL;
	textVersion[0] = '\0';
	memset(g_launcherversion, 0, 3);
	file = fopen("/CTGP-7/config/expectedVer.bin", "r");
	if (!file) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x1);
	fread(g_launcherversion, 1, 3, file);
	if (!g_modversion[0] || !g_launcherversion[0]) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x2);
	fclose(file);
	file = NULL;
}

void finishUpdate() {
	// Sigh... This is the only way to properly finish the 3dsx installation
	FILE* tmpFile = NULL;
	tmpFile = fopen(TOINSTALL_3DSX_PATH, "r");
	if (tmpFile) {
		fclose(tmpFile);
		FILE* endBrewFile = fopen_mkdir(FINAL_3DSX_PATH, "w"); // Generate path
		if (endBrewFile) {
			fclose(endBrewFile);
			remove(FINAL_3DSX_PATH);
			copy_file(TOINSTALL_3DSX_PATH, TEMPORAL_3DSX_PATH);
			rename(TOINSTALL_3DSX_PATH, FINAL_3DSX_PATH);
		}
	}
}

bool isExpectedVersion() {
	if (APP_VERSION_MAJOR < g_launcherversion[0]) {
		return false;
	}
	else if (APP_VERSION_MAJOR == g_launcherversion[0]) {
		if (APP_VERSION_MINOR < g_launcherversion[1]) {
			return false;
		}
		else if ((APP_VERSION_MINOR == g_launcherversion[1])) {
			if (APP_VERSION_MICRO < g_launcherversion[2]) {
				return false;
			}
		}
	}
	u32 vervar = (APP_VERSION_MAJOR) | (APP_VERSION_MINOR << 8) | (APP_VERSION_MICRO << 16);
	FILE* file = NULL;
	file = fopen("/CTGP-7/config/expectedVer.bin", "w");
	if (!file) customBreak(0xC001BEEF, 0xFEEDDEAD, 0xBAAAAAAD, 0x3);
	fwrite(&vervar, 1, 3, file);
	fclose(file);
	return true;
}

int mainLauncher(void)
{
    //u32         kernelVersion;

    gfxInitDefault();
    romfsInit();
    ptmSysmInit();
	acInit();
	csndInit();
	FSUSER_SetPriority(-16);
	drawInit();
	aptHook(&aptCookie, mainAptHook, NULL);

	getModVersion();
	bool isProperVer = false;
	if (g_modversion[0])
		isProperVer = isExpectedVersion();
    initUI();
	InitUpdatesUI();
	initCreditsResources();
	initMainMenuResources();
	if (!isProperVer) {
		if (!g_modversion[0]) {
			changeTopSprite(2);
			clearTop();
			newAppTop(COLOR_RED, BOLD | MEDIUM | CENTER, "Fatal error");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nNo CTGP-7 files detected. Use the");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "CTGP-7 installer to fix the files.");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, FONT_B": Exit");
			u32 keys = 0;
			bool installLoop = true;
			while (installLoop && aptMainLoop()) {
				if (keys & KEY_B) {
					PLAYBOOP();
					installLoop = false;
				}
				updateUI();
				keys = hidKeysDown();
			}
		}
		else {
			changeTopFooter(updaterControlsText);
			updaterControlsText->isHidden = false;
			setControlsMode(2);
			clearTop();
			newAppTop(COLOR_RED, BOLD | MEDIUM | CENTER, "Version mismatch error");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "This app version is older");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "than a previous launched one.");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "App version: %d.%d.%d", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_MICRO);
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Expected version: %d.%d.%d", g_launcherversion[0], g_launcherversion[1], g_launcherversion[2]);
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER | BOLD, "\nTo fix the error:");
			if (!envIsHomebrew()) {
				newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "- Install the following cia");
				newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"FINAL_CIA_PATH);
			}
			else {
				newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "- Copy this file");
				newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"TEMPORAL_3DSX_PATH);
				newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "to this location");
				newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"FINAL_3DSX_PATH);
			}
			u32 keys = 0;
			bool errorLoop = true;
			while (errorLoop && aptMainLoop()) {
				if (keys & KEY_B) {
					errorLoop = false;
					PLAYBOOP();
				}
				updateUI();
				keys = hidKeysDown();
			}
		}
		greyExit();
		updateUI();
		svcSleepThread(500000000);
	}
	else {
		newAppTop(DEFAULT_COLOR, BOLD | MEDIUM | CENTER, "Checking for updates...");
		updateUI();
		finishUpdate();
		forceUpdate = updateAvailable();
		initMainMenu();
		clearTop();
		removeAppTop();
		mainMenu();
	}
	exitMainMenu();
	exitUI();
	drawExit();
	aptUnhook(&aptCookie);

    acExit();
    amExit();
	csndExit();
    httpcExit();
    romfsExit();
    gfxExit();
    ptmSysmExit();
	if (launchAppID && launchAppMedtype != 0xFF) {
		APT_PrepareToDoApplicationJump(0, launchAppID, launchAppMedtype);
		APT_DoApplicationJump(NULL, 0, hmac);
		for (;;);
	}
    return (0);
}

int mainInstaller(void)
{
	//u32         kernelVersion;

	gfxInitDefault();
	romfsInit();
	ptmSysmInit();
	acInit();
	csndInit();
	drawInit();
	aptHook(&aptCookie, mainAptHook, NULL);

	getModVersion();
	initUI();
	InitUpdatesUI();
	changeTopSprite(2);
	FILE* zipFile = NULL;
	u64 zipSize = 0;
	bool continueInstall = true;
	zipFile = fopen("romfs:/CTGP-7.zip", "rb");
	if (zipFile) {
		fseek(zipFile, 0, SEEK_END);
		zipSize = ftell(zipFile);
		fclose(zipFile);
	}
	clearTop();
	if (g_modversion[0]) {
		clearTop();
		newAppTop(COLOR_ORANGE, MEDIUM | BOLD | CENTER, "WARNING");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Did you install the CTGP-7");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "installer by mistake?\n");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "To run the normal version:");
		if (!envIsHomebrew()) {
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "- Install the following cia");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"FINAL_CIA_PATH);
		}
		else {
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "- Copy this file");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"TEMPORAL_3DSX_PATH);
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "to this location");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "SD:"FINAL_3DSX_PATH);
		}
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\n"FONT_A": Continue   "FONT_B": Exit");
		u32 keys = 0;
		bool warningloop = true;
		while (warningloop && aptMainLoop())
		{
			if (keys & KEY_A) {
				warningloop = false;
				PLAYBEEP();
			}
			if (keys & KEY_B) {
				warningloop = false;
				continueInstall = false;
				PLAYBOOP();
			}
			updateUI();
			keys = hidKeysDown();
		}
	}
	if (continueInstall) {
		clearTop();
		newAppTop(DEFAULT_COLOR, BOLD | MEDIUM | CENTER, "CTGP-7 installer");
		if (!g_modversion[0]) {
			newAppTop(COLOR_GREEN, MEDIUM | CENTER, "\nStart with installation?\n");
		}
		else {
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nCTGP-7 files detected. Would you");
			newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "like to reinstall the entire mod?\n");
		}
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, FONT_A": Install");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, FONT_B": Exit");
		u32 keys = 0;
		bool installLoop = true;
		while (installLoop && aptMainLoop()) {
			if ((keys & KEY_A)) {
				g_modversion[0] = '\0';
				PLAYBEEP();
				u64 ret = installMod(updateProgBar, zipSize);
				appTop->sprite = topInfoSprite;
				u32 retlow = (u32)ret;
				u32 rethigh = (u32)(ret >> 32);
				clearTop();
				if (!retlow) {
					newAppTop(COLOR_GREEN, BOLD | MEDIUM | CENTER, "Installation successful!");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\n"FONT_B": Exit");
				}
				else if (retlow == 2) {
					newAppTop(COLOR_RED, BOLD | MEDIUM | CENTER, "Installation failed!");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\nNot enough free space, %dMB", rethigh);
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "are needed to install the mod.");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, FONT_B": Exit");
				}
				else if (retlow == 15) {
					newAppTop(COLOR_RED, BOLD | MEDIUM | CENTER, "Installation failed!");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Failed to create a backup");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "of the CTGP-7 save file.\n");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "\"SD:/CTGP-7savebak/\"");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "already exists.\n");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, FONT_B": Exit");
				}
				else {
					newAppTop(COLOR_RED, BOLD | MEDIUM | CENTER, "Installation failed!");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Error occured during");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "installation, error code:\n");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "%08X %08X\n", retlow, rethigh);
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "You can check the error here");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "https://pastebin.com/u5ZyfWqW\n");
					newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, FONT_B": Exit");
				}
				u32 keys2 = 0;
				bool installLoop2 = true;
				while (installLoop2 && aptMainLoop())
				{
					if (keys2 & KEY_B) {
						PLAYBOOP();
						installLoop2 = false;
					}
					updateUI();
					keys2 = hidKeysDown();
				}
				installLoop = false;
			}
			if (keys & KEY_B) {
				PLAYBOOP();
				installLoop = false;
			}
			updateUI();
			keys = hidKeysDown();
		}
	}
	greyExit();
	updateUI();
	svcSleepThread(500000000);

	exitMainMenu();
	exitUI();
	drawExit();
	aptUnhook(&aptCookie);
	acExit();
	amExit();
	csndExit();
	httpcExit();
	romfsExit();
	gfxExit();
	ptmSysmExit();
	return (0);
}

int main(void) {
#if LAUNCHER_MODE == 0
	return mainInstaller();
#else
	return mainLauncher();
#endif // LAUNCHER
}