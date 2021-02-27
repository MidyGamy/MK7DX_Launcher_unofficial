#include "main.h"
#include "Archives.h"
#include "errno.h"
#include "sys/stat.h"
#include <malloc.h>
#include "clock.h"
#include "sound.h"

#define EXPECTED_INSTALLER_VER "1"

progressbar_t* installProgBar;
extern appInfoObject_t         *appTop;
extern sprite_t         *topInfoSprite;
extern sprite_t		 *topInfoSpriteUpd;

Result deleteDirectory(char* sdDir) {
	FS_Archive sd;
	FS_Path sd_path = { PATH_EMPTY, 0, NULL };
	Result ret = 0;
	ret = FSUSER_OpenArchive(&sd, ARCHIVE_SDMC, sd_path);
	if (ret < 0) return ret;
	ret = FSUSER_DeleteDirectoryRecursively(sd, fsMakePath(PATH_ASCII, sdDir));
	FSUSER_CloseArchive(sd);
	return ret;
}

Result existsDirectory(char *sdDir) {
	FS_Archive sd;
	FS_Path sd_path = { PATH_EMPTY, 0, NULL };
	Result ret = 0;
	Handle dir = 0;
	ret = FSUSER_OpenArchive(&sd, ARCHIVE_SDMC, sd_path);
	if (ret < 0) return false;
	ret = FSUSER_OpenDirectory(&dir, sd, fsMakePath(PATH_ASCII, sdDir));
	if (ret >= 0) {
		FSDIR_Close(dir);
		FSUSER_CloseArchive(sd);
		return true;
	}
	FSUSER_CloseArchive(sd);
	return false;
}
 
Result renameDir(char* src, char* dst) {
	FS_Archive sd;
	FS_Path sd_path = { PATH_EMPTY, 0, NULL };
	Result ret = 0;
	ret = FSUSER_OpenArchive(&sd, ARCHIVE_SDMC, sd_path);
	if (ret < 0) return ret;
	ret = FSUSER_RenameDirectory(sd, fsMakePath(PATH_ASCII, src), sd, fsMakePath(PATH_ASCII, dst));
	FSUSER_CloseArchive(sd);
	return ret;
}

Result getFreeSpace(u64* out) {
	FS_ArchiveResource resource = { 0 };
	int ret = FSUSER_GetArchiveResource(&resource, SYSTEM_MEDIATYPE_SD);
	if (ret < 0) return ret;
	*out = (u64)resource.freeClusters * (u64)resource.clusterSize;
	return ret;
}

int zipCallBack(u32 curr, u32 total) {
	installProgBar->rectangle->amount = ((float)curr) / ((float)total);
	static u64 timer = 0;
	if (Timer_HasTimePassed(10, timer)) {
		timer = Timer_Restart();
		clearTop();
		newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "Installing CTGP-7");
		newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\n\nExtracting files\n");
		newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "%d / %d", curr, total);
		PLAYCLICK();
		updateUI();
	}
	return 0;
}

u64 installMod(progressbar_t* progbar, u64 zipsize) {
	char verRead[4] = { 0 };
	appTop->sprite = topInfoSpriteUpd;
	STARTLAG();
	clearTop();
	newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "Installing CTGP-7");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Preparing to install");
	updateUI();
	if (existsDirectory("/CTGP-7/savefs")) {
		if (renameDir("/CTGP-7/savefs", "/CTGP-7savebak")) {
			STOPLAG();
			return 15;
		}
	}
	deleteDirectory("/CTGP-7");
	deleteDirectory("/CTGP-7tmp");
	u64 freeBytes = 0;
	Result ret = getFreeSpace(&freeBytes);
	if (ret < 0) {
		STOPLAG();
		return 1 | ((u64)ret << 32);
	}
	if (freeBytes < (zipsize + 100000000)) {
		STOPLAG();
		return 2 | ((u64)(((zipsize + 100000000) - freeBytes) / 1000000) << 32);
	}
	ret = mkdir("/CTGP-7tmp", 777);
	if (ret != 0) {
		STOPLAG();
		return 3 | ((u64)ret << 32);
	}
	Zip* modZip = ZipOpen("romfs:/CTGP-7.zip");
	if (modZip == NULL) {
		STOPLAG();
		return 14;
	}
	chdir("/CTGP-7tmp");
	clearTop();
	newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "Installing CTGP-7");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\n\nExtracting files");
	updateUI();
	progbar->rectangle->amount = 0;
	progbar->isHidden = false;
	installProgBar = progbar;
	STOPLAG();
	ZipExtract(modZip, NULL, zipCallBack);
	chdir("/");
	progbar->isHidden = true;
	STARTLAG();
	clearTop();
	newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "Installing CTGP-7");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\nFinishing installation.");
	updateUI();
	FILE* vercheck = NULL;
	vercheck = fopen("/CTGP-7tmp/installerver.bin", "rb");
	if (!vercheck) {
		STOPLAG();
		deleteDirectory("/CTGP-7tmp");
		return 12;
	}
	fread(verRead, 1, 3, vercheck);
	fclose(vercheck);
	if (strcmp(EXPECTED_INSTALLER_VER, verRead) != 0) {
		STOPLAG();
		deleteDirectory("/CTGP-7tmp");
		return 13;
	}
	ret = renameDir("/CTGP-7tmp/files/CTGP-7", "/CTGP-7");
	deleteDirectory("/CTGP-7tmp");
	if (ret < 0) {
		STOPLAG();
		return 4 | ((u64)ret << 32);
	}
	FILE* ciaFile = NULL;
	ciaFile = fopen(FINAL_CIA_PATH, "rb");
	if (ciaFile) {
		amInit();
		AM_TitleEntry manInfo = { 0 };
		u64 tid = CTGP7_TID;
		AM_GetTitleInfo(MEDIATYPE_SD, 1, &tid, &manInfo);
		if (manInfo.size > 0) {
			Handle handle;
			Result ret = 0;
			ret = AM_StartCiaInstall(MEDIATYPE_SD, &handle);
			if (ret < 0) {
				STOPLAG();
				return 5 | ((u64)ret << 32);
			}
			fseek(ciaFile, 0L, SEEK_END);
			u32 fileSize = ftell(ciaFile);
			u32 filePos = 0;
			u32 chunk_sz = 0x60000;
			u8* tmpbuf = memalign(0x1000, chunk_sz);
			u32 singlePixel = fileSize / (progbar->rectangle->width);
			u32 oldprog = 0;
			u64 timer1 = Timer_Restart();
			u64 timer2 = Timer_Restart();
			progbar->isHidden = false;
			progbar->rectangle->amount = 0;
			float speed = 0;
			while (filePos < fileSize) {
				if (filePos + chunk_sz >= fileSize) {
					chunk_sz = fileSize - filePos;
				}
				u32 bytesWritten = 0;
				fseek(ciaFile, filePos, SEEK_SET);
				fread(tmpbuf, 1, chunk_sz, ciaFile);
				ret = FSFILE_Write(handle, &bytesWritten, filePos, tmpbuf, chunk_sz, 0);
				if (ret < 0) {
					STOPLAG();
					free(tmpbuf);
					fclose(ciaFile);
					AM_CancelCIAInstall(handle);
					amExit();
					return 6 | ((u64)ret << 32);
				}
				filePos += bytesWritten;
				clearTop();
				if (filePos - oldprog > singlePixel) {
					clearTop();
					progbar->rectangle->amount = ((float)filePos) / ((float)fileSize);
					timer2 = Timer_Restart();
					u32 delaymsec = getTimeInMsec(timer2) - getTimeInMsec(timer1);
					timer1 = Timer_Restart();
					speed = ((float)(filePos - oldprog)) / ((float)delaymsec);
					clearTop();
					newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "Installing CTGP-7");
					newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\nInstalling App");
					newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\n\n%s / %s", getProgText(filePos, 0), getProgText(fileSize, 1));
					newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "%.2f KB/s", speed);
					updateUI();
					oldprog = filePos;
				}
			}
			progbar->rectangle->amount = 1;
			clearTop();
			newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "Installing CTGP-7");
			newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\nInstalling App");
			newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\n\n%s / %s", getProgText(filePos, 0), getProgText(fileSize, 1));
			newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "%.2f KB/s", speed);
			updateUI();
			fclose(ciaFile);
			ciaFile = NULL;
			free(tmpbuf);
			ret = AM_FinishCiaInstall(handle);
			amExit();
			if (ret < 0 && ret != 0xC8E083FC) {
				STOPLAG();
				return 7 | ((u64)ret << 32);
			}
		}
	}
	else {
		STOPLAG();
		return 8;
	}
	FILE* brewFile = NULL;
	brewFile = fopen(TEMPORAL_3DSX_PATH, "rb");
	if (brewFile) {
		fclose(brewFile);
		FILE* endBrewFile = fopen_mkdir(FINAL_3DSX_PATH, "w"); // Generate path
		if (!endBrewFile) {
			STOPLAG();
			return 9;
		}
		fclose(endBrewFile);
		remove(FINAL_3DSX_PATH);
		if (copy_file(TEMPORAL_3DSX_PATH, FINAL_3DSX_PATH)) return 10;
	}
	else {
		STOPLAG();
		return 11;
	}
	progbar->isHidden = true;
	appTop->sprite = topInfoSprite;
	STOPLAG();
	return 0;
}

int nullCallback(u32 curr, u32 tot) {
	return 0;
}

u64 downloadMod(progressbar_t* progbar, u64 zipsize, bool get3dsx) {
	appTop->sprite = topInfoSpriteUpd;
	STARTLAG();
	clearTop();
	newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "CTGP-7 Downloader");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "Preparing to download.");
	updateUI();
	deleteDirectory("/CTGP-7tmp");
	u64 freeBytes = 0;
	Result ret = getFreeSpace(&freeBytes);
	if (ret < 0) {
		STOPLAG();
		return 1 | ((u64)ret << 32);
	}
	if (freeBytes < (zipsize + 100000000)) {
		STOPLAG();
		return 2 | ((u64)(((zipsize + 100000000) - freeBytes) / 1000000) << 32);
	}
	ret = mkdir("/CTGP-7tmp", 777);
	if (ret != 0) {
		STOPLAG();
		return 3 | ((u64)ret << 32);
	}

	STOPLAG();
	ret = downloadModFromInternet(progbar, get3dsx);
	if (ret) {
		return 16 | ((u64)ret << 32);
	}
	STARTLAG();

	Zip* modZip = ZipOpen("/CTGP-7tmp/CTGP-7.zip");
	if (modZip == NULL) {
		STOPLAG();
		return 17;
	}
	chdir("/CTGP-7tmp");
	clearTop();
	newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "CTGP-7 Downloader");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\nExtracting installer.\n");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "This can take up to 30 min.");
	updateUI();
	STOPLAG();
	ZipExtract(modZip, NULL, nullCallback);
	chdir("/");
	STARTLAG();
	clearTop();
	newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "CTGP-7 Downloader");
	newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\nInstalling CTGP-7 installer.");
	updateUI();
	FILE* testGood = NULL;
	testGood = fopen(get3dsx ? FULLMOD_DOWNLOAD_FINAL_3DSX : FULLMOD_DOWNLOAD_FINAL_CIA , "rb");
	if (!testGood) {
		STOPLAG();
		deleteDirectory("/CTGP-7tmp");
		return 18;
	}
	fclose(testGood);
	remove("/CTGP-7tmp/CTGP-7.zip");
	STOPLAG();
	FILE* ciaFile = NULL;
	ciaFile = fopen(FULLMOD_DOWNLOAD_FINAL_CIA, "rb");
	if (ciaFile) {
		amInit();
		AM_TitleEntry manInfo = { 0 };
		u64 tid = CTGP7_TID;
		AM_GetTitleInfo(MEDIATYPE_SD, 1, &tid, &manInfo);
		if (manInfo.size > 0) {
			Handle handle;
			Result ret = 0;
			ret = AM_StartCiaInstall(MEDIATYPE_SD, &handle);
			if (ret < 0) {
				return 19 | ((u64)ret << 32);
			}
			fseek(ciaFile, 0L, SEEK_END);
			u32 fileSize = ftell(ciaFile);
			u32 filePos = 0;
			u32 chunk_sz = 0x60000;
			u8* tmpbuf = memalign(0x1000, chunk_sz);
			u32 singlePixel = fileSize / (progbar->rectangle->width);
			u32 oldprog = 0;
			u64 timer1 = Timer_Restart();
			u64 timer2 = Timer_Restart();
			progbar->isHidden = false;
			progbar->rectangle->amount = 0;
			float speed = 0;
			while (filePos < fileSize) {
				if (filePos + chunk_sz >= fileSize) {
					chunk_sz = fileSize - filePos;
				}
				u32 bytesWritten = 0;
				fseek(ciaFile, filePos, SEEK_SET);
				fread(tmpbuf, 1, chunk_sz, ciaFile);
				ret = FSFILE_Write(handle, &bytesWritten, filePos, tmpbuf, chunk_sz, 0);
				if (ret < 0) {
					free(tmpbuf);
					fclose(ciaFile);
					AM_CancelCIAInstall(handle);
					amExit();
					return 20 | ((u64)ret << 32);
				}
				filePos += bytesWritten;
				clearTop();
				if (filePos - oldprog > singlePixel) {
					clearTop();
					progbar->rectangle->amount = ((float)filePos) / ((float)fileSize);
					timer2 = Timer_Restart();
					u32 delaymsec = getTimeInMsec(timer2) - getTimeInMsec(timer1);
					timer1 = Timer_Restart();
					speed = ((float)(filePos - oldprog)) / ((float)delaymsec);
					clearTop();
					newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "CTGP-7 Downloader");
					newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\nInstalling App");
					newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\n\n%s / %s", getProgText(filePos, 0), getProgText(fileSize, 1));
					newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "%.2f KB/s", speed);
					updateUI();
					oldprog = filePos;
				}
			}
			progbar->rectangle->amount = 1;
			clearTop();
			newAppTop(DEFAULT_COLOR, CENTER | BOLD | MEDIUM, "CTGP-7 Downloader");
			newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\nInstalling App");
			newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "\n\n%s / %s", getProgText(filePos, 0), getProgText(fileSize, 1));
			newAppTop(DEFAULT_COLOR, CENTER | MEDIUM, "%.2f KB/s", speed);
			updateUI();
			fclose(ciaFile);
			ciaFile = NULL;
			free(tmpbuf);
			ret = AM_FinishCiaInstall(handle);
			amExit();
			if (ret < 0 && ret != 0xC8E083FC) {
				return 21 | ((u64)ret << 32);
			}
		}
	}
	FILE* brewFile = NULL;
	brewFile = fopen(FULLMOD_DOWNLOAD_FINAL_3DSX, "rb");
	if (brewFile) {
		fclose(brewFile);
		FILE* endBrewFile = fopen_mkdir(FINAL_3DSX_INSTALLER_PATH, "w"); // Generate path
		if (!endBrewFile) {
			return 22;
		}
		fclose(endBrewFile);
		remove(FINAL_3DSX_INSTALLER_PATH);
		if (rename(FULLMOD_DOWNLOAD_FINAL_3DSX, FINAL_3DSX_INSTALLER_PATH)) return 23;
	}
	deleteDirectory("/CTGP-7tmp");
	progbar->isHidden = true;
	appTop->sprite = topInfoSprite;
	return 0;
}