/*
 * PCMSound.h
 *
 *  Created on: Aug 11, 2010
 *      Author: rgreen
 */

#ifndef PCMSOUND_H_
#define PCMSOUND_H_

#include "../primitives.h"

class PCMSound {
public:
	PCMSound(S16 *pcmData, U32 length, U32 rate, U8 channels, const char *assetName, S32 soundId);
	virtual ~PCMSound();
	S16 *pcmData;
	U32 length;
	U32 rate;
	U8 channels;
	S32 soundId;
	const char *assetName;
};

#endif /* PCMSOUND_H_ */