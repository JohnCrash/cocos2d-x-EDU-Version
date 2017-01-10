/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 ****************************************************************************/
package org.cocos2dx.lib;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.graphics.Rect;
import android.opengl.GLSurfaceView;
import android.util.Log;

import org.cocos2dx.lib.Cocos2dxHelper;
public class Cocos2dxRenderer implements GLSurfaceView.Renderer {
	private  Cocos2dxCallback _activityCallback = null;
	public void setActivityCallback( Cocos2dxCallback funcs )
	{
		_activityCallback = funcs;
	}	
	// ===========================================================
	// Constants
	// ===========================================================

	private final static long NANOSECONDSPERSECOND = 1000000000L;
	private final static long NANOSECONDSPERMICROSECOND = 1000000;

	private static long sAnimationInterval = (long) (1.0 / 60 * Cocos2dxRenderer.NANOSECONDSPERSECOND);

	// ===========================================================
	// Fields
	// ===========================================================

	private long mLastTickInNanoSeconds;
	private int mScreenWidth;
	private int mScreenHeight;

	// ===========================================================
	// Constructors
	// ===========================================================

	// ===========================================================
	// Getter & Setter
	// ===========================================================

	public static void setAnimationInterval(final double pAnimationInterval) {
		Cocos2dxRenderer.sAnimationInterval = (long) (pAnimationInterval * Cocos2dxRenderer.NANOSECONDSPERSECOND);
	}

	public void setScreenWidthAndHeight(final int pSurfaceWidth, final int pSurfaceHeight) {
		this.mScreenWidth = pSurfaceWidth;
		this.mScreenHeight = pSurfaceHeight;
	}

	// ===========================================================
	// Methods for/from SuperClass/Interfaces
	// ===========================================================

	private boolean isCocosNativeInit = false;
	@Override
	public void onSurfaceCreated(final GL10 pGL10, final EGLConfig pEGLConfig) {
		Log.w("Cocos2dxRenderer","onSurfaceCreated");
		Cocos2dxRenderer.nativeInit(this.mScreenWidth, this.mScreenHeight);
		this.mLastTickInNanoSeconds = System.nanoTime();
		isCocosNativeInit = true;
	}

	@Override
	public void onSurfaceChanged(final GL10 pGL10, final int pWidth, final int pHeight) {
		Log.w("Cocos2dxRenderer","onSurfaceChanged");
		Cocos2dxRenderer.nativeOnSurfaceChanged(pWidth, pHeight);
	}

	@Override
	public void onDrawFrame(final GL10 gl) {
		/*
		 * FPS controlling algorithm is not accurate, and it will slow down FPS
		 * on some devices. So comment FPS controlling code.
		 */
		
		/*
		final long nowInNanoSeconds = System.nanoTime();
		final long interval = nowInNanoSeconds - this.mLastTickInNanoSeconds;
		*/

		// should render a frame when onDrawFrame() is called or there is a
		// "ghost"
		Cocos2dxRenderer.nativeRender();

		if(_activityCallback!=null)
			_activityCallback.onDrawFrame();
		/*
		// fps controlling
		if (interval < Cocos2dxRenderer.sAnimationInterval) {
			try {
				// because we render it before, so we should sleep twice time interval
				Thread.sleep((Cocos2dxRenderer.sAnimationInterval - interval) / Cocos2dxRenderer.NANOSECONDSPERMICROSECOND);
			} catch (final Exception e) {
			}
		}

		this.mLastTickInNanoSeconds = nowInNanoSeconds;
		*/
	}

	// ===========================================================
	// Methods
	// ===========================================================

	private static native void nativeTouchesBegin(final int pID, final float pX, final float pY);
	private static native void nativeTouchesEnd(final int pID, final float pX, final float pY);
	private static native void nativeTouchesMove(final int[] pIDs, final float[] pXs, final float[] pYs);
	private static native void nativeTouchesCancel(final int[] pIDs, final float[] pXs, final float[] pYs);
	private static native boolean nativeKeyDown(final int pKeyCode);
	private static native void nativeRender();
	private static native void nativeInit(final int pWidth, final int pHeight);
	private static native void nativeOnSurfaceChanged(final int pWidth, final int pHeight);
	private static native void nativeOnPause();
	private static native void nativeOnResume();
	
	public void handleActionDown(final int pID, final float pX, final float pY) {
		Cocos2dxRenderer.nativeTouchesBegin(pID, pX, pY);
	}

	public void handleActionUp(final int pID, final float pX, final float pY) {
		Cocos2dxRenderer.nativeTouchesEnd(pID, pX, pY);
	}

	public void handleActionCancel(final int[] pIDs, final float[] pXs, final float[] pYs) {
		Cocos2dxRenderer.nativeTouchesCancel(pIDs, pXs, pYs);
	}

	public void handleActionMove(final int[] pIDs, final float[] pXs, final float[] pYs) {
		Cocos2dxRenderer.nativeTouchesMove(pIDs, pXs, pYs);
	}

	public void handleKeyDown(final int pKeyCode) {
		Cocos2dxRenderer.nativeKeyDown(pKeyCode);
	}

	public void handleOnPause() {
		Cocos2dxHelper.onEnterBackground();
		if(isCocosNativeInit)
			Cocos2dxRenderer.nativeOnPause();
	}

	public void handleOnResume() {
		Cocos2dxHelper.onEnterForeground();
		if(isCocosNativeInit)
			Cocos2dxRenderer.nativeOnResume();
	}

	private static native void nativeInsertText(final String pText);
	private static native void nativeSetText(final String pText);
	private static native void nativeDeleteBackward();
	private static native String nativeGetContentText();
	private static native void nativeGetContentRect();

	static int _left,_right,_top,_bottom;
	
	private static void SetContentTextRect(final int x,final int y,final int width,final int height)
	{
		_left = x;
		_top = y;
		_right = x+width;
		_bottom = y+height;
	}
	
	public void handleInsertText(final String pText) {
		Cocos2dxRenderer.nativeInsertText(pText);
	}

	public void handleSetText(final String pText)
	{
		Cocos2dxRenderer.nativeSetText(pText);
	}
	public void handleDeleteBackward() {
		Cocos2dxRenderer.nativeDeleteBackward();
	}

	public String getContentText() {
		return Cocos2dxRenderer.nativeGetContentText();
	}

	public Rect getContextRect()
	{
		Cocos2dxRenderer.nativeGetContentRect();
		return new Rect( _left,_top,_right,_bottom );
	}
	// ===========================================================
	// Inner and Anonymous Classes
	// ===========================================================
}
