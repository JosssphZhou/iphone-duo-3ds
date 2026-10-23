#include "widget_remove.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    for(int portrait=0;portrait<2;portrait++)for(int dock=0;dock<2;dock++){
        Model m;modelInit(&m);m.portrait=portrait;
        if(dock)assert(modelSwapSlots(&m,4,35));
        int slot=dock?35:4;UiRect icon=uiIconRect(portrait,slot);int x=icon.x+icon.w/2,y=icon.y+icon.h/2;
        WidgetRemove s={0};widgetRemoveCancel(&s);
        modelTouchBegin(&m,x,y,1000);modelTouchMove(&m,x,y,1450);
        widgetRemoveObserve(&s,&m,false,1450);assert(s.phase==REMOVE_IDLE); // no widget: original drag stays active
        assert(m.touch.dragging);widgetRemoveObserve(&s,&m,true,1450);assert(s.phase==REMOVE_HOLD);
        modelTouchMove(&m,x+12,y,1460);widgetRemoveObserve(&s,&m,true,1460);
        assert(s.phase==REMOVE_IDLE&&m.touch.dragging); // movement keeps original reorder path
        modelCancelTouch(&m,true);modelTouchBegin(&m,x,y,2000);modelTouchMove(&m,x,y,2449);
        widgetRemoveObserve(&s,&m,true,2449);assert(s.phase==REMOVE_IDLE);
        modelTouchMove(&m,x,y,2450);widgetRemoveObserve(&s,&m,true,2450);assert(s.phase==REMOVE_HOLD);
        assert(widgetRemoveReleaseHold(&s)&&s.phase==REMOVE_MENU);assert(!widgetRemoveReleaseHold(&s));
        UiRect menu=widgetRemoveMenuRect(portrait,slot);const UiGeometry*g=uiGeometry(portrait);
        assert(menu.x>=0&&menu.y>=0&&menu.x+menu.w<=g->bottomWidth&&menu.y+menu.h<=g->bottomHeight);
        widgetRemoveTouchEnd(&s,portrait,menu.x+20,menu.y+20,2500);assert(s.phase==REMOVE_IDLE); // first release cannot activate
        s.phase=REMOVE_MENU;s.slot=slot;widgetRemoveTouchBegin(&s,portrait,0,0);widgetRemoveTouchEnd(&s,portrait,0,0,2600);assert(s.phase==REMOVE_IDLE);
        s.phase=REMOVE_MENU;s.slot=slot;widgetRemoveCancel(&s);assert(s.phase==REMOVE_IDLE); // B/L/lock lifecycle use same cancel
        s.phase=REMOVE_MENU;s.slot=slot;widgetRemoveTouchBegin(&s,portrait,menu.x+20,menu.y+20);
        widgetRemoveTouchEnd(&s,portrait,menu.x+20,menu.y+20,3000);assert(s.phase==REMOVE_FLIGHT&&s.slot==slot);
        widgetRemoveTouchBegin(&s,portrait,0,0);widgetRemoveTouchEnd(&s,portrait,0,0,3100);assert(s.phase==REMOVE_FLIGHT);
        assert(!widgetRemoveStep(&s,3599));assert(widgetRemoveStep(&s,3600));assert(!widgetRemoveStep(&s,3700));
        s.phase=REMOVE_FLIGHT;s.started=4000;widgetRemoveCancel(&s);assert(!widgetRemoveStep(&s,5000));
        assert(m.layout[slot]==APP_MUSIC&&modelValidLayout(m.layout));
        modelCancelTouch(&m,true);widgetRemoveCancel(&s);
        int target=slot==4?5:4;int old=m.layout[target];UiRect drop=uiIconRect(portrait,target);
        modelTouchBegin(&m,x,y,6000);modelTouchMove(&m,x,y,6450);widgetRemoveObserve(&s,&m,true,6450);
        modelTouchMove(&m,drop.x+10,drop.y+10,6460);widgetRemoveObserve(&s,&m,true,6460);assert(s.phase==REMOVE_IDLE);
        assert(modelTouchEnd(&m,drop.x+10,drop.y+10,6500));
        assert(m.layout[target]==APP_MUSIC&&m.layout[slot]==old&&modelValidLayout(m.layout));
    }
    puts("widget_remove_test: PASS (450ms hold, no-widget path, moved drag, menu release separation, outside/B cancel, flight exclusion, both orientations and dock)");
}
