/****************************************************************************
Copyright (c) 2010      cocos2d-x.org
Copyright (C) 2013-2014 Chukong Technologies Inc.
 
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

#include "base/CCIMEDispatcher.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <CCGLView.h>
#include <CCDirector.h>
#include <CCActionInterval.h>
#include <CCScene.h>
#endif
#include <list>

NS_CC_BEGIN
static bool isAttachIME = false;

//////////////////////////////////////////////////////////////////////////
// add/remove delegate in IMEDelegate Cons/Destructor
//////////////////////////////////////////////////////////////////////////

IMEDelegate::IMEDelegate()
{
    IMEDispatcher::sharedDispatcher()->addDelegate(this);
}

IMEDelegate::~IMEDelegate()
{
    IMEDispatcher::sharedDispatcher()->removeDelegate(this);
}

bool IMEDelegate::attachWithIME()
{
    return IMEDispatcher::sharedDispatcher()->attachDelegateWithIME(this);
}

bool IMEDelegate::detachWithIME()
{
    return IMEDispatcher::sharedDispatcher()->detachDelegateWithIME(this);
}

//////////////////////////////////////////////////////////////////////////

typedef std::list< IMEDelegate * > DelegateList;
typedef std::list< IMEDelegate * >::iterator  DelegateIter;

//////////////////////////////////////////////////////////////////////////
// Delegate List manage class
//////////////////////////////////////////////////////////////////////////

class IMEDispatcher::Impl
{
public:
    Impl()
    {
    }

    ~Impl()
    {

    }

    void init()
    {
        _delegateWithIme = 0;
    }

    DelegateIter findDelegate(IMEDelegate* delegate)
    {
        DelegateIter end = _delegateList.end();
        for (DelegateIter iter = _delegateList.begin(); iter != end; ++iter)
        {
            if (delegate == *iter)
            {
                return iter;
            }
        }
        return end;
    }

    DelegateList    _delegateList;
    IMEDelegate*  _delegateWithIme;
};

//////////////////////////////////////////////////////////////////////////
// Cons/Destructor
//////////////////////////////////////////////////////////////////////////

IMEDispatcher::IMEDispatcher()
: _impl(new IMEDispatcher::Impl)
{
	isAttachIME = false;
    _impl->init();
}

IMEDispatcher::~IMEDispatcher()
{
	isAttachIME = false;
    CC_SAFE_DELETE(_impl);
}

//////////////////////////////////////////////////////////////////////////
// Add/Attach/Remove IMEDelegate
//////////////////////////////////////////////////////////////////////////

void IMEDispatcher::addDelegate(IMEDelegate* delegate)
{
	isAttachIME = false;
    if (! delegate || ! _impl)
    {
        return;
    }
    if (_impl->_delegateList.end() != _impl->findDelegate(delegate))
    {
        // pDelegate already in list
        return;
    }
    _impl->_delegateList.push_front(delegate);
}

bool IMEDispatcher::attachDelegateWithIME(IMEDelegate * delegate)
{
    bool ret = false;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	if (isAttachIME)return false; //∑Òæˆ»´≤ø‘Ÿ¥Œattach≤Ÿ◊˜
#endif
    do
    {
        CC_BREAK_IF(! _impl || ! delegate);

        DelegateIter end  = _impl->_delegateList.end();
        DelegateIter iter = _impl->findDelegate(delegate);

        // if pDelegate is not in delegate list, return
        CC_BREAK_IF(end == iter);

        if (_impl->_delegateWithIme)
        {
            // if old delegate canDetachWithIME return false 
            // or pDelegate canAttachWithIME return false,
            // do nothing.

            CC_BREAK_IF(! _impl->_delegateWithIme->canDetachWithIME()
                || ! delegate->canAttachWithIME());

            // detach first
            IMEDelegate * oldDelegate = _impl->_delegateWithIme;
            _impl->_delegateWithIme = 0;
            oldDelegate->didDetachWithIME();

            _impl->_delegateWithIme = *iter;
            delegate->didAttachWithIME();
            ret = true;
            break;
        }
        // delegate hasn't attached to IME yet
        CC_BREAK_IF(! delegate->canAttachWithIME());

        _impl->_delegateWithIme = *iter;
        delegate->didAttachWithIME();
        ret = true;
    } while (0);
	if ( ret )
		isAttachIME = true;
    return ret;
}

bool IMEDispatcher::detachDelegateWithIME(IMEDelegate * delegate)
{
    bool ret = false;
    do
    {
        CC_BREAK_IF(! _impl || ! delegate);

        // if pDelegate is not the current delegate attached to IME, return
        CC_BREAK_IF(_impl->_delegateWithIme != delegate);

        CC_BREAK_IF(! delegate->canDetachWithIME());

        _impl->_delegateWithIme = 0;
        delegate->didDetachWithIME();
		CCLOG("detachDelegateWithIME");
        ret = true;
    } while (0);
	if(ret)
		isAttachIME = false;
    return ret;
}

void IMEDispatcher::removeDelegate(IMEDelegate* delegate)
{
	isAttachIME = false;
    do 
    {
        CC_BREAK_IF(! delegate || ! _impl);

        DelegateIter iter = _impl->findDelegate(delegate);
        DelegateIter end  = _impl->_delegateList.end();
        CC_BREAK_IF(end == iter);

        if (_impl->_delegateWithIme)

        if (*iter == _impl->_delegateWithIme)
        {
            _impl->_delegateWithIme = 0;
        }
        _impl->_delegateList.erase(iter);
    } while (0);
}

//////////////////////////////////////////////////////////////////////////
// dispatch text message
//////////////////////////////////////////////////////////////////////////

void IMEDispatcher::dispatchInsertText(const char * text, size_t len)
{
    do 
    {
        CC_BREAK_IF(! _impl || ! text || len <= 0);

        // there is no delegate attached to IME
        CC_BREAK_IF(! _impl->_delegateWithIme);

        _impl->_delegateWithIme->insertText(text, len);
    } while (0);
}

void IMEDispatcher::dispatchSetText(const char * text, size_t len)
{
    do 
    {
        CC_BREAK_IF(! _impl || ! text || len <= 0);

        // there is no delegate attached to IME
        CC_BREAK_IF(! _impl->_delegateWithIme);

        _impl->_delegateWithIme->setText(text, len);
    } while (0);
}

/*
 传递输入法输入时的临时字符串
 */
void IMEDispatcher::dispatchSetIMEText(const char * text, size_t len)
{
    do
    {        
        // there is no delegate attached to IME
        CC_BREAK_IF(! _impl->_delegateWithIme);
        
        _impl->_delegateWithIme->setIMEText(text, len);
    } while (0);
}

void IMEDispatcher::dispatchDeleteBackward()
{
    do 
    {
        CC_BREAK_IF(! _impl);

        // there is no delegate attached to IME
        CC_BREAK_IF(! _impl->_delegateWithIme);

        _impl->_delegateWithIme->deleteBackward();
    } while (0);
}

void IMEDispatcher::dispatchMoveCursor(int a,bool b)
{
    do 
    {
        CC_BREAK_IF(! _impl);

        // there is no delegate attached to IME
        CC_BREAK_IF(! _impl->_delegateWithIme);

		_impl->_delegateWithIme->moveCursor(a,b);
    } while (0);
}

void IMEDispatcher::dispatchOptKey( int key )
{
    do 
    {
        CC_BREAK_IF(! _impl);

        // there is no delegate attached to IME
        CC_BREAK_IF(! _impl->_delegateWithIme);

		_impl->_delegateWithIme->optKey(key);
    } while (0);
}

void IMEDispatcher::dispatchDeleteForward()
{
    do 
    {
        CC_BREAK_IF(! _impl);

        // there is no delegate attached to IME
        CC_BREAK_IF(! _impl->_delegateWithIme);

		_impl->_delegateWithIme->deleteForward();
    } while (0);
}

const std::string& IMEDispatcher::getContentText()
{
    if (_impl && _impl->_delegateWithIme)
    {
        return _impl->_delegateWithIme->getContentText();
    }
    return STD_STRING_EMPTY;
}

Rect IMEDispatcher::getContentRect()
{
    if (_impl && _impl->_delegateWithIme)
    {
        return _impl->_delegateWithIme->getContentRect();
    }
	return Rect();
}
//////////////////////////////////////////////////////////////////////////
// dispatch keyboard message
//////////////////////////////////////////////////////////////////////////
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
static Scene *s_Scene = nullptr;
static MoveTo *s_pmove = nullptr;
static bool s_isClose;
#endif
static void moveSceneIfNeed(IMEKeyboardNotificationInfo& info,const Rect& textBoxRect,bool b)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
        Director * pDirector = Director::getInstance();
        if( pDirector )
        {
            GLView * pview = pDirector->getOpenGLView();
            Size size = pview->getDesignResolutionSize();
            Scene *pscene = pDirector->getRunningScene();
            Vec2 pp = pscene->getPosition();
            if( b )
            {
                Vec2 p;
                //刚打开又要关闭
                if( s_pmove && !s_pmove->isDone() && s_isClose )
                {
                    s_pmove->stop();
                    s_pmove->release();
                    s_pmove = nullptr;
                }				
                if( pp.y == 0 && s_Scene==nullptr)
                {
                    // int vy = textBoxRect.origin.y+info.end.size.width + textBoxRect.size.height;
                    int vy;
                    if( info.begin.origin.x > 0 )
                        vy = textBoxRect.origin.y - info.end.size.width;
                    else
                        vy = textBoxRect.origin.y - info.end.size.height;
                    if( vy < 0 )
                    {
                        p.x = 0;
                        p.y = -vy+12; //多向上推一点
                        if(s_pmove)s_pmove->release();
                        s_pmove = MoveTo::create(info.duration,p);
                        s_isClose = false;
                        s_pmove->retain();
                        pscene->runAction( s_pmove );			  
                    }
                    else{
                        p.x = 0;
                        p.y = 0;
                    }
                }else
                {
                    p.x = 0;
                    if( info.begin.origin.x > 0 )
                        p.y = pp.y + info.end.size.width - info.begin.size.width;
                    else
						p.y = pp.y + info.end.size.height - info.begin.size.height;
                    pscene->setPosition(p);
                }
                s_Scene = pscene;
            }else if(s_Scene == pscene )
            {
                if(s_pmove)s_pmove->release();
                s_pmove = MoveTo::create(info.duration,Vec2(0,0));
                s_isClose = true;
                s_pmove->retain();
                pscene->runAction( s_pmove );			 
                s_Scene = nullptr;
            }
            else
            {
                s_Scene = nullptr;
                CCLOG("moveSceneIfNeed close s_Scene != pscene");
            }
          //  pmove->release();
        }
#endif
}

void IMEDispatcher::dispatchKeyboardWillShow(IMEKeyboardNotificationInfo& info)
{
    if (_impl)
    {
        IMEDelegate * delegate = 0;
        DelegateIter last = _impl->_delegateList.end();
        for (DelegateIter first = _impl->_delegateList.begin(); first != last; ++first)
        {
            delegate = *(first);
            if (delegate)
            {
                delegate->keyboardWillShow(info);
            }
        }
        /*
         */
        moveSceneIfNeed( info,getContentRect(),true );
    }
}

void IMEDispatcher::dispatchKeyboardDidShow(IMEKeyboardNotificationInfo& info)
{
    if (_impl)
    {
        IMEDelegate * delegate = 0;
        DelegateIter last = _impl->_delegateList.end();
        for (DelegateIter first = _impl->_delegateList.begin(); first != last; ++first)
        {
            delegate = *(first);
            if (delegate)
            {
                delegate->keyboardDidShow(info);
            }
        }
    }
}

void IMEDispatcher::dispatchKeyboardWillHide(IMEKeyboardNotificationInfo& info)
{
    if (_impl)
    {
        IMEDelegate * delegate = 0;
        DelegateIter last = _impl->_delegateList.end();
        for (DelegateIter first = _impl->_delegateList.begin(); first != last; ++first)
        {
            delegate = *(first);
            if (delegate)
            {
                delegate->keyboardWillHide(info);
            }
        }
        moveSceneIfNeed( info,getContentRect(),false );
    }
}

void IMEDispatcher::dispatchKeyboardDidHide(IMEKeyboardNotificationInfo& info)
{
    if (_impl)
    {
        IMEDelegate * delegate = 0;
        DelegateIter last = _impl->_delegateList.end();
        for (DelegateIter first = _impl->_delegateList.begin(); first != last; ++first)
        {
            delegate = *(first);
            if (delegate)
            {
                delegate->keyboardDidHide(info);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// protected member function
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// static member function
//////////////////////////////////////////////////////////////////////////

IMEDispatcher* IMEDispatcher::sharedDispatcher()
{
    static IMEDispatcher s_instance;
    return &s_instance;
}

NS_CC_END
