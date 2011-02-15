/*
 * PropertiesIO.cpp
 *
 *  Created on: Jan 3, 2011
 *      Author: rgreen
 */

#include "PropertiesIO.h"
#include "../platform/platformgeneral.h"
#include "../Logger.h"
#include <string.h>
#include <stdio.h>
#include "TextFileUtil.h"

PropertiesIO::PropertiesIO() {
}

PropertiesIO::~PropertiesIO() {
}

void PropertiesIO::saveProperties(ManagedArray<Property> *properties, const char* path) {
	char myPath[255];
	_platform_convert_path(path, myPath);
	logmsg("Saving to file:");
	logmsg(myPath);
	if (!_platform_path_exists(myPath)) {
		char basepath[255];
		_platform_get_basename(myPath, basepath);
		_platform_path_create_recursive(basepath);
	}
	FILE *file = fopen(myPath, "w");
	char buf[255];
	for (S32 i = 0; i < properties->getSize(); i++) {
		Property *prop = properties->array[i];
		buf[0] = '\0';
		strcat(buf, prop->getName());
		strcat(buf, "=");
		strcat(buf, prop->getValue());
		strcat(buf, "\n");
		fwrite(buf, sizeof(char), strlen(buf), file);
	}
	fclose(file);
	logmsg("Properties Saved");

}

// Loads properties from a file at path.  You are responsible for freeing returned data structure.
ManagedArray<Property>* PropertiesIO::loadPropertiesFromFile(const char* path) {
	char text[1024];
	FILE *file = fopen(path, "rb");
	fseek(file, 0L, SEEK_END);
	S32 size = ftell(file);
	rewind(file);
	fread(text, sizeof(char), size, file);
	fclose(file);
	text[size] = '\0';
	// read number of lines in file first to count
	char line[255];
	char nameBuf[255];
	char valueBuf[255];
	S32 pos = 0;
	BOOL32 isDone = FALSE;
	S32 lineCount = 0;
	while (!isDone) {
		isDone = !TextFileUtil::readLine(line, text, &pos);
		if (strlen(line) > 0) {
			lineCount++;
		}
	}
	pos = 0;
	isDone = FALSE;
	ManagedArray<Property> *properties = new ManagedArray<Property>(lineCount);
	while (!isDone) {
		isDone = !TextFileUtil::readLine(line, text, &pos);
		if (strlen(line) > 0) {
			logmsg(line);
			// find equals sign, break into before and after.
			char *sepPtr = strchr(line, '=');
			strncpy(nameBuf, line, sepPtr-line);
			nameBuf[sepPtr-line] = '\0';
			strncpy(valueBuf, sepPtr+1, strlen(line)-(sepPtr-line));
			valueBuf[strlen(line)-(sepPtr-line)] = '\0';
			properties->add(new Property(nameBuf, valueBuf));
		}
	}
	return properties;
}