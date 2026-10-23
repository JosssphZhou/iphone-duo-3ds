#pragma once
#include <stdbool.h>

typedef struct { float x,y,w,h; } UiRect;
typedef struct { float x,y; } UiPoint;
typedef struct {
    int topWidth,topHeight,bottomWidth,bottomHeight;
    float gridX,gridY,cellWidth,cellHeight,columnStep,rowStep,iconInsetX,iconInsetY;
    float dockX,dockY,dockCellWidth,dockCellHeight,dockStep,dockIconInsetX,dockIconInsetY;
} UiGeometry;
const UiGeometry *uiGeometry(bool portrait);
UiRect uiSlotRect(bool portrait,int slot);
UiRect uiIconRect(bool portrait,int slot);
UiRect uiCalculatorKey(bool portrait,int key);
bool uiContains(UiRect rect,float x,float y);
UiPoint uiToPhysical(bool portrait,bool top,UiPoint point);
UiPoint uiToVirtual(bool portrait,bool top,UiPoint point);
void uiLogicalDirection(bool portrait,int *dx,int *dy);
