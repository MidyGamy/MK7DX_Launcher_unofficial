#include "graphics.h"
#include "drawableObject.h"
#include "button.h"
#include "sound.h"

sprite_t         *bottomSprite = NULL;
sprite_t         *topSprite = NULL;

sprite_t         *topInfoSprite = NULL;
sprite_t		 *topInfoSpriteUpd = NULL;
sprite_t         *launchTextSprite = NULL;
sprite_t		 *updateTextSprite = NULL;
sprite_t         *launchControlsSprite = NULL;
sprite_t		 *launcherText = NULL;
sprite_t         *installerText = NULL;
sprite_t         *creditsText = NULL;
extern sprite_t  *changelogTextsprite;
drawableScreen_t *botScreen = NULL;
drawableScreen_t *topScreen = NULL;
appInfoObject_t  *appTop = NULL;

extern char g_modversion[15];

char textVersion[20] = { 0 };

void    initUI(void)
{
    backgroundScreen_t *bg;

    newSpriteFromPNG(&topSprite, "romfs:/sprites/topBackground.png");
    newSpriteFromPNG(&bottomSprite, "romfs:/sprites/bottomBackground.png");    

    newSpriteFromPNG(&topInfoSprite, "romfs:/sprites/topInfoBackground.png");
	newSpriteFromPNG(&topInfoSpriteUpd, "romfs:/sprites/topInfoBackgroundUpd.png");

	newSpriteFromPNG(&launchControlsSprite, "romfs:/sprites/textSprites/launchControlsSprite.png");

	newSpriteFromPNG(&installerText, "romfs:/sprites/textSprites/installerText.png");
	newSpriteFromPNG(&launcherText,  "romfs:/sprites/textSprites/launcherText.png");

	setSpritePos(installerText, 150.f, 0.f);
	setSpritePos(launcherText, 150.f, 0.f);

    setSpritePos(topSprite, 0, 0);
    setSpritePos(bottomSprite, 0, 0);
	setSpritePos(launchControlsSprite, 0.0f, 220.0f);
    
    bg = newBackgroundObject(bottomSprite, NULL, NULL);
    botScreen = newDrawableScreen(bg);
    bg = newBackgroundObject(topSprite, NULL, NULL);
    topScreen = newDrawableScreen(bg);

    setSpritePos(topInfoSprite, 50, 20);
	setSpritePos(topInfoSpriteUpd, 50, 20);
    appTop = newAppInfoObject(topInfoSprite, 14, 58.0f, 30.0f);
    appInfoSetTextBoundaries(appTop, 343.0f, 220.0f);

    //sfx_sound = cwavLoadFromFile("romfs:/sound/beepboopclick.bcwav", 3);
    //lag_sound = cwavLoadFromFile("romfs:/sound/lag.bcwav", 1);
}

void changeTopSprite(int id) {
	switch (id) {
	case 1:
		changeTopHeader(changelogTextsprite);
		break;
	case 2:
		changeTopHeader(installerText);
		break;
	case 3:
		changeTopHeader(launcherText);
		break;
	case 4:
		changeTopHeader(creditsText);
		break;
	default:
		changeTopHeader(NULL);
	}
}

void    exitUI(void)
{
    deleteAppInfoObject(appTop);
    deleteSprite(bottomSprite);
    deleteSprite(topSprite);
    deleteSprite(topInfoSprite);
	deleteSprite(launchControlsSprite);
	deleteSprite(launcherText);
	deleteSprite(installerText);
}

static inline void drawUITop(void)
{
    setScreen(GFX_TOP);
    
    topScreen->draw(topScreen);
    
	if (g_modversion[0]) {
        setTextColor(COLOR_BLANK);
		if (textVersion[0] == '\0') {
			sprintf(textVersion, "Ver. %s", g_modversion);
		}
		renderText(1.0f, 1.0f, 0.4f, 0.45f, false, textVersion, NULL, 0.0f);
	}
    drawAppInfo(appTop);
}

static inline void drawUIBottom(void)
{
    setScreen(GFX_BOTTOM);
    
    botScreen->draw(botScreen);
}

int   updateUI(void)
{
    hidScanInput();
    drawUITop();
    drawUIBottom();
    updateScreen();
    return (1);
}

void    addTopObject(void *object)
{
    addObjectToScreen(topScreen, object);
}

void    setExTopObject(void *object)
{
	setExTopObjectToScreen(topScreen, object);
}

void    addBottomObject(void *object)
{
    addObjectToScreen(botScreen, object);
}

void    changeTopFooter(sprite_t *footer)
{
    changeBackgroundFooter(topScreen->background, footer);
}

void    changeTopHeader(sprite_t *header)
{
    changeBackgroundHeader(topScreen->background, header);
}

void    changeBottomFooter(sprite_t *footer)
{
    changeBackgroundFooter(botScreen->background, footer);
}

void    changeBottomHeader(sprite_t *header)
{
    changeBackgroundHeader(botScreen->background, header);
}

void    clearTopScreen(void)
{
    clearObjectListFromScreen(topScreen);
}

void    clearBottomScreen(void)
{
    clearObjectListFromScreen(botScreen);
}