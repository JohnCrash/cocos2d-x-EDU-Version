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

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.inputmethod.InputMethodManager;
import android.graphics.Rect;
import android.widget.FrameLayout;
import android.os.ResultReceiver;
import android.os.Bundle;
import android.view.ViewTreeObserver;

public class Cocos2dxGLSurfaceView extends GLSurfaceView {
	//
	private  Cocos2dxCallback _activityCallback = null;
	public void setActivityCallback( Cocos2dxCallback funcs )
	{
		_activityCallback = funcs;
	}
	// ===========================================================
	// Constants
	// ===========================================================

	private static final String TAG = Cocos2dxGLSurfaceView.class.getSimpleName();

	private final static int HANDLER_OPEN_IME_KEYBOARD = 2;
	private final static int HANDLER_CLOSE_IME_KEYBOARD = 3;

	// ===========================================================
	// Fields
	// ===========================================================

	/*
	 	加入一组像素转换函数
	 */
    public static int px2sp(Context context, float pxValue) { 
        final float fontScale = context.getResources().getDisplayMetrics().scaledDensity; 
        return (int) (pxValue / fontScale + 0.5f); 
    }
    public static int sp2px(Context context, float spValue) { 
        final float fontScale = context.getResources().getDisplayMetrics().scaledDensity; 
        return (int) (spValue * fontScale + 0.5f); 
    }
    public static int dip2px(Context context, float dipValue) { 
        final float scale = context.getResources().getDisplayMetrics().density; 
        return (int) (dipValue * scale + 0.5f); 
    } 
    public static int px2dip(Context context, float pxValue) { 
        final float scale = context.getResources().getDisplayMetrics().density; 
        return (int) (pxValue / scale + 0.5f); 
    } 
	// TODO Static handler -> Potential leak!
	private static Handler sHandler;

	private static Cocos2dxGLSurfaceView mCocos2dxGLSurfaceView;
	private static Cocos2dxTextInputWraper sCocos2dxTextInputWraper;

	private Cocos2dxRenderer mCocos2dxRenderer;
	private Cocos2dxEditText mCocos2dxEditText;
	private Cocos2dxEditText mCocos2dxEditMask;
	private static int sEditTextMinHeight = 96;
	// ===========================================================
	// Constructors
	// ===========================================================

	public Cocos2dxGLSurfaceView(final Context context) {
		super(context);

		this.initView();
	}

	public Cocos2dxGLSurfaceView(final Context context, final AttributeSet attrs) {
		super(context, attrs);
		
		this.initView();
	}

	protected void enableEdit(boolean b)
	{
		if(null == Cocos2dxGLSurfaceView.this.mCocos2dxEditText)return;
		if(b)
		{
			Rect rect = Cocos2dxGLSurfaceView.mCocos2dxGLSurfaceView.getContentRect();
			Cocos2dxGLSurfaceView.this.mCocos2dxEditText.setVisibility(android.view.View.VISIBLE);
			int height = rect.height() > sEditTextMinHeight ?rect.height():sEditTextMinHeight;
		//	Cocos2dxGLSurfaceView.this.mCocos2dxEditText.setTextSize(px2sp(getContext(),rect.height()));
			int dy = (height - rect.height())/2;
			FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(rect.width(),height);
			int screenHeight = mCocos2dxGLSurfaceView.getHeight();
			if( rect.top-dy + height > screenHeight )
			{
				dy += (rect.top-dy + height-screenHeight);
			}
			lp.setMargins(rect.left,rect.top-dy,rect.right,rect.bottom);
			Cocos2dxGLSurfaceView.this.mCocos2dxEditText.setLayoutParams(lp);
		}
		else
		{
			Cocos2dxGLSurfaceView.this.mCocos2dxEditText.setVisibility(android.view.View.INVISIBLE);
			FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(1,1);
			lp.setMargins(0, 0, 1, 1);
			Cocos2dxGLSurfaceView.this.mCocos2dxEditText.setLayoutParams(lp);
		}
	}
	private static int _softinputHeight = 0;
	/*
	 	下面代码当软键盘打开时给调用，可以计算出软键盘的高度(heightDifference)
	 */
	protected void calcSoftKeyboardHeight()
	{
		mCocos2dxGLSurfaceView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
			 @Override
             public void onGlobalLayout() {
                 // TODO Auto-generated method stub
                 Rect r = new Rect();
                 mCocos2dxGLSurfaceView.getWindowVisibleDisplayFrame(r);

                 int screenHeight = mCocos2dxGLSurfaceView.getRootView().getHeight();
                 int heightDifference = screenHeight - (r.bottom - r.top);
                 if( heightDifference > 0 )
                 {
                	 _softinputHeight = heightDifference; //最近一次软键盘的高度
                 }
             }			
		});
	}

	protected void initView() {
		this.setEGLContextClientVersion(2);
		this.setFocusableInTouchMode(true);
		/*
		 * 暂时没有使用因为我发现在FrameLayout中加入多加入一个EditText，弹出时就可以正常看到编辑框了
		 */
		//calcSoftKeyboardHeight();
		Cocos2dxGLSurfaceView.mCocos2dxGLSurfaceView = this;
		Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper = new Cocos2dxTextInputWraper(this);
		Cocos2dxGLSurfaceView.sHandler = new Handler() {
			@Override
			public void handleMessage(final Message msg) {
				switch (msg.what) {
					case HANDLER_OPEN_IME_KEYBOARD:
						if (null != Cocos2dxGLSurfaceView.this.mCocos2dxEditText && Cocos2dxGLSurfaceView.this.mCocos2dxEditText.requestFocus()) {
							enableEdit(true);
							Cocos2dxGLSurfaceView.this.mCocos2dxEditText.removeTextChangedListener(Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper);
							Cocos2dxGLSurfaceView.this.mCocos2dxEditText.setText("");
							final String text = (String) msg.obj;
							Cocos2dxGLSurfaceView.this.mCocos2dxEditText.append(text);
							Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper.setOriginText(text);
							Cocos2dxGLSurfaceView.this.mCocos2dxEditText.addTextChangedListener(Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper);
							final InputMethodManager imm = (InputMethodManager) Cocos2dxGLSurfaceView.mCocos2dxGLSurfaceView.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
							imm.showSoftInput(Cocos2dxGLSurfaceView.this.mCocos2dxEditText, 0);
							Log.d("GLSurfaceView", "showSoftInput");
						}
						break;

					case HANDLER_CLOSE_IME_KEYBOARD:
						if (null != Cocos2dxGLSurfaceView.this.mCocos2dxEditText) {
							//enableEdit(false);
							//final String text = Cocos2dxGLSurfaceView.this.mCocos2dxEditText.getText().toString();
							//Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper.setOriginText(text);
							//Log.d("GLSurfaceView", "HideSoftInput"+text);
							//Cocos2dxGLSurfaceView.this.mCocos2dxEditText.setVisibility(4);
							
							Cocos2dxGLSurfaceView.this.mCocos2dxEditText.removeTextChangedListener(Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper);
							final InputMethodManager imm = (InputMethodManager) Cocos2dxGLSurfaceView.mCocos2dxGLSurfaceView.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
							imm.hideSoftInputFromWindow(Cocos2dxGLSurfaceView.this.mCocos2dxEditText.getWindowToken(), 0);
							Cocos2dxGLSurfaceView.this.requestFocus();
							Log.d("GLSurfaceView", "HideSoftInput");
						}
						break;
				}
			}
		};
	}

	// ===========================================================
	// Getter & Setter
	// ===========================================================


       public static Cocos2dxGLSurfaceView getInstance() {
	   return mCocos2dxGLSurfaceView;
       }

       public static void queueAccelerometer(final float x, final float y, final float z, final long timestamp) {	
	   mCocos2dxGLSurfaceView.queueEvent(new Runnable() {
		@Override
		    public void run() {
			    Cocos2dxAccelerometer.onSensorChanged(x, y, z, timestamp);
		}
	    });
	}

	public void setCocos2dxRenderer(final Cocos2dxRenderer renderer) {
		this.mCocos2dxRenderer = renderer;
		this.setRenderer(this.mCocos2dxRenderer);
	}

	private String getContentText() {
		return this.mCocos2dxRenderer.getContentText();
	}

	private Rect getContentRect()
	{
		return this.mCocos2dxRenderer.getContextRect();
	}
	public Cocos2dxEditText getCocos2dxEditText() {
		return this.mCocos2dxEditText;
	}

	public void setCocos2dxEditText(final Cocos2dxEditText pCocos2dxEditText) {
		this.mCocos2dxEditText = pCocos2dxEditText;
		
		if (null != this.mCocos2dxEditText && null != Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper) {
			this.mCocos2dxEditText.setOnEditorActionListener(Cocos2dxGLSurfaceView.sCocos2dxTextInputWraper);
			this.mCocos2dxEditText.setCocos2dxGLSurfaceView(this);
			this.requestFocus();
		}
	}

	// ===========================================================
	// Methods for/from SuperClass/Interfaces
	// ===========================================================

	@Override
	public void onResume() {
		super.onResume();
		this.setRenderMode(RENDERMODE_CONTINUOUSLY);
		this.queueEvent(new Runnable() {
			@Override
			public void run() {
				Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleOnResume();
			}
		});
	}

	@Override
	public void onPause() {
		this.queueEvent(new Runnable() {
			@Override
			public void run() {
				Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleOnPause();
			}
		});
		this.setRenderMode(RENDERMODE_WHEN_DIRTY);
		//super.onPause();
	}

	@Override
	public boolean onTouchEvent(final MotionEvent pMotionEvent) {
		// these data are used in ACTION_MOVE and ACTION_CANCEL
		final int pointerNumber = pMotionEvent.getPointerCount();
		final int[] ids = new int[pointerNumber];
		final float[] xs = new float[pointerNumber];
		final float[] ys = new float[pointerNumber];

		for (int i = 0; i < pointerNumber; i++) {
			ids[i] = pMotionEvent.getPointerId(i);
			xs[i] = pMotionEvent.getX(i);
			ys[i] = pMotionEvent.getY(i);
		}

		switch (pMotionEvent.getAction() & MotionEvent.ACTION_MASK) {
			case MotionEvent.ACTION_POINTER_DOWN:
				final int indexPointerDown = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
				final int idPointerDown = pMotionEvent.getPointerId(indexPointerDown);
				final float xPointerDown = pMotionEvent.getX(indexPointerDown);
				final float yPointerDown = pMotionEvent.getY(indexPointerDown);

				this.queueEvent(new Runnable() {
					@Override
					public void run() {
						Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleActionDown(idPointerDown, xPointerDown, yPointerDown);
					}
				});
				break;

			case MotionEvent.ACTION_DOWN:
				// there are only one finger on the screen
				final int idDown = pMotionEvent.getPointerId(0);
				final float xDown = xs[0];
				final float yDown = ys[0];

				this.queueEvent(new Runnable() {
					@Override
					public void run() {
						Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleActionDown(idDown, xDown, yDown);
					}
				});
				break;

			case MotionEvent.ACTION_MOVE:
				this.queueEvent(new Runnable() {
					@Override
					public void run() {
						Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleActionMove(ids, xs, ys);
					}
				});
				break;

			case MotionEvent.ACTION_POINTER_UP:
				final int indexPointUp = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
				final int idPointerUp = pMotionEvent.getPointerId(indexPointUp);
				final float xPointerUp = pMotionEvent.getX(indexPointUp);
				final float yPointerUp = pMotionEvent.getY(indexPointUp);

				this.queueEvent(new Runnable() {
					@Override
					public void run() {
						Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleActionUp(idPointerUp, xPointerUp, yPointerUp);
					}
				});
				break;

			case MotionEvent.ACTION_UP:
				// there are only one finger on the screen
				final int idUp = pMotionEvent.getPointerId(0);
				final float xUp = xs[0];
				final float yUp = ys[0];

				this.queueEvent(new Runnable() {
					@Override
					public void run() {
						Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleActionUp(idUp, xUp, yUp);
					}
				});
				break;

			case MotionEvent.ACTION_CANCEL:
				this.queueEvent(new Runnable() {
					@Override
					public void run() {
						Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleActionCancel(ids, xs, ys);
					}
				});
				break;
		}

        /*
		if (BuildConfig.DEBUG) {
			Cocos2dxGLSurfaceView.dumpMotionEvent(pMotionEvent);
		}
		*/
		return true;
	}

	/*
	 * This function is called before Cocos2dxRenderer.nativeInit(), so the
	 * width and height is correct.
	 */
	@Override
	protected void onSizeChanged(final int pNewSurfaceWidth, final int pNewSurfaceHeight, final int pOldSurfaceWidth, final int pOldSurfaceHeight) {
		if(!this.isInEditMode()) {
			this.mCocos2dxRenderer.setScreenWidthAndHeight(pNewSurfaceWidth, pNewSurfaceHeight);
		}
		if( _activityCallback!=null )
			_activityCallback.onSizeChanged(pNewSurfaceWidth, pNewSurfaceHeight);
	}

	@Override
	public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
		switch (pKeyCode) {
			case KeyEvent.KEYCODE_BACK:
				Cocos2dxVideoHelper.mVideoHandler.sendEmptyMessage(Cocos2dxVideoHelper.KeyEventBack);
			case KeyEvent.KEYCODE_MENU:
			case KeyEvent.KEYCODE_DPAD_LEFT:
			case KeyEvent.KEYCODE_DPAD_RIGHT:
			case KeyEvent.KEYCODE_DPAD_UP:
			case KeyEvent.KEYCODE_DPAD_DOWN:
			case KeyEvent.KEYCODE_ENTER:
			case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
			case KeyEvent.KEYCODE_DPAD_CENTER:
				this.queueEvent(new Runnable() {
					@Override
					public void run() {
						Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleKeyDown(pKeyCode);
					}
				});
				return true;
			default:
				return super.onKeyDown(pKeyCode, pKeyEvent);
		}
	}

	// ===========================================================
	// Methods
	// ===========================================================

	// ===========================================================
	// Inner and Anonymous Classes
	// ===========================================================

	public static void openIMEKeyboard() {
		final Message msg = new Message();
		msg.what = Cocos2dxGLSurfaceView.HANDLER_OPEN_IME_KEYBOARD;
		msg.obj = Cocos2dxGLSurfaceView.mCocos2dxGLSurfaceView.getContentText();
		Cocos2dxGLSurfaceView.sHandler.sendMessage(msg);
	}

	public static void closeIMEKeyboard() {
		final Message msg = new Message();
		msg.what = Cocos2dxGLSurfaceView.HANDLER_CLOSE_IME_KEYBOARD;
		Cocos2dxGLSurfaceView.sHandler.sendMessage(msg);
	}

	public void insertText(final String pText) {
		this.queueEvent(new Runnable() {
			@Override
			public void run() {
				Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleInsertText(pText);
			}
		});
	}

	public void setText(final String pText) {
		this.queueEvent(new Runnable() {
			@Override
			public void run() {
				Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleSetText(pText);
			}
		});
	}
	
	public void deleteBackward() {
		this.queueEvent(new Runnable() {
			@Override
			public void run() {
				Log.d("deleteBackward","");
				Cocos2dxGLSurfaceView.this.mCocos2dxRenderer.handleDeleteBackward();
			}
		});
	}

	private static void dumpMotionEvent(final MotionEvent event) {
		final String names[] = { "DOWN", "UP", "MOVE", "CANCEL", "OUTSIDE", "POINTER_DOWN", "POINTER_UP", "7?", "8?", "9?" };
		final StringBuilder sb = new StringBuilder();
		final int action = event.getAction();
		final int actionCode = action & MotionEvent.ACTION_MASK;
		sb.append("event ACTION_").append(names[actionCode]);
		if (actionCode == MotionEvent.ACTION_POINTER_DOWN || actionCode == MotionEvent.ACTION_POINTER_UP) {
			sb.append("(pid ").append(action >> MotionEvent.ACTION_POINTER_INDEX_SHIFT);
			sb.append(")");
		}
		sb.append("[");
		for (int i = 0; i < event.getPointerCount(); i++) {
			sb.append("#").append(i);
			sb.append("(pid ").append(event.getPointerId(i));
			sb.append(")=").append((int) event.getX(i));
			sb.append(",").append((int) event.getY(i));
			if (i + 1 < event.getPointerCount()) {
				sb.append(";");
			}
		}
		sb.append("]");
		Log.d(Cocos2dxGLSurfaceView.TAG, sb.toString());
	}
}
