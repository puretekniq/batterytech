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
// Name        : PropertiesIO.h
// Description : Reads and writes properties to and from text file.
//============================================================================

#ifndef PROPERTIESIO_H_
#define PROPERTIESIO_H_

#include "ManagedArray.h"
#include "Property.h"

class PropertiesIO {
public:
	PropertiesIO();
	virtual ~PropertiesIO();
	void saveProperties(ManagedArray<Property> *properties, const char* path);
	ManagedArray<Property>* loadPropertiesFromFile(const char* path);
};

#endif /* PROPERTIESIO_H_ */
