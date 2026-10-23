#pragma once
#include "model.h"
#include "geometry.h"
typedef enum { REMOVE_IDLE, REMOVE_HOLD, REMOVE_MENU, REMOVE_FLIGHT } RemovePhase;
typedef struct {
    RemovePhase phase;
    int slot;
    bool pressed,touchActive;
    float startX,startY;
    uint64_t started;
} WidgetRemove;
void widgetRemoveCancel(WidgetRemove *state);
void widgetRemoveObserve(WidgetRemove *state,const Model *model,bool visible,uint64_t now);
bool widgetRemoveReleaseHold(WidgetRemove *state);
UiRect widgetRemoveMenuRect(bool portrait,int slot);
void widgetRemoveTouchBegin(WidgetRemove *state,bool portrait,float x,float y);
void widgetRemoveTouchEnd(WidgetRemove *state,bool portrait,float x,float y,uint64_t now);
bool widgetRemoveStep(WidgetRemove *state,uint64_t now);
