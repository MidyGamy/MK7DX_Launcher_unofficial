#include "button.h"
#include "sound.h"
#define MAX_BUTTON_COUNT 10

static button_t* buttonList[MAX_BUTTON_COUNT] = { NULL };

static void addButtonToList(button_t* button) {
	for (int i = 0; i < MAX_BUTTON_COUNT; i++) {
		if (buttonList[i] == NULL) {
			buttonList[i] = button;
			break;
		}
	}
}

static void removeButtonFromList(button_t* button) {
	for (int i = 0; i < MAX_BUTTON_COUNT; i++) {
		if (buttonList[i] == button) {
			buttonList[i] = NULL;
			break;
		}
	}
}

button_t    *newButton(float posX, float posY, void(*action)(u32), u32 arg, sprite_t *button, sprite_t *text, sprite_t *selButton, u32 selTextColor)
{
	button_t    *retButton;

	if (!button || !text) goto error;
	retButton = (button_t *)calloc(1, sizeof(button_t));
	retButton->posX = posX;
	retButton->posY = posY;
	retButton->height = button->height;
	retButton->width = button->width;
	retButton->visible = false;
	retButton->isSelected = false;
	retButton->execute = true;
	retButton->isGreyedOut = false;
	retButton->buttonSprite = button;
	retButton->selButtonSprite = selButton;
	retButton->textSprite = text;
	retButton->selTextColor = selTextColor;
	retButton->arg = arg;
	retButton->action = action;
	retButton->hide = hideButton;
	retButton->show = showButton;
	retButton->enable = enableButton;
	retButton->disable = disableButton;
	retButton->draw = drawAndExecuteButton;
	retButton->isTouched = buttonIsTouched;
	retButton->destroy = destroyButton;
	addButtonToList(retButton);
	return (retButton);
error:
	return (NULL);
}

void        hideButton(button_t *button)
{
	if (!button) goto error;
	button->visible = false;
error:
	return;
}

void        showButton(button_t *button)
{
	if (!button) goto error;
	button->visible = true;
error:
	return;
}

void        enableButton(button_t *button)
{
	if (!button) goto error;
	button->execute = true;
error:
	return;
}

void        disableButton(button_t *button)
{
	if (!button) goto error;
	button->execute = false;
error:
	return;
}

bool        executeButton(button_t *button)
{
	if (!button || !button->execute) goto error;
	if (button->action)
		button->action(button->arg);
	return (true);
error:
	return (false);
}

void        drawButton(button_t *button)
{
	float   posX;
	float   posY;
	float   marginX;
	float   marginY;

	if (!button || !button->visible) goto error;
	posX = button->posX;
	posY = button->posY;
	setSpritePos(button->buttonSprite, posX, posY);
	setSpritePos(button->selButtonSprite, posX, posY);
	if (button->isGreyedOut) {
		button->textSprite->isGreyedOut = true;
		button->buttonSprite->isGreyedOut = true;
		button->selButtonSprite->isGreyedOut = true;
	}
	else {
		button->textSprite->isGreyedOut = false;
		button->buttonSprite->isGreyedOut = false;
		button->selButtonSprite->isGreyedOut = false;
	}
	if (button->isSelected) {
		button->textSprite->drawColor = button->selTextColor;
		drawSprite(button->selButtonSprite);
	}
	else {
		button->textSprite->drawColor = 0xFFFFFFFF;
		drawSprite(button->buttonSprite);
	}
	marginX = button->buttonSprite->width - button->textSprite->width;
	if (marginX > 1) marginX = (float)((u32)marginX / 2);
	posX += marginX;
	marginY = button->buttonSprite->height - button->textSprite->height;
	if (marginY > 1) marginY = (float)((u32)marginY / 2);
	posY += marginY;
	setSpritePos(button->textSprite, posX, posY);
	drawSprite(button->textSprite);
error:
	return;
}

bool    drawAndExecuteButton(void *button)
{
	u32 keys = hidKeysDown() | hidKeysHeld();
	drawButton((button_t *)button);
	if (!((button_t *)button)->execute) return false;
	if (buttonIsTouched((button_t *)button)) {
		if (((button_t *)button)->isSelected) {
			return executeButton((button_t *)button);
		} else {
			for (int i = 0; i < MAX_BUTTON_COUNT; i++) {
				if (buttonList[i] != NULL) {
					buttonList[i]->isSelected = false;
				}
			}
			PLAYBEEP();
			((button_t *)button)->isSelected = true;
			return false;
		}
	}
	if (keys & KEY_A) {
		if (((button_t *)button)->isSelected) {
			return executeButton((button_t *)button);
		} else {
			return false;
		}
	}
	return false;
}

bool    buttonIsTouched(button_t *button)
{
    touchPosition  touchPos;

	bool ret = false;

	u32 keys = hidKeysDown();

    if (!button || !button->visible || !(keys & KEY_TOUCH)) goto error;
    hidTouchRead(&touchPos);
    if (touchPos.px >= button->posX && touchPos.px <= button->posX + button->width)
    {
		if (touchPos.py >= button->posY && touchPos.py <= button->posY + button->height)
			ret = true;
    }
error:
    return ret;
}

void    destroyButton(button_t *button)
{
    if (!button) goto error;
	removeButtonFromList(button);
    if (button->textSprite)
        deleteSprite(button->textSprite);
    free(button);
error:
    return;
}

void    moveButton(button_t *button, float posX, float posY)
{
    if (!button) goto error;
    button->posX = posX;
    button->posY = posY;
error:
    return;
}