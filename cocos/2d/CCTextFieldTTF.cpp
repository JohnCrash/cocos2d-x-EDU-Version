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

#include "CCTextFieldTTF.h"
#include "2d/CCSprite.h"
#include "base/CCDirector.h"
#include "CCGLView.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCCustomCommand.h"
#include "CCDrawingPrimitives.h"
#include "base/ccUTF8.h"

NS_CC_BEGIN

TextFieldTTF *TextFieldTTF::_currentActive = nullptr;

static int _calcCharCount(const char * text)
{
    int n = 0;
    char ch = 0;
    while ((ch = *text))
    {
        CC_BREAK_IF(! ch);

        if (0x80 != (0xC0 & ch))
        {
            ++n;
        }
        ++text;
    }
    return n;
}

static int utf8length(const std::string& str)
{
	return _calcCharCount(str.c_str());
}

static void utf8index(const std::string& str,int idx,int* pidx,int* plen )
{
	int len = str.length();
	int n = 0;
	*plen = -1;
	*pidx = -1;
	for( int i = 0;i<len;i++ )
	{
		char ch = str.at(i);
		if( n == idx )
			*pidx = i;
		else if( n == idx + 1 )
		{
			*plen = i-*pidx;
			return;
		}
		if( (0xC0&ch)!=0x80 )
		{
			++n;
		}
	}
	if( *pidx != -1 && *plen != -1 )
	{
		return;
	}
	else if( *pidx != -1 )
	{
		*plen = len-*pidx;
	}
	else
	{
		*pidx = len;
		*plen = 0;
	}
}
//////////////////////////////////////////////////////////////////////////
// constructor and destructor
//////////////////////////////////////////////////////////////////////////

TextFieldTTF::TextFieldTTF()
: _delegate(0)
, _charCount(0)
, _inputText("")
, _placeHolder("")   // prevent Label initWithString assertion
, _secureTextEntry(false)
, _colorText(Color4B::WHITE)
,_cpos(0)
,_cursordt(0)
,_cursorb(false)
,_cx(0),_cy(0),_cwidth(1),_cheight(32)
,_showcursor(false)
,_selpos(-1)
,_imepos(-1)
{
    _colorSpaceHolder.r = _colorSpaceHolder.g = _colorSpaceHolder.b = 127;
    _colorSpaceHolder.a = 255;
}

TextFieldTTF::~TextFieldTTF()
{
}

void TextFieldTTF::updateCursor()
{
	if( _cpos > _inputText.length() )
		_cpos = _inputText.length();
	if( _cpos < 0 )_cpos = 0;
	std::string s = _inputText.substr(0,_cpos);
	if( s.length() > 0 )
	{
		FontDefinition fontDef = _fontDefinition;
		_cheight = fontDef._fontSize;
		Size size;
		getStringSize(s,size);
		_cx = size.width;
        int dh = (size.height-_cheight)/2;
		_cy = dh;//(float)_labelHeight - _cheight - dh;
		if( _cy < 0 )_cy = 0;
	}
	else
	{
		Size size;
		size = getContentSize();	
		_cheight = _fontDefinition._fontSize;
		_cy =  (size.height-_cheight)/2;
		if( _cy < 0 )_cy = 0;
		_cx = 0;
	}
	_cursorb = 0.5; //show cursor
}

void TextFieldTTF::getStringSize(const std::string& str,Size& size)
{
	FontDefinition fontDef = _fontDefinition;
	fontDef._dimensions.width = 0;
	fontDef._dimensions.height = 0;
	auto texture = new Texture2D();
	texture->initWithString(str.c_str(),fontDef);
	size = texture->getContentSize();
	texture->release();
	if( size.height<= 0 )size.height = fontDef._fontSize;
}

int TextFieldTTF::cursorPos(Vec2 pt)
{
	Size size;
	pt = convertToNodeSpace(pt);
	int len = utf8length(_inputText);//_inputText.length();
	int cpos = 0;
	int idx,uclen; //char index
	getStringSize(_inputText,size);
	if( size.width > 0 )
	{
		float v = pt.x/size.width;
		if( v>=0 && v < 1 )
		{
			int i = len*v; //utf8 index
			int flag = 0;
			float last_width;
			do{
				if( i==len )
					idx = _inputText.length();
				else
					utf8index(_inputText,i,&idx,&uclen);
				std::string s = _inputText.substr(0,idx);
				getStringSize(s,size);
				if( size.width > pt.x )
				{
					if( flag == 2 )
					{
						if( pt.x > (last_width+size.width)/2 )
							cpos = i;
						else
							cpos = i-1;
						break;
					}
					i--;
					flag = 1;
				}
				else
				{
					if( flag == 1 )
					{
						if( pt.x > (last_width+size.width)/2 )
							cpos = i;
						else
							cpos = i-1;
						break;
					}
					i++;
					flag = 2;
				}
				last_width = size.width;
			}while(i>0 &&i<=len);
		}
		else
		{
			if( v >= 1 )cpos = len;
			if( v < 0 )cpos = 0;
		}
	}
	if(cpos < 0 )cpos = 0;
	if(cpos > len )cpos = len;
	if( cpos == len )
		idx = _inputText.length();
	else
		utf8index(_inputText,cpos,&idx,&uclen);
	return idx;
}

void TextFieldTTF::selectDrag(Vec2 pt)
{
	if(_imepos>=0)return;
	
	if( _selpos < 0 )
	{//first
		_selx = _cx;
		_selpos = _cpos;
	}
	_cpos = cursorPos(pt);

	Size size;
	std::string s = _inputText.substr(0,_cpos);
	if( s.length()==0 )
	{
		_cx = 0;
	}
	else
		getStringSize( s,size);
	_cx = size.width;
}

void TextFieldTTF::onDrawCursor(const cocos2d::Mat4 &transform, uint32_t flags)
{
	GLint mode;
	auto director = Director::getInstance();
	MATRIX_STACK_TYPE currentActiveStackType = MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW;
	
	director->pushMatrix(currentActiveStackType);
	director->loadMatrix(currentActiveStackType,transform);
	if(_selpos>=0)
		DrawPrimitives::drawSolidRect(Vec2(_cx,_cy),Vec2(_selx,_cy+_cheight),Color4F(0,0,1,0.5));
	if(_cursorb)
	{
		GL::blendFunc(GL_ONE ,GL_ONE );
		glGetIntegerv(GL_BLEND_EQUATION_RGB,&mode);
		glBlendEquation(GL_FUNC_SUBTRACT);
		glLineWidth(_cwidth);
		DrawPrimitives::setDrawColor4B(255,255,255,255);
		DrawPrimitives::drawLine(Vec2(_cx,_cy),Vec2(_cx,_cy+_cheight));
		glBlendEquation(mode);
	}
	director->popMatrix(currentActiveStackType);
}

void TextFieldTTF::drawCursor(Renderer *renderer,const cocos2d::Mat4 &transform, uint32_t flags)
{
	if( _showcursor && _currentActive==this )
	{
		auto director = Director::getInstance();
		double d = director->getAnimationInterval();
		_cursordt += d;
		if( _cursordt>0.5 )
		{
			_cursorb = !_cursorb;
			_cursordt-=0.5;
		}
		_renderCmd.init(_globalZOrder);
		_renderCmd.func = CC_CALLBACK_0(TextFieldTTF::onDrawCursor, this, transform, flags);
		renderer->addCommand(&_renderCmd);
	}
}

//////////////////////////////////////////////////////////////////////////
// static constructor
//////////////////////////////////////////////////////////////////////////

TextFieldTTF * TextFieldTTF::textFieldWithPlaceHolder(const std::string& placeholder, const Size& dimensions, TextHAlignment alignment, const std::string& fontName, float fontSize)
{
    TextFieldTTF *ret = new TextFieldTTF();
    if(ret && ret->initWithPlaceHolder("", dimensions, alignment, fontName, fontSize))
    {
        ret->autorelease();
        if (placeholder.size()>0)
        {
            ret->setPlaceHolder(placeholder);
        }
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

TextFieldTTF * TextFieldTTF::textFieldWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize)
{
    TextFieldTTF *ret = new TextFieldTTF();
    if(ret && ret->initWithPlaceHolder("", fontName, fontSize))
    {
        ret->autorelease();
        if (placeholder.size()>0)
        {
            ret->setPlaceHolder(placeholder);
        }
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialize
//////////////////////////////////////////////////////////////////////////

bool TextFieldTTF::initWithPlaceHolder(const std::string& placeholder, const Size& dimensions, TextHAlignment alignment, const std::string& fontName, float fontSize)
{
    _placeHolder = placeholder;
    setDimensions(dimensions.width,dimensions.height);
    setSystemFontName(fontName);
    setSystemFontSize(fontSize);
    setAlignment(alignment,TextVAlignment::CENTER);
    Label::setTextColor(_colorSpaceHolder);
    Label::setString(_placeHolder);

    return true;
}
bool TextFieldTTF::initWithPlaceHolder(const std::string& placeholder, const std::string& fontName, float fontSize)
{
    _placeHolder = std::string(placeholder);
    setSystemFontName(fontName);
    setSystemFontSize(fontSize);
    Label::setTextColor(_colorSpaceHolder);
    Label::setString(_placeHolder);

    return true;
}

void TextFieldTTF::onClick(Vec2 pt)
{
	if(_imepos>=0)return;	
	
	_cpos = cursorPos(pt);
	_selpos = -1;
	updateCursor();
}
//////////////////////////////////////////////////////////////////////////
// IMEDelegate
//////////////////////////////////////////////////////////////////////////

bool TextFieldTTF::attachWithIME()
{
    bool ret = IMEDelegate::attachWithIME();
    if (ret)
    {
        // open keyboard
        GLView * pGlView = Director::getInstance()->getOpenGLView();
        if (pGlView)
        {
            pGlView->setIMEKeyboardState(true);
			CCLOG("===attachWithIME===");

			_currentActive = this;
			_showcursor = true;
			_cpos = _inputText.length();
			updateCursor();
        }
    }
    return ret;
}

bool TextFieldTTF::detachWithIME()
{
    bool ret = IMEDelegate::detachWithIME();
    if (ret)
    {
        // close keyboard
        GLView * glView = Director::getInstance()->getOpenGLView();
        if (glView)
        {
            glView->setIMEKeyboardState(false);
			CCLOG("===detachWithIME===");
			_currentActive = nullptr;
			_showcursor = false;
        }
    }
    return ret;
}

bool TextFieldTTF::canAttachWithIME()
{
    return (_delegate) ? (! _delegate->onTextFieldAttachWithIME(this)) : true;
}

bool TextFieldTTF::canDetachWithIME()
{
    return (_delegate) ? (! _delegate->onTextFieldDetachWithIME(this)) : true;
}

void TextFieldTTF::setText(const char * text, size_t len)
{
    std::string insert(text, len);

	if( _selpos >= 0 )
	{
		deleteSelect();
	}
	bool isEnter = false;
    // insert \n means input end
    int pos = static_cast<int>(insert.find('\n'));
    if ((int)insert.npos != pos)
    {
        len = pos;
        insert.erase(pos);
		isEnter = true;
    }
	//¥•∑¢ ¬º˛
	if (_delegate && _delegate->onTextFieldInsertText(this, insert.c_str(), len))
	{
		// delegate doesn't want to insert text
		return;
	}
	_charCount = _calcCharCount(insert.c_str());
	_cpos = len;
	setString(insert);
	if ( isEnter )
		detachWithIME();	
}

/*
 输入法中间字符串显示
 */
void TextFieldTTF::setIMEText(const char * text, size_t len)
{
    if(len>0){
        if( _imepos<0 )
        {
            _selx = _cx;
            _imepos = _cpos;
            insertText(text, len);
            _selpos = _imepos;
        }else{
            insertText(text, len);
            _selpos = _imepos;
        }
    }else
    {
        if( _selpos >= 0 )
        {
            deleteSelect();
        }
        _imepos = -1;
        _selpos = -1;
    }
}

void TextFieldTTF::insertText(const char * text, size_t len)
{
    std::string insert(text, len);

	if( _selpos >= 0 )
	{
		deleteSelect();
	}
    // insert \n means input end
    int pos = static_cast<int>(insert.find('\n'));
    if ((int)insert.npos != pos)
    {
        len = pos;
        insert.erase(pos);
    }

    if (len > 0)
    {
        if (_delegate && _delegate->onTextFieldInsertText(this, insert.c_str(), len))
        {
            // delegate doesn't want to insert text
            return;
        }

        _charCount += _calcCharCount(insert.c_str());
        std::string sText(_inputText);

		if(!(_cpos>=0||_cpos<=sText.length()))
		{
			_cpos = 0;
		}
		sText.insert(_cpos,insert);

		_cpos += len;
        setString(sText);
    }

    if ((int)insert.npos == pos) {
        return;
    }

    // '\n' inserted, let delegate process first
    if (_delegate && _delegate->onTextFieldInsertText(this, "\n", 1))
    {
        return;
    }

    // if delegate hasn't processed, detach from IME by default
    detachWithIME();
}

void TextFieldTTF::deleteSelect()
{
	int _min,_max;
	_min = _cpos > _selpos?_selpos:_cpos;
	_max = _cpos < _selpos?_selpos:_cpos;
	std::string s = _inputText.substr(0,_min);
	s += _inputText.substr(_max);
	_cpos = _min;
	_selpos = -1;
	int len = utf8length(_inputText);
    if (_delegate && _delegate->onTextFieldDeleteBackward(this, _inputText.c_str() + len - (_max-_min), static_cast<int>(_max-_min)))
    {
        // delegate doesn't wan't to delete backwards
        return;
    }
	if( s.length() == 0 )
	{
        _inputText = "";
        _charCount = 0;
		_cpos = 0;
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
		updateCursor();
		return;
	}
	setString(s);
}

void TextFieldTTF::deleteForward()
{
    size_t len = _inputText.length();
    if (! len)
    {
        // there is no string
        return;
    }

	if( _selpos >= 0 )
	{
		deleteSelect();
		return; 
	}
    // get the delete byte number
    size_t deleteLen = 1;    // default, erase 1 byte

	if( _cpos+deleteLen > len )
		return;
	if( _cpos+deleteLen!=len )
	{
		while(0x80 == (0xC0 & _inputText.at(_cpos + deleteLen)))
		{
			deleteLen++;
			if( _cpos + deleteLen >= len )
				break;
		}
	}
    if (_delegate && _delegate->onTextFieldDeleteBackward(this, _inputText.c_str() + len - deleteLen, static_cast<int>(deleteLen)))
    {
        // delegate doesn't wan't to delete backwards
        return;
    }

    // if all text deleted, show placeholder string
    if (len <= deleteLen)
    {
        _inputText = "";
        _charCount = 0;
		_cpos = 0;
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
		updateCursor();
        return;
    }

    // set new input text
	int bpos = _cpos + deleteLen;

    std::string text(_inputText.c_str(), _cpos);
	text += _inputText.substr(bpos);
    setString(text);
}

void TextFieldTTF::moveCursor(int a,bool sel)
{
#ifdef _WIN32
	int len = _inputText.length();
	if( sel )
	{
		if( _selpos<0 )
		{
			_selpos = _cpos;
			_selx = _cx;
		}
	}
	else
	{
		_selpos = -1;
	}
	switch(a)
	{
	case GLFW_KEY_LEFT: //left
		if( _cpos > 0 )
		{
			while(0x80 == (0xC0 & _inputText.at(--_cpos)))
			{
				if( _cpos <= 0 )break;
			}
		}
		break;
	case GLFW_KEY_RIGHT: //right
		if( _cpos < len-1 )
		{
			while(0x80 == (0xC0 & _inputText.at(++_cpos)))
			{
				if( _cpos >= len-1 )
				{
					_cpos = len;
					break;
				}
			}
		}else
			_cpos = len;
		break;
	case GLFW_KEY_HOME: //home
		_cpos = 0;
		break;
	case GLFW_KEY_END: //end
		_cpos = len;
		break;
	}
	if( _cpos < 0 )_cpos = 0;
	if( _cpos > len) _cpos = len;
	updateCursor();
#endif
}

void TextFieldTTF::optKey(int key)
{
#ifdef _WIN32
	if( key == GLFW_KEY_C)
	{
		if( _selpos >= 0 )
		{
			int _min,_max;
			_min = _cpos > _selpos?_selpos:_cpos;
			_max = _cpos < _selpos?_selpos:_cpos;
			if( _max > _min )
			{
				std::string s =_inputText.substr(_min,_max-_min);
				if(!s.empty() && OpenClipboard(NULL))
				{
					EmptyClipboard();
					std::u16string u16;
					StringUtils::UTF8ToUTF16(s,u16);
					HGLOBAL hcpy = GlobalAlloc(GMEM_MOVEABLE,(u16.length()+1)*sizeof(TCHAR));
					if( hcpy )
					{
						TCHAR * pc = (TCHAR*)GlobalLock(hcpy);
						if( pc )
						{
							pc[u16.length()] = 0;
							memcpy(pc,u16.c_str(),u16.length()*sizeof(TCHAR));
						}
						GlobalUnlock(hcpy);
					}
					SetClipboardData(CF_UNICODETEXT,hcpy);
					CloseClipboard();
				}
			}
		}
	}
	else if(key==GLFW_KEY_V)
	{
		if( !IsClipboardFormatAvailable(CF_UNICODETEXT)) return;
		if( OpenClipboard(NULL) )
		{
			if( _selpos >= 0 )
				deleteSelect();
			HGLOBAL hcpy = GetClipboardData(CF_UNICODETEXT);
			if( hcpy )
			{
				TCHAR *pc =(TCHAR *)GlobalLock(hcpy);
				if( pc )
				{
					std::u16string u16((char16_t*)pc);
					std::string u8;
					StringUtils::UTF16ToUTF8(u16,u8);
					insertText(u8.c_str(),u8.length());
					GlobalUnlock(hcpy);
				}
			}
			CloseClipboard();
		}
	}
	else if(key==GLFW_KEY_A)
	{
		_selpos = 0;
		_selx = 0;
		_cpos = _inputText.length();
		updateCursor();
	}
	else if(key==GLFW_KEY_X)
	{
		if( _selpos >= 0 )
		{
			optKey(GLFW_KEY_C);
			deleteSelect();
		}
	}
	else if(key==GLFW_KEY_Z)
	{
		//undo
	}
#endif
}

void TextFieldTTF::deleteBackward()
{
    size_t len = _inputText.length();
    if (! len)
    {
        // there is no string
        return;
    }

	if( _selpos >= 0 )
	{
		deleteSelect();
		return; 
	}
    // get the delete byte number
    size_t deleteLen = 1;    // default, erase 1 byte

	if( _cpos <= 0 )
		return;

    while(0x80 == (0xC0 & _inputText.at(_cpos - deleteLen)))
    {
        ++deleteLen;
    }

    if (_delegate && _delegate->onTextFieldDeleteBackward(this, _inputText.c_str() + len - deleteLen, static_cast<int>(deleteLen)))
    {
        // delegate doesn't wan't to delete backwards
        return;
    }

    // if all text deleted, show placeholder string
    if (len <= deleteLen)
    {
        _inputText = "";
        _charCount = 0;
		_cpos = 0;
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
		updateCursor();
        return;
    }

    // set new input text
	int bpos = _cpos - deleteLen;

    std::string text(_inputText.c_str(), bpos);
	text += _inputText.substr(_cpos);
	_cpos -= deleteLen;
    setString(text);
}

const std::string& TextFieldTTF::getContentText()
{
    return _inputText;
}

Vec2 TextFieldTTF::convertToWindowSpace2(const Vec2& nodePoint)const
{
	Vec2 worldPoint = this->convertToWorldSpace(nodePoint);
	return Director::getInstance()->convertToUI2(worldPoint);
}
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
/*
	’‚¿Ô◊¯±Í◊™ªª∫Ø ˝convertToUI¥Ê‘⁄Œ Ã‚£¨Œ“Ω´∆‰∏ƒΩ¯Œ™convertToUI2
*/
Rect TextFieldTTF::getContentRect()
{
	Vec2 p = getPosition();
	Size s = getContentSize();
	Vec2 ap = this->getAnchorPoint();
//	CCLOG("p = %f,%f", p.x, p.y);
	p.x -= s.width*ap.x;
	p.y = p.y - s.height*ap.y + s.height;
	Vec2 pp;
	pp.x = p.x + s.width;
	pp.y = p.y + s.height;
//	CCLOG("anthor p = %f,%f", p.x,p.y);
//	CCLOG("anthor size_p = %f,%f", pp.x, pp.y);
	Vec2 wp = convertToWindowSpace2(p);
	Vec2 wpp = convertToWindowSpace2(pp);
//	CCLOG("wp = %f,%f", wp.x, wp.y);
//	CCLOG("size_wp = %f,%f", wpp.x, wpp.y);
	//convertToWindowSpace≤¢≤ªøº¬«÷°ª∫≥ÂµΩ∆¡ƒªµƒ”≥…‰
	//À¸ΩˆΩˆ”≥…‰µΩ…Ëº∆ ”Õº
	GLView * pglview = Director::getInstance()->getOpenGLView();
	Size frame_size = pglview->getFrameSize();
	Size design_size = pglview->getDesignResolutionSize();
	float sx = frame_size.width / design_size.width;
	float sy = frame_size.height / design_size.height;
	float dh = (frame_size.height - design_size.height*sx) / 2;
	float dw = (frame_size.width - design_size.width*sy) / 2;
//	CCLOG("sx,sy = %f,%f", sx,sy);
//	CCLOG("dh,dw = %f,%f", dh, dw);
	switch (pglview->getResolutionPolicy())
	{
	case ResolutionPolicy::FIXED_WIDTH:
		wp.x *= sx;
		wp.y =wp.y*sx + dh;
		wpp.x *= sx;
		wpp.y = wpp.y*sx + dh;
	case ResolutionPolicy::FIXED_HEIGHT:
		wp.x = wp.y*sy + dw;
		wp.y *= sy;
		wpp.x = wpp.x*sy + dw;
		wpp.y *= sy;
	case ResolutionPolicy::NO_BORDER: //≤ªÃ´¿ÌΩ‚∫¨“Â£¨‘› ±∞¥SHOW_ALL¥¶¿Ì
	case ResolutionPolicy::SHOW_ALL:
		{
			float df = design_size.height / design_size.width;
			float ff = frame_size.height / frame_size.width;
			if (ff > df){//fixed_width
				wp.x *= sx;
				wp.y = wp.y*sx + dh;
				wpp.x *= sx;
				wpp.y = wpp.y*sx + dh;
			}
			else
			{
				wp.x = wp.x*sy + dw;
				wp.y *= sy;
				wpp.x = wpp.x*sy + dw;
				wpp.y *= sy;
			}
		}
		break;
	case ResolutionPolicy::UNKNOWN:
	case ResolutionPolicy::EXACT_FIT:
	default:
		wp.x *= sx;
		wp.y *= sy;
		wpp.x *= sx;
		wpp.y *= sy;
		break;
	}
	/*
	float ssf = Director::getInstance()->getContentScaleFactor();
	Size wsize = Director::getInstance()->getWinSizeInPixels();
	Size vsize = Director::getInstance()->getVisibleSize();
	Vec2 vorg = Director::getInstance()->getVisibleOrigin();
	Size glsize = Director::getInstance()->getWinSize();
	CCLOG("getContentScaleFactor = %f", ssf);
	CCLOG("getWinSizeInPixels = %f,%f", wsize.width, wsize.height);
	CCLOG("getWinSize = %f,%f", glsize.width, glsize.height);
	CCLOG("getVisibleSize = %f,%f", vsize.width, vsize.height);
	CCLOG("getVisibleOrigin = %f,%f", vorg.x, vorg.y);
	GLView *pgl = Director::getInstance()->getOpenGLView();
	Size fsize = pgl->getFrameSize();
	Size dsize = pgl->getDesignResolutionSize();
//	float fzf = pgl->getFrameZoomFactor();
//	float rf = pgl->getRetinaFactor();
	float sx = pgl->getScaleX();
	float sy = pgl->getScaleY();
	CCLOG("getFrameSize = %f,%f", fsize.width, fsize.height);
	CCLOG("getDesignResolutionSize = %f,%f", dsize.width, dsize.height);
//	CCLOG("getFrameZoomFactor = %f", fzf);
//	CCLOG("getRetinaFactor = %f", rf);
	CCLOG("Scale = %f,%f", sx,sy);
	*/
	return Rect(wp.x, wp.y, abs(wpp.x-wp.x),abs(wpp.y-wp.y));
}
#else
Rect TextFieldTTF::getContentRect()
{
    Vec2 p = getPosition();
    Size s = getContentSize();
    Vec2 ap = this->getAnchorPoint();
    //	CCLOG("p = %f,%f", p.x, p.y);
    p.x -= s.width*ap.x;
    p.y -= s.height*ap.y;
    Vec2 pp;
    pp.x = p.x + s.width;
    pp.y = p.y + s.height;
    Vec2 wp = convertToWorldSpace(p);
    Vec2 wpp = convertToWorldSpace(pp);
    return Rect(wp.x, wp.y, abs(wpp.x-wp.x),abs(wpp.y-wp.y));
}
#endif

void TextFieldTTF::setTextColor(const Color4B &color)
{
    _colorText = color;
    Label::setTextColor(_colorText);
}

void TextFieldTTF::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
    if (_delegate && _delegate->onVisit(this,renderer,parentTransform,parentFlags))
    {
        return;
    }

	uint32_t flags = processParentFlags(parentTransform, parentFlags);
	drawCursor(renderer,_modelViewTransform,flags);
    Label::visit(renderer,parentTransform,parentFlags);
}

const Color4B& TextFieldTTF::getColorSpaceHolder()
{
    return _colorSpaceHolder;
}

void TextFieldTTF::setColorSpaceHolder(const Color3B& color)
{
    _colorSpaceHolder.r = color.r;
    _colorSpaceHolder.g = color.g;
    _colorSpaceHolder.b = color.b;
    _colorSpaceHolder.a = 255;
}

void TextFieldTTF::setColorSpaceHolder(const Color4B& color)
{
    _colorSpaceHolder = color;
}

//////////////////////////////////////////////////////////////////////////
// properties
//////////////////////////////////////////////////////////////////////////

// input text property
void TextFieldTTF::setString(const std::string &text)
{
    static char bulletString[] = {(char)0xe2, (char)0x80, (char)0xa2, (char)0x00};
    std::string displayText;
    size_t length;

    if (text.length()>0)
    {
        _inputText = text;
        displayText = _inputText;
        if (_secureTextEntry)
        {
            displayText = "";
            length = _inputText.length();
            while (length)
            {
                displayText.append(bulletString);
                --length;
            }
        }
    }
    else
    {
        _inputText = "";
    }

    // if there is no input text, display placeholder instead
    if (! _inputText.length())
    {
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
    }
    else
    {
        Label::setTextColor(_colorText);
        Label::setString(displayText);
    }
    _charCount = _calcCharCount(_inputText.c_str());
	_selpos = -1;
	updateCursor();
}

const std::string& TextFieldTTF::getString() const
{
    return _inputText;
}

// place holder text property
void TextFieldTTF::setPlaceHolder(const std::string& text)
{
    _placeHolder = text;
    if (! _inputText.length())
    {
        Label::setTextColor(_colorSpaceHolder);
        Label::setString(_placeHolder);
    }
}

const std::string& TextFieldTTF::getPlaceHolder() const
{
    return _placeHolder;
}

// secureTextEntry
void TextFieldTTF::setSecureTextEntry(bool value)
{
    if (_secureTextEntry != value)
    {
        _secureTextEntry = value;
        setString(_inputText);
    }
}

bool TextFieldTTF::isSecureTextEntry()
{
    return _secureTextEntry;
}

NS_CC_END
