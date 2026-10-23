#include "widget_remove.h"
#include <math.h>
#include <string.h>
void widgetRemoveCancel(WidgetRemove*s){memset(s,0,sizeof(*s));s->slot=-1;}
void widgetRemoveObserve(WidgetRemove*s,const Model*m,bool visible,uint64_t now){
    if(s->phase==REMOVE_HOLD&&(!m->touch.active||m->touch.moved)){widgetRemoveCancel(s);return;}
    if(s->phase!=REMOVE_IDLE||!visible||m->locked||m->app!=APP_HOME)return;
    const TouchGesture*t=&m->touch;
    if(t->active&&!t->moved&&t->origin>=0&&t->origin<LAYOUT_SLOTS&&m->layout[t->origin]==APP_MUSIC&&now>=t->startedAt&&now-t->startedAt>=LONG_PRESS_MS){s->phase=REMOVE_HOLD;s->slot=t->origin;}
}
bool widgetRemoveReleaseHold(WidgetRemove*s){if(s->phase!=REMOVE_HOLD)return false;s->phase=REMOVE_MENU;return true;}
UiRect widgetRemoveMenuRect(bool portrait,int slot){
    const UiGeometry*g=uiGeometry(portrait);UiRect icon=uiIconRect(portrait,slot);
    float x=icon.x+icon.w/2-80,y=icon.y-54;
    if(x<8)x=8;
    if(x>g->bottomWidth-168)x=g->bottomWidth-168;
    if(y<8)y=icon.y+icon.h+18;
    if(y>g->bottomHeight-52)y=g->bottomHeight-52;
    return(UiRect){roundf(x),roundf(y),160,44};
}
void widgetRemoveTouchBegin(WidgetRemove*s,bool portrait,float x,float y){
    if(s->phase!=REMOVE_MENU)return;
    s->touchActive=true;s->startX=x;s->startY=y;s->pressed=uiContains(widgetRemoveMenuRect(portrait,s->slot),x,y);
}
void widgetRemoveTouchEnd(WidgetRemove*s,bool portrait,float x,float y,uint64_t now){
    if(s->phase!=REMOVE_MENU)return;
    bool activate=s->touchActive&&s->pressed&&fabsf(x-s->startX)<8&&fabsf(y-s->startY)<8&&uiContains(widgetRemoveMenuRect(portrait,s->slot),x,y);
    if(activate){s->phase=REMOVE_FLIGHT;s->started=now;s->touchActive=false;s->pressed=false;}
    else widgetRemoveCancel(s);
}
bool widgetRemoveStep(WidgetRemove*s,uint64_t now){
    if(s->phase!=REMOVE_FLIGHT||now<s->started||now-s->started<600)return false;
    widgetRemoveCancel(s);return true;
}
