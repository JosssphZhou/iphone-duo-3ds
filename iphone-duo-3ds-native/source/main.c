#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include "layout_storage.h"
#include "model.h"
#include "geometry.h"
#include "orientation.h"
#include "assets.h"
#include "showcase.h"
#include "showcase_assets.h"
#include "interaction.h"
#include "official_assets.h"
#include "lockscreen.h"
#include "widget_remove.h"
#include "language.h"

static Model model;
static Showcase showcase;
static C2D_SpriteSheet showSheet,officialSheet;
static const OfficialAppAssets *officialPage;
static const ShowPageAssets*showPage;
static int showFocus=-1;
static char showFocusAction[48];
static UiGesture pointer;
static LockMotion lockMotion;
static WidgetRemove widgetRemoval;
static DuoLanguage language=DUO_ENGLISH;
static LanguageKeys languageKeys;
#define ENGLISH_BASE TOTAL_IMAGES
static struct {bool active,closing;uint64_t started;UiPoint anchor;} transition;
static unsigned lastAtlasLoadMs;
static bool musicWidgetVisible;
static struct {bool active;uint64_t started;UiPoint release;} musicFlight;
static void drawMusicDesktopTop(void);
static void drawMusicDesktopBottom(void);
static int touchApp;
static bool touchPortrait;
static const ShowPageAssets*touchPage;
static C2D_SpriteSheet sheet,portraitSheet,musicWidgetSheet,labelSheet,iconNativeSheet,lockSheet,englishSheets[4];
static u32 white,ink,muted,blue;
static const char *names[]={"FaceTime","Calendar","Photos","Camera","Mail","Notes","Clock","Maps","TV","News","App Store","Games","Health","Wallet","Siri","Settings","Phone","Safari","Messages","Music","Calculator"};
static const char *keyLabels[]={"AC","+/-","%","/","7","8","9","x","4","5","6","-","1","2","3","+","0",".","DEL","="};
static const char keyValues[]={'C','N','%','/','7','8','9','*','4','5','6','-','1','2','3','+','0','.','<','='};
static unsigned long frame;
static uint64_t nowMs;
static char statusMessage[96];
static uint64_t statusUntil;
static bool wasHomeTouch,ignoreTouchUntilRelease;
static OrientationFilter orientation;
static bool accelerometerEnabled,accelerometerFailed;
static bool sensorDiagnostics,diagnosticTouchBusy,haveAcceleration;
static Result accelerometerEnableResult=(Result)-1;
static accelVector lastAcceleration;
static uint64_t lastAccelerationRead;
#define BUILD_VERSION "official previews"
#define OFFICIAL_PREVIEW_ONLY 1
static const char *layoutPath="sdmc:/3ds/iphone-duo/layout.bin";
static void setStatus(const char*message,unsigned duration){snprintf(statusMessage,sizeof(statusMessage),"%s",message);statusUntil=nowMs+duration;}
static void disableAccelerometer(void){
    if(accelerometerEnabled&&R_SUCCEEDED(HIDUSER_DisableAccelerometer()))accelerometerEnabled=false;
}
static bool setAutomaticRotation(bool enabled){
    orientationSetAutomatic(&orientation,false);
    if(!enabled){disableAccelerometer();return true;}
    if(!accelerometerEnabled){
        Result result=HIDUSER_EnableAccelerometer();
        accelerometerEnableResult=result;
        if(R_FAILED(result)){accelerometerFailed=true;setStatus("Auto rotate unavailable. L still works.",5000);return false;}
        accelerometerEnabled=true;
    }
    accelerometerFailed=false;orientationSetAutomatic(&orientation,true);return true;
}
static void updateAutomaticRotation(bool interactionBusy){
    diagnosticTouchBusy=interactionBusy;
    if(!accelerometerEnabled||!orientation.automatic)return;
    hidAccelRead(&lastAcceleration);haveAcceleration=true;lastAccelerationRead=nowMs;
    bool nextPortrait=model.portrait;
    if(orientationUpdate(&orientation,lastAcceleration.x,lastAcceleration.y,lastAcceleration.z,nowMs,interactionBusy,model.portrait,&nextPortrait)&&nextPortrait!=model.portrait){
        modelToggleOrientation(&model);wasHomeTouch=false;
    }
}
static void saveLayout(void){
    mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/iphone-duo",0777);
    bool saved=layoutSave(&model,layoutPath);setStatus(saved?"Layout saved":"Could not save layout. Check SD card.",saved?900:5000);
}

static void rect(float x,float y,float w,float h,u32 c) {C2D_DrawRectSolid(x,y,0,w,h,c);}
static C2D_Image getImage(int index){if(index>=ENGLISH_BASE)return C2D_SpriteSheetGetImage(englishSheets[(index-ENGLISH_BASE)/4],(index-ENGLISH_BASE)%4);if(index>=LABEL_BASE&&index<LABEL_BASE+APP_COUNT)return C2D_SpriteSheetGetImage(labelSheet,index-LABEL_BASE+(language==DUO_ENGLISH?APP_COUNT:0));return index>=TOP_PORTRAIT_BASE?C2D_SpriteSheetGetImage(portraitSheet,index-TOP_PORTRAIT_BASE):C2D_SpriteSheetGetImage(sheet,index);}
static void image(int i,float x,float y,float w,float h) {
    C2D_Image img=getImage(i);
    C2D_DrawImageAt(img,x,y,0,NULL,w/img.subtex->width,h/img.subtex->height);
}
static float textWidth(const char*s,float scale){float w=0;for(;*s;s++)if(*s>=32&&*s<127)w+=glyph_widths[(unsigned char)*s-32]*scale;return w;}
static void text(const char*s,float x,float y,float scale,u32 color) {
    C2D_ImageTint tint;C2D_PlainImageTint(&tint,color,1);
    float left=x;
    for(;*s;s++){
        if(*s=='\n'){x=left;y+=25*scale;continue;}
        unsigned char c=(unsigned char)*s;if(c<32||c>=127)c='?';
        C2D_Image img=C2D_SpriteSheetGetImage(sheet,GLYPH_BASE+c-32);
        C2D_DrawImageAt(img,x,y,0,&tint,scale,scale);x+=glyph_widths[c-32]*scale;
    }
}
static void center(const char*s,float cx,float y,float scale,u32 color){text(s,cx-textWidth(s,scale)/2,y,scale,color);}
static void outline(float x,float y,float w,float h,u32 c){rect(x,y,w,2,c);rect(x,y+h-2,w,2,c);rect(x,y,2,h,c);rect(x+w-2,y,2,h,c);}
static void button(float x,float y,float w,float h,const char*label,bool primary){
    u32 c=primary?blue:C2D_Color32(226,231,238,255);rect(x,y,w,h,c);
    float scale=fminf(.6f,(w-12)/fmaxf(1,textWidth(label,1)));center(label,x+w/2,y+(h-25*scale)/2,scale,primary?white:ink);
}
static void buttonRect(UiRect r,const char*label,bool primary){button(r.x,r.y,r.w,r.h,label,primary);}
static void fitText(const char*s,float x,float y,float width,float scale,u32 color){text(s,x,y,fminf(scale,width/fmaxf(1,textWidth(s,1))),color);}
static const char*orientationName(OrientationTarget target){
    return target==ORIENTATION_BOOK?"BOOK":target==ORIENTATION_LANDSCAPE?"LANDSCAPE":"NONE";
}
static void drawSensorDiagnostics(void){
    if(!sensorDiagnostics)return;
    const UiGeometry*g=uiGeometry(model.portrait);float w=g->topWidth,h=g->topHeight;
    rect(0,0,w,h,C2D_Color32(14,20,29,250));
    char line[128];float y=12;
    fitText("iPhone Duo " BUILD_VERSION,12,y,w-24,.72f,white);y+=25;
    snprintf(line,sizeof(line),"Enable Result: 0x%08lX",(unsigned long)(u32)accelerometerEnableResult);
    fitText(line,12,y,w-24,.62f,white);y+=20;
    snprintf(line,sizeof(line),"automatic: %s   enabled: %s",orientation.automatic?"ON":"OFF",accelerometerEnabled?"YES":"NO");
    fitText(line,12,y,w-24,.62f,white);y+=20;
    bool sampling=orientation.automatic&&accelerometerEnabled;
    fitText(sampling?"sampling: LIVE":"sampling: STOPPED (last read retained)",12,y,w-24,.62f,white);y+=20;
    snprintf(line,sizeof(line),"X %d   Y %d   Z %d",lastAcceleration.x,lastAcceleration.y,lastAcceleration.z);
    fitText(haveAcceleration?line:"X --   Y --   Z -- (no read yet)",12,y,w-24,.72f,white);y+=23;
    snprintf(line,sizeof(line),"classifier: %s   view: %s",haveAcceleration?orientationName(orientationClassify(lastAcceleration.x,lastAcceleration.y,lastAcceleration.z)):"NO SAMPLE",model.portrait?"BOOK":"LANDSCAPE");
    fitText(line,12,y,w-24,.62f,white);y+=20;
    snprintf(line,sizeof(line),"touch busy: %s",diagnosticTouchBusy?"YES":"NO");
    fitText(line,12,y,w-24,.62f,white);y+=20;
    uint64_t elapsed=orientation.candidate!=ORIENTATION_UNKNOWN&&nowMs>=orientation.candidateSince?nowMs-orientation.candidateSince:0;
    snprintf(line,sizeof(line),"candidate: %s  %llums / %d",orientationName(orientation.candidate),(unsigned long long)elapsed,ORIENTATION_STABLE_MS);
    fitText(line,12,y,w-24,.62f,white);y+=20;
    snprintf(line,sizeof(line),"read age: %llums  samples: %u",(unsigned long long)(haveAcceleration&&nowMs>=lastAccelerationRead?nowMs-lastAccelerationRead:0),orientation.sampleCount);
    fitText(haveAcceleration?line:"read age: --",12,y,w-24,.55f,muted);
    snprintf(line,sizeof(line),"last page load: %u ms",lastAtlasLoadMs);fitText(line,12,y+14,w-24,.45f,muted);
    fitText("R: close panel. Settings: enable auto rotation.",12,h-22,w-24,.48f,muted);
}
static void wrappedText(const char*s,float x,float y,float width,float height,float scale,u32 color){
    char line[128];size_t n=0;float used=0,top=y;
    for(;;s++){
        unsigned char ch=(unsigned char)*s;float advance=ch>=32&&ch<127?glyph_widths[ch-32]*scale:0;
        if(ch==0||ch=='\n'||(used+advance>width&&n)){
            line[n]=0;if(y+25*scale>top+height)return;text(line,x,y,scale,color);y+=25*scale;n=0;used=0;if(ch==0)return;if(ch=='\n')continue;
        }
        if(n<sizeof(line)-1){line[n++]=(char)ch;used+=advance;}
    }
}
static int topBackground(void){if(language==DUO_ENGLISH)return ENGLISH_BASE+(model.portrait?2:0)+(model.night?1:0);return model.portrait?TOP_PORTRAIT_BASE+(model.night?1:0):(model.night?2:0);}
static int bottomBackground(void){if(language==DUO_ENGLISH)return ENGLISH_BASE+8+(model.portrait?4:0)+model.page*2+(model.night?1:0);return model.portrait?BOTTOM_PORTRAIT_BASE+model.page*2+(model.night?1:0):(model.page?PAGE2_BASE+(model.night?1:0):(model.night?3:1));}
static void imageCover(int index,float x,float y,float w,float h){
    C2D_Image im=getImage(index);float scale=fmaxf(w/im.subtex->width,h/im.subtex->height);
    image(index,x+(w-im.subtex->width*scale)/2,y+(h-im.subtex->height*scale)/2,im.subtex->width*scale,im.subtex->height*scale);
}
static void imageContain(int index,UiRect r){
    rect(r.x,r.y,r.w,r.h,C2D_Color32(16,20,27,255));
    C2D_Image im=getImage(index);float scale=fminf(r.w/im.subtex->width,r.h/im.subtex->height);
    float w=im.subtex->width*scale,h=im.subtex->height*scale;
    image(index,r.x+(r.w-w)/2,r.y+(r.h-h)/2,w,h);
}
typedef enum {CONTROL_HOME,CONTROL_PHOTO_PREV,CONTROL_PHOTO_NEXT,CONTROL_THEME,CONTROL_AUTO_ROTATE,CONTROL_LOCK,CONTROL_NOTES} Control;
static UiRect controlRect(Control control){
    const UiGeometry*g=uiGeometry(model.portrait);float w=g->bottomWidth,h=g->bottomHeight;
    switch(control){
    case CONTROL_HOME:return(UiRect){10,h-28,90,22};
    case CONTROL_PHOTO_PREV:return(UiRect){18,model.portrait?220:155,(w-48)/2,40};
    case CONTROL_PHOTO_NEXT:return(UiRect){w/2+6,model.portrait?220:155,(w-48)/2,40};
    case CONTROL_THEME:return(UiRect){18,model.portrait?64:54,w-36,model.portrait?48:36};
    case CONTROL_AUTO_ROTATE:return(UiRect){18,model.portrait?132:100,w-36,model.portrait?48:36};
    case CONTROL_LOCK:return(UiRect){18,h-73,w-36,35};
    case CONTROL_NOTES:return(UiRect){18,78,w-36,48};
    }return(UiRect){0,0,0,0};
}
static void appTitle(void){text(names[model.app],18,13,.55f,muted);rect(18,38,uiGeometry(model.portrait)->topWidth-36,1,C2D_Color32(218,223,230,255));}
static void timeText(char*out,size_t n){time_t raw=time(NULL);struct tm tmv;localtime_r(&raw,&tmv);snprintf(out,n,"%02d:%02d",tmv.tm_hour,tmv.tm_min);}
static void widgetImage(int index,UiRect r){
    C2D_Image im=C2D_SpriteSheetGetImage(musicWidgetSheet,index);
    C2D_DrawImageAt(im,r.x,r.y,0,NULL,r.w/im.subtex->width,r.h/im.subtex->height);
}
static void homeTop(void){
    const UiGeometry*g=uiGeometry(model.portrait);
    if(musicWidgetVisible||musicFlight.active||model.touch.musicTransfer){
        if(language==DUO_ENGLISH)image(ENGLISH_BASE+4+(model.portrait?2:0)+(model.night?1:0),0,0,g->topWidth,g->topHeight);
        else widgetImage((model.portrait?3:1)+(model.night?1:0),(UiRect){0,0,g->topWidth,g->topHeight});
    }
    else image(topBackground(),0,0,g->topWidth,g->topHeight);
    drawMusicDesktopTop();
}
static void rotatedImage(int index,float x,float y,float w,float h,float angle,float alpha) {
    C2D_Image im=getImage(index);
    C2D_DrawParams params={.pos={x+w/2,y+h/2,w,h},.center={w/2,h/2},.depth=0,.angle=angle};
    C2D_ImageTint tint;C2D_PlainImageTint(&tint,C2D_Color32(255,255,255,(unsigned char)(alpha*255)),0);
    C2D_DrawImage(im,&params,&tint);
}
static void slotPosition(int slot,float*x,float*y,float*size) {
    UiRect r=uiIconRect(model.portrait,slot);*x=r.x;*y=r.y;*size=r.w;
}
static void drawApp(int app,float x,float y,float size,bool label,float angle,float alpha) {
    if(app<0||app>=APP_COUNT)return;
    if(angle==0&&alpha==1&&(size==37||size==32)){
        C2D_Image im=C2D_SpriteSheetGetImage(iconNativeSheet,app+(size==32?APP_COUNT:0));
        C2D_DrawImageAt(im,roundf(x),roundf(y),0,NULL,1,1);
    }else rotatedImage(ICON_BASE+app,x,y,size,size,angle,alpha);
    if(label)rotatedImage(LABEL_BASE+app,roundf(x+(size-56)/2),roundf(y+38),56,16,angle,alpha);
}
static UiRect musicWidgetRect(void){
    return model.portrait?(UiRect){76,285,88,88}:(UiRect){238.5f,120,102,102};
}
static UiPoint lerpPoint(UiPoint a,UiPoint b,float t){return(UiPoint){a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t};}
static UiPoint musicWidgetIconCenter(void){UiRect r=musicWidgetRect();return(UiPoint){r.x+r.w/2,r.y+r.h/2};}
static void drawMusicCard(float amount){
    UiRect full=musicWidgetRect();float size=42+(full.w-42)*amount;
    widgetImage(0,(UiRect){full.x+(full.w-size)/2,full.y+(full.h-size)/2,size,size});
}
static void drawMusicDesktopTop(void){
    const UiGeometry*g=uiGeometry(model.portrait);
    if(widgetRemoval.phase==REMOVE_FLIGHT){
        uint64_t elapsed=nowMs>=widgetRemoval.started?nowMs-widgetRemoval.started:0;
        if(elapsed<180){drawMusicCard(1-transitionEase(elapsed/180.f));return;}
        if(elapsed<330){
            UiRect icon=uiIconRect(model.portrait,widgetRemoval.slot);
            UiPoint target={icon.x+icon.w/2,icon.y+icon.h/2};
            const UiGeometry*b=uiGeometry(model.portrait);
            UiPoint seam=model.portrait?(UiPoint){g->topWidth+24,target.y*g->topHeight/b->bottomHeight}:(UiPoint){target.x*g->topWidth/b->bottomWidth,g->topHeight+24};
            UiPoint at=lerpPoint(musicWidgetIconCenter(),seam,transitionEase((elapsed-180)/150.f));
            image(ICON_BASE+APP_MUSIC,at.x-21,at.y-21,42,42);
        }
        return;
    }
    if(musicWidgetVisible&&!musicFlight.active)drawMusicCard(1);
    if(model.touch.musicTransfer){
        if(model.portrait)rect(g->topWidth-5,8,5,g->topHeight-16,C2D_Color32(255,117,164,235));
        else rect(8,g->topHeight-5,g->topWidth-16,5,C2D_Color32(255,117,164,235));
        UiRect r=musicWidgetRect();widgetImage(0,r);outline(r.x-2,r.y-2,r.w+4,r.h+4,C2D_Color32(247,83,140,255));
    }
    if(!musicFlight.active)return;
    uint64_t elapsed=nowMs>=musicFlight.started?nowMs-musicFlight.started:0;
    if(elapsed<180)return;
    UiPoint dest=musicWidgetIconCenter();
    if(elapsed<360){
        UiPoint from=model.portrait?(UiPoint){g->topWidth+28,musicFlight.release.y*g->topHeight/uiGeometry(true)->bottomHeight}:(UiPoint){musicFlight.release.x*g->topWidth/uiGeometry(false)->bottomWidth,g->topHeight+28};
        float progress=transitionEase((elapsed-180)/180.f);
        UiPoint at=lerpPoint(from,dest,progress);float size=44-2*progress;
        image(ICON_BASE+APP_MUSIC,at.x-size/2,at.y-size/2,size,size);
    }else drawMusicCard(transitionEase((elapsed-360)/240.f));
}
static void drawMusicDesktopBottom(void){
    const UiGeometry*g=uiGeometry(model.portrait);
    if(widgetRemoval.phase==REMOVE_FLIGHT){
        uint64_t elapsed=nowMs>=widgetRemoval.started?nowMs-widgetRemoval.started:0;
        if(elapsed>=330){
            UiRect icon=uiIconRect(model.portrait,widgetRemoval.slot);
            UiPoint dest={icon.x+icon.w/2,icon.y+icon.h/2};
            UiPoint from=model.portrait?(UiPoint){-24,dest.y}:(UiPoint){dest.x,-24};
            float size=icon.w;UiPoint at=dest;
            if(elapsed<480){float t=transitionEase((elapsed-330)/150.f);at=lerpPoint(from,dest,t);size=42+(icon.w-42)*t;}
            else size*=1+.14f*sinf(fminf(1,(elapsed-480)/120.f)*3.14159265f);
            image(ICON_BASE+APP_MUSIC,at.x-size/2,at.y-size/2,size,size);
        }
        return;
    }
    if(model.touch.musicTransfer){
        if(model.portrait)rect(0,0,5,g->bottomHeight,C2D_Color32(255,117,164,235));
        else rect(0,0,g->bottomWidth,5,C2D_Color32(255,117,164,235));
    }
    if(!musicFlight.active||nowMs<musicFlight.started||nowMs-musicFlight.started>=180)return;
    UiPoint out=model.portrait?(UiPoint){-28,musicFlight.release.y}:(UiPoint){musicFlight.release.x,-28};
    UiPoint at=lerpPoint(musicFlight.release,out,transitionEase((nowMs-musicFlight.started)/180.f));
    image(ICON_BASE+APP_MUSIC,at.x-21,at.y-21,42,42);
}
static void startMusicFlight(int x,int y){
    musicFlight.active=true;musicFlight.started=nowMs;musicFlight.release=(UiPoint){x,y};
    modelCancelTouch(&model,true);gestureCancel(&pointer);wasHomeTouch=false;ignoreTouchUntilRelease=true;
}
static void drawWidgetRemoveMenu(void){
    if(widgetRemoval.phase!=REMOVE_HOLD&&widgetRemoval.phase!=REMOVE_MENU)return;
    UiRect r=widgetRemoveMenuRect(model.portrait,widgetRemoval.slot);
    u32 color=widgetRemoval.pressed?C2D_Color32(234,220,224,255):C2D_Color32(252,244,247,255);
    // Keep the rounded fill on one rectangular draw path under either
    // orientation, without seams between circle and rectangle geometry.
    const float radius=9;
    for(int row=0;row<(int)r.h;row++){
        float dy=row<radius?radius-row-.5f:(row>=r.h-radius?row+.5f-(r.h-radius):0);
        float inset=dy>0?ceilf(radius-sqrtf(radius*radius-dy*dy)):0;
        rect(r.x+inset+1,r.y+row+3,r.w-2*inset,1,C2D_Color32(0,0,0,75));
    }
    for(int row=0;row<(int)r.h;row++){
        float dy=row<radius?radius-row-.5f:(row>=r.h-radius?row+.5f-(r.h-radius):0);
        float inset=dy>0?ceilf(radius-sqrtf(radius*radius-dy*dy)):0;
        rect(r.x+inset,r.y+row,r.w-2*inset,1,color);
    }
    center("Remove Widget",r.x+r.w/2,r.y+13,.65f,C2D_Color32(213,46,70,255));
}
static void homeBottom(void){
    const UiGeometry*g=uiGeometry(model.portrait);
    image(bottomBackground(),0,0,g->bottomWidth,g->bottomHeight);
    for(int i=0;i<20;i++){
        int slot=i<16?model.page*16+i:DOCK_FIRST+i-16;
        int app=model.layout[slot];float x,y,size;slotPosition(slot,&x,&y,&size);
        bool dragging=model.touch.dragging&&slot==model.touch.origin;
        if(widgetRemoval.phase==REMOVE_FLIGHT&&app==APP_MUSIC)continue;
        if(app==SLOT_EMPTY){if(model.editing)outline(x+6,y+6,size-12,size-12,C2D_Color32(255,255,255,65));continue;}
        if(dragging)continue;
        float angle=model.editing?.035f*sinf((float)nowMs/110+slot*1.7f):0;
        bool pressed=wasHomeTouch&&model.touch.active&&!model.touch.dragging&&slot==model.touch.origin;
        float shrink=pressed?size*.03f:0;drawApp(app,x+shrink,y+shrink,size-2*shrink,i<16,angle,pressed?.8f:1);
        if(model.selectionVisible&&i==model.selection)outline(x-3,y-3,size+6,size+6,white);
    }
    if(model.editing){
        int target=model.touch.dragging?modelSlotAt(&model,model.touch.x,model.touch.y):-1;
        if(target>=0){float x,y,size;slotPosition(target,&x,&y,&size);outline(x-4,y-4,size+8,size+8,C2D_Color32(255,255,255,190));}
        if(model.touch.dragging){int app=model.layout[model.touch.origin];float size=42;drawApp(app,model.touch.x-size/2,model.touch.y-size/2-8,size,false,0,.9f);}
        rect(2,g->bottomHeight-14,84,12,C2D_Color32(0,0,0,155));text("B: Done",8,g->bottomHeight-15,.48f,white);
    }
    if(statusMessage[0]&&nowMs<statusUntil){rect(1,g->bottomHeight-29,g->bottomWidth-2,17,C2D_Color32(15,22,30,230));fitText(statusMessage,5,g->bottomHeight-27,g->bottomWidth-10,.46f,white);}
    drawMusicDesktopBottom();drawWidgetRemoveMenu();
}
static const OfficialAppAssets *officialFor(App app){
    for(size_t i=0;i<officialAppCount;i++)if(officialApps[i].app==app)return &officialApps[i];
    return NULL;
}
static void releaseUnusedOfficialPage(void){
    const OfficialAppAssets *next=model.locked?NULL:officialFor(model.app);
    if(officialPage&&officialPage!=next){
        if(officialSheet){C2D_SpriteSheetFree(officialSheet);}
        officialSheet=NULL;officialPage=NULL;
        C2D_Prepare();
    }
}
static void ensureOfficialPage(void){
    const OfficialAppAssets *next=model.locked?NULL:officialFor(model.app);
    if(next==officialPage)return;
    if(officialSheet)C2D_SpriteSheetFree(officialSheet);
    officialPage=next;officialSheet=NULL;
    if(next){
        officialSheet=C2D_SpriteSheetLoad(next->atlasPath);
        if(officialSheet)for(size_t i=0;i<C2D_SpriteSheetCount(officialSheet);i++)C3D_TexSetFilter(C2D_SpriteSheetGetImage(officialSheet,i).tex,GPU_LINEAR,GPU_LINEAR);
    }
    C2D_Prepare();
}
static void drawOfficial(bool topScreen){
    const UiGeometry*g=uiGeometry(model.portrait);
    float w=topScreen?g->topWidth:g->bottomWidth,h=topScreen?g->topHeight:g->bottomHeight;
    if(!officialSheet||C2D_SpriteSheetCount(officialSheet)<4){rect(0,0,w,h,ink);return;}
    C2D_Image im=C2D_SpriteSheetGetImage(officialSheet,(model.portrait?2:0)+(topScreen?0:1));
    C2D_DrawImageAt(im,0,0,0,NULL,w/im.subtex->width,h/im.subtex->height);
}
static bool isShowcase(void){return !OFFICIAL_PREVIEW_ONLY&&!model.locked&&!officialFor(model.app)&&(model.app==APP_MUSIC||model.app==APP_STORE);}
static bool showMusic(void){return model.app==APP_MUSIC;}
static void ensureShowPage(void){
    const ShowPageAssets*next=NULL;
    if(isShowcase())for(size_t i=0;i<showPageCount;i++)if(!strcmp(showPages[i].route,showcaseRoute(&showcase,showMusic()))&&showPages[i].variant==showcaseVariant(&showcase,showMusic())){next=&showPages[i];break;}
    if(next==showPage)return;
    bool sameRoute=next&&showPage&&!strcmp(next->route,showPage->route);
    if(!sameRoute)showFocusAction[0]=0;
    uint64_t loadStart=svcGetSystemTick();
    if(showSheet)C2D_SpriteSheetFree(showSheet);
    showSheet=NULL;showPage=next;showFocus=-1;
    if(next){showSheet=C2D_SpriteSheetLoad(next->atlasPath);if(showSheet)for(size_t i=0;i<C2D_SpriteSheetCount(showSheet);i++)C3D_TexSetFilter(C2D_SpriteSheetGetImage(showSheet,i).tex,GPU_LINEAR,GPU_LINEAR);}
    /* FrameBegin(SYNCDRAW) has retired the previous GPU work. malloc may reuse
       the old sheet's C3D_Tex address, but Citro2D caches texture identity by
       pointer. Invalidate that cache so new dimensions/data are bound. */
    lastAtlasLoadMs=(unsigned)((svcGetSystemTick()-loadStart)/(SYSCLOCK_ARM11/1000));
    C2D_Prepare();
}
static const ShowView*showView(bool topScreen){return showPage?&showPage->views[(model.portrait?2:0)+(topScreen?0:1)]:NULL;}
static void showImage(int index,UiRect r){if(index<0||!showSheet)return;C2D_Image im=C2D_SpriteSheetGetImage(showSheet,index);C2D_DrawImageAt(im,r.x,r.y,0,NULL,r.w/im.subtex->width,r.h/im.subtex->height);}
static bool showOpened(void){return !showMusic()&&showcase.opened;}
static const ShowRegion*showRegions(const ShowView*v,size_t*n){if(showOpened()&&v->openedImage>=0){*n=v->openedRegionCount;return v->openedRegions;}*n=v->regionCount;return v->regions;}
static void restoreShowFocus(void){
    const ShowView*v=showView(false);showFocus=-1;if(!v||!showFocusAction[0])return;
    size_t n;const ShowRegion*rs=showRegions(v,&n);
    showFocus=showcaseFocusIndex(rs,n,showFocusAction);
    if(showFocus<0)showFocusAction[0]=0;
}
static void showDraw(bool topScreen){
    const UiGeometry*g=uiGeometry(model.portrait);const ShowView*v=showView(topScreen);float w=topScreen?g->topWidth:g->bottomWidth,h=topScreen?g->topHeight:g->bottomHeight;
    if(!v||!showSheet){rect(0,0,w,h,white);fitText("Page assets unavailable. B: Home",10,40,w-20,.6f,ink);return;}
    showImage(showOpened()&&v->openedImage>=0?v->openedImage:v->image,(UiRect){0,0,w,h});
    if(!showOpened())for(size_t i=0;i<v->overlayCount;i++){
        const ShowOverlay*o=&v->overlays[i];UiRect r=o->rect;
        if(o->backgroundImage>=0)showImage(o->backgroundImage,o->backgroundRect);
        if(o->kind==SHOW_OVERLAY_MINIPLAYER)showImage(o->images[showcase.album*3+showcase.track],r);
        else if(o->kind==SHOW_OVERLAY_PLAY)showImage(o->images[showcase.playing?1:0],r);
        else if(o->kind==SHOW_OVERLAY_GET){unsigned app=o->item>=0?(unsigned)o->item:showcase.storeApp;showImage(o->images[showcase.downloads[app]],r);if(showcase.downloads[app]==DOWNLOAD_PROGRESS)rect(r.x,r.y+r.h-2,r.w*showcase.downloadMillis[app]/3000.f,2,blue);}
        else if(o->kind==SHOW_OVERLAY_SHUFFLE||o->kind==SHOW_OVERLAY_REPEAT)showImage(o->images[(o->kind==SHOW_OVERLAY_SHUFFLE?showcase.shuffle:showcase.repeat)?1:0],r);
        else if(o->kind==SHOW_OVERLAY_PROGRESS){rect(r.x,r.y,r.w,r.h,o->trackColor);rect(r.x,r.y,r.w*showcase.positionSeconds/showcaseTrackSeconds(&showcase),r.h,o->foreground);C2D_DrawCircleSolid(r.x+r.w*showcase.positionSeconds/showcaseTrackSeconds(&showcase),r.y+r.h/2,0,2.5f,o->foreground);}
        else if(o->kind==SHOW_OVERLAY_ELAPSED||o->kind==SHOW_OVERLAY_REMAINING){unsigned sec=o->kind==SHOW_OVERLAY_ELAPSED?(unsigned)showcase.positionSeconds:showcaseTrackSeconds(&showcase)-(unsigned)showcase.positionSeconds;char t[16];snprintf(t,sizeof(t),"%s%u:%02u",o->kind==SHOW_OVERLAY_REMAINING?"-":"",sec/60,sec%60);if(o->backgroundImage<0)rect(r.x-1,r.y-1,r.w+2,r.h+3,o->color);fitText(t,r.x,r.y,r.w,.35f,o->foreground);}
    }
    if(!topScreen&&showFocus>=0){size_t count;const ShowRegion*rs=showRegions(v,&count);if((size_t)showFocus<count){UiRect r=rs[showFocus].rect;outline(r.x,r.y,r.w,r.h,blue);}}
}
static void showAction(const char*action){if(!showcaseAction(&showcase,action))setStatus("Unsupported action",2500);}
static void showTap(int x,int y){const ShowView*v=showView(false);if(!v)return;size_t n;const ShowRegion*rs=showRegions(v,&n);for(size_t i=0;i<n;i++)if(uiContains(rs[i].rect,x,y)){if(!strcmp(rs[i].action,"music.seek-forward")||!strcmp(rs[i].action,"music.seek"))showcaseSeek(&showcase,(x-rs[i].rect.x)/rs[i].rect.w);else showAction(rs[i].action);return;}}
static void showActivate(void){const ShowView*v=showView(false);if(!v)return;size_t n;const ShowRegion*rs=showRegions(v,&n);if(n){size_t selected=showFocus>=0&&(size_t)showFocus<n?(size_t)showFocus:0;snprintf(showFocusAction,sizeof(showFocusAction),"%s",rs[selected].action);showAction(rs[selected].action);}}
static void showMove(int dx,int dy){const ShowView*v=showView(false);if(!v)return;size_t n;const ShowRegion*rs=showRegions(v,&n);if(!n)return;if(showFocus<0||(size_t)showFocus>=n){showFocus=0;snprintf(showFocusAction,sizeof(showFocusAction),"%s",rs[0].action);return;}UiRect from=rs[showFocus].rect;float x=from.x+from.w/2,y=from.y+from.h/2,best=1e20f;int target=showFocus;for(size_t i=0;i<n;i++){float ax=rs[i].rect.x+rs[i].rect.w/2-x,ay=rs[i].rect.y+rs[i].rect.h/2-y;if(ax*dx+ay*dy<=0)continue;float score=ax*ax+ay*ay+fabsf(ax*dy-ay*dx)*100;if(score<best){best=score;target=(int)i;}}showFocus=target;snprintf(showFocusAction,sizeof(showFocusAction),"%s",rs[target].action);}
static void drawLock(bool topScreen){
    int index=(model.night?4:0)+(model.portrait?2:0)+(topScreen?0:1);
    C2D_Image im=C2D_SpriteSheetGetImage(lockSheet,index);
    C2D_DrawImageAt(im,0,0,0,NULL,1,1);
}
static void top(void){
    const UiGeometry*g=uiGeometry(model.portrait);float w=g->topWidth,h=g->topHeight;
    if(model.locked){drawLock(true);return;}
    if(model.app==APP_HOME){homeTop();return;}
    if(officialFor(model.app)){drawOfficial(true);return;}
    if(OFFICIAL_PREVIEW_ONLY){homeTop();return;}
    if(isShowcase()){showDraw(true);return;}
    rect(0,0,w,h,C2D_Color32(247,248,250,255));appTitle();
    if(model.app==APP_CALCULATOR){
        rect(0,0,w,h,C2D_Color32(23,26,31,255));text("Calculator",18,13,.6f,muted);
        float scale=fminf(2.7f,(w-40)/fmaxf(1,textWidth(model.calc.input,1)));text(model.calc.input,w-20-textWidth(model.calc.input,scale),h*.55f,scale,white);
    }else if(model.app==APP_PHOTOS){
        rect(0,0,w,h,C2D_Color32(14,16,21,255));
        if(model.photo==0){float scale=fminf((w-30)/177,(h-50)/195);image(5,(w-177*scale)/2,(h-35-195*scale)/2,177*scale,195*scale);}
        else imageCover(4,0,0,w,h);
        rect(0,h-35,w,35,C2D_Color32(0,0,0,130));text(model.photo?"Desert Duo":"On This Day",15,h-29,.65f,white);
    }else if(model.app==APP_SETTINGS){
        fitText("Make it yours.",18,53,w-36,1.1f,ink);text("Launch wallpaper",18,93,.65f,muted);
        image(topBackground(),18,120,w-36,h-135);
    }else if(model.app==APP_CLOCK){char now[20];timeText(now,sizeof(now));text("Local time",18,54,.9f,ink);center(now,w/2,h*.43f,3,ink);}
    else if(model.app==APP_NOTES){fitText("A little room to explore.",18,53,w-36,.9f,ink);wrappedText(model.note,18,93,w-36,h-110,.65f,ink);}
    else if(model.app==APP_CAMERA){
        imageCover(4,0,0,w,h);rect(0,0,w,40,C2D_Color32(0,0,0,150));fitText("Preview only - camera not connected",12,10,w-24,.6f,white);outline(w*.2f,h*.3f,w*.6f,h*.4f,white);
    }else{
        fitText(names[model.app],18,57,w-36,1.4f,ink);text("Interface preview",18,111,.8f,muted);
        wrappedText("This feature is not connected.\nNo network requests or account access.",18,151,w-36,h-160,.63f,muted);
    }
}
static void bottom(void){
    const UiGeometry*g=uiGeometry(model.portrait);float w=g->bottomWidth,h=g->bottomHeight;
    if(model.locked){drawLock(false);return;}
    if(model.app==APP_HOME){homeBottom();return;}
    if(officialFor(model.app)){drawOfficial(false);return;}
    if(OFFICIAL_PREVIEW_ONLY){homeBottom();return;}
    if(isShowcase()){showDraw(false);return;}
    rect(0,0,w,h,C2D_Color32(246,247,249,255));
    if(model.app==APP_CALCULATOR){rect(0,0,w,h,C2D_Color32(23,26,31,255));for(int i=0;i<20;i++)buttonRect(uiCalculatorKey(model.portrait,i),keyLabels[i],i%4==3||i==19);}
    else if(model.app==APP_PHOTOS){center(model.photo?"2 / 2":"1 / 2",w/2,18,.8f,ink);
        float ty=model.portrait?95:55,th=model.portrait?100:76,tw=(w-48)/2;
        imageContain(5,(UiRect){18,ty,tw,th});imageContain(4,(UiRect){w/2+6,ty,tw,th});outline(model.photo?w/2+6:18,ty,tw,th,blue);
        fitText("Slide thumbnails to preview above",18,ty+th+5,w-36,.45f,muted);buttonRect(controlRect(CONTROL_PHOTO_PREV),"Previous",false);buttonRect(controlRect(CONTROL_PHOTO_NEXT),"Next",true);}
    else if(model.app==APP_SETTINGS){
        text("Appearance",18,20,.9f,ink);buttonRect(controlRect(CONTROL_THEME),model.night?"Night wallpaper: ON":"Night wallpaper: OFF",true);
        const char*autoLabel=orientation.automatic?"Auto rotate: ON":(accelerometerFailed?"Retry auto rotate":"Auto rotate: OFF");
        buttonRect(controlRect(CONTROL_AUTO_ROTATE),autoLabel,orientation.automatic);
        fitText("L switches and locks orientation.",18,model.portrait?190:142,w-36,.5f,muted);
        buttonRect(controlRect(CONTROL_LOCK),"Lock screen",false);
    }
    else if(model.app==APP_NOTES){text("Notes",18,20,1,ink);buttonRect(controlRect(CONTROL_NOTES),"Edit with system keyboard",true);wrappedText("ASCII text in this first build.\nChanges stay in memory.",18,145,w-36,h-180,.6f,muted);}
    else {center(names[model.app],w/2,65,1.1f,ink);wrappedText(model.app==APP_CLOCK?"Uses the console's clock.":"A small preview of the interface.",18,110,w-36,h-150,.65f,muted);}
    buttonRect(controlRect(CONTROL_HOME),"B  Home",false);
}
static void touchTap(int x,int y){
    if(model.locked)return;
    if(model.app==APP_HOME||OFFICIAL_PREVIEW_ONLY||officialFor(model.app))return;
    if(isShowcase()){showTap(x,y);return;}
    if(uiContains(controlRect(CONTROL_HOME),x,y)){modelHome(&model);return;}
    if(model.app==APP_CALCULATOR){for(int i=0;i<20;i++)if(uiContains(uiCalculatorKey(model.portrait,i),x,y)){calcKey(&model.calc,keyValues[i]);return;}}
    else if(model.app==APP_PHOTOS){if(uiContains(controlRect(CONTROL_PHOTO_PREV),x,y)||uiContains(controlRect(CONTROL_PHOTO_NEXT),x,y))model.photo=1-model.photo;}
    else if(model.app==APP_SETTINGS){
        if(uiContains(controlRect(CONTROL_THEME),x,y))model.night=!model.night;
        else if(uiContains(controlRect(CONTROL_AUTO_ROTATE),x,y))setAutomaticRotation(!orientation.automatic);
        else if(uiContains(controlRect(CONTROL_LOCK),x,y))modelSetLocked(&model,true);
    }
    else if(model.app==APP_NOTES&&uiContains(controlRect(CONTROL_NOTES),x,y)){
        SwkbdState keyboard;swkbdInit(&keyboard,SWKBD_TYPE_NORMAL,2,255);swkbdSetInitialText(&keyboard,model.note);swkbdSetHintText(&keyboard,"A note for Duo");char edited[256];
        if(swkbdInputText(&keyboard,edited,sizeof(edited))==SWKBD_BUTTON_CONFIRM){strncpy(model.note,edited,sizeof(model.note)-1);model.note[255]=0;}
    }
}
static UiRect photoStrip(void){const UiGeometry*g=uiGeometry(model.portrait);return(UiRect){18,model.portrait?95:55,g->bottomWidth-36,model.portrait?100:76};}
static void cancelPointer(bool untilRelease){
    bool removing=widgetRemoval.phase!=REMOVE_IDLE;
    gestureCancel(&pointer);lockMotionCancel(&lockMotion);widgetRemoveCancel(&widgetRemoval);showcase.scrubbing=false;modelCancelTouch(&model,removing);wasHomeTouch=false;
    if(untilRelease)ignoreTouchUntilRelease=true;
}
static void updatePointer(float x,float y){
    if(model.locked){lockMotionMove(&lockMotion,x,y);return;}
    gestureMove(&pointer,x,y);
    if(pointer.active&&pointer.kind==GESTURE_SEEK)showcaseSeek(&showcase,gestureFraction(&pointer));
    if(pointer.active&&pointer.kind==GESTURE_PHOTO)model.photo=gestureFraction(&pointer)>=.5f;
}
static void beginPointer(int x,int y){
    const UiGeometry*g=uiGeometry(model.portrait);showFocus=-1;showFocusAction[0]=0;
    if(model.locked){lockMotionBegin(&lockMotion,x,y,g->bottomHeight);return;}
    if(gestureEdgeStart(y,g->bottomHeight,false,model.app==APP_HOME)){gestureBegin(&pointer,GESTURE_EDGE,(UiRect){0,g->bottomHeight-10,g->bottomWidth,10},x,y);return;}
    if(OFFICIAL_PREVIEW_ONLY||officialFor(model.app))return;
    if(isShowcase()){
        const ShowView*v=showView(false);if(!v)return;size_t n;const ShowRegion*rs=showRegions(v,&n);
        for(size_t i=0;i<n;i++){
            bool seek=!strcmp(rs[i].action,"music.seek-forward")||!strcmp(rs[i].action,"music.seek");
            if(uiContains(seek?gestureSeekTarget(rs[i].rect):rs[i].rect,x,y)){
                gestureBegin(&pointer,seek?GESTURE_SEEK:GESTURE_TAP,rs[i].rect,x,y);
                if(seek)showcase.scrubbing=true;
                updatePointer(x,y);return;
            }
        }
        return;
    }
    if(model.app==APP_PHOTOS&&uiContains(photoStrip(),x,y)){gestureBegin(&pointer,GESTURE_PHOTO,photoStrip(),x,y);updatePointer(x,y);return;}
    UiRect r=controlRect(CONTROL_HOME);
    if(uiContains(r,x,y)){gestureBegin(&pointer,GESTURE_TAP,r,x,y);return;}
    if(model.app==APP_CALCULATOR){for(int i=0;i<20;i++){r=uiCalculatorKey(model.portrait,i);if(uiContains(r,x,y)){gestureBegin(&pointer,GESTURE_TAP,r,x,y);return;}}}
    else{
        const Control*list=NULL;size_t count=0;
        static const Control photos[]={CONTROL_PHOTO_PREV,CONTROL_PHOTO_NEXT};
        static const Control settings[]={CONTROL_THEME,CONTROL_AUTO_ROTATE,CONTROL_LOCK};
        static const Control notes[]={CONTROL_NOTES};
        if(model.app==APP_PHOTOS){list=photos;count=2;}else if(model.app==APP_SETTINGS){list=settings;count=3;}else if(model.app==APP_NOTES){list=notes;count=1;}
        for(size_t i=0;i<count;i++){r=controlRect(list[i]);if(uiContains(r,x,y)){gestureBegin(&pointer,GESTURE_TAP,r,x,y);return;}}
    }
}
static void startTransition(bool closing,App app){
    if(model.locked)return;
    const UiGeometry*g=uiGeometry(model.portrait);UiPoint anchor={g->bottomWidth/2.f,g->bottomHeight/2.f};
    for(int i=0;i<20;i++){int slot=i<16?model.page*16+i:DOCK_FIRST+i-16;if(model.layout[slot]==app){UiRect r=uiIconRect(model.portrait,slot);anchor=(UiPoint){r.x+r.w/2,r.y+r.h/2};break;}}
    transition.active=true;transition.closing=closing;transition.started=0;transition.anchor=anchor;
    cancelPointer(true);
}
static void finishTransition(void){if(transition.active&&transition.closing)modelHome(&model);transition.active=false;}
static void advanceTransition(void){if(transition.active&&transition.started&&nowMs-transition.started>=240)finishTransition();}
static void drawTouchFeedback(void){
    if(model.locked||model.app==APP_HOME||transition.active||officialFor(model.app))return;
    const UiGeometry*g=uiGeometry(model.portrait);
    rect(g->bottomWidth/2.f-18,g->bottomHeight-4,36,2,C2D_Color32(180,180,190,155));
    if(pointer.active&&pointer.kind==GESTURE_TAP&&uiContains(pointer.region,pointer.x,pointer.y)){
        UiRect r=pointer.region;rect(r.x,r.y,r.w,r.h,C2D_Color32(0,0,0,45));
    }else if(pointer.active&&pointer.kind==GESTURE_EDGE){
        float lift=fminf(24,fmaxf(0,pointer.startY-pointer.y));
        rect(g->bottomWidth/2.f-24,g->bottomHeight-6-lift,48,3,C2D_Color32(245,245,245,225));
    }
}
static void beginScene(C3D_RenderTarget*target,bool topScreen){
    C2D_SceneBegin(target);C2D_ViewReset();
    if(model.portrait){
        // Virtual portrait coordinates -> physical framebuffer: (W-y, x).
        C3D_Mtx matrix;Mtx_Identity(&matrix);
        matrix.r[0].x=0;matrix.r[0].y=-1;matrix.r[0].w=topScreen?400:320;
        matrix.r[1].x=1;matrix.r[1].y=0;
        C2D_ViewRestore(&matrix);
    }
}
static void renderScreen(C3D_RenderTarget*target,bool topScreen){
    C2D_TargetClear(target,ink);beginScene(target,topScreen);
    C3D_Mtx base;C2D_ViewSave(&base);
    if(model.locked){
        if(topScreen)homeTop();else homeBottom();
        const UiGeometry*g=uiGeometry(model.portrait);
        float offset=lockMotion.offset*(topScreen?(float)g->topHeight/g->bottomHeight:1);
        C3D_Mtx moved=base;moved.r[0].w-=base.r[0].y*offset;moved.r[1].w-=base.r[1].y*offset;
        C2D_ViewRestore(&moved);drawLock(topScreen);C2D_ViewRestore(&base);
        if(topScreen){drawSensorDiagnostics();}
        return;
    }
    if(transition.active&&!model.locked){
        if(topScreen)homeTop();else homeBottom();
        float progress=transition.started?(nowMs-transition.started)/240.f:0;
        float amount=transitionEase(progress);if(transition.closing)amount=1-amount;
        float scale=topScreen?.60f+.40f*amount:.12f+.88f*amount;
        const UiGeometry*g=uiGeometry(model.portrait);
        UiPoint anchor=topScreen?(UiPoint){g->topWidth/2.f,g->topHeight/2.f}:transition.anchor;
        float ox=(1-scale)*anchor.x,oy=(1-scale)*anchor.y;
        C3D_Mtx matrix;Mtx_Identity(&matrix);
        if(model.portrait){matrix.r[0].x=0;matrix.r[0].y=-scale;matrix.r[0].w=(topScreen?400:320)-oy;matrix.r[1].x=scale;matrix.r[1].y=0;matrix.r[1].w=ox;}
        else{matrix.r[0].x=scale;matrix.r[1].y=scale;matrix.r[0].w=ox;matrix.r[1].w=oy;}
        C2D_ViewRestore(&matrix);
    }
    if(topScreen)top();else bottom();
    C2D_ViewRestore(&base);
    if(topScreen)drawSensorDiagnostics();else drawTouchFeedback();
}
static void lifecycle(APT_HookType type,void*param){
    (void)param;
    showcaseResetClock(&showcase);memset(&languageKeys,0,sizeof(languageKeys));
    gestureCancel(&pointer);lockMotionCancel(&lockMotion);widgetRemoveCancel(&widgetRemoval);showcase.scrubbing=false;transition.active=false;musicFlight.active=false;
    orientationReset(&orientation);
    if(type==APTHOOK_ONSUSPEND||type==APTHOOK_ONEXIT){modelCancelTouch(&model,true);wasHomeTouch=false;ignoreTouchUntilRelease=true;}
    if(type==APTHOOK_ONSLEEP||type==APTHOOK_ONWAKEUP){modelSetLocked(&model,true);wasHomeTouch=false;ignoreTouchUntilRelease=true;}
}
static int fail(const char*message){consoleInit(GFX_BOTTOM,NULL);printf("iPhone Duo could not start.\n%s\nPress START to exit.\n",message);while(aptMainLoop()){hidScanInput();if(hidKeysDown()&KEY_START)break;gspWaitForVBlank();}return 1;}
int main(void){
    gfxInitDefault();gfxSet3D(false);
    if(R_FAILED(romfsInit())){int r=fail("Missing RomFS assets.");gfxExit();return r;}
    if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)){int r=fail("GPU initialization failed.");romfsExit();gfxExit();return r;}
    if(!C2D_Init(C2D_DEFAULT_MAX_OBJECTS)){int r=fail("2D initialization failed.");C3D_Fini();romfsExit();gfxExit();return r;}
    C2D_Prepare();
    C3D_RenderTarget*topTarget=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT);
    C3D_RenderTarget*bottomTarget=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);
    sheet=C2D_SpriteSheetLoad("romfs:/gfx/ui.t3x");
    portraitSheet=C2D_SpriteSheetLoad("romfs:/gfx/portrait.t3x");
    musicWidgetSheet=C2D_SpriteSheetLoad("romfs:/gfx/music_widget.t3x");
    labelSheet=C2D_SpriteSheetLoad("romfs:/gfx/labels.t3x");
    iconNativeSheet=C2D_SpriteSheetLoad("romfs:/gfx/icons_native.t3x");
    lockSheet=C2D_SpriteSheetLoad("romfs:/gfx/lockscreen.t3x");
    bool englishReady=true;
    for(int i=0;i<4;i++){char path[48];snprintf(path,sizeof(path),"romfs:/gfx/english%d.t3x",i);englishSheets[i]=C2D_SpriteSheetLoad(path);if(!englishSheets[i]||C2D_SpriteSheetCount(englishSheets[i])<4)englishReady=false;}
    if(!topTarget||!bottomTarget||!sheet||!portraitSheet||!musicWidgetSheet||!labelSheet||!iconNativeSheet||!lockSheet||!englishReady||C2D_SpriteSheetCount(lockSheet)<8||C2D_SpriteSheetCount(iconNativeSheet)<APP_COUNT*2||C2D_SpriteSheetCount(labelSheet)<APP_COUNT*2||C2D_SpriteSheetCount(musicWidgetSheet)<5||C2D_SpriteSheetCount(sheet)<UI_IMAGES||C2D_SpriteSheetCount(portraitSheet)<PORTRAIT_IMAGES){if(sheet)C2D_SpriteSheetFree(sheet);for(int i=0;i<4;i++)if(englishSheets[i])C2D_SpriteSheetFree(englishSheets[i]);if(labelSheet)C2D_SpriteSheetFree(labelSheet);if(lockSheet)C2D_SpriteSheetFree(lockSheet);if(iconNativeSheet)C2D_SpriteSheetFree(iconNativeSheet);if(musicWidgetSheet)C2D_SpriteSheetFree(musicWidgetSheet);if(portraitSheet)C2D_SpriteSheetFree(portraitSheet);C2D_Fini();C3D_Fini();int r=fail("Missing or invalid texture atlas.");romfsExit();gfxExit();return r;}
    // Smooth sprite downsampling and subpixel placement; this only updates texture state.
    for(size_t i=0;i<TOTAL_IMAGES;i++){
        C2D_Image sprite=getImage((int)i);
        C3D_TexSetFilter(sprite.tex,GPU_LINEAR,GPU_LINEAR);
    }
    for(size_t i=0;i<C2D_SpriteSheetCount(musicWidgetSheet);i++)C3D_TexSetFilter(C2D_SpriteSheetGetImage(musicWidgetSheet,i).tex,GPU_LINEAR,GPU_LINEAR);
    for(size_t i=0;i<C2D_SpriteSheetCount(labelSheet);i++)C3D_TexSetFilter(C2D_SpriteSheetGetImage(labelSheet,i).tex,GPU_NEAREST,GPU_NEAREST);
    for(size_t i=0;i<C2D_SpriteSheetCount(iconNativeSheet);i++)C3D_TexSetFilter(C2D_SpriteSheetGetImage(iconNativeSheet,i).tex,GPU_NEAREST,GPU_NEAREST);
    for(size_t i=0;i<C2D_SpriteSheetCount(lockSheet);i++)C3D_TexSetFilter(C2D_SpriteSheetGetImage(lockSheet,i).tex,GPU_NEAREST,GPU_NEAREST);
    white=C2D_Color32(255,255,255,255);ink=C2D_Color32(34,38,45,255);muted=C2D_Color32(120,128,139,255);blue=C2D_Color32(26,126,242,255);
    if(R_SUCCEEDED(cfguInit())){u8 systemLanguage=CFG_LANGUAGE_EN;if(R_SUCCEEDED(CFGU_GetSystemLanguage(&systemLanguage))&&(systemLanguage==CFG_LANGUAGE_ZH||systemLanguage==CFG_LANGUAGE_TW))language=DUO_CHINESE;cfguExit();}
    languageLoad("sdmc:/3ds/iphone-duo/language.bin",&language);
    modelInit(&model);showcaseInit(&showcase);nowMs=svcGetSystemTick()/(SYSCLOCK_ARM11/1000);
    orientationInit(&orientation);setAutomaticRotation(true);
    LayoutLoadResult loaded=layoutLoad(&model,layoutPath);
    if(loaded==LAYOUT_INVALID)setStatus("Invalid layout: restored defaults.",8000);
    else if(loaded==LAYOUT_IO_ERROR)setStatus("Could not read layout. Using defaults.",8000);
    else if(loaded==LAYOUT_MIGRATED){
        bool saved=layoutSave(&model,layoutPath);
        setStatus(saved?"Layout updated":"Layout updated; SD save failed.",saved?900:5000);
    }
    aptHookCookie cookie;aptHook(&cookie,lifecycle,NULL);
    while(aptMainLoop()){
        nowMs=svcGetSystemTick()/(SYSCLOCK_ARM11/1000);advanceTransition();
        bool justUnlocked=model.locked&&lockMotionStep(&lockMotion,nowMs);
        if(justUnlocked){modelHome(&model);modelSetLocked(&model,false);cancelPointer(true);}
        if(musicFlight.active&&nowMs>=musicFlight.started&&nowMs-musicFlight.started>=600){musicFlight.active=false;musicWidgetVisible=true;}
        if(widgetRemoveStep(&widgetRemoval,nowMs))musicWidgetVisible=false;
        App previousApp=model.app;
        hidScanInput();u32 down=hidKeysDown(),held=hidKeysHeld(),up=hidKeysUp();
        if(justUnlocked)down&=KEY_START|KEY_R;
        unsigned languageAction=justUnlocked?0:languageKeysUpdate(&languageKeys,(down&KEY_SELECT)!=0,(up&KEY_SELECT)!=0,(held&KEY_SELECT)!=0,(held&KEY_Y)!=0);
        down&=~KEY_SELECT;
        if(languageAction&LANGUAGE_SELECT)down|=KEY_SELECT;
        if(languageAction&LANGUAGE_CONSUME_Y)down&=~KEY_Y;
        if(languageAction&LANGUAGE_TOGGLE){
            cancelPointer(true);modelCancelTouch(&model,true);finishTransition();musicFlight.active=false;
            language=language==DUO_ENGLISH?DUO_CHINESE:DUO_ENGLISH;
            mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/iphone-duo",0777);
            bool saved=languageSave("sdmc:/3ds/iphone-duo/language.bin",language);
            setStatus(saved?(language==DUO_ENGLISH?"Language: English":"Language: Chinese"):"Language changed; SD save failed.",saved?1200:5000);
        }
        if(down&KEY_START){cancelPointer(true);break;}
        if(down&KEY_R)sensorDiagnostics=!sensorDiagnostics;
        if(widgetRemoval.phase!=REMOVE_IDLE){
            if(down&KEY_B){cancelPointer((held&KEY_TOUCH)!=0);down&=~KEY_B;}
            down&=~(KEY_A|KEY_X|KEY_Y|KEY_DUP|KEY_DDOWN|KEY_DLEFT|KEY_DRIGHT);
        }
        u32 navigation=KEY_A|KEY_B|KEY_L|KEY_SELECT|KEY_X|KEY_Y|KEY_DUP|KEY_DDOWN|KEY_DLEFT|KEY_DRIGHT;
        if((down&navigation)&&((held|up)&KEY_TOUCH))cancelPointer(true);
        if(down&(KEY_L|KEY_SELECT)){finishTransition();musicFlight.active=false;previousApp=model.app;}
        if(down&KEY_L){setAutomaticRotation(false);modelToggleOrientation(&model);cancelPointer((held&KEY_TOUCH)!=0);setStatus("Manual orientation. Auto rotate off.",900);}
        if(down&KEY_SELECT){cancelPointer((held&KEY_TOUCH)!=0);modelSetLocked(&model,!model.locked);}
        if(!transition.active&&!musicFlight.active){
            if(down&KEY_A){if(isShowcase())showActivate();else modelActivate(&model);}
            if(down&KEY_B){if(isShowcase()){if(!showcaseBack(&showcase,showMusic()))modelHome(&model);}else modelBack(&model);}
            if(!model.locked){
                if(down&KEY_X)model.night=!model.night;
                if(down&KEY_Y)modelNextPage(&model);
                int dx=(down&KEY_DRIGHT?1:0)-(down&KEY_DLEFT?1:0);
                int dy=(down&KEY_DDOWN?1:0)-(down&KEY_DUP?1:0);
                uiLogicalDirection(model.portrait,&dx,&dy);
                if(officialFor(model.app)){}
                else if(isShowcase()){if(dx||dy)showMove(dx,dy);}
                else if(model.app==APP_PHOTOS){if(dx)model.photo=1-model.photo;}
                else if(dx||dy)modelMove(&model,dx,dy);
            }
        }
        touchPosition touch;static int lastX,lastY;
        if((down&KEY_TOUCH)&&(transition.active||musicFlight.active||widgetRemoval.phase==REMOVE_FLIGHT||lockMotion.phase==LOCK_LEAVING))ignoreTouchUntilRelease=true;
        if((down&KEY_TOUCH)&&!ignoreTouchUntilRelease){
            hidTouchRead(&touch);UiPoint point=uiToVirtual(model.portrait,false,(UiPoint){touch.px,touch.py});lastX=(int)point.x;lastY=(int)point.y;
            touchApp=model.app;touchPortrait=model.portrait;touchPage=showPage;
            wasHomeTouch=model.app==APP_HOME&&!model.locked&&widgetRemoval.phase!=REMOVE_MENU;
            if(widgetRemoval.phase==REMOVE_MENU)widgetRemoveTouchBegin(&widgetRemoval,model.portrait,lastX,lastY);
            else if(wasHomeTouch)modelTouchBegin(&model,lastX,lastY,nowMs);else beginPointer(lastX,lastY);
        }
        if((held&KEY_TOUCH)&&!ignoreTouchUntilRelease){
            hidTouchRead(&touch);UiPoint point=uiToVirtual(model.portrait,false,(UiPoint){touch.px,touch.py});lastX=(int)point.x;lastY=(int)point.y;
            if(wasHomeTouch){modelTouchMove(&model,lastX,lastY,nowMs);widgetRemoveObserve(&widgetRemoval,&model,musicWidgetVisible,nowMs);}
            else if(widgetRemoval.phase!=REMOVE_MENU)updatePointer(lastX,lastY);
        }
        if((up&KEY_TOUCH)&&!ignoreTouchUntilRelease){
            if(widgetRemoveReleaseHold(&widgetRemoval)){modelCancelTouch(&model,true);wasHomeTouch=false;}
            else if(widgetRemoval.phase==REMOVE_MENU)widgetRemoveTouchEnd(&widgetRemoval,model.portrait,lastX,lastY,nowMs);
            else if(wasHomeTouch){if(modelMusicTransferCandidate(&model,lastX,lastY))startMusicFlight(lastX,lastY);else if(modelTouchEnd(&model,lastX,lastY,nowMs))saveLayout();}
            else{
                if(model.locked){lockMotionEnd(&lockMotion,lastX,lastY,uiGeometry(model.portrait)->bottomHeight,nowMs);}
                else{
                    bool samePage=touchApp==model.app&&touchPortrait==model.portrait&&touchPage==showPage;
                    GestureResult result=gestureEnd(&pointer,lastX,lastY);showcase.scrubbing=false;
                    if(samePage&&result==GESTURE_HOME)modelHome(&model);
                    else if(samePage&&result==GESTURE_CLICK)touchTap(lastX,lastY);
                }
            }
            wasHomeTouch=false;
        }
        if(OFFICIAL_PREVIEW_ONLY&&previousApp==APP_HOME&&model.app!=APP_HOME&&!officialFor(model.app)){modelHome(&model);setStatus("No official preview",900);}
        if(previousApp==APP_HOME&&model.app!=APP_HOME&&!model.locked)startTransition(false,model.app);
        else if(previousApp!=APP_HOME&&model.app==APP_HOME&&!model.locked){model.app=previousApp;startTransition(true,previousApp);}
        updateAutomaticRotation(((held|up)&KEY_TOUCH)!=0||model.touch.active||pointer.active||lockMotion.phase!=LOCK_STILL||transition.active||musicFlight.active||widgetRemoval.phase!=REMOVE_IDLE||ignoreTouchUntilRelease);
        if(!(held&KEY_TOUCH))ignoreTouchUntilRelease=false;
        showcaseTick(&showcase,nowMs);
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);releaseUnusedOfficialPage();ensureShowPage();ensureOfficialPage();restoreShowFocus();
        nowMs=svcGetSystemTick()/(SYSCLOCK_ARM11/1000);
        if(transition.active&&!transition.started)transition.started=nowMs;
        renderScreen(topTarget,true);renderScreen(bottomTarget,false);C3D_FrameEnd(0);frame++;
    }
    orientationSetAutomatic(&orientation,false);disableAccelerometer();
    aptUnhook(&cookie);if(officialSheet)C2D_SpriteSheetFree(officialSheet);if(showSheet)C2D_SpriteSheetFree(showSheet);for(int i=0;i<4;i++)C2D_SpriteSheetFree(englishSheets[i]);C2D_SpriteSheetFree(lockSheet);C2D_SpriteSheetFree(iconNativeSheet);C2D_SpriteSheetFree(labelSheet);C2D_SpriteSheetFree(musicWidgetSheet);C2D_SpriteSheetFree(portraitSheet);C2D_SpriteSheetFree(sheet);C2D_Fini();C3D_Fini();romfsExit();gfxExit();return 0;
}
