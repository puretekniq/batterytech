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
// Name        : DemoAppView.java
// Description : An implementation of this will be the main view of BatteryTech
//============================================================================

package com.batterypoweredgames.demoapp;

public interface DemoAppView {

	public abstract DemoAppRenderer getRenderer();

	public abstract void onPause();

	public abstract void onResume();

	public abstract void release();

}