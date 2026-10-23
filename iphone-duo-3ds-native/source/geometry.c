#include "geometry.h"

static const UiGeometry landscape={400,240,320,240,13,10,56.5f,53.25f,64.5f,54.25f,9.75f,2.125f,269,32,44,41,41,4.5f,5};
static const UiGeometry book={240,400,240,320,8,16,41.5f,64,47.5f,72,2.25f,7.5f,198,74,31,32,44,-.5f,0};
const UiGeometry*uiGeometry(bool portrait){return portrait?&book:&landscape;}
UiRect uiSlotRect(bool portrait,int slot){
    const UiGeometry*g=uiGeometry(portrait);
    if(slot<0||slot>=36)return(UiRect){0,0,0,0};
    if(slot>=32)return(UiRect){g->dockX,g->dockY+(slot-32)*g->dockStep,g->dockCellWidth,g->dockCellHeight};
    int local=slot%16;return(UiRect){g->gridX+(local%4)*g->columnStep,g->gridY+(local/4)*g->rowStep,g->cellWidth,g->cellHeight};
}
UiRect uiIconRect(bool portrait,int slot){
    const UiGeometry*g=uiGeometry(portrait);UiRect r=uiSlotRect(portrait,slot);
    if(slot>=32){r.x+=g->dockIconInsetX;r.y+=g->dockIconInsetY;r.w=r.h=32;}
    else {r.x+=g->iconInsetX;r.y+=g->iconInsetY;r.w=r.h=37;}
    return r;
}
UiRect uiCalculatorKey(bool portrait,int key){
    const UiGeometry*g=uiGeometry(portrait);float width=(g->bottomWidth-38)/4.0f,height=(g->bottomHeight-60)/5.0f;
    return(UiRect){10+(key%4)*(width+6),8+(key/4)*(height+5),width,height};
}
bool uiContains(UiRect r,float x,float y){return r.w>0&&r.h>0&&x>=r.x&&x<r.x+r.w&&y>=r.y&&y<r.y+r.h;}
UiPoint uiToPhysical(bool portrait,bool top,UiPoint p){if(!portrait)return p;return(UiPoint){(top?399:319)-p.y,p.x};}
UiPoint uiToVirtual(bool portrait,bool top,UiPoint p){if(!portrait)return p;return(UiPoint){p.y,(top?399:319)-p.x};}
void uiLogicalDirection(bool portrait,int*dx,int*dy){if(portrait){int old=*dx;*dx=*dy;*dy=-old;}}
