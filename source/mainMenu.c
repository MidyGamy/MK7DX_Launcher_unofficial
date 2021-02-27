#include "main.h"
#include "draw.h"
#include "button.h"
#include <time.h>
#include "clock.h"
#include "sound.h"
#include "drawableObject.h"

extern drawableScreen_t	*botScreen;
extern drawableScreen_t *topScreen;
extern bool				forceUpdate;
extern progressbar_t	*updateProgBar;
extern appInfoObject_t  *appTop;
extern sprite_t         *topInfoSprite; 
extern sprite_t			*topInfoSpriteUpd;

static button_t         *V32Button = NULL;
static button_t         *V33Button = NULL;
extern sprite_t         *launchTextSprite;
extern sprite_t			*updateTextSprite;
static sprite_t         *tinyButtonBGSprite = NULL;
static sprite_t         *selTinyButtonBGSprite = NULL;
sprite_t				*pressExitSprite = NULL;
static bool             userTouch = false;
static bool				optionSelected = false;
static u32				optionTodo = 0xFF;

extern bool restartneeded;

extern u64 launchAppID;
extern FS_MediaType launchAppMedtype;

CWAV					*sfx_sound = NULL;
CWAV					*lag_sound = NULL;
CWAV					*cred_sound = NULL;

static u32 getNextRbwColor(u32 counter) {
	#define RNBW_FREQ 0.05235987755
	#define RNBW_CENTER 128.0f
	#define RNBW_WIDTH 110.0f

	u32 red = (u32)(sin(RNBW_FREQ*counter + 2) * RNBW_WIDTH + RNBW_CENTER);
	u32 green = (u32)(sin(RNBW_FREQ*counter + 0) * RNBW_WIDTH * 0.75f + RNBW_CENTER) << 8;
	u32 blue = (u32)(sin(RNBW_FREQ*counter + 4) * RNBW_WIDTH * 0.75f + RNBW_CENTER) << 16;
	return (0xFF000000 | red | green | blue);
}

void greyBottomScreen(bool state) {
	botScreen->background->footerText->isGreyedOut = state;
	botScreen->background->background->isGreyedOut = state;
}
void greyExit() {
	if(V32Button) V32Button->isGreyedOut = true;
	if (V33Button) V33Button->isGreyedOut = true;
	if (botScreen->background->footerText) botScreen->background->footerText->isHidden = true;
	botScreen->background->background->isGreyedOut = true;
	topScreen->background->background->isGreyedOut = true;
	if (topInfoSprite) topInfoSprite->isGreyedOut = true;
	if (topInfoSpriteUpd) topInfoSpriteUpd->isGreyedOut = true;
	if (updateProgBar) updateProgBar->isHidden = true;
	if (pressExitSprite) pressExitSprite->isHidden = true;
}

void    selectVersion(u32 mode)
{
	if (optionSelected) return;
	if (userTouch && !optionSelected) return;
	PLAYBEEP();
	V32Button->disable(V32Button);
	V33Button->disable(V33Button);
	optionSelected = true;
	optionTodo = mode;
}

void initMainMenuResources(void) {
	newSpriteFromPNG(&tinyButtonBGSprite, "romfs:/sprites/tinyButtonBackground.png");
	newSpriteFromPNG(&selTinyButtonBGSprite, "romfs:/sprites/selTinyButtonBackground.png");
	newSpriteFromPNG(&launchTextSprite, "romfs:/sprites/textSprites/launchText.png");
	newSpriteFromPNG(&updateTextSprite, "romfs:/sprites/textSprites/updateText.png");
	newSpriteFromPNG(&pressExitSprite, "romfs:/sprites/textSprites/pressBExit.png");
}

void    initMainMenu(void)
{
    V32Button = newButton(5.0f, 48.0f, selectVersion, 1, tinyButtonBGSprite, launchTextSprite, selTinyButtonBGSprite, 0xFF0000FF);
	V33Button = newButton(5.0f, 132.0f, selectVersion, 2, tinyButtonBGSprite, updateTextSprite, selTinyButtonBGSprite, 0xFF0000FF);

    V32Button->show(V32Button);
    V33Button->show(V33Button);
    addBottomObject(V32Button);
    addBottomObject(V33Button);

    setSpritePos(pressExitSprite, 0, 217.0f);

    changeBottomFooter(pressExitSprite); 

}

void    exitMainMenu(void)
{
    destroyButton(V32Button);
    destroyButton(V33Button);
    deleteSprite(tinyButtonBGSprite);
    deleteSprite(pressExitSprite);
	V32Button = NULL;
	V33Button = NULL;
	tinyButtonBGSprite = NULL;
	pressExitSprite = NULL;
	clearUpdateSprites();
	cwavFree(sfx_sound);
	cwavFree(lag_sound);
	freeCreditsResources();
}

int     mainMenu(void)
{
    u32         keys;
	u32			exitkey;

    //waitAllKeysReleased();
    keys = 0;
	exitkey = 0;
	u32 framecount = 0;
	u64 timer = Timer_Restart();
	u64 timer2 = Timer_Restart();
	V32Button->isSelected = true;
	bool neeedToUpdate = forceUpdate;
    while (userTouch == false && aptMainLoop())
    {
		if (Timer_HasTimePassed(17, timer)) {
			timer = Timer_Restart();
			framecount++;
			if (framecount >= 120) {
				framecount = 0;
			}
		}
		V32Button->selTextColor = getNextRbwColor(framecount);
		if (neeedToUpdate) {
			V33Button->isSelected = true;
			V32Button->isSelected = false;
			selectVersion(2);
			neeedToUpdate = false;
		}
		if (keys & (KEY_CPAD_UP | KEY_CPAD_DOWN | KEY_DUP | KEY_DDOWN)) {
			if (Timer_HasTimePassed(250, timer2)) {
				PLAYBEEP();
				timer2 = Timer_Restart();
				V32Button->isSelected = !V32Button->isSelected;
				V33Button->isSelected = !V33Button->isSelected;
			}
		}
		if (exitkey & KEY_B) {
			userTouch = true;
			PLAYBOOP();
		}
		else if (exitkey & KEY_Y) {
			selectVersion(3);
		}
		else if (exitkey & KEY_X) {
			selectVersion(4);
		}
		updateUI();
		keys = hidKeysDown() | hidKeysHeld();
		exitkey = hidKeysDown();
		if (optionSelected) {
			switch (optionTodo)
			{
			case 1:
				greyBottomScreen(true);
				V33Button->isGreyedOut = true;
				pressExitSprite->isHidden = true;
				changeTopSprite(3);
				launchMod();
				if (!(u32)(launchAppID)) {
					V33Button->isGreyedOut = false;
					greyBottomScreen(false);
					V33Button->isGreyedOut = false;
					pressExitSprite->isHidden = false;
				}
				else {
					userTouch = true;
				}
				changeTopSprite(0);
				break;
			case 2:
				greyBottomScreen(true);
				V32Button->isGreyedOut = true;
				pressExitSprite->isHidden = true;
				UpdatesMenu();
				pressExitSprite->isHidden = false;
				greyBottomScreen(false);
				V32Button->isGreyedOut = false;
				if (restartneeded) userTouch = true;
				break;
			case 3:
				greyBottomScreen(true);
				V32Button->isGreyedOut = true;
				V33Button->isGreyedOut = true;
				pressExitSprite->isHidden = true;
				creditsMenu();
				pressExitSprite->isHidden = false;
				greyBottomScreen(false);
				V32Button->isGreyedOut = false;
				V33Button->isGreyedOut = false;
				break;
			case 4:
				greyBottomScreen(true);
				V32Button->isGreyedOut = true;
				V33Button->isGreyedOut = true;
				pressExitSprite->isHidden = true;
				//bootntrMenu();
				pressExitSprite->isHidden = false;
				greyBottomScreen(false);
				V32Button->isGreyedOut = false;
				V33Button->isGreyedOut = false;
				break;
			default:
				break;
			}
			V32Button->enable(V32Button);
			V33Button->enable(V33Button);
			optionSelected = false;
			optionTodo = 0xFF;
			exitkey = keys = 0;
			hidScanInput();
		}
    }
	greyExit();
	updateUI();
	svcSleepThread(1000000000);
	return (1);
}