/*
 * batterytech_globals.h
 *
 *  Created on: Feb 20, 2011
 *      Author: rgreen
 */

#ifndef BATTERYTECH_GLOBALS_H_
#define BATTERYTECH_GLOBALS_H_

#include "primitives.h"

#define USE_SHADERS_WHEN_SUPPORTED TRUE
#define REFERENCE_WIDTH 480
#define REFERENCE_HEIGHT 800
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 800
#define TICK_SMOOTHER_SAMPLES 15
#define WINDOWED_APP_NAME "BatteryTech Demo App"
#define CONSOLE_LOG_ENABLED_WHEN_AVAILABLE TRUE
// UI and Text
#define INITIAL_FONT_TEXTURE_WIDTH 256
#define INITIAL_FONT_TEXTURE_HEIGHT 256
#define MAX_UI_SUBCOMPONENTS 10
// audio
#define PREFER_PLATFORM_AUDIO_MANAGEMENT TRUE
#define MAX_AUDIO_STREAMS 20
#define MAX_PCM_SOUNDS 200
#define PLAYBACK_RATE 44100
#define PLAYBACK_CHANNELS 2
#define STREAM_BUFFER_SIZE 64000
#define CHUNKED_READ_BUFFER_SIZE 16384
#define VORBIS_MAX_LEGAL_FRAME 8192


#endif /* BATTERYTECH_GLOBALS_H_ */