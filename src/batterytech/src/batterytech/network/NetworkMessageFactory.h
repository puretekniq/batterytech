/*
 * BatteryTech
 * Copyright (c) 2010 Battery Powered Games, LLC.
 *
 * This code is a component of BatteryTech and is subject to the 'BatteryTech
 * End User License Agreement'.  Among other important provisions, this
 * license prohibits the distribution of source code to anyone other than
 * authorized parties.  If you have any questions or would like an additional
 * copy of the license, please contact: support@batterypoweredgames.com
 */

//============================================================================
// Name        : NetworkMessageFactory.h
// Description : Subclass with your own message factory to instantiate all of the different message types you've created.
//============================================================================

#ifndef NETWORKMESSAGEFACTORY_H_
#define NETWORKMESSAGEFACTORY_H_

#include <batterytech/primitives.h>

namespace BatteryTech {

	class NetworkMessage;

	class NetworkMessageFactory {
	public:
		virtual ~NetworkMessageFactory(){};
		virtual NetworkMessage* createMessage(S32 messageType)=0;
	};

}
#endif /* NETWORKMESSAGEFACTORY_H_ */
