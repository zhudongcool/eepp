#ifndef EE_GRAPHICSCCONSOLE_H
#define EE_GRAPHICSCCONSOLE_H

#include <deque>
#include <eepp/graphics/base.hpp>
#include <eepp/graphics/font.hpp>
#include <eepp/graphics/primitives.hpp>
#include <eepp/graphics/text.hpp>
#include <eepp/window/inputtextbuffer.hpp>

namespace EE { namespace Window {
class Window;
class InputTextBuffer;
class InputEvent;
}} // namespace EE::Window

using namespace EE::Window;

namespace EE { namespace Graphics {

class EE_API Console : protected LogReaderInterface {
  public:
	//! The Console Callback return a vector of parameters ( String )
	typedef std::function<void( const std::vector<String>& )> ConsoleCallback;

	/** Instances the console but doesn't create it, you must call Create to initialize the console.
	 */
	Console( EE::Window::Window* window = NULL );

	/** Creates the console */
	Console( Font* Font, const bool& MakeDefaultCommands = true, const bool& AttachToLog = true,
			 const unsigned int& MaxLogLines = 1024, const Uint32& textureId = 0,
			 EE::Window::Window* window = NULL );

	virtual ~Console();

	/** Set the Console Height ( percent, between 0 and 1 ) when it's not in fullscreen  */
	void setConsoleMinimizedHeight( const Float& MinHeight );

	/** Get the Console Height when it's Minimized ( Not Maximized ) */
	const Float& getConsoleMinimizedHeight() const;

	/** Set the Texture Id for the Background, 0 will disable texture background */
	void setBackgroundTextureId( const Uint32& TexId );

	/** Get the Background Texture Id */
	Uint32 getBackgroundTextureId() const;

	/** Set the Console Background Color */
	void setBackgroundColor( const Color& BackColor );

	void setCharacterSize( const Uint32& characterSize );

	/** Get the Console Background Color */
	const Color& getBackgroundColor() const;

	/** Set the Console Border Line Background Color */
	void setBackgroundLineColor( const Color& BackColor );

	/** Get the Console Border Line Background Color */
	const Color& getBackgroundLineColor() const;

	/** Set the Console Font Color */
	void setFontColor( const Color& FntColor );

	/** Get the Console Font Color */
	const Color& getFontColor() const;

	/** Set the Console Client Input ( Writeable Line ) Font Color */
	void setFontLineColor( const Color& FntColor );

	/** Get the Console Client Input ( Writeable Line ) Font Color */
	const Color& getFontLineColor() const;

	/** Toogle the console between visible and hided with Fade In or Fade Out effect. */
	void toggle();

	/** Make visible the console */
	void fadeIn();

	/** Hide the console */
	void fadeOut();

	/** @return If Console Active ( Visible ) */
	bool isActive() const;

	/** Maximize or Minimize the Console */
	void setExpanded( const bool& Exp );

	/** @return If console is maximized */
	bool isExpanded() const;

	/** Set the fade time */
	void setFadeSpeed( const Time& fadespeed );

	/** @return The fading speed in ms */
	const Time& getFadeSpeed() const;

	/** @brief Creates the new console
	 * @param Font The Font pointer to class
	 * @param MakeDefaultCommands Register the default commands provided by the class?
	 * @param AttachToLog Attach the console to the Log instance
	 * @param MaxLogLines Maximum number of lines stored on the console
	 * @param textureId Background texture id ( 0 for no texture )
	 */
	void create( Font* Font, const bool& MakeDefaultCommands = true, const bool& AttachToLog = true,
				 const unsigned int& MaxLogLines = 1024, const Uint32& textureId = 0 );

	/** Add Text to Console */
	void pushText( const String& str );

	/** Add formated Text to console */
	void pushText( const char* format, ... );

	/** Adds a new Command
	 * @param Command The Command Name ( raise the event )
	 * @param CB The Callback for the Command
	 */
	void addCommand( const String& Command, ConsoleCallback CB );

	/** Draw the Console ( allways call it, visible or not ) */
	void draw( const Time& elapsedTime = Time::Zero );

	/** Set the line height ( distance between lines ) */
	void setLineHeight( const Float& LineHeight );

	/** Use this if you need to ignore some char to activate the console, for example '~'. A common
	 * char to activate a console. */
	void ignoreCharOnPrompt( const Uint32& ch );

	/** @return If the console is rendering the FPS count. */
	const bool& isShowingFps() const;

	/** Activate/Deactive fps rendering */
	void showFps( const bool& Show );

	FontStyleConfig getFontStyleConfig() const;

	void setFontStyleConfig( const FontStyleConfig& fontStyleConfig );

	const bool& isFading() const;

  protected:
	Mutex mMutex;
	std::map<String, ConsoleCallback> mCallbacks;
	std::deque<String> mCmdLog;
	std::deque<String> mLastCommands;

	EE::Window::Window* mWindow;

	Color mConColor;
	Color mConLineColor;
	Color mFontLineColor;

	Float mWidth;
	Float mHeight;
	Float mHeightMin;
	Float mCurHeight;
	Float mY;
	Float mA;
	Float mMaxAlpha;
	Float mTempY;
	Float mFontSize;
	Time mFadeSpeed;

	Uint32 mMyCallback;
	Uint32 mVidCb;
	Uint32 mEx;
	Uint32 mMaxLogLines;
	int mLastLogPos;

	InputTextBuffer* mTBuf;

	Primitives mPri;
	Uint32 mTexId;

	struct sCon {
		int ConMin;
		int ConMax;
		int ConModif;
	};
	sCon mCon;

	Float mCurAlpha;
	std::vector<Text> mTextCache;
	FontStyleConfig mFontStyleConfig;
	bool mEnabled;
	bool mVisible;
	bool mFadeIn;
	bool mFadeOut;
	bool mExpand;
	bool mFading;
	bool mShowFps;
	bool mCurSide;

	void createDefaultCommands();

	void fade( const Time& elapsedTime );

	/** Internal Callback for default command ( clear ) */
	void cmdClear( const std::vector<String>& params );

	/** Internal Callback for default command ( maximize ) */
	void cmdMaximize( const std::vector<String>& params );

	/** Internal Callback for default command ( minimize ) */
	void cmdMinimize( const std::vector<String>& params );

	/** Internal Callback for default command ( quit ) */
	void cmdQuit( const std::vector<String>& params );

	/** Internal Callback for default command ( cmdlist ) */
	void cmdCmdList( const std::vector<String>& params );

	/** Internal Callback for default command ( showcursor ) */
	void cmdShowCursor( const std::vector<String>& params );

	/** Internal Callback for default command ( setfpslimit ) */
	void cmdFrameLimit( const std::vector<String>& params );

	/** Internal Callback for default command ( getlog ) */
	void cmdGetLog( const std::vector<String>& params );

	/** Internal Callback for default command ( setgamma ) */
	void cmdSetGamma( const std::vector<String>& params );

	/** Internal Callback for default command ( setvolume ) */
	void cmdSetVolume( const std::vector<String>& params );

	/** Internal Callback for default command ( getgpuextensions ) */
	void cmdGetGpuExtensions( const std::vector<String>& params );

	/** Internal Callback for default command ( dir and ls ) */
	void cmdDir( const std::vector<String>& params );

	/** Internal Callback for default command ( showfps ) */
	void cmdShowFps( const std::vector<String>& params );

	/** Internal Callback for default command ( gettexturememory ) */
	void cmdGetTextureMemory( const std::vector<String>& params );

	/** Internal Callback for default command ( hide ) */
	void cmdHideConsole( const std::vector<String>& params );

	/** The Default Commands Callbacks for the Console ( don't call it ) */
	void privInputCallback( InputEvent* Event );

	/** Clear the Console */
	void cmdClear();

	/** Add the current log to the console */
	void cmdGetLog();

	/** Add the GPU Extensions supported to the console */
	void cmdGetGpuExtensions();

	/** Internal Callback to Process the new line ( when return pressed ) */
	void processLine();

	void privPushText( const String& str );

	void printCommandsStartingWith( const String& start );

	void privVideoResize( EE::Window::Window* win );

	void writeLog( const std::string& Text );

	void getFilesFrom( std::string txt, const Uint32& curPos );

	Int32 linesOnScreen();

	Int32 maxLinesOnScreen();

	String getLastCommonSubStr( std::list<String>& cmds );
};

}} // namespace EE::Graphics

#endif
