#include "model.h"
#include "geometry.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void calcReset(Calculator *c) { memset(c,0,sizeof(*c)); strcpy(c->input,"0"); c->replace=true; }
void modelResetLayout(Model *m) {
    for(int i=0;i<LAYOUT_SLOTS;i++) m->layout[i]=SLOT_EMPTY;
    for(int i=0;i<16;i++) m->layout[i]=(int8_t)i;
    m->layout[16]=APP_CALCULATOR;
    for(int i=0;i<4;i++) m->layout[DOCK_FIRST+i]=(int8_t)(APP_PHONE+i);
    m->layout[4]=APP_MUSIC;m->layout[35]=APP_MAIL;
}
bool modelValidLayout(const int8_t *layout) {
    if(!layout) return false;
    bool seen[APP_COUNT]={false};
    for(int i=0;i<LAYOUT_SLOTS;i++) {
        int app=layout[i];
        if(app==SLOT_EMPTY)continue;
        if(app<0||app>=APP_COUNT||seen[app])return false;
        seen[app]=true;
    }
    for(int i=0;i<APP_COUNT;i++)if(!seen[i])return false;
    return true;
}
void modelInit(Model *m) {
    memset(m,0,sizeof(*m)); m->app=APP_HOME; m->touch.origin=-1;
    modelResetLayout(m);
    strcpy(m->note,"Hello, Duo!\nA little room to explore."); calcReset(&m->calc);
}
void modelCancelTouch(Model *m,bool exitEditing) {
    memset(&m->touch,0,sizeof(m->touch));m->touch.origin=-1;
    if(exitEditing)m->editing=false;
}
void modelSetLocked(Model *m,bool locked){modelCancelTouch(m,true);m->locked=locked;}
void modelToggleOrientation(Model*m){modelCancelTouch(m,true);m->portrait=!m->portrait;}
void modelHome(Model *m) { modelCancelTouch(m,true);m->app=APP_HOME; m->selectionVisible=false; }
void modelOpen(Model *m, App app) {
    if(app<0||app>=APP_COUNT)return;
    modelCancelTouch(m,true);m->app=app; m->selectionVisible=false;
}
int modelSelectedSlot(const Model*m) {return m->selection<16?m->page*16+m->selection:DOCK_FIRST+m->selection-16;}
App modelSelected(const Model *m) {
    int slot=modelSelectedSlot(m);
    if(slot<0||slot>=LAYOUT_SLOTS)return APP_HOME;
    return (App)m->layout[slot];
}
void modelActivate(Model *m) {
    if(m->locked)return;
    if(m->app==APP_HOME&&!m->editing)modelOpen(m,modelSelected(m));
}
void modelBack(Model *m) {
    if(m->editing||m->touch.active){modelCancelTouch(m,true);return;}
    if(m->locked)return;
    modelHome(m);
}
void modelNextPage(Model *m) {
    if(m->app==APP_HOME&&!m->locked){m->page=1-m->page;m->selection=0;m->selectionVisible=false;}
}
void modelMove(Model *m,int dx,int dy) {
    if(m->app!=APP_HOME||m->locked||m->touch.dragging)return;
    m->selectionVisible=true;
    if(m->selection>=16) { if(dx<0) m->selection=(m->selection-16)*4+3; else m->selection=16+(m->selection-16+dy+4)%4; }
    else { int row=m->selection/4,col=m->selection%4; if(dx>0&&col==3)m->selection=16+row; else {col=(col+dx+4)%4;row=(row+dy+4)%4;m->selection=row*4+col;} }
}
int modelSlotAt(const Model*m,int x,int y) {
    for(int i=0;i<20;i++){
        int slot=i<16?m->page*16+i:DOCK_FIRST+i-16;
        if(uiContains(uiSlotRect(m->portrait,slot),x,y))return slot;
    }
    return -1;
}
bool modelSwapSlots(Model*m,int from,int to) {
    if(from<0||from>=LAYOUT_SLOTS||to<0||to>=LAYOUT_SLOTS||from==to||m->layout[from]==SLOT_EMPTY)return false;
    int8_t app=m->layout[from];m->layout[from]=m->layout[to];m->layout[to]=app;
    return true;
}
void modelTouchBegin(Model*m,int x,int y,uint64_t now) {
    modelCancelTouch(m,false);
    if(m->locked||m->app!=APP_HOME)return;
    m->selectionVisible=false;
    TouchGesture*t=&m->touch;t->active=true;t->origin=modelSlotAt(m,x,y);
    t->startX=t->x=x;t->startY=t->y=y;t->startedAt=now;
    if(t->origin>=0&&m->layout[t->origin]!=SLOT_EMPTY&&m->editing)t->dragging=true;
}
bool modelMusicTransferCandidate(const Model*m,int x,int y){
    if(m->locked||m->app!=APP_HOME||!m->touch.active||!m->touch.dragging||m->touch.origin<0||m->touch.origin>=LAYOUT_SLOTS)return false;
    if(m->layout[m->touch.origin]!=APP_MUSIC)return false;
    const UiGeometry*g=uiGeometry(m->portrait);
    if(x<0||y<0||x>=g->bottomWidth||y>=g->bottomHeight)return false;
    return m->portrait?x<=16:y<=16;
}
void modelTouchMove(Model*m,int x,int y,uint64_t now) {
    TouchGesture*t=&m->touch;if(!t->active)return;
    t->x=x;t->y=y;
    if(abs(x-t->startX)>8||abs(y-t->startY)>8)t->moved=true;
    if(!t->dragging&&!t->moved&&t->origin>=0&&m->layout[t->origin]!=SLOT_EMPTY&&now>=t->startedAt&&now-t->startedAt>=LONG_PRESS_MS){
        m->editing=true;t->dragging=true;
    }
    if(!t->dragging)return;
    t->musicTransfer=modelMusicTransferCandidate(m,x,y);
    if(t->musicTransfer){t->edge=0;t->edgeLatched=false;t->edgeStartedAt=now;return;}
    const UiGeometry*g=uiGeometry(m->portrait);
    int edge=(y>=0&&y<g->bottomHeight-30)?(x<=12?-1:(x>=g->bottomWidth-13?1:0)):0;
    if(edge!=t->edge){t->edge=edge;t->edgeStartedAt=now;t->edgeLatched=false;}
    if(!edge){t->edgeLatched=false;return;}
    if(!t->edgeLatched&&now>=t->edgeStartedAt&&now-t->edgeStartedAt>=EDGE_HOLD_MS){
        if((edge>0&&m->page==0)||(edge<0&&m->page==1)){m->page=1-m->page;m->selection=0;}
        t->edgeLatched=true;
    }
}
bool modelTouchEnd(Model*m,int x,int y,uint64_t now) {
    if(!m->touch.active)return false;
    modelTouchMove(m,x,y,now);
    TouchGesture gesture=m->touch;
    bool changed=false;
    if(gesture.musicTransfer){modelCancelTouch(m,true);return false;}
    if(gesture.dragging){changed=modelSwapSlots(m,gesture.origin,modelSlotAt(m,x,y));modelCancelTouch(m,false);return changed;}
    modelCancelTouch(m,false);
    if(m->editing)return false;
    int dx=x-gesture.startX,dy=y-gesture.startY;
    if(abs(dx)>40&&abs(dx)>abs(dy)){modelNextPage(m);return false;}
    if(!gesture.moved&&gesture.origin>=0&&modelSlotAt(m,x,y)==gesture.origin)modelOpen(m,(App)m->layout[gesture.origin]);
    else if(y>=uiGeometry(m->portrait)->bottomHeight-15&&abs(dx)<8&&abs(dy)<8)modelNextPage(m);
    return false;
}
static bool evaluate(Calculator*c,double rhs) {
    double out=rhs;
    if(c->op=='+')out=c->lhs+rhs;else if(c->op=='-')out=c->lhs-rhs;
    else if(c->op=='*')out=c->lhs*rhs;else if(c->op=='/'){if(rhs==0) return false;out=c->lhs/rhs;}
    if(!isfinite(out)||fabs(out)>1e15) return false;
    c->lhs=out; snprintf(c->input,sizeof(c->input),"%.10g",out);return true;
}
void calcKey(Calculator*c,char key) {
    if(key=='C'){calcReset(c);return;}
    if(c->error){calcReset(c);if(!(key>='0'&&key<='9'))return;}
    if((key>='0'&&key<='9')||key=='.'){
        if(c->replace){strcpy(c->input,key=='.'?"0":"");c->replace=false;}
        if(key=='.'&&strchr(c->input,'.'))return;
        size_t n=strlen(c->input);if(n<15){if(n==1&&c->input[0]=='0'&&key!='.')n=0;c->input[n]=key;c->input[n+1]=0;}return;
    }
    if(key=='<'){if(!c->replace){size_t n=strlen(c->input);if(n)c->input[n-1]=0;if(!c->input[0]||!strcmp(c->input,"-"))strcpy(c->input,"0");}return;}
    double value=strtod(c->input,NULL);
    if(key=='N'){value=-value;snprintf(c->input,sizeof(c->input),"%.10g",value);return;}
    if(key=='%'){value/=100;snprintf(c->input,sizeof(c->input),"%.10g",value);return;}
    if(key=='+'||key=='-'||key=='*'||key=='/'||key=='='){
        if(c->op&&!c->replace) {if(!evaluate(c,value)){strcpy(c->input,"Error");c->error=true;c->op=0;return;}}
        else c->lhs=value;
        c->op=key=='='?0:key;c->replace=true;
    }
}
