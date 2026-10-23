#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif
#include "model.h"
#include "layout_storage.h"
#include "geometry.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void keys(Calculator*c,const char*s){for(;*s;s++)calcKey(c,*s);}
static void checksum(unsigned char*b,size_t n){uint32_t h=2166136261u;for(size_t i=0;i<n-4;i++){h^=b[i];h*=16777619u;}for(int i=0;i<4;i++)b[n-4+i]=(unsigned char)(h>>(i*8));}
static void writeBytes(const char*path,unsigned char*b,size_t n){FILE*f=fopen(path,"wb");assert(f);assert(fwrite(b,1,n,f)==n);assert(!fclose(f));}
static void dragTo(Model*m,int fromX,int fromY,int toX,int toY){modelTouchBegin(m,fromX,fromY,1000);modelTouchMove(m,fromX,fromY,1450);assert(m->editing&&m->touch.dragging);assert(modelTouchEnd(m,toX,toY,1500));assert(m->app==APP_HOME);assert(modelValidLayout(m->layout));}
int main(void){
    Model m;modelInit(&m);assert(m.app==APP_HOME&&modelValidLayout(m.layout));assert(m.layout[4]==APP_MUSIC&&m.layout[35]==APP_MAIL);
    modelMove(&m,1,0);assert(modelSelected(&m)==APP_CALENDAR&&m.selectionVisible);m.selection=3;modelMove(&m,1,0);assert(modelSelected(&m)==APP_PHONE);modelMove(&m,0,1);assert(modelSelected(&m)==APP_SAFARI);
    modelNextPage(&m);assert(modelSelected(&m)==APP_CALCULATOR);modelActivate(&m);assert(m.app==APP_CALCULATOR);keys(&m.calc,"7*8=");assert(!strcmp(m.calc.input,"56"));keys(&m.calc,"C9/0=");assert(m.calc.error);keys(&m.calc,"2+3=");assert(!strcmp(m.calc.input,"5"));keys(&m.calc,"C1.25+2.5=");assert(!strcmp(m.calc.input,"3.75"));keys(&m.calc,"C2+3*4=");assert(!strcmp(m.calc.input,"20"));
    modelInit(&m);modelTouchBegin(&m,40,30,1000);modelTouchMove(&m,40,30,1449);assert(!m.editing);modelTouchMove(&m,40,30,1450);assert(m.editing&&m.touch.dragging);modelCancelTouch(&m,true);assert(!m.touch.active&&!m.editing&&m.layout[0]==APP_FACETIME);
    modelTouchBegin(&m,40,30,1000);assert(!modelTouchEnd(&m,40,30,1100));assert(m.app==APP_FACETIME);modelHome(&m);
    modelTouchBegin(&m,40,30,1000);modelTouchMove(&m,70,30,1100);modelTouchMove(&m,70,30,2000);assert(!m.editing&&!m.touch.dragging);modelTouchEnd(&m,70,30,2100);assert(m.app==APP_HOME);
    dragTo(&m,40,30,100,30);assert(m.layout[0]==APP_CALENDAR&&m.layout[1]==APP_FACETIME);modelBack(&m);assert(!m.editing);
    modelInit(&m);dragTo(&m,40,30,288,50);assert(m.layout[0]==APP_PHONE&&m.layout[32]==APP_FACETIME);
    modelInit(&m);modelTouchBegin(&m,40,30,1000);modelTouchMove(&m,40,30,1450);modelTouchMove(&m,319,100,1500);modelTouchMove(&m,319,100,2049);assert(m.page==0);modelTouchMove(&m,319,100,2050);assert(m.page==1);modelTouchMove(&m,319,100,6000);assert(m.page==1);modelTouchMove(&m,105,30,6001);assert(modelTouchEnd(&m,105,30,6100));assert(m.layout[0]==SLOT_EMPTY&&m.layout[17]==APP_FACETIME&&modelValidLayout(m.layout));assert(m.app==APP_HOME);modelBack(&m);m.selection=1;modelActivate(&m);assert(m.app==APP_FACETIME);
    modelInit(&m);modelTouchBegin(&m,40,30,1000);modelTouchMove(&m,40,30,1450);modelTouchMove(&m,100,30,1460);modelSetLocked(&m,true);assert(!m.touch.active&&!m.editing);assert(!modelTouchEnd(&m,100,30,1500));assert(m.layout[0]==APP_FACETIME);modelActivate(&m);assert(m.locked);modelBack(&m);assert(m.locked);modelSetLocked(&m,false);
    modelInit(&m);dragTo(&m,40,30,100,30);modelBack(&m);m.selection=0;modelActivate(&m);assert(m.app==APP_CALENDAR);
    modelInit(&m);modelTouchBegin(&m,40,30,1000);modelTouchMove(&m,40,30,1450);assert(!modelTouchEnd(&m,500,500,1500));assert(m.layout[0]==APP_FACETIME&&m.editing);modelBack(&m);assert(!m.editing);
    for(int i=0;i<1000;i++){int a=(i*7)%LAYOUT_SLOTS,b=(i*13+5)%LAYOUT_SLOTS;modelSwapSlots(&m,a,b);assert(modelValidLayout(m.layout));}
    modelInit(&m);
    assert(!modelSwapSlots(&m,-1,3));assert(!modelSwapSlots(&m,1,36));assert(!modelSwapSlots(&m,20,1));assert(!modelSwapSlots(&m,0,0));
    for(int portrait=0;portrait<2;portrait++){
        for(int top=0;top<2;top++){
            int width=top?400:320;UiPoint corners[]={{0,0},{width-1,0},{0,239},{width-1,239},{width/2,120}};
            for(unsigned i=0;i<sizeof(corners)/sizeof(corners[0]);i++){
                UiPoint point=uiToVirtual(portrait,top,corners[i]);UiPoint back=uiToPhysical(portrait,top,point);
                assert(fabsf(back.x-corners[i].x)<.001f&&fabsf(back.y-corners[i].y)<.001f);
                if(portrait){assert(point.x>=0&&point.x<240);assert(point.y>=0&&point.y<width);}
            }
        }
        modelInit(&m);m.portrait=portrait;
        for(int page=0;page<2;page++){
            m.page=page;
            for(int i=0;i<20;i++){
                int slot=i<16?page*16+i:32+i-16;UiRect r=uiIconRect(portrait,slot);
                assert(modelSlotAt(&m,(int)(r.x+r.w/2),(int)(r.y+r.h/2))==slot);
            }
        }
        for(int i=0;i<20;i++){UiRect key=uiCalculatorKey(portrait,i);assert(uiContains(key,key.x+key.w/2,key.y+key.h/2));assert(key.x+key.w<=uiGeometry(portrait)->bottomWidth);assert(key.y+key.h<uiGeometry(portrait)->bottomHeight-28);}
    }
    modelInit(&m);int dx=0,dy=-1;uiLogicalDirection(true,&dx,&dy);assert(dx==-1&&dy==0);dx=1;dy=0;uiLogicalDirection(true,&dx,&dy);assert(dx==0&&dy==-1);
    modelTouchBegin(&m,40,30,1000);modelTouchMove(&m,40,30,1450);assert(m.touch.dragging);int8_t original[LAYOUT_SLOTS];memcpy(original,m.layout,sizeof(original));modelToggleOrientation(&m);assert(m.portrait&&!m.touch.active&&!m.editing);assert(!modelTouchEnd(&m,100,30,1500));assert(!memcmp(original,m.layout,sizeof(original)));
    UiRect first=uiIconRect(true,0),second=uiIconRect(true,1);int fx=(int)(first.x+first.w/2),fy=(int)(first.y+first.h/2),sx=(int)(second.x+second.w/2),sy=(int)(second.y+second.h/2);
    dragTo(&m,fx,fy,sx,sy);assert(m.layout[1]==APP_FACETIME&&m.layout[0]==APP_CALENDAR);modelBack(&m);modelTouchBegin(&m,sx,sy,2000);modelTouchEnd(&m,sx,sy,2100);assert(m.app==APP_FACETIME);modelToggleOrientation(&m);assert(!m.portrait&&m.app==APP_FACETIME);
    for(int portrait=0;portrait<2;portrait++){
        modelInit(&m);assert(modelSwapSlots(&m,4,35));m.portrait=portrait;m.page=portrait?1:0;
        UiRect music=uiIconRect(portrait,35);int mx=(int)(music.x+music.w/2),my=(int)(music.y+music.h/2);
        int edgeX=portrait?1:160,edgeY=portrait?100:1;
        modelTouchBegin(&m,mx,my,1000);assert(!modelMusicTransferCandidate(&m,edgeX,edgeY));
        modelTouchMove(&m,mx,my,1450);assert(m.touch.dragging);
        modelTouchMove(&m,edgeX,edgeY,1500);assert(m.touch.musicTransfer&&modelMusicTransferCandidate(&m,edgeX,edgeY));
        modelTouchMove(&m,edgeX,edgeY,3000);assert(m.page==(portrait?1:0));
        int8_t before[LAYOUT_SLOTS];memcpy(before,m.layout,sizeof(before));
        assert(!modelTouchEnd(&m,edgeX,edgeY,3100));assert(!memcmp(before,m.layout,sizeof(before))&&!m.touch.active);
        modelTouchBegin(&m,mx,my,4000);modelTouchMove(&m,mx,my,4450);modelSetLocked(&m,true);
        assert(!modelMusicTransferCandidate(&m,edgeX,edgeY));modelSetLocked(&m,false);
        modelTouchBegin(&m,mx,my,5000);modelTouchMove(&m,mx,my,5450);modelToggleOrientation(&m);
        assert(!modelMusicTransferCandidate(&m,edgeX,edgeY));m.portrait=portrait;
        modelTouchBegin(&m,mx,my,6000);modelTouchMove(&m,mx,my,6450);modelCancelTouch(&m,true);
        assert(!modelMusicTransferCandidate(&m,edgeX,edgeY));
        UiRect target=uiIconRect(portrait,m.page*16);int tx=(int)(target.x+target.w/2),ty=(int)(target.y+target.h/2);
        modelTouchBegin(&m,mx,my,7000);modelTouchMove(&m,mx,my,7450);assert(!modelMusicTransferCandidate(&m,tx,ty));
        assert(modelTouchEnd(&m,tx,ty,7500));assert(m.layout[m.page*16]==APP_MUSIC&&modelValidLayout(m.layout));
        modelInit(&m);m.portrait=portrait;
        target=uiIconRect(portrait,0);tx=(int)(target.x+target.w/2);ty=(int)(target.y+target.h/2);
        modelTouchBegin(&m,tx,ty,8000);modelTouchMove(&m,tx,ty,8450);
        assert(!modelMusicTransferCandidate(&m,edgeX,edgeY));
    }
    /* The new default grid location still reaches the cross-screen target. */
    for(int portrait=0;portrait<2;portrait++){
        modelInit(&m);m.portrait=portrait;UiRect music=uiIconRect(portrait,4);
        int x=(int)(music.x+music.w/2),y=(int)(music.y+music.h/2);
        modelTouchBegin(&m,x,y,1000);modelTouchMove(&m,x,y,1450);
        assert(modelMusicTransferCandidate(&m,portrait?1:160,portrait?160:1));
    }
    char dir[]="/tmp/duo-layout-test-XXXXXX";assert(mkdtemp(dir));char path[256];snprintf(path,sizeof(path),"%s/layout.bin",dir);
    assert(layoutLoad(&m,path)==LAYOUT_MISSING);assert(modelValidLayout(m.layout));assert(modelSwapSlots(&m,0,17));assert(layoutSave(&m,path));Model loaded;modelInit(&loaded);assert(layoutLoad(&loaded,path)==LAYOUT_LOADED);assert(!memcmp(m.layout,loaded.layout,LAYOUT_SLOTS));
    /* A second host replacement must succeed, and no temp file remains. */
    assert(modelSwapSlots(&m,32,1));assert(layoutSave(&m,path));assert(layoutLoad(&loaded,path)==LAYOUT_LOADED);assert(!memcmp(m.layout,loaded.layout,LAYOUT_SLOTS));
    char tempPath[270];snprintf(tempPath,sizeof(tempPath),"%s.tmp",path);
    assert(!mkdir(tempPath,0700));assert(!layoutSave(&m,path));assert(layoutLoad(&loaded,path)==LAYOUT_LOADED);assert(!memcmp(m.layout,loaded.layout,LAYOUT_SLOTS));assert(!rmdir(tempPath));
    /* Old v1 records migrate the two identities once, preserving other moves. */
    Model old;modelInit(&old);assert(modelSwapSlots(&old,4,35));
    assert(modelSwapSlots(&old,35,17));assert(modelSwapSlots(&old,4,6));
    assert(layoutSave(&old,path));
    unsigned char oldBytes[48];FILE*legacy=fopen(path,"rb");assert(legacy&&fread(oldBytes,1,48,legacy)==48);fclose(legacy);
    oldBytes[4]=1;checksum(oldBytes,48);writeBytes(path,oldBytes,48);
    assert(layoutLoad(&loaded,path)==LAYOUT_MIGRATED);
    assert(loaded.layout[6]==APP_MUSIC&&loaded.layout[17]==APP_MAIL&&loaded.layout[4]==APP_CLOCK);
    for(int i=0;i<LAYOUT_SLOTS;i++)if(i!=6&&i!=17)assert(loaded.layout[i]==old.layout[i]);
    assert(layoutSave(&loaded,path));Model migrated=loaded;
    assert(layoutLoad(&loaded,path)==LAYOUT_LOADED);assert(!memcmp(loaded.layout,migrated.layout,LAYOUT_SLOTS));
    /* Subsequent user customization is version 2 and is never swapped again. */
    assert(modelSwapSlots(&loaded,6,0));assert(layoutSave(&loaded,path));migrated=loaded;
    assert(layoutLoad(&loaded,path)==LAYOUT_LOADED);assert(!memcmp(loaded.layout,migrated.layout,LAYOUT_SLOTS));
    assert(layoutSave(&m,path));
    unsigned char bytes[49];FILE*f=fopen(path,"rb");assert(f&&fread(bytes,1,48,f)==48);fclose(f);
    bytes[12]^=1;writeBytes(path,bytes,48);assert(layoutLoad(&loaded,path)==LAYOUT_INVALID&&modelValidLayout(loaded.layout));
    bytes[12]^=1;bytes[4]=9;checksum(bytes,48);writeBytes(path,bytes,48);assert(layoutLoad(&loaded,path)==LAYOUT_INVALID);
    bytes[4]=1;bytes[8]=bytes[9];checksum(bytes,48);writeBytes(path,bytes,48);assert(layoutLoad(&loaded,path)==LAYOUT_INVALID);
    writeBytes(path,bytes,10);assert(layoutLoad(&loaded,path)==LAYOUT_INVALID);
    bytes[48]=0;writeBytes(path,bytes,49);assert(layoutLoad(&loaded,path)==LAYOUT_INVALID);
    assert(!layoutSave(&m,"/no-such-duo-directory/layout.bin"));
    loaded.layout[0]=99;assert(!layoutSave(&loaded,path));
    unlink(path);assert(!rmdir(dir));
    puts("model_test: PASS (v1-to-v2 one-time migration, defaults, Music transfer, drag, cancellation, calculator, bounded storage)");
}
