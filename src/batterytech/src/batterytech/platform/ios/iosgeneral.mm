/*
 *  iosgeneral.cpp
 *  batterytech-ios
 *
 *  Created by Apple on 10/21/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE

#include "../platformgeneral.h"
#include "batterytechKeyboardDelegate.h"
#include <stdlib.h>
#include <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#include <errno.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define ASSETS_DIR "assets/"

SoundManager *_iosSndMgr;
UITextView *myTextView;
batterytechKeyboardDelegate *kbDelegate;

void _platform_log(const char* message) {
	NSLog(@"%s", message);
}

unsigned char* _platform_load_asset(const char *assetName, int *size) {
	char *data;
	char *lastDot = NULL;
	char *lastSep = NULL;
	// assetName will be "dir1/dir2/my_file.ogg"
	char path[1024]; // path should be "/assets/dir1/dir2"
	char extension[10]; // extension should be "ogg"
	char filename[1024]; // filename should be "my_file"
	lastSep = strrchr(assetName, '/');
	strcpy(filename, lastSep + 1);
	lastDot = strrchr(filename, '.');
	if (lastDot) {
		int lastDotPos = lastDot - filename;
		int extLength = strlen(filename) - lastDotPos - 1;
		strcpy(path, ASSETS_DIR);
		strncat(path, assetName, lastSep - assetName);
		path[strlen(ASSETS_DIR) + (lastSep - assetName)] = '\0';
		strncpy(extension, filename + lastDotPos + 1, extLength);
		extension[extLength] = '\0';
		// now that we've pulled the extension off, we can chop the filename to become the resource name.
		filename[lastDot - filename] = '\0';
		NSString *resourceName = [NSString stringWithCString:filename encoding: NSUTF8StringEncoding];
		NSString *extString = [NSString stringWithCString:extension encoding: NSUTF8StringEncoding];
		NSString *pathName = [NSString stringWithCString:path encoding: NSUTF8StringEncoding];
		NSString *filePath = [[NSBundle mainBundle] pathForResource:resourceName ofType:extString inDirectory:pathName];
		const char *filePathCString = [filePath UTF8String];
		FILE *handle;
		unsigned char *data = 0;
		handle = fopen(filePathCString, "rb");
		if (!handle) {
			NSLog(@"No File Handle");		
		}
		fseek(handle, 0L, SEEK_END);
		*size = ftell(handle);
		//fseek(handle, 0L, SEEK_SET);
		rewind(handle);
		data = (unsigned char*) malloc(sizeof(unsigned char) * *size);
		if (data) {
			int bytesRead = fread(data, sizeof(unsigned char), *size, handle);
			//cout << "malloc success, " << bytesRead << " bytes read of " << *size << endl;
		}
		int error = ferror(handle);
		if (error) {
			//cout << "IO error " << error << endl;
		}
		if (feof(handle)) {
			//cout << "EOF reached " << endl;
		}
		fclose(handle);
		return data;
	}
	return 0;
}

void _platform_free_asset(unsigned char *ptr) {
	if (ptr) {
		free(ptr);
	}
}

void _platform_init_sound(SoundManager *soundManager) {
	_iosSndMgr = soundManager;
}

void _platform_stop_sound() {
}

void _platform_get_external_storage_dir_name(char* buf, S32 buflen) {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	const char *docDirCString = [documentsDirectory UTF8String];
	strcpy(buf, docDirCString);
	buf[strlen(docDirCString)] = '\0';
}

const char* _platform_get_path_separator() {
	return "/";
}

BOOL32 _platform_path_exists(const char* path) {
	return (access(path, F_OK) != -1);
}

BOOL32 _platform_path_can_read(const char* path) {
	return (access(path, R_OK) != -1);
}

BOOL32 _platform_path_can_write(const char* path) {
	return (access(path, W_OK) != -1);
}

BOOL32 _platform_path_create(const char* path) {
	return (mkdir(path, (mode_t)0755) == 0);
}

void _platform_play_vibration_effect(S32 effectId, F32 intensity) {
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

void _platform_start_vibration_effect(S32 effectId, F32 intensity) {
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

void _platform_stop_vibration_effect(S32 effectId) {
}

void _platform_stop_all_vibration_effects() {
}

BOOL32 _platform_implements_soundpool() {
	return FALSE;
}

void _platform_init_soundpool(S32 streams) {
}

void _platform_release_soundpool() {
}

void _platform_load_sound(const char* asset) {
}

S32 _platform_play_sound(const char* asset, F32 leftVol, F32 rightVol, S32 loops, F32 rate) {
	return 0;
}

void _platform_stop_sound(S32 streamId) {
}

void _platform_stop_sound(const char* asset) {
}

void _platform_stop_all_sounds() {
}

void _platform_unload_sound(const char *asset) {
}

void _platform_show_keyboard() {
	NSLog(@"Showing keyboard");
	extern UIView *batterytechRootView;
	// create hidden textview and set as first responder
	myTextView = [[UITextView alloc] init];
	[batterytechRootView addSubview:myTextView];
	myTextView.frame.size.height = 1;
	myTextView.frame.size.width = 1;
	kbDelegate = [[batterytechKeyboardDelegate alloc] init];
	myTextView.delegate = kbDelegate;
	[myTextView becomeFirstResponder];
	
}

void _platform_hide_keyboard() {
	NSLog(@"Hiding keyboard");
	// resign responder from textview
	// release textview and null out
	if (myTextView) {
		[myTextView resignFirstResponder];
		[myTextView removeFromSuperview]; 
		[myTextView release];
		myTextView = NULL;
	}
	if (kbDelegate) {
		[kbDelegate release];
		kbDelegate = NULL;
	}
	if (FALSE) {
		// uncomment this if you need failsafe keyboard removal
		UIWindow* tempWindow;
		for(int c = 0; c < [[[UIApplication sharedApplication] windows] count]; c ++)
		{
			tempWindow = [[[UIApplication sharedApplication] windows] objectAtIndex:c];
			for(int i = 0; i < [tempWindow.subviews count]; i++)
			{
				UIView *keyboard = [tempWindow.subviews objectAtIndex:i];
				if([[keyboard description] hasPrefix:@"<UIKeyboard"] == YES
				   || [[keyboard description] hasPrefix:@"<UIPeripheralHostView"] == YES 
				   || [[keyboard description] hasPrefix:@"<UISnap"] == YES ){	
					[keyboard setHidden:TRUE];
				}
			}
		}
	}
}

void _platform_init_network() {}
void _platform_release_network() {}

void _platform_make_non_blocking(SOCKET socket) {
	fcntl(socket, F_SETFL, O_NONBLOCK);
}

S32 _platform_get_socket_last_error() {
	return errno;
}

char** _platform_get_ifaddrs(int *count) {
	//__android_log_print(ANDROID_LOG_DEBUG, "Batterytech", "Getting host names");
	*count = 0;
	char** hostnames = NULL;
	struct ifreq *ifr;
	struct ifconf ifc;
	int s, i;
	int numif;
	
	// find number of interfaces.
	memset(&ifc, 0, sizeof(ifc));
	ifc.ifc_ifcu.ifcu_req = NULL;
	ifc.ifc_len = 0;
	
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return hostnames;
	}
	if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
		return hostnames;
	}
	if ((ifr = (struct ifreq*) malloc(ifc.ifc_len)) == NULL) {
		return hostnames;
	}
	ifc.ifc_ifcu.ifcu_req = ifr;
	if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
		return hostnames;
	}
	close(s);
	numif = ifc.ifc_len / sizeof(struct ifreq);
	*count = numif;
	hostnames = new char*[numif];
	for (i = 0; i < numif; i++) {
		struct ifreq *r = &ifr[i];
		struct sockaddr_in *sin = (struct sockaddr_in *)&r->ifr_addr;
		hostnames[i] = new char[80];
		strcpy(hostnames[i], inet_ntoa(sin->sin_addr));
	}
	free(ifr);
	return hostnames;
}

void _platform_free_ifaddrs(char** ifaddrs, int count) {
	for (S32 i = 0; i < count; i++) {
		delete ifaddrs[i];
	}
	delete [] ifaddrs;
}
#endif