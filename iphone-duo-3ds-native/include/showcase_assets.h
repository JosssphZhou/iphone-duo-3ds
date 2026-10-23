#pragma once
#include <stddef.h>
#include <stdint.h>
#include "geometry.h"

typedef enum {
    SHOW_OVERLAY_MINIPLAYER, SHOW_OVERLAY_PLAY, SHOW_OVERLAY_GET, SHOW_OVERLAY_PROGRESS,
    SHOW_OVERLAY_DOWNLOAD_PROGRESS, SHOW_OVERLAY_TRACK_TITLE,
    SHOW_OVERLAY_SHUFFLE, SHOW_OVERLAY_REPEAT, SHOW_OVERLAY_ELAPSED, SHOW_OVERLAY_REMAINING, SHOW_OVERLAY_OPENED
} ShowOverlayKind;
typedef struct { UiRect rect;const char *action; } ShowRegion;
typedef struct {
    UiRect rect;
    ShowOverlayKind kind;
    int images[9];
    uint32_t color,foreground,trackColor;
    int backgroundImage;
    UiRect backgroundRect;
    int item;
} ShowOverlay;
typedef struct {
    int image,openedImage;
    const ShowRegion *openedRegions;size_t openedRegionCount;
    const ShowRegion *regions;size_t regionCount;
    const ShowOverlay *overlays;size_t overlayCount;
} ShowView;
typedef struct {
    const char *route;
    unsigned variant;
    const char *atlasPath;
    ShowView views[4]; /* landscape top/bottom, portrait top/bottom */
} ShowPageAssets;

extern const ShowPageAssets showPages[];
extern const size_t showPageCount;
