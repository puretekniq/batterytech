#ifdef _WIN32

#include <batterytech/primitives.h>
#include <batterytech/util/strx.h>
#include <batterytech/batterytech.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <windows.h>

using namespace std;

// This file is required for a Windows BatteryTech build.
// It implements several application-specific, platform-dependent functions.

DWORD CallbackThreadStart (LPVOID lpdwThreadParam );

void doCallback(const char *str) {
	char callback[512];
	sprintf(callback, "purchaseSucceeded %s", str);
	while (!btCallback(callback)) {
		Sleep(100);
	}
}

DWORD CallbackThreadStart (LPVOID lpdwThreadParam ) {
	doCallback((const char*) lpdwThreadParam);
	return 0;
}


void winHook(const char *hook, char *result, S32 resultLen) {
	// Handle custom hooks here
	if (strStartsWith(hook, "requestPurchase")) {
		// call back with success
		char hookData[512];
		strcpy(hookData, hook);
		strtok(hookData, " ");
		char *productId = strtok(NULL, " ");
		char callback[512];
		sprintf(callback, "purchaseSucceeded %s", productId);
		// if queue is full this won't work
		if (!btCallback(callback)) {
			cout << "Callback queue is full" << endl;
		}
	} else if (strStartsWith(hook, "restorePurchases")) {
		DWORD dwThreadId;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CallbackThreadStart, (LPVOID)"com.touchscreenpromotion.diesel.batmotriple", 0, &dwThreadId);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CallbackThreadStart, (LPVOID)"com.touchscreenpromotion.diesel.rockwellrearend", 0, &dwThreadId);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CallbackThreadStart, (LPVOID)"com.touchscreenpromotion.diesel.pipekittriple", 0, &dwThreadId);
	}
}

void winShowAd() {}

void winHideAd() {}

void winPlayVibrationEffect(S32 effectId, F32 intensity) {
	//cout << "Playing vibration effect " << effectId << " at " << intensity << endl;
}

void winStartVibrationEffect(S32 effectId, F32 intensity) {
	//cout << "Starting vibration effect " << effectId << " at " << intensity << endl;
}

void winStopVibrationEffect(S32 effectId) {
	//cout << "Stopping vibration effect " << effectId << endl;
}

void winStopAllVibrationEffects() {
	//cout << "Stopping all vibration effects" << endl;
}

#endif
