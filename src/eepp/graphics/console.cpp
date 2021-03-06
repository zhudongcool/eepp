#include <algorithm>
#include <cstdarg>
#include <eepp/audio/listener.hpp>
#include <eepp/graphics/console.hpp>
#include <eepp/graphics/pixeldensity.hpp>
#include <eepp/graphics/renderer/renderer.hpp>
#include <eepp/system/filesystem.hpp>
#include <eepp/system/lock.hpp>
#include <eepp/window/cursormanager.hpp>
#include <eepp/window/engine.hpp>
#include <eepp/window/input.hpp>
#include <eepp/window/window.hpp>

using namespace EE::Window;

namespace EE { namespace Graphics {

Console::Console( EE::Window::Window* window ) :
	mWindow( window ),
	mConColor( 0x201F1FEE ),
	mConLineColor( 0x666666EE ),
	mFontLineColor( 255, 255, 255, 230 ),
	mWidth( 0 ),
	mHeight( 0 ),
#if EE_PLATFORM == EE_PLATFORM_ANDROID || EE_PLATFORM == EE_PLATFORM_IOS
	mHeightMin( 0.5f ),
#else
	mHeightMin( 0.6f ),
#endif
	mY( 0.0f ),
	mA( 0.0f ),
	mFadeSpeed( Milliseconds( 250.f ) ),
	mMyCallback( 0 ),
	mVidCb( 0 ),
	mMaxLogLines( 1024 ),
	mLastLogPos( 0 ),
	mTBuf( InputTextBuffer::New() ),
	mTexId( 0 ),
	mCurAlpha( 0 ),
	mEnabled( false ),
	mVisible( false ),
	mFadeIn( false ),
	mFadeOut( false ),
	mExpand( false ),
	mFading( false ),
	mShowFps( false ),
	mCurSide( false ) {
	mFontStyleConfig.FontColor = Color( 0xCFCFCFFF );

	if ( NULL == mWindow ) {
		mWindow = Engine::instance()->getCurrentWindow();
	}
}

Console::Console( Font* font, const bool& MakeDefaultCommands, const bool& AttachToLog,
				  const unsigned int& MaxLogLines, const Uint32& TextureId,
				  EE::Window::Window* window ) :
	mWindow( window ),
	mConColor( 0x201F1FEE ),
	mConLineColor( 0x666666EE ),
	mFontLineColor( 255, 255, 255, 230 ),
	mWidth( 0 ),
	mHeight( 0 ),
#if EE_PLATFORM == EE_PLATFORM_ANDROID || EE_PLATFORM == EE_PLATFORM_IOS
	mHeightMin( 0.5f ),
#else
	mHeightMin( 0.6f ),
#endif
	mY( 0.0f ),
	mA( 0.0f ),
	mFadeSpeed( Milliseconds( 250.f ) ),
	mMyCallback( 0 ),
	mVidCb( 0 ),
	mMaxLogLines( 1024 ),
	mLastLogPos( 0 ),
	mTBuf( InputTextBuffer::New() ),
	mTexId( 0 ),
	mCurAlpha( 0 ),
	mEnabled( false ),
	mVisible( false ),
	mFadeIn( false ),
	mFadeOut( false ),
	mExpand( false ),
	mFading( false ),
	mShowFps( false ),
	mCurSide( false ) {
	mFontStyleConfig.FontColor = Color( 0xCFCFCFFF );

	if ( NULL == mWindow ) {
		mWindow = Engine::instance()->getCurrentWindow();
	}

	create( font, MakeDefaultCommands, AttachToLog, MaxLogLines, TextureId );
}

Console::~Console() {
	if ( mMyCallback && NULL != Engine::existsSingleton() &&
		 Engine::instance()->existsWindow( mWindow ) ) {
		mWindow->getInput()->popCallback( mMyCallback );
		mWindow->popResizeCallback( mVidCb );
	}

	eeSAFE_DELETE( mTBuf );

	if ( Log::existsSingleton() ) {
		Log::instance()->removeLogReader( this );
	}
}

void Console::setConsoleMinimizedHeight( const EE::Float& MinHeight ) {
	mHeightMin = eemax( 0.f, eemin( MinHeight, 1.f ) );

	if ( mVisible && !mExpand )
		mCurHeight = eefloor( mHeightMin * mWindow->getHeight() );
}

const Float& Console::getConsoleMinimizedHeight() const {
	return mHeightMin;
}

void Console::setBackgroundTextureId( const Uint32& TexId ) {
	mTexId = TexId;
}

Uint32 Console::getBackgroundTextureId() const {
	return mTexId;
}

void Console::setBackgroundColor( const Color& BackColor ) {
	mConColor = BackColor;
	mMaxAlpha = mConColor.a;
}

void Console::setCharacterSize( const EE::Uint32& characterSize ) {
	mFontStyleConfig.CharacterSize = characterSize;
	mFontSize = ( Float )( mFontStyleConfig.Font->getFontHeight(
		PixelDensity::dpToPxI( mFontStyleConfig.CharacterSize ) ) );
}

const Color& Console::getBackgroundColor() const {
	return mConColor;
}

void Console::setBackgroundLineColor( const Color& BackColor ) {
	mConLineColor = BackColor;
}

const Color& Console::getBackgroundLineColor() const {
	return mConLineColor;
}

void Console::setFontColor( const Color& FntColor ) {
	mFontStyleConfig.FontColor = FntColor;
}

const Color& Console::getFontColor() const {
	return mFontStyleConfig.FontColor;
}

void Console::setFontLineColor( const Color& FntColor ) {
	mFontLineColor = FntColor;
}

const Color& Console::getFontLineColor() const {
	return mFontLineColor;
}

void Console::create( Font* Font, const bool& MakeDefaultCommands, const bool& AttachToLog,
					  const unsigned int& MaxLogLines, const Uint32& TextureId ) {
	if ( NULL == mWindow ) {
		mWindow = Engine::instance()->getCurrentWindow();
	}

	mFontStyleConfig.Font = Font;
	mFontSize = ( Float )( mFontStyleConfig.Font->getFontHeight(
		PixelDensity::dpToPxI( mFontStyleConfig.CharacterSize ) ) );

	if ( TextureId > 0 )
		mTexId = TextureId;

	mMaxLogLines = MaxLogLines;
	mMaxAlpha = (Float)mConColor.a;

	mEnabled = true;

	if ( MakeDefaultCommands )
		createDefaultCommands();

	mWidth = (Float)mWindow->getWidth();
	mHeight = (Float)mWindow->getHeight();

	mTextCache.resize( maxLinesOnScreen() );

	if ( NULL != Engine::existsSingleton() && Engine::instance()->existsWindow( mWindow ) ) {
		mMyCallback =
			mWindow->getInput()->pushCallback( cb::Make1( this, &Console::privInputCallback ) );
		mVidCb = mWindow->pushResizeCallback( cb::Make1( this, &Console::privVideoResize ) );
	}

	mTBuf->setReturnCallback( cb::Make0( this, &Console::processLine ) );
	mTBuf->start();
	mTBuf->isNewLineEnabled( false );
	mTBuf->setActive( false );
	ignoreCharOnPrompt( KEY_TAB );

	mCon.ConModif = 0;

	cmdGetLog();

	if ( AttachToLog ) {
		Log::instance()->addLogReader( this );
	}
}

void Console::addCommand( const String& Command, ConsoleCallback CB ) {
	if ( !( mCallbacks.count( Command ) > 0 ) )
		mCallbacks[Command] = CB;
}

void Console::draw( const Time& elapsedTime ) {
	Lock l( mMutex );
	if ( mEnabled && NULL != mFontStyleConfig.Font ) {
		fade( elapsedTime == Time::Zero ? mWindow->getElapsed() : elapsedTime );

		if ( mY > 0.0f ) {
			if ( mTexId == 0 ) {
				mPri.setColor(
					Color( mConColor.r, mConColor.g, mConColor.b, static_cast<Uint8>( mA ) ) );
				mPri.drawRectangle( Rectf( Vector2f( 0.0f, 0.0f ), Sizef( mWidth, mY ) ) );
			} else {
				Color C( mConColor.r, mConColor.g, mConColor.b, static_cast<Uint8>( mA ) );

				Texture* Tex = TextureFactory::instance()->getTexture( mTexId );

				if ( NULL != Tex )
					Tex->drawEx( 0.0f, 0.0f, mWidth, mY, 0.0f, Vector2f::One, C, C, C, C );
			}
			mPri.setColor( Color( mConLineColor.r, mConLineColor.g, mConLineColor.b,
								  static_cast<Uint8>( mA ) ) );
			mPri.drawRectangle(
				Rectf( Vector2f( 0.0f, mY ), Sizef( mWidth, PixelDensity::dpToPx( 2.0f ) ) ) );

			Int32 linesInScreen = this->linesOnScreen();

			if ( static_cast<Int32>( mCmdLog.size() ) > linesInScreen )
				mEx = ( Uint32 )( mCmdLog.size() - linesInScreen );
			else
				mEx = 0;
			mTempY = -mCurHeight;

			Uint16 Pos = 0;
			Float CurY;

			mCon.ConMin = mEx;
			mCon.ConMax = (int)mCmdLog.size() - 1;

			for ( int i = mCon.ConMax - mCon.ConModif; i >= mCon.ConMin - mCon.ConModif; i-- ) {
				if ( i < static_cast<Int16>( mCmdLog.size() ) && i >= 0 ) {
					CurY = mTempY + mY + mCurHeight - Pos * mFontSize - mFontSize * 2;

					Text& text = mTextCache[Pos];

					text.setFont( mFontStyleConfig.Font );
					text.setFontSize( mFontStyleConfig.CharacterSize );
					text.setStyle( mFontStyleConfig.Style );
					text.setOutlineThickness( mFontStyleConfig.OutlineThickness );
					text.setOutlineColor( mFontStyleConfig.OutlineColor );
					text.setFillColor(
						Color( mFontStyleConfig.FontColor.r, mFontStyleConfig.FontColor.g,
							   mFontStyleConfig.FontColor.b, static_cast<Uint8>( mA ) ) );
					text.setString( mCmdLog[i] );
					text.draw( mFontSize, CurY );

					Pos++;
				}
			}

			CurY = mTempY + mY + mCurHeight - mFontSize - 1;

			Text& text = mTextCache[mTextCache.size() - 1];
			text.setFont( mFontStyleConfig.Font );
			text.setFontSize( mFontStyleConfig.CharacterSize );
			text.setStyle( mFontStyleConfig.Style );
			text.setOutlineThickness( mFontStyleConfig.OutlineThickness );
			text.setOutlineColor( mFontStyleConfig.OutlineColor );
			text.setFillColor( Color( mFontLineColor.r, mFontLineColor.g, mFontLineColor.b,
									  static_cast<Uint8>( mA ) ) );
			text.setString( "> " + mTBuf->getBuffer() );
			text.draw( mFontSize, CurY );

			Text& text2 = mTextCache[mTextCache.size() - 2];
			text2.setFont( mFontStyleConfig.Font );
			text2.setFontSize( mFontStyleConfig.CharacterSize );
			text2.setStyle( mFontStyleConfig.Style );
			text2.setOutlineThickness( mFontStyleConfig.OutlineThickness );
			text2.setOutlineColor( mFontStyleConfig.OutlineColor );
			text2.setFillColor( Color( mFontLineColor.r, mFontLineColor.g, mFontLineColor.b,
									   static_cast<Uint8>( mCurAlpha ) ) );

			if ( (unsigned int)mTBuf->getCursorPosition() == mTBuf->getBuffer().size() ) {
				Uint32 width = text.getTextWidth();
				text2.setString( "_" );
				text2.draw( mFontSize + width, CurY );
			} else {
				text2.setString( "> " +
								 mTBuf->getBuffer().substr( 0, mTBuf->getCursorPosition() ) );
				Uint32 width = mFontSize + text2.getTextWidth();
				text2.setString( "_" );
				text2.draw( width, CurY );
			}
		}
	}

	if ( mShowFps && NULL != mFontStyleConfig.Font ) {
		Text& text = mTextCache[mTextCache.size() - 3];
		Color OldColor1( text.getColor() );
		text.setStyleConfig( mFontStyleConfig );
		text.setFillColor( Color::White );
		text.setString( "FPS: " + String::toString( mWindow->getFPS() ) );
		text.draw( mWindow->getWidth() - text.getTextWidth() - 15, 6 );
		text.setFillColor( OldColor1 );
	}
}

void Console::setLineHeight( const Float& LineHeight ) {
	mFontSize = LineHeight;
}

void Console::fadeIn() {
	if ( !mFading ) {
		mFading = true;
		mFadeIn = true;
		mVisible = true;
		mY = 0.0f;
		mTBuf->setActive( true );
	}
}

void Console::fadeOut() {
	if ( !mFading ) {
		mFading = true;
		mFadeOut = true;
		mVisible = false;
		mTBuf->setActive( false );
	}
}

bool Console::isActive() const {
	return mVisible;
}

void Console::setExpanded( const bool& Exp ) {
	mExpand = Exp;
}

bool Console::isExpanded() const {
	return mExpand;
}

void Console::setFadeSpeed( const Time& fadespeed ) {
	mFadeSpeed = fadespeed;
}

const Time& Console::getFadeSpeed() const {
	return mFadeSpeed;
}

static std::vector<String> SplitCommandParams( String str ) {
	std::vector<String> params = String::split( str, ' ' );
	std::vector<String> rparams;
	String tstr;

	for ( size_t i = 0; i < params.size(); i++ ) {
		String tparam = params[i];

		if ( !tparam.empty() ) {
			if ( '"' == tparam[0] ) {
				tstr += tparam;
			} else if ( '"' == tparam[tparam.size() - 1] ) {
				tstr += " " + tparam;

				rparams.push_back( String::trim( tstr, '"' ) );

				tstr = "";
			} else if ( !tstr.empty() ) {
				tstr += " " + tparam;
			} else {
				rparams.push_back( tparam );
			}
		}
	}

	if ( !tstr.empty() ) {
		rparams.push_back( String::trim( tstr, '"' ) );
	}

	return rparams;
}

void Console::processLine() {
	String str = mTBuf->getBuffer();
	std::vector<String> params = SplitCommandParams( str );

	mLastCommands.push_back( str );
	mLastLogPos = (int)mLastCommands.size();

	if ( mLastCommands.size() > 20 )
		mLastCommands.pop_front();

	if ( str.size() > 0 ) {
		privPushText( "> " + str );

		if ( mCallbacks.find( params[0] ) != mCallbacks.end() ) {
			mCallbacks[params[0]]( params );
		} else {
			privPushText( "Unknown Command: '" + params[0] + "'" );
		}
	}
	mTBuf->clear();
}

void Console::privPushText( const String& str ) {
	Lock l( mMutex );
	mCmdLog.push_back( str );

	if ( mCmdLog.size() >= mMaxLogLines )
		mCmdLog.pop_front();
}

void Console::pushText( const String& str ) {
	if ( std::string::npos != str.find_first_of( '\n' ) ) {
		std::vector<String> Strings = String::split( String( str ) );

		for ( Uint32 i = 0; i < Strings.size(); i++ ) {
			privPushText( Strings[i] );
		}
	} else {
		privPushText( str );
	}
}

void Console::pushText( const char* format, ... ) {
	int n, size = 256;
	std::string tstr( size, '\0' );

	va_list args;

	while ( 1 ) {
		va_start( args, format );

		n = vsnprintf( &tstr[0], size, format, args );

		if ( n > -1 && n < size ) {
			tstr.resize( n );

			pushText( tstr );

			va_end( args );

			return;
		}

		if ( n > -1 )	  // glibc 2.1
			size = n + 1; // precisely what is needed
		else			  // glibc 2.0
			size *= 2;	  // twice the old size

		tstr.resize( size );
	}
}

void Console::toggle() {
	if ( mVisible )
		fadeOut();
	else
		fadeIn();
}

void Console::fade( const Time& elapsedTime ) {
	if ( mCurSide ) {
		mCurAlpha -= 255.f * elapsedTime.asMilliseconds() / mFadeSpeed.asMilliseconds();
		if ( mCurAlpha <= 0.0f ) {
			mCurAlpha = 0.0f;
			mCurSide = !mCurSide;
		}
	} else {
		mCurAlpha += 255.f * elapsedTime.asMilliseconds() / mFadeSpeed.asMilliseconds();
		if ( mCurAlpha >= 255.f ) {
			mCurAlpha = 255.f;
			mCurSide = !mCurSide;
		}
	}

	if ( mExpand )
		mCurHeight = mHeight;
	else
		mCurHeight = eefloor( mHeightMin * mHeight );

	if ( mFadeIn ) {
		mFadeOut = false;
		mY += mCurHeight * elapsedTime.asMilliseconds() / mFadeSpeed.asMilliseconds();

		mA = ( mY * mMaxAlpha / mCurHeight );
		if ( mY > mCurHeight ) {
			mY = mCurHeight;
			mFadeIn = false;
			mFading = false;
		}
	}

	if ( mFadeOut ) {
		mFadeIn = false;
		mY -= mCurHeight * elapsedTime.asMilliseconds() / mFadeSpeed.asMilliseconds();

		mA = ( mY * mMaxAlpha / mCurHeight );
		if ( mY <= 0.0f ) {
			mY = 0.0f;
			mFadeOut = false;
			mFading = false;
		}
	}

	if ( mA > 255.0f )
		mA = 255.0f;
	if ( mA < 0.0f )
		mA = 0.0f;
}

String Console::getLastCommonSubStr( std::list<String>& cmds ) {
	String lastCommon( mTBuf->getBuffer() );
	String strTry( lastCommon );

	std::list<String>::iterator ite;

	bool found = false;

	do {
		found = false;

		bool allEqual = true;

		String strBeg( ( *cmds.begin() ) );

		if ( strTry.size() + 1 <= strBeg.size() ) {
			strTry = String( strBeg.substr( 0, strTry.size() + 1 ) );

			for ( ite = ++cmds.begin(); ite != cmds.end(); ++ite ) {
				String& strCur = ( *ite );

				if ( !( strTry.size() <= strCur.size() &&
						strTry == strCur.substr( 0, strTry.size() ) ) ) {
					allEqual = false;
				}
			}

			if ( allEqual ) {
				lastCommon = strTry;

				found = true;
			}
		}
	} while ( found );

	return lastCommon;
}

void Console::printCommandsStartingWith( const String& start ) {
	std::list<String> cmds;
	std::map<String, ConsoleCallback>::iterator it;

	for ( it = mCallbacks.begin(); it != mCallbacks.end(); ++it ) {
		if ( String::startsWith( it->first, start ) ) {
			cmds.push_back( it->first );
		}
	}

	if ( cmds.size() > 1 ) {
		privPushText( "> " + mTBuf->getBuffer() );

		std::list<String>::iterator ite;

		for ( ite = cmds.begin(); ite != cmds.end(); ++ite )
			privPushText( ( *ite ) );

		String newStr( getLastCommonSubStr( cmds ) );

		if ( newStr != mTBuf->getBuffer() ) {
			mTBuf->setBuffer( newStr );
			mTBuf->cursorToEnd();
		}
	} else if ( cmds.size() ) {
		mTBuf->setBuffer( cmds.front() );
		mTBuf->cursorToEnd();
	}
}

void Console::privVideoResize( EE::Window::Window* win ) {
	mWidth = (Float)mWindow->getWidth();
	mHeight = (Float)mWindow->getHeight();
	mTextCache.resize( maxLinesOnScreen() );

	if ( mVisible ) {
		if ( mExpand )
			mCurHeight = mHeight;
		else
			mCurHeight = eefloor( mHeightMin * mHeight );

		mY = mCurHeight;
	}
}

void Console::getFilesFrom( std::string txt, const Uint32& curPos ) {
	static char OSSlash = FileSystem::getOSSlash().at( 0 );
	size_t pos;

	if ( std::string::npos != ( pos = txt.find_last_of( OSSlash ) ) && pos <= curPos ) {
		size_t fpos = txt.find_first_of( OSSlash );

		std::string dir( txt.substr( fpos, pos - fpos + 1 ) );
		std::string file( txt.substr( pos + 1 ) );

		if ( FileSystem::isDirectory( dir ) ) {
			size_t count = 0, lasti = 0;
			std::vector<std::string> files = FileSystem::filesGetInPath( dir, true, true );
			String res;
			bool again = false;

			do {
				std::vector<std::string> foundFiles;
				res = "";
				count = 0;
				again = false;

				for ( size_t i = 0; i < files.size(); i++ ) {
					if ( !file.size() || String::startsWith( files[i], file ) ) {
						res += "\t" + files[i] + "\n";
						count++;
						lasti = i;
						foundFiles.push_back( files[i] );
					}
				}

				if ( count > 1 ) {
					bool allBigger = true;
					bool allStartsWith = true;

					do {
						allBigger = true;

						for ( size_t i = 0; i < foundFiles.size(); i++ ) {
							if ( foundFiles[i].size() < file.size() + 1 ) {
								allBigger = false;
								break;
							}
						}

						if ( allBigger ) {
							std::string tfile = foundFiles[0].substr( 0, file.size() + 1 );
							allStartsWith = true;

							for ( size_t i = 0; i < foundFiles.size(); i++ ) {
								if ( !String::startsWith( foundFiles[i], tfile ) ) {
									allStartsWith = false;
									break;
								}
							}

							if ( allStartsWith ) {
								file = tfile;
								again = true;
							}
						}
					} while ( allBigger && allStartsWith );
				}
			} while ( again );

			if ( count == 1 ) {
				std::string slash = "";

				if ( FileSystem::isDirectory( dir + files[lasti] ) ) {
					slash = FileSystem::getOSSlash();
				}

				mTBuf->setBuffer( mTBuf->getBuffer().substr( 0, pos + 1 ) + files[lasti] + slash );
				mTBuf->cursorToEnd();
			} else if ( count > 1 ) {
				privPushText( "Directory file list:" );
				pushText( res );

				mTBuf->setBuffer( mTBuf->getBuffer().substr( 0, pos + 1 ) + file );
				mTBuf->cursorToEnd();
			}
		}
	}
}

Int32 Console::linesOnScreen() {
	return static_cast<Int32>( ( mCurHeight / mFontSize ) - 1 );
}

Int32 Console::maxLinesOnScreen() {
	return static_cast<Int32>( ( mHeight / mFontSize ) + 3 );
}

void Console::privInputCallback( InputEvent* Event ) {
	Uint8 etype = Event->Type;

	if ( mVisible ) {
		Uint32 KeyCode = (Uint32)Event->key.keysym.sym;
		Uint32 KeyMod = (Uint32)Event->key.keysym.mod;
		Uint32 Button = Event->button.button;

		if ( InputEvent::KeyDown == etype ) {
			if ( ( KeyCode == KEY_TAB ) &&
				 (unsigned int)mTBuf->getCursorPosition() == mTBuf->getBuffer().size() ) {
				printCommandsStartingWith( mTBuf->getBuffer() );
				getFilesFrom( mTBuf->getBuffer().toUtf8(), mTBuf->getCursorPosition() );
			}

			if ( KeyMod & KEYMOD_SHIFT ) {
				if ( KeyCode == KEY_UP ) {
					if ( mCon.ConMin - mCon.ConModif > 0 )
						mCon.ConModif++;
				}

				if ( KeyCode == KEY_DOWN ) {
					if ( mCon.ConModif > 0 )
						mCon.ConModif--;
				}

				if ( KeyCode == KEY_HOME ) {
					size_t size;
					{
						Lock l( mMutex );
						size = mCmdLog.size();
					}
					if ( static_cast<Int32>( size ) > linesOnScreen() )
						mCon.ConModif = mCon.ConMin;
				}

				if ( KeyCode == KEY_END ) {
					mCon.ConModif = 0;
				}

				if ( KeyCode == KEY_PAGEUP ) {
					if ( mCon.ConMin - mCon.ConModif - linesOnScreen() / 2 > 0 )
						mCon.ConModif += linesOnScreen() / 2;
					else
						mCon.ConModif = mCon.ConMin;
				}

				if ( KeyCode == KEY_PAGEDOWN ) {
					if ( mCon.ConModif - linesOnScreen() / 2 > 0 )
						mCon.ConModif -= linesOnScreen() / 2;
					else
						mCon.ConModif = 0;
				}
			} else {
				if ( mLastCommands.size() > 0 ) {
					if ( KeyCode == KEY_UP && mLastLogPos > 0 ) {
						mLastLogPos--;
					}

					if ( KeyCode == KEY_DOWN &&
						 mLastLogPos < static_cast<int>( mLastCommands.size() ) ) {
						mLastLogPos++;
					}

					if ( KeyCode == KEY_UP || KeyCode == KEY_DOWN ) {
						if ( mLastLogPos == static_cast<int>( mLastCommands.size() ) ) {
							mTBuf->setBuffer( "" );
						} else {
							mTBuf->setBuffer( mLastCommands[mLastLogPos] );
							mTBuf->cursorToEnd();
						}
					}
				}
			}
		} else if ( InputEvent::MouseButtonUp == etype ) {
			if ( Button == EE_BUTTON_WHEELUP ) {
				if ( mCon.ConMin - mCon.ConModif - 6 > 0 ) {
					mCon.ConModif += 6;
				} else {
					mCon.ConModif = mCon.ConMin;
				}
			}

			if ( Button == EE_BUTTON_WHEELDOWN ) {
				if ( mCon.ConModif - 6 > 0 ) {
					mCon.ConModif -= 6;
				} else {
					mCon.ConModif = 0;
				}
			}
		}
	}
}

void Console::createDefaultCommands() {
	addCommand( "clear", cb::Make1( this, &Console::cmdClear ) );
	addCommand( "quit", cb::Make1( this, &Console::cmdQuit ) );
	addCommand( "maximize", cb::Make1( this, &Console::cmdMaximize ) );
	addCommand( "minimize", cb::Make1( this, &Console::cmdMinimize ) );
	addCommand( "cmdlist", cb::Make1( this, &Console::cmdCmdList ) );
	addCommand( "help", cb::Make1( this, &Console::cmdCmdList ) );
	addCommand( "showcursor", cb::Make1( this, &Console::cmdShowCursor ) );
	addCommand( "setfpslimit", cb::Make1( this, &Console::cmdFrameLimit ) );
	addCommand( "getlog", cb::Make1( this, &Console::cmdGetLog ) );
	addCommand( "setgamma", cb::Make1( this, &Console::cmdSetGamma ) );
	addCommand( "setvolume", cb::Make1( this, &Console::cmdSetVolume ) );
	addCommand( "getgpuextensions", cb::Make1( this, &Console::cmdGetGpuExtensions ) );
	addCommand( "dir", cb::Make1( this, &Console::cmdDir ) );
	addCommand( "ls", cb::Make1( this, &Console::cmdDir ) );
	addCommand( "showfps", cb::Make1( this, &Console::cmdShowFps ) );
	addCommand( "gettexturememory", cb::Make1( this, &Console::cmdGetTextureMemory ) );
	addCommand( "hide", cb::Make1( this, &Console::cmdHideConsole ) );
}

void Console::cmdClear() {
	Uint16 CutLines;
	if ( mExpand ) {
		CutLines = ( Uint16 )( mHeight / mFontSize );
	} else {
		CutLines = ( Uint16 )( ( mHeightMin * mHeight ) / mFontSize );
	}

	for ( Uint16 i = 0; i < CutLines; i++ )
		privPushText( "" );
}

void Console::cmdClear( const std::vector<String>& params ) {
	cmdClear();
}

void Console::cmdMaximize( const std::vector<String>& params ) {
	mExpand = true;
	mY = mHeight;
	privPushText( "Console Maximized" );
}

void Console::cmdMinimize( const std::vector<String>& params ) {
	mExpand = false;
	mY = eefloor( mHeightMin * mHeight );
	privPushText( "Console Minimized" );
}

void Console::cmdQuit( const std::vector<String>& params ) {
	mWindow->close();
}

void Console::cmdGetTextureMemory( const std::vector<String>& params ) {
	privPushText( "Total texture memory used: " +
				  FileSystem::sizeToString( TextureFactory::instance()->getTextureMemorySize() ) );
}

void Console::cmdCmdList( const std::vector<String>& params ) {
	std::map<String, ConsoleCallback>::iterator itr;
	for ( itr = mCallbacks.begin(); itr != mCallbacks.end(); ++itr ) {
		privPushText( "\t" + itr->first );
	}
}

void Console::cmdShowCursor( const std::vector<String>& params ) {
	if ( params.size() >= 2 ) {
		Int32 tInt = 0;

		bool Res = String::fromString<Int32>( tInt, params[1] );

		if ( Res && ( tInt == 0 || tInt == 1 ) ) {
			mWindow->getCursorManager()->setVisible( 0 != tInt );
		} else
			privPushText( "Valid parameters are 0 or 1." );
	} else {
		privPushText( "No parameters. Valid parameters are 0 ( hide ) or 1 ( show )." );
	}
}

void Console::cmdFrameLimit( const std::vector<String>& params ) {
	if ( params.size() >= 2 ) {
		Int32 tInt = 0;

		bool Res = String::fromString<Int32>( tInt, params[1] );

		if ( Res && ( tInt >= 0 && tInt <= 10000 ) ) {
			mWindow->setFrameRateLimit( tInt );
			return;
		}
	}

	privPushText( "Valid parameters are between 0 and 10000 (0 = no limit)." );
}

void Console::cmdGetLog() {
	std::vector<String> tvec =
		String::split( String( String::toString( Log::instance()->getBuffer() ) ) );
	if ( tvec.size() > 0 ) {
		for ( unsigned int i = 0; i < tvec.size(); i++ )
			privPushText( tvec[i] );
	}
}

void Console::cmdGetLog( const std::vector<String>& params ) {
	cmdGetLog();
}

void Console::cmdGetGpuExtensions() {
	std::vector<String> tvec = String::split( String( GLi->getExtensions() ), ' ' );
	if ( tvec.size() > 0 ) {
		for ( unsigned int i = 0; i < tvec.size(); i++ )
			privPushText( tvec[i] );
	}
}

void Console::cmdGetGpuExtensions( const std::vector<String>& params ) {
	cmdGetGpuExtensions();
}

void Console::cmdSetGamma( const std::vector<String>& params ) {
	if ( params.size() >= 2 ) {
		Float tFloat = 0.f;

		bool Res = String::fromString<Float>( tFloat, params[1] );

		if ( Res && ( tFloat > 0.1f && tFloat <= 10.0f ) ) {
			mWindow->setGamma( tFloat, tFloat, tFloat );
			return;
		}
	}

	privPushText( "Valid parameters are between 0.1 and 10." );
}

void Console::cmdSetVolume( const std::vector<String>& params ) {
	if ( params.size() >= 2 ) {
		Float tFloat = 0.f;

		bool Res = String::fromString<Float>( tFloat, params[1] );

		if ( Res && ( tFloat >= 0.0f && tFloat <= 100.0f ) ) {
			EE::Audio::Listener::setGlobalVolume( tFloat );
			return;
		}
	}

	privPushText( "Valid parameters are between 0 and 100." );
}

void Console::cmdDir( const std::vector<String>& params ) {
	if ( params.size() >= 2 ) {
		String Slash( FileSystem::getOSSlash() );
		String myPath = params[1];
		String myOrder;

		if ( params.size() > 2 ) {
			myOrder = params[2];
		}

		if ( FileSystem::isDirectory( myPath ) ) {
			unsigned int i;

			std::vector<String> mFiles = FileSystem::filesGetInPath( myPath );
			std::sort( mFiles.begin(), mFiles.end() );

			privPushText( "Directory: " + myPath );

			if ( myOrder == "ff" ) {
				std::vector<String> mFolders;
				std::vector<String> mFile;

				for ( i = 0; i < mFiles.size(); i++ ) {
					if ( FileSystem::isDirectory( myPath + Slash + mFiles[i] ) ) {
						mFolders.push_back( mFiles[i] );
					} else {
						mFile.push_back( mFiles[i] );
					}
				}

				if ( mFolders.size() )
					privPushText( "Folders: " );

				for ( i = 0; i < mFolders.size(); i++ )
					privPushText( "	" + mFolders[i] );

				if ( mFolders.size() )
					privPushText( "Files: " );

				for ( i = 0; i < mFile.size(); i++ )
					privPushText( "	" + mFile[i] );

			} else {
				for ( i = 0; i < mFiles.size(); i++ )
					privPushText( "	" + mFiles[i] );
			}
		} else {
			if ( myPath == "help" )
				privPushText(
					"You can use a third parameter to show folders first, the parameter is ff." );
			else
				privPushText( "Path \"" + myPath + "\" is not a directory." );
		}
	} else {
		privPushText( "Expected a path to list. Example of usage: ls /home" );
	}
}

void Console::cmdShowFps( const std::vector<String>& params ) {
	if ( params.size() >= 2 ) {
		Int32 tInt = 0;

		bool Res = String::fromString<Int32>( tInt, params[1] );

		if ( Res && ( tInt == 0 || tInt == 1 ) ) {
			mShowFps = 0 != tInt;
			return;
		}
	}

	privPushText( "Valid parameters are 0 ( hide ) or 1 ( show )." );
}

void Console::cmdHideConsole( const std::vector<String>& params ) {
	fadeOut();
}

void Console::ignoreCharOnPrompt( const Uint32& ch ) {
	mTBuf->pushIgnoredChar( ch );
}

const bool& Console::isShowingFps() const {
	return mShowFps;
}

void Console::showFps( const bool& Show ) {
	mShowFps = Show;
}

FontStyleConfig Console::getFontStyleConfig() const {
	return mFontStyleConfig;
}

void Console::setFontStyleConfig( const FontStyleConfig& fontStyleConfig ) {
	mFontStyleConfig = fontStyleConfig;
}

const bool& Console::isFading() const {
	return mFading;
}

void Console::writeLog( const std::string& Text ) {
	std::vector<String> Strings = String::split( String( Text ) );

	for ( Uint32 i = 0; i < Strings.size(); i++ ) {
		privPushText( Strings[i] );
	}
}

}} // namespace EE::Graphics
