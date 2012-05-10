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
// Name        : batterytechViewController.h
// Description : Primary driver class for Batterytech on IOS
//============================================================================

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include <CoreFoundation/CFURL.h>

#import "RemoteIOPlayer.h"

#import "MPAdView.h"
#import "OpenFeint.h"
#import "MPInterstitialAdController.h"

#define MAX_TOUCHES 10

@interface batterytechViewController : UIViewController <UIAccelerometerDelegate, MPAdViewDelegate, OpenFeintDelegate, MPInterstitialAdControllerDelegate, OFNotificationDelegate> {
    EAGLContext *context;
     
    BOOL animating;
    BOOL isContextInitialized;
    BOOL displayLinkSupported;
    NSInteger animationFrameInterval;
    /*
	 Use of the CADisplayLink class is the preferred method for controlling your animation timing.
	 CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
	 The NSTimer object is used only as fallback when running on a pre-3.1 device where CADisplayLink isn't available.
	 */
    id displayLink;
    NSThread *animationThread;
	RemoteIOPlayer *player;
	UITouch **touchIds;
    MPAdView *adBannerView;
    BOOL adBannerViewIsLoaded;
    NSString *kADBannerContentSizeIdentifierPortrait;
    NSString *kADBannerContentSizeIdentifierLandscape;
    int vpWidth;
    int vpHeight;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (nonatomic, retain) RemoteIOPlayer *player;
@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, retain) MPAdView *adBannerView;
@property (nonatomic) BOOL adBannerViewIsLoaded;

- (void)startAnimation;
- (void)stopAnimation;
- (void)createAd;
- (void)removeAd;
- (void)showAd;
- (void)showFullscreenAd;
- (void)hideAd;
- (void)initOF;
- (void)runMainLoop;
- (void)drawFrame;

@end
