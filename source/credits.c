#include "main.h"
#include "credits.h"
#include "sound.h"
#include "clock.h"

static credits_t g_credits = { 0 };
static u32 g_creditColors[] = { DEFAULT_COLOR, COLOR_BLUE, COLOR_GREEN, COLOR_ORANGE, COLOR_RED, COLOR_YELLOW, COLOR_LIMEGREEN, COLOR_CYAN, COLOR_FUCHSIA, COLOR_SILVER };
extern sprite_t* creditsText;
extern sprite_t *updaterControlsText;
extern appInfoObject_t         *appTop;
extern sprite_t         *topInfoSprite;

char* tryDownloadCredits() {
	char* ret = NULL;
	downloadString("https://raw.githubusercontent.com/mariohackandglitch/CTGP-7updates/master/credits.txt", &ret); //Put the correct URL
	return ret;
}

void freeCredits() {
	if (g_credits.entries) {
		for (int i = 0; i < g_credits.count; i++) {
			free(g_credits.entries[i]->name);
			free(g_credits.entries[i]->whatHeDo);
			free(g_credits.entries[i]);
		}
	}
	if (g_credits.entries) free(g_credits.entries);
	g_credits.count = 0;
	g_credits.entries = NULL;
}

static void shuffle_credits(credits_entry_t** array, size_t n)
{
	srand(time(NULL));
	if (n > 1)
	{
		size_t i;
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			credits_entry_t* t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

static void shuffle_colors(u32* array, size_t n)
{
	srand(time(NULL));
	if (n > 1)
	{
		size_t i;
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			u32 t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

void strcpyline(char* dst, char* src) {
	while ((*src != '\0') && (*src != '\r') && (*src != '\n')) {
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\0';
}

bool tryParseCredits() {
	if (g_credits.count != 0) return true;
	char* downstr = tryDownloadCredits();
	if (downstr == NULL) return false;
	int linecount = 0;
	int downlen = strlen(downstr);
	for (int i = 0; i < downlen; i++) {
		if (downstr[i] == '\n')
			linecount++;
		else if (downstr[i] == '\r' && downstr[i + 1] == '\n') {
			linecount++;
			i++;
		}
	}
	g_credits.entries = malloc(linecount * sizeof(credits_entry_t*));
	char* curr = downstr;
	char* old = curr;
	char* mid = curr;
	int i = 0;
	while (true)
	{
		if (*curr == '\n' || (*curr == '\r' && *(curr + 1) == '\n')) {
			for (;(*mid != '=') && (mid != curr); mid++);
			g_credits.entries[i] = malloc(sizeof(credits_entry_t));
			g_credits.entries[i]->name = malloc((mid - old) + 1);
			g_credits.entries[i]->whatHeDo = malloc(curr - mid);
			*mid = '\0';
			strcpyline(g_credits.entries[i]->name, old);
			strcpyline(g_credits.entries[i]->whatHeDo, mid + 1);
			*mid = '=';
			i++;
			if (*curr == '\r') curr++;
			curr++;
			old = mid = curr;
		}
		else {
			curr++;
		}
		if (!(*curr)) break;
	}
	g_credits.count = linecount;
	shuffle_credits(g_credits.entries, g_credits.count);
	free(downstr);
	return true; 
}

void getParseCredits() {
	static bool creditsParsed = false;
	if (!creditsParsed && tryParseCredits()) creditsParsed = true;
}

void initCreditsResources() {
	static bool resourcesLoaded = false;
	if (resourcesLoaded) return;
	resourcesLoaded = true;
	//cred_sound = cwavLoadFromFile("romfs:/sound/credits.bcwav", 1);
	newSpriteFromPNG(&creditsText, "romfs:/sprites/textSprites/creditsText.png");
	setSpritePos(creditsText, 150.f, 0.f);
}

void freeCreditsResources() {
	if (cred_sound) cwavFree(cred_sound);
	if (creditsText) deleteSprite(creditsText);
	freeCredits();
	creditsText = NULL;
	cred_sound = NULL;
}

void drawCreditsText(int pageid, int totentries) {
	clearTop();
	bool isName = pageid % 2 == 0;
	pageid >>= 1;
	newAppTop(DEFAULT_COLOR, MEDIUM | BOLD | CENTER, "Page %d/%d", pageid + 1, (int)ceil(totentries / 10.f));
	for (int i = 0; i < 10; i++) {
		if (i + 10 * pageid >= totentries) break;
		newAppTop(g_creditColors[i], MEDIUM | CENTER, "%s", isName ? g_credits.entries[i + 10 * pageid]->name : g_credits.entries[i + 10 * pageid]->whatHeDo);
	}
}

void creditsLoop() {
	int key = 0;
	bool creditsloop = true;
	int totentries = g_credits.count << 1;
	int totpages = (int)ceil(totentries / 10.f);
	int currentry = 0;
	if (totentries == 0) {
		clearTop();
		newAppTop(COLOR_ORANGE, MEDIUM | BOLD | CENTER, "Couldn't download credits.");
		newAppTop(DEFAULT_COLOR, MEDIUM | CENTER, "Error info:");
		newAppTopMultiline(DEFAULT_COLOR, SMALL | CENTER, CURL_lastErrorCode);
		while (creditsloop && aptMainLoop())
		{
			if (key & KEY_B) {
				creditsloop = false;
				PLAYBOOP();
			}
			updateUI();
			key = hidKeysDown();
		}
	}
	else {
		u32 waitTime = 5000;
		STARTCRED();
		u64 timer = 10000000;
		while (creditsloop && aptMainLoop())
		{
			if (Timer_HasTimePassed(waitTime, timer)) {
				timer = Timer_Restart();
				if (currentry % 2 == 0) shuffle_colors(g_creditColors, sizeof(g_creditColors) / sizeof(u32));
				drawCreditsText(currentry, totentries >> 1);
				currentry++;
				if (currentry >= totpages) currentry = 0;
			}
			if (key & KEY_B) {
				creditsloop = false;
				PLAYBOOP();
			}
			updateUI();
			key = hidKeysDown();
		}
		STOPCRED();
	}
}

void creditsMenu() {
	STARTLAG();
	getParseCredits();
	STOPLAG();
	clearTop();
	changeTopSprite(4);
	setControlsMode(2);
	changeTopFooter(updaterControlsText);
	appTop->sprite = topInfoSprite;
	creditsLoop();
	clearTop();
	changeTopSprite(0);
	setControlsMode(0);
	changeTopFooter(NULL);
}