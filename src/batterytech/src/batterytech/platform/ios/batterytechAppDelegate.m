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
// Name        : batterytechAppDelegate.m
// Description : IOS Batterytech App Delegate
//============================================================================

#import "batterytechAppDelegate.h"
#import "batterytechViewController.h"

@implementation batterytechAppDelegate

@synthesize window;
@synthesize viewController;

- (void)applicationWillResignActive:(UIApplication *)application
{
    //NSLog(@"applicationWillResignActive");
    [viewController stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
     //NSLog(@"applicationDidBecomeActive");
    [viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
     //NSLog(@"applicationWillTerminate");
    [viewController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    //NSLog(@"applicationDidEnterBackground");
    [viewController stopAnimation];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
     //NSLog(@"applicationWillEnterForeground");
    // Handle any foreground procedures not related to animation here.
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    // Override point for customization after app launch.
    // Add the controller's view to the window.
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];
    return YES;
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}

@end
