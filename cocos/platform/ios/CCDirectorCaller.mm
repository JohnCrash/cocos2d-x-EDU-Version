/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#include "base/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>
#import "CCDirectorCaller.h"
#import "CCDirector.h"
#import "CCGLView.h"
#import "CCEAGLView.h"

static id s_sharedDirectorCaller;

@interface NSObject(CADisplayLink)
+(id) displayLinkWithTarget: (id)arg1 selector:(SEL)arg2;
-(void) addToRunLoop: (id)arg1 forMode: (id)arg2;
-(void) setFrameInterval: (NSInteger)interval;
-(void) invalidate;
@end

@implementation CCDirectorCaller_v3

@synthesize interval;

+(id) sharedDirectorCaller
{
    if (s_sharedDirectorCaller == nil)
    {
        s_sharedDirectorCaller = [CCDirectorCaller_v3 new];
    }
    
    return s_sharedDirectorCaller;
}

+(void) destroy
{
    cocos2d::Director* director = cocos2d::Director::getInstance();
    [(CCEAGLView*)director->getOpenGLView()->getEAGLView() releaseRendererImmediate];
    [s_sharedDirectorCaller stopMainLoop];
    [s_sharedDirectorCaller release];
    s_sharedDirectorCaller = nil;
}

-(void) alloc
{
        interval = 1;
}

-(void) dealloc
{
    [displayLink release];
    [super dealloc];
}

-(void) startMainLoop
{
        // Director::setAnimationInterval() is called, we should invalidate it first
    [self stopMainLoop];
    
    displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(doCaller:)];
    [displayLink setFrameInterval: self.interval];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

-(void) stopMainLoop
{
    [displayLink invalidate];
    displayLink = nil;
}

-(void) setAnimationInterval:(double)intervalNew
{
    // Director::setAnimationInterval() is called, we should invalidate it first
    [self stopMainLoop];
        
    self.interval = 60.0 * intervalNew;
        
    displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(doCaller:)];
    [displayLink setFrameInterval: self.interval];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}
                      
-(void) doCaller: (id) sender
{
    cocos2d::Director* director = cocos2d::Director::getInstance();
    [EAGLContext setCurrentContext: [(CCEAGLView*)director->getOpenGLView()->getEAGLView() context]];
    director->mainLoop();
}

@end

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS

