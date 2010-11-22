#include "cuitextbox.hpp"
#include "cuimanager.hpp"
#include "cuithememanager.hpp"

namespace EE { namespace UI {

cUITextBox::cUITextBox( const cUITextBox::CreateParams& Params ) :
	cUIControlAnim( Params ),
	mFontColor( Params.FontColor ),
	mFontShadowColor( Params.FontShadowColor ),
	mAlignOffset( 0.f, 0.f )
{
	mType |= UI_TYPE_GET(UI_TYPE_TEXTBOX);

	mTextCache = eeNew( cTextCache, () );
	mTextCache->Font( Params.Font );
	mTextCache->Color( mFontColor );
	mTextCache->ShadowColor( mFontShadowColor );

	if ( NULL == Params.Font && NULL != cUIThemeManager::instance()->DefaultFont() ) {
		mTextCache->Font( cUIThemeManager::instance()->DefaultFont() );
	} else {
		eePRINT( "cUITextBox::cUITextBox : Created a UI TextBox without a defined font." );
	}

	AutoAlign();
}

cUITextBox::~cUITextBox() {
	eeSAFE_DELETE( mTextCache );
}

void cUITextBox::Draw() {
	if ( mVisible ) {
		cUIControl::Draw();

		if ( mTextCache->GetTextWidth() ) {
			if ( mFlags & UI_CLIP_ENABLE )
				cUIManager::instance()->ClipEnable( mScreenPos.x + mPadding.Left, mScreenPos.y + mPadding.Top, mSize.Width() - mPadding.Left - mPadding.Right, mSize.Height() - mPadding.Bottom );

			mTextCache->Draw( (eeFloat)mScreenPos.x + mAlignOffset.x + (eeFloat)mPadding.Left + 1.f, (eeFloat)mScreenPos.y + mAlignOffset.y + (eeFloat)mPadding.Top, Flags(), 1.f, 0.f, mBlend );

			if ( mFlags & UI_CLIP_ENABLE )
				cUIManager::instance()->ClipDisable();
		}
	}
}

cFont * cUITextBox::Font() const {
	return mTextCache->Font();
}

void cUITextBox::Font( cFont * font ) {
	mTextCache->Font( font );
	AutoShrink();
	AutoSize();
	AutoAlign();
	OnFontChanged();
}

const std::wstring& cUITextBox::Text() {
	return mTextCache->Text();
}

void cUITextBox::Text( const std::string& text ) {
	Text( stringTowstring( text ) );
}

void cUITextBox::Text( const std::wstring& text ) {
	mTextCache->Text( text );
	AutoShrink();
	AutoSize();
	AutoAlign();
	OnTextChanged();
}

const eeColorA& cUITextBox::Color() const {
	return mFontColor;
}

void cUITextBox::Color( const eeColorA& color ) {
	mFontColor = color;
	Alpha( color.A() );
	mTextCache->Color( mFontColor );
}

const eeColorA& cUITextBox::ShadowColor() const {
	return mFontShadowColor;
}

void cUITextBox::ShadowColor( const eeColorA& color ) {
	mFontShadowColor = color;
	Alpha( color.A() );
}

void cUITextBox::Alpha( const eeFloat& alpha ) {
	cUIControlAnim::Alpha( alpha );
	mFontColor.Alpha = (Uint8)alpha;
	mFontShadowColor.Alpha = (Uint8)alpha;
}

void cUITextBox::AutoShrink() {
	if ( Flags() & UI_AUTO_SHRINK_TEXT ) {
		mTextCache->Font()->ShrinkText( mTextCache->Text(), mSize.Width() );
	}
}

void cUITextBox::AutoSize() {
	if ( Flags() & UI_AUTO_SIZE ) {
		mSize.Width( (eeInt)mTextCache->GetTextWidth() );
		mSize.Height( (eeInt)mTextCache->GetTextHeight() );
	}
}

void cUITextBox::AutoAlign() {
	switch ( FontHAlignGet( Flags() ) ) {
		case UI_HALIGN_CENTER:
			mAlignOffset.x = (eeFloat)( (Int32)( mSize.x - mTextCache->GetTextWidth() ) / 2 );
			break;
		case UI_HALIGN_RIGHT:
			mAlignOffset.x = ( (eeFloat)mSize.x - (eeFloat)mTextCache->GetTextWidth() );
			break;
		case UI_HALIGN_LEFT:
			mAlignOffset.x = 0.f;
			break;
	}

	switch ( FontVAlignGet( Flags() ) ) {
		case UI_VALIGN_CENTER:
			mAlignOffset.y = (eeFloat)( ( (Int32)( mSize.y - mTextCache->GetTextHeight() ) ) / 2 );
			break;
		case UI_VALIGN_BOTTOM:
			mAlignOffset.y = ( (eeFloat)mSize.y - (eeFloat)mTextCache->GetTextHeight() );
			break;
		case UI_VALIGN_TOP:
			mAlignOffset.y = 0.f;
			break;
	}
}

void cUITextBox::OnSizeChange() {
	AutoShrink();
	AutoSize();
	AutoAlign();

	cUIControlAnim::OnSizeChange();

	mTextCache->Cache();
}

void cUITextBox::OnTextChanged() {
	SendCommonEvent( cUIEvent::EventOnTextChanged );
}

void cUITextBox::OnFontChanged() {
	SendCommonEvent( cUIEvent::EventOnFontChanged );
}

void cUITextBox::Padding( const eeRecti& padding ) {
	mPadding = padding;
}

const eeRecti& cUITextBox::Padding() const {
	return mPadding;
}

void cUITextBox::SetTheme( cUITheme * Theme ) {
	cUIControlAnim::SetTheme( Theme );

	if ( NULL == mTextCache->Font() && NULL != Theme->DefaultFont() ) {
		mTextCache->Font( Theme->DefaultFont() );
	}
}

cTextCache * cUITextBox::GetTextCache() {
	return mTextCache;
}

eeFloat cUITextBox::GetTextWidth() {
	return mTextCache->GetTextWidth();
}

eeFloat cUITextBox::GetTextHeight() {
	return mTextCache->GetTextHeight();
}

const eeInt& cUITextBox::GetNumLines() const {
	return mTextCache->GetNumLines();
}

}}
