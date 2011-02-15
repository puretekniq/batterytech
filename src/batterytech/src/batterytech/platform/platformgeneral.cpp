/*
 * platformgeneral.cpp
 *
 *  Created on: Dec 8, 2010
 *      Author: rgreen
 */

#include "platformgeneral.h"
#include <string.h>
#include "../Logger.h"

void _platform_convert_path(const char* path, char *newPath) {
	strcpy(newPath, path);
	const char *pathSep = _platform_get_path_separator();
	int sLength = strlen(newPath);
	int i;
	for (i = 0; i < sLength; i++) {
		if (pathSep[0] == '\\') {
			if (newPath[i] == '/') {
				newPath[i] = '\\';
			}
		} else {
			if (newPath[i] == '\\') {
				newPath[i] = '/';
			}
		}
	}
}

void _platform_get_basename(const char* path, char *baseName) {
	int lastSepIdx = -1;
	const char *pathSep = _platform_get_path_separator();
	int sLength = strlen(path);
	int i;
	for (i = 0; i < sLength; i++) {
		if (path[i] == pathSep[0]) {
			lastSepIdx = i;
		}
	}
	if (lastSepIdx == -1) {
		baseName[0] = '\0';
	} else {
		strncpy(baseName, path, lastSepIdx);
		baseName[lastSepIdx] = '\0';
	}
}

int _platform_find_first_basename(const char* path, char *basename, int idx) {
	// finds the first path separator after path[idx] and returns full basename with trailing separator
	const char *pathSep = _platform_get_path_separator();
	int sLength = strlen(path);
	int i;
	for (i = idx; i < sLength; i++) {
		if (path[i] == pathSep[0]) {
			strncpy(basename, path, i + 1);
			basename[i + 1] = '\0';
			return i + 1;
		}
	}
	basename[0] = '\0';
	return 0;

}

void _platform_path_create_recursive(const char* path) {
	logmsg("Creating path recursively:");
	logmsg(path);
	// path must be a basename, does NOT have a file on the end!
	// find first basename,
	// while basename
	// check/create
	// find next basename
	char basename[255];
	int idx = _platform_find_first_basename(path, basename, 0);
	while (strlen(basename) > 0) {
		if (!_platform_path_exists(basename)) {
			logmsg("Creating path:");
			logmsg(basename);
			_platform_path_create(basename);
		}
		idx = _platform_find_first_basename(path, basename, idx);
	}
	// now finally the full path
	if (!_platform_path_exists(path)) {
		_platform_path_create(path);
	}
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}