#include "showcase.h"
#include <string.h>

void showcaseInit(Showcase*s){
    memset(s,0,sizeof(*s));s->musicPage=MUSIC_HOME;s->storePage=STORE_TODAY;
    static const unsigned durations[3][3]={{225,210,223},{214,160,198},{220,180,303}};
    memcpy(s->trackSeconds,durations,sizeof(durations));
}
void showcaseResetClock(Showcase*s){s->clockStarted=false;}
unsigned showcaseTrackSeconds(const Showcase*s){return s->trackSeconds[s->album][s->track];}
double showcaseDownloadProgress(const Showcase*s){return s->downloadMillis[s->storeApp]/3000.0;}
void showcaseTick(Showcase*s,uint64_t now){
    if(!s->clockStarted||now<s->lastTick){s->lastTick=now;s->clockStarted=true;return;}
    uint64_t elapsed=now-s->lastTick;s->lastTick=now;
    /* Suspended/paused intervals must not advance the local demonstration. */
    if(elapsed>1000)return;
    if(s->playing&&!s->scrubbing){s->positionSeconds+=elapsed/1000.0;if(s->positionSeconds>=showcaseTrackSeconds(s)){s->positionSeconds=0;if(s->repeat){/* Keep the current track. */}else if(s->shuffle)s->track=(s->track+2)%3;else if(s->track==2){s->playing=false;}else s->track++;}}
    for(unsigned i=0;i<3;i++)if(s->downloads[i]==DOWNLOAD_PROGRESS){
        s->downloadMillis[i]+=(uint32_t)elapsed;
        if(s->downloadMillis[i]>=3000){s->downloadMillis[i]=3000;s->downloads[i]=DOWNLOAD_READY;}
    }
}
static bool suffixIndex(const char*text,const char*prefix,unsigned*index){
    size_t n=strlen(prefix);if(strncmp(text,prefix,n))return false;
    if(text[n]<'0'||text[n]>'2'||text[n+1])return false;
    *index=(unsigned)(text[n]-'0');return true;
}
bool showcaseAction(Showcase*s,const char*action){
    if(!action)return false;
    if(!strcmp(action,"music.home")){s->musicPage=MUSIC_HOME;return true;}
    if(!strcmp(action,"music.new")){s->musicPage=MUSIC_NEW;return true;}
    if(!strcmp(action,"music.library")){s->musicPage=MUSIC_LIBRARY;return true;}
    if(!strcmp(action,"music.search")){s->musicPage=MUSIC_SEARCH;return true;}
    if(!strcmp(action,"music.album")){s->browseAlbum=s->album;s->musicPage=MUSIC_ALBUM;return true;}
    if(!strcmp(action,"music.player")){s->musicPage=MUSIC_PLAYER;return true;}
    if(!strcmp(action,"music.playalbum")){s->album=s->browseAlbum;s->track=0;s->positionSeconds=0;s->playing=true;s->musicPage=MUSIC_PLAYER;return true;}
    if(!strcmp(action,"music.shuffle")){s->shuffle=!s->shuffle;return true;}
    if(!strcmp(action,"music.repeat")){s->repeat=!s->repeat;return true;}
    if(!strcmp(action,"music.play")){if(!s->playing&&s->positionSeconds>=showcaseTrackSeconds(s))s->positionSeconds=0;s->playing=!s->playing;return true;}
    if(!strcmp(action,"music.next")){s->track=(s->track+1)%3;s->positionSeconds=0;return true;}
    if(!strcmp(action,"music.prev")){s->track=(s->track+2)%3;s->positionSeconds=0;return true;}
    if(!strcmp(action,"music.seek-forward")||!strcmp(action,"music.seek")){s->positionSeconds+=15;if(s->positionSeconds>showcaseTrackSeconds(s))s->positionSeconds=showcaseTrackSeconds(s);return true;}
    unsigned selected;
    if(suffixIndex(action,"music.album.",&selected)){s->browseAlbum=selected;s->musicPage=MUSIC_ALBUM;return true;}
    if(suffixIndex(action,"music.track.",&selected)){s->album=s->browseAlbum;s->track=selected;s->positionSeconds=0;s->playing=true;s->musicPage=MUSIC_PLAYER;return true;}
    if(!strcmp(action,"store.today")){s->storePage=STORE_TODAY;s->opened=false;return true;}
    if(!strcmp(action,"store.games")){s->storePage=STORE_GAMES;s->opened=false;return true;}
    if(!strcmp(action,"store.apps")){s->storePage=STORE_APPS;s->opened=false;return true;}
    if(!strcmp(action,"store.search")){s->storePage=STORE_SEARCH;s->opened=false;return true;}
    if(!strcmp(action,"store.detail")){s->storePage=STORE_DETAIL;return true;}
    if(suffixIndex(action,"store.detail.",&selected)){s->storeApp=selected;s->storePage=STORE_DETAIL;s->opened=false;return true;}
    if(!strcmp(action,"store.dismiss")){s->opened=false;return true;}
    if(!strcmp(action,"store.get")||suffixIndex(action,"store.get.",&selected)){
        if(strcmp(action,"store.get"))s->storeApp=selected;
        if(s->downloads[s->storeApp]==DOWNLOAD_IDLE){s->downloads[s->storeApp]=DOWNLOAD_PROGRESS;s->downloadMillis[s->storeApp]=0;}
        else if(s->downloads[s->storeApp]==DOWNLOAD_PROGRESS){s->downloads[s->storeApp]=DOWNLOAD_IDLE;s->downloadMillis[s->storeApp]=0;}
        else if(s->downloads[s->storeApp]==DOWNLOAD_READY){s->opened=true;s->storePage=STORE_DETAIL;}
        return true;
    }
    if(!strcmp(action,"music.back"))return showcaseBack(s,true);
    if(!strcmp(action,"store.back"))return showcaseBack(s,false);
    return false;
}
bool showcaseBack(Showcase*s,bool music){
    if(music&&(s->musicPage==MUSIC_PLAYER||s->musicPage==MUSIC_ALBUM)){s->musicPage=MUSIC_HOME;return true;}
    if(!music&&s->storePage==STORE_DETAIL){s->storePage=STORE_TODAY;s->opened=false;return true;}
    return false;
}
const char*showcaseRoute(const Showcase*s,bool music){
    static const char*musicRoutes[]={"music.home","music.new","music.library","music.search","music.album","music.player"};
    static const char*storeRoutes[]={"store.today","store.games","store.apps","store.search","store.detail"};
    return music?musicRoutes[s->musicPage]:storeRoutes[s->storePage];
}
unsigned showcaseVariant(const Showcase*s,bool music){
    if(music)return s->musicPage==MUSIC_PLAYER?s->album*3+s->track:(s->musicPage==MUSIC_ALBUM?s->browseAlbum:0);
    return s->storePage==STORE_DETAIL?s->storeApp:0;
}

void showcaseSeek(Showcase*s,double fraction){if(!(fraction>=0))fraction=0;if(fraction>1)fraction=1;s->positionSeconds=fraction*showcaseTrackSeconds(s);}

int showcaseFocusIndex(const ShowRegion*regions,size_t count,const char*action){if(!action||!action[0])return -1;for(size_t i=0;i<count;i++)if(!strcmp(regions[i].action,action))return (int)i;return -1;}
