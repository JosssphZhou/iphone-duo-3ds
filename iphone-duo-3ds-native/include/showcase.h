#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "showcase_assets.h"

/* Local UI simulation only: no playback, store account, network or installation API. */
typedef enum { MUSIC_HOME, MUSIC_NEW, MUSIC_LIBRARY, MUSIC_SEARCH, MUSIC_ALBUM, MUSIC_PLAYER } MusicPage;
typedef enum { STORE_TODAY, STORE_GAMES, STORE_APPS, STORE_SEARCH, STORE_DETAIL } StorePage;
typedef enum { DOWNLOAD_IDLE, DOWNLOAD_PROGRESS, DOWNLOAD_READY } DownloadPhase;
typedef struct {
    MusicPage musicPage;
    StorePage storePage;
    bool playing,opened,shuffle,repeat,scrubbing;
    unsigned album,track,storeApp,browseAlbum;
    double positionSeconds;
    unsigned trackSeconds[3][3];
    uint32_t downloadMillis[3];
    DownloadPhase downloads[3];
    uint64_t lastTick;
    bool clockStarted;
} Showcase;
void showcaseInit(Showcase *state);
void showcaseResetClock(Showcase *state);
void showcaseTick(Showcase *state,uint64_t now);
bool showcaseAction(Showcase *state,const char *action);
bool showcaseBack(Showcase *state,bool music);
const char *showcaseRoute(const Showcase *state,bool music);
unsigned showcaseVariant(const Showcase *state,bool music);
unsigned showcaseTrackSeconds(const Showcase *state);
double showcaseDownloadProgress(const Showcase *state);

void showcaseSeek(Showcase *state,double fraction);

int showcaseFocusIndex(const ShowRegion *regions,size_t count,const char *action);
