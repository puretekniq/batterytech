/*
 * Context.h
 *
 *  Created on: Oct 8, 2010
 *      Author: rgreen
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "game/World.h"

class Context {
public:
	Context();
	virtual ~Context();
	World *world;
};

#endif /* CONTEXT_H_ */