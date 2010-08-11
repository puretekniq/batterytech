/*
 * PCMSound.cpp
 *
 *  Created on: Aug 11, 2010
 *      Author: rgreen
 */

#include "PCMSound.h"
#include "../platform/platformgeneral.h"

PCMSound::PCMSound(S16 *pcmData, U32 length, U32 rate, U8 channels) {
	this->pcmData = pcmData;
	this->length = length;
	this->rate = rate;
	this->channels = channels;
}

PCMSound::~PCMSound() {
}

void PCMSound::release() {
	if (pcmData) {
		// we do not allocate the pcmData - the decoder does, but we are responsible for deallocating it when done.
		free(pcmData);
	}
}