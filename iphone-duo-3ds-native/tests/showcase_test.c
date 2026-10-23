#include "showcase.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "showcase_contract_actions.h"
int main(void){
    Showcase s;showcaseInit(&s);assert(!strcmp(showcaseRoute(&s,true),"music.home"));assert(!strcmp(showcaseRoute(&s,false),"store.today"));
    assert(showcaseAction(&s,"music.library"));assert(showcaseAction(&s,"music.album.2"));assert(s.browseAlbum==2&&s.album==0&&s.musicPage==MUSIC_ALBUM);
    assert(showcaseAction(&s,"music.track.1"));assert(s.track==1&&s.playing&&s.musicPage==MUSIC_PLAYER);
    showcaseTick(&s,1000);showcaseTick(&s,1500);assert(s.positionSeconds==.5);showcaseAction(&s,"music.play");showcaseTick(&s,2000);assert(s.positionSeconds==.5);showcaseAction(&s,"music.next");assert(s.track==2&&s.positionSeconds==0);showcaseAction(&s,"music.prev");assert(s.track==1);
    assert(showcaseBack(&s,true)&&s.musicPage==MUSIC_HOME);assert(!showcaseBack(&s,true));assert(!showcaseAction(&s,"music.track.9"));assert(!showcaseAction(&s,"music.track.1x"));assert(!showcaseAction(&s,"music.album.999"));
    showcaseAction(&s,"store.games");showcaseAction(&s,"store.detail.1");assert(s.storeApp==1);showcaseAction(&s,"store.get");assert(s.downloads[1]==DOWNLOAD_PROGRESS);for(unsigned t=2500;t<=5000;t+=500)showcaseTick(&s,t);assert(s.downloads[1]==DOWNLOAD_READY);showcaseAction(&s,"store.get");assert(s.opened);assert(showcaseBack(&s,false)&&s.storePage==STORE_TODAY);
    showcaseAction(&s,"store.detail.2");assert(s.downloads[2]==DOWNLOAD_IDLE);showcaseAction(&s,"store.get");assert(s.downloads[2]==DOWNLOAD_PROGRESS&&s.downloads[1]==DOWNLOAD_READY);
    showcaseAction(&s,"music.play");s.positionSeconds=179.5;showcaseTick(&s,5500);assert(s.positionSeconds==0&&s.track==2);showcaseResetClock(&s);showcaseTick(&s,100000);assert(s.positionSeconds==0);showcaseTick(&s,99999);assert(s.positionSeconds==0);
    showcaseInit(&s);showcaseAction(&s,"music.album.1");showcaseAction(&s,"music.playalbum");assert(s.album==1&&s.track==0&&s.playing&&s.musicPage==MUSIC_PLAYER);
    showcaseAction(&s,"music.repeat");s.positionSeconds=213.5;showcaseTick(&s,0);showcaseTick(&s,500);assert(s.track==0&&s.positionSeconds==0&&s.playing);
    showcaseAction(&s,"music.repeat");showcaseAction(&s,"music.shuffle");s.positionSeconds=213.5;showcaseTick(&s,1000);assert(s.track==2&&s.shuffle);
    showcaseAction(&s,"music.shuffle");s.positionSeconds=197.5;showcaseTick(&s,1500);assert(!s.playing&&s.track==2);
    showcaseAction(&s,"store.get.0");assert(s.storeApp==0&&s.downloads[0]==DOWNLOAD_PROGRESS);for(unsigned t=2000;t<=4500;t+=500)showcaseTick(&s,t);showcaseAction(&s,"store.get.0");assert(s.opened&&s.storePage==STORE_DETAIL);showcaseAction(&s,"store.apps");assert(!s.opened);
    for(unsigned i=0;i<sizeof(contractActions)/sizeof(contractActions[0]);i++){showcaseInit(&s);if(!strcmp(contractActions[i],"store.back"))s.storePage=STORE_DETAIL;assert(showcaseAction(&s,contractActions[i]));}
    showcaseInit(&s);showcaseAction(&s,"music.track.1");s.positionSeconds=42;
    showcaseAction(&s,"music.home");showcaseAction(&s,"music.album.2");
    assert(s.browseAlbum==2&&s.album==0&&s.track==1&&s.positionSeconds==42&&s.playing);
    assert(showcaseVariant(&s,true)==2);showcaseAction(&s,"music.player");assert(showcaseVariant(&s,true)==1);
    showcaseAction(&s,"music.album");assert(s.browseAlbum==0);showcaseAction(&s,"music.album.2");showcaseAction(&s,"music.track.1");assert(s.album==2&&s.track==1&&s.positionSeconds==0);
    s.scrubbing=true;showcaseTick(&s,1000);showcaseSeek(&s,.5);showcaseTick(&s,1500);assert(s.positionSeconds==90&&s.playing);s.scrubbing=false;showcaseTick(&s,2000);assert(s.positionSeconds==90.5);
    ShowRegion one[]={{{0,0,10,10},"music.seek-forward"},{{0,10,10,10},"music.play"},{{0,20,10,10},"music.next"}};
    ShowRegion reordered[]={{{0,0,10,10},"music.next"},{{0,10,10,10},"music.play"}};
    assert(showcaseFocusIndex(one,3,"music.play")==1);assert(showcaseFocusIndex(reordered,2,"music.next")==0);assert(showcaseFocusIndex(one,3,"")==-1);
    showcaseInit(&s);showcaseSeek(&s,.5);assert(s.positionSeconds==112.5);showcaseSeek(&s,-1);assert(s.positionSeconds==0);showcaseSeek(&s,3);assert(s.positionSeconds==225);
    showcaseInit(&s);showcaseAction(&s,"store.get.1");showcaseTick(&s,0);showcaseTick(&s,600);assert(s.downloadMillis[1]==600);showcaseAction(&s,"store.get.1");assert(s.downloads[1]==DOWNLOAD_IDLE&&s.downloadMillis[1]==0);showcaseTick(&s,900);assert(s.downloads[1]==DOWNLOAD_IDLE&&s.downloadMillis[1]==0);showcaseAction(&s,"store.get.1");assert(s.downloads[1]==DOWNLOAD_PROGRESS&&s.downloadMillis[1]==0);
    showcaseSeek(&s,1);assert(!s.playing);showcaseAction(&s,"music.play");assert(s.playing&&s.positionSeconds==0);showcaseAction(&s,"music.play");assert(!s.playing);showcaseSeek(&s,.5);showcaseAction(&s,"music.play");assert(s.positionSeconds==112.5);
    assert(!showcaseAction(&s,"store.get.3"));assert(!showcaseAction(&s,"unknown"));puts("showcase_test: PASS (navigation, player controls, per-app simulated download, clock reset)");
}
