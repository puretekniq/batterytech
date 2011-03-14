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
// Name        : batterytechGLView.mm
// Description : Primary driver for Batterytech on OSX
//============================================================================

#import "batterytechGLView.h"
#import <QuartzCore/QuartzCore.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#include <batterytech/batterytech.h>
#include <batterytech/render/GraphicsConfiguration.h>

#define USE_SHADERS TRUE

using namespace BatteryTech;

static GraphicsConfiguration *gConfig;
static double currentTime;
static double lastTime;
double getCurrentTime();
static int frameWidth;
static int frameHeight;

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

@implementation BatterytechGLView

@synthesize player;

+ (NSOpenGLPixelFormat*) basicPixelFormat
{
    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,	// double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
        (NSOpenGLPixelFormatAttribute)nil
    };
    return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
}

// Method initWithFrame:
// for this to be called properly when playing this view using IB, you need to actually drag and drop a plain NSView
// from the library, then set the classname property to BatterytechGLView.  If you drag this view directly, the
// other coded init method will be called (thank apple for that quirk).
-(id) initWithFrame: (NSRect) frameRect {
	NSOpenGLPixelFormat * pf = [BatterytechGLView basicPixelFormat];
	self = [super initWithFrame: frameRect pixelFormat: pf];
	frameWidth = frameRect.size.width;
	frameHeight = frameRect.size.height;
    return self;
}

// Method awakeFromNib
// GL init has already happened since initWithFrame is called first.  This sets up the main loop, audio and GL capabilities.
- (void) awakeFromNib {
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval]; // set to vbl sync
	const char *vendor = (const char*)glGetString(GL_VENDOR);
	const char *renderer = (const char*)glGetString(GL_RENDERER);
	const char *version = (const char*)glGetString(GL_VERSION);
	NSLog(@"OpenGL Vendor [%s]", vendor);
	NSLog(@"OpenGL Renderer [%s]", renderer);
	NSLog(@"OpenGL Version [%s]", version);
	gConfig = new GraphicsConfiguration;
	gConfig->supportsHWmipmapgen = TRUE;
	gConfig->supportsVBOs = TRUE;
	gConfig->supportsUVTransform = TRUE;
	if (USE_SHADERS) {
		gConfig->supportsShaders = TRUE;
	}
	btInit(gConfig, frameWidth, frameHeight);
	currentTime = getCurrentTime();
	//allocate the audio player∫
	player = [[RemoteIOPlayer alloc]init];
	//initialize the audio player
	[player initialiseAudio];
	[player start];
	
	// start animation timer with a 1ms update (it's vsynched so it will run at full monitor refresh rate
	timer = [NSTimer timerWithTimeInterval:(0.001f) target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode]; // ensure timer fires during resize
}

- (void)dealloc {
	[player cleanUp];
	[player release];
	btRelease();
    [super dealloc];
}

- (void)animationTimer:(NSTimer *)timer {
	lastTime = currentTime;
	currentTime = getCurrentTime();
	btUpdate(currentTime - lastTime);
	[[self openGLContext] makeCurrentContext];
	btDraw();
	[[self openGLContext] flushBuffer];
}

// this can be a troublesome call to do anything heavyweight, as it is called on window moves, resizes, and display config changes.  So be
// careful of doing too much here.
- (void) update // window resizes, moves and display changes (resize, depth and display config change)
{
	//[msgStringTex setString:[NSString stringWithFormat:@"update at %0.1f secs", msgTime]  withAttributes:stanStringAttrib];
	[super update];
	if (![self inLiveResize])  {// if not doing live resize
	}
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (BOOL)becomeFirstResponder {
	return  YES;
}

- (BOOL)resignFirstResponder {
	return YES;
}

- (void)mouseDown:(NSEvent *)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if ([theEvent modifierFlags] & NSControlKeyMask) {
	} else if ([theEvent modifierFlags] & NSAlternateKeyMask) {
	}
	btSetPointerState(0, TRUE, location.x, frameHeight - (location.y - 1)); 
}

- (void)mouseUp:(NSEvent *)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	btSetPointerState(0, FALSE, location.x, frameHeight - (location.y - 1)); 
}

- (void)mouseDragged:(NSEvent *)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	btSetPointerState(0, TRUE, location.x, frameHeight - (location.y - 1)); 
}

-(void)keyDown:(NSEvent *)theEvent {
    NSString *characters = [theEvent characters];
    if ([characters length]) {
        unichar character = [characters characterAtIndex:0];
		btKeyPressed(character, BatteryTech::SKEY_NULL);
	}
}

@end
