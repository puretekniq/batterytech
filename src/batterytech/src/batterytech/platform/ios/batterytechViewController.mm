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
// Name        : batterytechViewController.mm
// Description : Primary driver class for Batterytech on IOS
//============================================================================

#import "batterytechViewController.h"
#import <QuartzCore/QuartzCore.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#import "EAGLView.h"
#include <batterytech/batterytech.h>
#include <batterytech/render/GraphicsConfiguration.h>
#include <batterytech/Logger.h>
#include "IAPManager.h"

#define USE_GLES2_WHEN_AVAILABLE TRUE

using namespace BatteryTech;

static GraphicsConfiguration *gConfig;
static double currentTime;
static double lastTime;
double getCurrentTime();
UIView *batterytechRootView;
// static reference for iosgeneral.mm to reach into for calls to this view (there will only ever be one of this view)
batterytechViewController *btViewController;

double getCurrentTime() {
	static mach_timebase_info_data_t sTimebaseInfo;
	uint64_t time = mach_absolute_time();
	uint64_t nanos;
	// If this is the first time we've run, get the timebase.
	// We can use denom == 0 to indicate that sTimebaseInfo is
	// uninitialised because it makes no sense to have a zero
	// denominator is a fraction.
	if ( sTimebaseInfo.denom == 0 ) {
		(void) mach_timebase_info(&sTimebaseInfo);
	}	
	// Do the maths.  We hope that the multiplication doesn't
	// overflow; the price you pay for working in fixed point.
	nanos = time * sTimebaseInfo.numer / sTimebaseInfo.denom;
	return ((double)nanos / 1000000000.0);
}

@implementation batterytechViewController

@synthesize context;
@synthesize player;

- (void)awakeFromNib
{
    btViewController = self;
	batterytechRootView = self.view;
	//touchIds = [[NSMutableArray alloc] initWithCapacity:10];
	touchIds = new UITouch*[MAX_TOUCHES];
	for (int i = 0; i < MAX_TOUCHES; i++) {
		touchIds[i] = 0;
	}
	gConfig = new GraphicsConfiguration;
	EAGLContext *aContext = NULL;
	if (USE_GLES2_WHEN_AVAILABLE) {
		aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	}
	if (!aContext) {
		aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		if (aContext) {
			NSLog(@"Initialized GLES 1.0/1.1 Context");
		}
	} else {
		NSLog(@"Initialized GLES 2.0 Context");
		gConfig->supportsShaders = TRUE;
	}
    
    if (!aContext) {
        NSLog(@"Failed to create ES context");
	} else if (![EAGLContext setCurrentContext:aContext]) {
        NSLog(@"Failed to set ES context current");
	}
    
	self.context = aContext;
	[aContext release];
	
    [(EAGLView *)self.view setContext:context];
    [(EAGLView *)self.view setFramebuffer];
	[(EAGLView *)self.view setMultipleTouchEnabled:YES];
	
    animating = FALSE;
    displayLinkSupported = FALSE;
    animationFrameInterval = 1;
    displayLink = nil;
    
    // Use of CADisplayLink requires iOS version 3.1 or greater.
	// The NSTimer object is used as fallback when it isn't available.
    NSString *reqSysVer = @"3.1";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
        displayLinkSupported = TRUE;
	const char *vendor = (const char*)glGetString(GL_VENDOR);
	const char *renderer = (const char*)glGetString(GL_RENDERER);
	const char *version = (const char*)glGetString(GL_VERSION);
	char buf[255];
	sprintf(buf, "OpenGL Vendor [%s]", vendor);
	logmsg(buf);
	sprintf(buf, "OpenGL Renderer [%s]", renderer);
	logmsg(buf);
	sprintf(buf, "OpenGL Version [%s]", version);
	logmsg(buf);
	
	gConfig->supportsHWmipmapgen = TRUE;
	gConfig->supportsVBOs = TRUE;
	gConfig->supportsUVTransform = TRUE;
	// TODO - why do we need to reverse these for landscape?? Should be width, height
    bool forceLandscape = TRUE;
	vpHeight = [(EAGLView *)self.view getFBHeight];
	vpWidth = [(EAGLView *)self.view getFBWidth];
    if (forceLandscape) {
        if (vpWidth < vpHeight) {
            // this is portrait and will be rotated to landscape - flip the values
            int temp = vpWidth;
            vpWidth = vpHeight;
            vpHeight = temp;
        }
    }
	btInit(gConfig, vpWidth, vpHeight);
	currentTime = getCurrentTime();
	//allocate the audio player∫
	player = [[RemoteIOPlayer alloc]init];
	//initialize the audio player
	[player initialiseAudio];
	[player start];
	// initialize accelerometer
	UIAccelerometer *accel = [UIAccelerometer sharedAccelerometer];
	accel.updateInterval = 1/30.0f;
	accel.delegate = self;
    isContextInitialized = TRUE;
}

- (void)dealloc
{
    isContextInitialized = FALSE;
    animating = FALSE;
    @synchronized(self) {
        btRelease();
    }
    // Tear down context.
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
	}
    [self removeAd];
    [context release];
	[player cleanUp];
	[player release];
	delete [] touchIds;
    [super dealloc];
}

- (void)showAd {
}

- (void)hideAd {
}

- (void)showFullscreenAd {
}

- (void)viewWillAppear:(BOOL)animated {
     //NSLog(@"viewWillAppear");
    [self startAnimation];
    [super viewWillAppear:animated];
	btResume();
}

- (void)viewDidAppear:(BOOL)animated {
    [self becomeFirstResponder];
    if (![self isFirstResponder]) {
        NSLog(@"batterytechViewController is not first responder!");
    } else {
        NSLog(@"batterytechViewController is first responder.");
    }
}

- (void)viewWillDisappear:(BOOL)animated {
    //NSLog(@"viewWillDisappear");
	btSuspend();
    [self stopAnimation];
    [super viewWillDisappear:animated];
}

- (void)viewDidUnload {
    //NSLog(@"viewDidUnload");
	[super viewDidUnload];
	
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;
	[player stop];
}

- (NSInteger)animationFrameInterval {
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval {
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation {
    @synchronized(self) {
        if (!animating) {
            animating = TRUE;
            animationThread = [[NSThread alloc] initWithTarget:self selector:@selector(runMainLoop) object:nil];
            [animationThread start];
        }
    }
}

- (void)stopAnimation {
    @synchronized(self) {
        if (animating) {
             animating = FALSE;
        }
    }
}

- (void)runMainLoop {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    while (animating) {
        if (isContextInitialized) {
            [self drawFrame];
        }
    }
    [animationThread release];
    animationThread = nil;
    [pool drain];
}
                                                                                                                                              
- (void)drawFrame {
	lastTime = currentTime;
	currentTime = getCurrentTime();
    @synchronized(self) {
        if (animating) {
            btUpdate(currentTime - lastTime);
            //NSLog(@"btUpdate %f", currentTime - lastTime);
            [(EAGLView *)self.view setFramebuffer];
            btDraw();
            [(EAGLView *)self.view presentFramebuffer];
        }
    }
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
    NSLog(@"Received memory warning!");
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    if (toInterfaceOrientation == UIInterfaceOrientationLandscapeRight) {
        return TRUE;
    } else {
        return FALSE;
    }
}

- (BOOL)canBecomeFirstResponder {
	return TRUE;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	// store memory address of UITouch using (int)(void*)touch in an array of max 10 size to track touch ids
	for (UITouch *touch in touches) {
		//NSLog(@"New Touch %i", (int)(void*)touch);
		//NSNumber *touchId = [NSNumber numberWithInt:(int)(void*)touch];
		int idx = -1;
		for (int i = 0; i < MAX_TOUCHES; i++) {
			if (touchIds[i] == 0) {
				touchIds[i] = touch;
				idx = i;
				break;
			}
		}
		CGPoint touchLocation = [touch locationInView: [touch view]];
		CGFloat scale = [(EAGLView *)self.view scaleFactor];
        if (isContextInitialized) {
            btSetPointerState(idx, TRUE, touchLocation.x * scale, touchLocation.y * scale);
        }
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		//NSNumber *touchId = [NSNumber numberWithInt:(int)(void*)touch];
		//NSLog(@"Touch Moved %i", (int)(void*)touch);
		//NSUInteger idx = [touchIds indexOfObject:touch];
		int idx = -1;
		for (int i = 0; i < MAX_TOUCHES; i++) {
			if (touchIds[i] == touch) {
				idx = i;
				break;
			}
		}
		if (idx > -1) {
			CGPoint touchLocation = [touch locationInView: [touch view]];
			CGFloat scale = [(EAGLView *)self.view scaleFactor];
            if (isContextInitialized) {
                btSetPointerState(idx, TRUE, touchLocation.x * scale, touchLocation.y * scale);
            }
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		//NSLog(@"Touch Ended %i", (int)(void*)touch);
		//NSNumber *touchId = [NSNumber numberWithInt:(int)(void*)touch];
		int idx = -1;
		for (int i = 0; i < MAX_TOUCHES; i++) {
			if (touchIds[i] == touch) {
				idx = i;
				break;
			}
		}
		if (idx > -1) {
			CGPoint touchLocation = [touch locationInView: [touch view]];
			CGFloat scale = [(EAGLView *)self.view scaleFactor];
            if (isContextInitialized) {
                btSetPointerState(idx, FALSE, touchLocation.x * scale, touchLocation.y * scale);
            }
			touchIds[idx] = 0;
		}
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		//NSLog(@"Touch Cancelled %i", (int)(void*)touch);
		//NSNumber *touchId = [NSNumber numberWithInt:(int)(void*)touch];
		int idx = -1;
		for (int i = 0; i < MAX_TOUCHES; i++) {
			if (touchIds[i] == touch) {
				idx = i;
				break;
			}
		}
		if (idx > -1) {
			CGPoint touchLocation = [touch locationInView: [touch view]];
			CGFloat scale = [(EAGLView *)self.view scaleFactor];	
            if (isContextInitialized) {
                btSetPointerState(idx, FALSE, touchLocation.x * scale, touchLocation.y * scale);
            }
			touchIds[idx] = 0;		
		}
	}
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration {
	UIAccelerationValue x,y,z;
	x = acceleration.x;
	y = acceleration.y;
	z = acceleration.z;
    if (isContextInitialized) {
        btAccelerometerChanged(-9.8f*x,-9.8f*y,-9.8f*z);
    }
}

@end
