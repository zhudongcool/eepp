#ifndef EE_UI_UICODEEDIT_HPP
#define EE_UI_UICODEEDIT_HPP

#include <eepp/ui/doc/textdocument.hpp>
#include <eepp/ui/uifontstyleconfig.hpp>
#include <eepp/ui/uiwidget.hpp>

using namespace EE::Graphics;
using namespace EE::UI::Doc;

namespace EE { namespace Graphics {
class Font;
}} // namespace EE::Graphics

namespace EE { namespace UI {

class EE_API UICodeEdit : public UIWidget, public TextDocument::Client {
  public:
	static UICodeEdit* New();

	UICodeEdit();

	virtual ~UICodeEdit();

	virtual Uint32 getType() const;

	virtual bool isType( const Uint32& type ) const;

	virtual void setTheme( UITheme* Theme );

	virtual void draw();

	virtual void scheduledUpdate( const Time& time );

	void reset();

	void loadFromFile( const std::string& path );

	Font* getFont() const;

	void setFont( Font* font );

	const Uint32& getTabWidth() const;
	void setTabWidth( const Uint32& tabWidth );

  protected:
	Font* mFont;
	UIFontStyleConfig mFontStyleConfig;
	Doc::TextDocument mDoc;
	Vector2f mScrollPos;
	Clock mBlinkTimer;
	bool mDirtyEditor;
	bool mCursorVisible;
	Uint32 mTabWidth;
	Int64 mLastColOffset;
	Vector2f mScroll;

	void invalidateEditor();

	virtual Uint32 onKeyDown( const KeyEvent& Event );

	virtual Uint32 onMouseDown( const Vector2i& position, const Uint32& flags );

	virtual void onSizeChange();

	virtual void onPaddingChange();

	void updateEditor();

	void onDocumentTextChanged();
	void onDocumentCursorChange( const TextPosition& );
	void onDocumentSelectionChange( const TextRange& );

	std::pair<int, int> getVisibleLineRange();

	int getVisibleLinesCount();

	void scrollToMakeVisible( const TextPosition& position );

	Float getXOffsetCol( const TextPosition& position ) const;

	Int64 getColFromXOffset( Int64 line, const Float& offset ) const;

	Float getLineHeight() const;

	Float getCharacterSize() const;

	Float getGlyphWidth() const;

	void updateLastColumnOffset();

	TextPosition resolveScreenPosition( const Vector2f& position ) const;
};

}} // namespace EE::UI

#endif // EE_UI_UICODEEDIT_HPP
