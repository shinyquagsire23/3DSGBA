#ifndef __UI_H__
#define __UI_H__

typedef enum _ui_result {
    NONE,
    SUCCESS,
    BACK,
    HIDE,
    EXIT
} UIResult;

UIResult uiGetResult();
void uiClear();
const char* uiSelectFile(const char* directory, const char* extension);

#endif