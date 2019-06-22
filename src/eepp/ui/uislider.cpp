#include <eepp/ui/uislider.hpp>
#include <eepp/ui/uithememanager.hpp>
#include <eepp/graphics/textureregion.hpp>
#include <pugixml/pugixml.hpp>

namespace EE { namespace UI {

UISlider * UISlider::New() {
	return eeNew( UISlider, ( UI_VERTICAL ) );
}

UISlider * UISlider::NewVertical() {
	return eeNew( UISlider, ( UI_VERTICAL ) );
}

UISlider * UISlider::NewHorizontal() {
	return eeNew( UISlider, ( UI_HORIZONTAL ) );
}

UISlider::UISlider( const UI_ORIENTATION& orientation ) :
	UIWidget( "slider" ),
	mOrientation( orientation ),
	mAllowHalfSliderOut( false ),
	mExpandBackground( false ),
	mBackSlider( NULL ),
	mSlider( NULL ),
	mMinValue( 0.f ),
	mMaxValue( 1.f ),
	mValue( 0.f ),
	mClickStep( 0.1f ),
	mPageStep( 0 ),
	mOnPosChange( false )
{
	Sizef bgSize;

	if ( UI_HORIZONTAL == mOrientation )
		bgSize = Sizef( mDpSize.getWidth() - 16, 8 );
	else
		bgSize = Sizef( 8, mDpSize.getHeight() - 16 );

	mBackSlider = UIWidget::NewWithTag( "slider::back" );
	mBackSlider->setParent( this );
	mBackSlider->setVisible( true );
	mBackSlider->setEnabled( true );
	mBackSlider->setSize( bgSize );
	mBackSlider->center();

	mSlider = Private::UISliderButton::New();
	mSlider->setParent( this );
	mSlider->setEnabled( true );
	mSlider->setVisible( true );
	mSlider->setDragEnabled( true );
	mSlider->setSize( 16, 16 );
	mSlider->setPosition( 0, 0 );

	if ( UI_HORIZONTAL == mOrientation )
		mSlider->centerVertical();
	else
		mSlider->centerHorizontal();

	applyDefaultTheme();
}

UISlider::~UISlider() {
}

Uint32 UISlider::getType() const {
	return UI_TYPE_SLIDER;
}

bool UISlider::isType( const Uint32& type ) const {
	return UISlider::getType() == type ? true : UIWidget::isType( type );
}

void UISlider::setTheme( UITheme * Theme ) {
	UIWidget::setTheme( Theme );

	if ( UI_HORIZONTAL == mOrientation ) {
		setThemeSkin( Theme, "hslider" );

		mBackSlider->setThemeSkin( Theme, "hslider_bg" );
		mSlider->setThemeSkin( Theme, "hslider_button" );
	} else {
		setThemeSkin( Theme, "vslider" );

		mBackSlider->setThemeSkin( Theme, "vslider_bg" );
		mSlider->setThemeSkin( Theme, "vslider_button" );
	}

	adjustChilds();

	setValue( mValue );

	onThemeLoaded();
}

void UISlider::onSizeChange() {
	UIWidget::onSizeChange();
	adjustChilds();
}

void UISlider::onPaddingChange() {
	adjustChilds();
	UIWidget::onPaddingChange();
}

void UISlider::adjustChilds() {
	UISkin * tSkin = NULL;

	tSkin = mSlider->getSkin();

	if ( NULL != tSkin ) {
		if ( mPageStep == 0 ) {
			mSlider->setSize( tSkin->getSize() );
		} else {
			Float percent = ( mPageStep / ( mMaxValue - mMinValue ) );

			if ( UI_HORIZONTAL == mOrientation ) {
				Float size = eemax( ( (Float)mDpSize.getWidth() * percent ), tSkin->getSize().getWidth() );

				mSlider->setSize( size, tSkin->getSize().getHeight() );
			} else {
				Float size = eemax( ( (Float)mDpSize.getHeight() * percent ), tSkin->getSize().getHeight() );

				mSlider->setSize( tSkin->getSize().getWidth(), size );
			}
		}

		if ( UI_HORIZONTAL == mOrientation ) {
			mSlider->centerVertical();
		} else {
			mSlider->centerHorizontal();
		}
	}

	tSkin = mBackSlider->getSkin();

	if ( NULL != tSkin ) {
		if ( UI_HORIZONTAL == mOrientation ) {
			Float Height;

			if ( mExpandBackground )
				Height = mDpSize.getHeight() - mPadding.Top - mPadding.Bottom;
			else
				Height = tSkin->getSize().getHeight();

			if ( mAllowHalfSliderOut )
				mBackSlider->setSize( Sizef( mDpSize.getWidth() - mSlider->getSize().getWidth() - mPadding.Left - mPadding.Right, Height ) );
			else
				mBackSlider->setSize( Sizef( mDpSize.getWidth() - mPadding.Left - mPadding.Right, Height ) );
		} else {
			Float Width;

			if ( mExpandBackground )
				Width = mDpSize.getWidth() - mPadding.Left - mPadding.Right;
			else
				Width = tSkin->getSize().getWidth();

			if ( mAllowHalfSliderOut )
				mBackSlider->setSize( Sizef( Width, mDpSize.getHeight() - mSlider->getSize().getHeight() - mPadding.Top - mPadding.Bottom ) );
			else
				mBackSlider->setSize( Sizef( Width, mDpSize.getHeight() ) );
		}

		mBackSlider->center();
	}

	fixSliderPos();
}

void UISlider::fixSliderPos() {
	if ( !mOnPosChange ) {
		mOnPosChange = true;

		if ( UI_HORIZONTAL == mOrientation ) {
			mSlider->setPosition( mSlider->getPosition().x, 0 );

			if ( mSlider->getPosition().x < mPadding.Left )
				mSlider->setPosition( mPadding.Left, 0 );

			if ( mAllowHalfSliderOut ) {
				if ( mSlider->getPosition().x > mBackSlider->getSize().getWidth() + mPadding.Left  )
					mSlider->setPosition( mBackSlider->getSize().getWidth() + mPadding.Left, 0 );
			} else {
				if ( mSlider->getPosition().x > mBackSlider->getSize().getWidth() - mSlider->getSize().getWidth() + mPadding.Left )
					mSlider->setPosition( mBackSlider->getSize().getWidth() - mSlider->getSize().getWidth() + mPadding.Left, 0 );
			}

			mSlider->centerVertical();

			if ( mAllowHalfSliderOut )
				setValue( mMinValue + ( mSlider->getPosition().x - mPadding.Left ) * ( mMaxValue - mMinValue ) / (Float)mBackSlider->getSize().getWidth() );
			else
				setValue( mMinValue + ( mSlider->getPosition().x - mPadding.Left ) * ( mMaxValue - mMinValue ) / ( (Float)mDpSize.getWidth() - mSlider->getSize().getWidth() ) );
		} else {
			mSlider->setPosition( 0, mSlider->getPosition().y );

			if ( mSlider->getPosition().y < mPadding.Top )
				mSlider->setPosition( 0, mPadding.Top );

			if ( mAllowHalfSliderOut ) {
				if ( mSlider->getPosition().y > mBackSlider->getSize().getHeight() + mPadding.Top )
					mSlider->setPosition( 0, mBackSlider->getSize().getHeight() + mPadding.Top );
			} else {
				if ( mSlider->getPosition().y > mBackSlider->getSize().getHeight() - mSlider->getSize().getHeight() + mPadding.Top ) {
					mSlider->setPosition( 0, mBackSlider->getSize().getHeight() - mSlider->getSize().getHeight() + mPadding.Top );
				}
			}

			mSlider->centerHorizontal();

			if ( mAllowHalfSliderOut )
				setValue( mMinValue + ( mSlider->getPosition().y - mPadding.Top ) * ( mMaxValue - mMinValue ) / (Float)mBackSlider->getSize().getHeight() );
			else
				setValue( mMinValue + ( mSlider->getPosition().y - mPadding.Top ) * ( mMaxValue - mMinValue ) / ( (Float)mDpSize.getHeight() - mSlider->getSize().getHeight() ) );
		}

		mOnPosChange = false;
	}
}

void UISlider::setValue( Float Val ) {
	if ( mValue == Val )
		return;

	if ( Val < mMinValue ) Val = mMinValue;
	if ( Val > mMaxValue ) Val = mMaxValue;

	if ( Val >= mMinValue && Val <= mMaxValue ) {
		mValue = Val;

		if ( !mOnPosChange ) {
			Float Percent = ( Val - mMinValue ) / ( mMaxValue - mMinValue );

			mOnPosChange = true;

			if ( UI_HORIZONTAL == mOrientation ) {
				if ( mAllowHalfSliderOut )
					mSlider->setPosition( mPadding.Left + (Int32)( (Float)mBackSlider->getSize().getWidth() * Percent ), mSlider->getPosition().y );
				else
					mSlider->setPosition( mPadding.Left + (Int32)( ( (Float)mDpSize.getWidth() - mPadding.Left - mPadding.Top - mSlider->getSize().getWidth() ) * Percent ), mSlider->getPosition().y );
			} else {
				if ( mAllowHalfSliderOut )
					mSlider->setPosition( mSlider->getPosition().x, mPadding.Top + (Int32)( (Float)mBackSlider->getSize().getHeight() * Percent ) );
				else
					mSlider->setPosition( mSlider->getPosition().x, mPadding.Top + (Int32)( ( (Float)mDpSize.getHeight() - mPadding.Top - mPadding.Bottom - mSlider->getSize().getHeight() ) * Percent ) );
			}

			mOnPosChange = false;
		}

		onValueChange();
	}
}

const Float& UISlider::getValue() const {
	return mValue;
}

void UISlider::setMinValue( const Float& MinVal ) {
	mMinValue = MinVal;

	if ( mValue < mMinValue )
		mValue = mMinValue;

	fixSliderPos();
}

const Float& UISlider::getMinValue() const {
	return mMinValue;
}

void UISlider::setMaxValue( const Float& MaxVal ) {
	mMaxValue = MaxVal;

	if ( mValue > mMaxValue )
		mValue = mMaxValue;

	fixSliderPos();
}

const Float& UISlider::getMaxValue() const {
	return mMaxValue;
}

void UISlider::setClickStep( const Float& step ) {
	mClickStep = step;
}

const Float& UISlider::getClickStep() const {
	return mClickStep;
}

bool UISlider::isVertical() const {
	return mOrientation == UI_VERTICAL;
}

Uint32 UISlider::onKeyDown( const KeyEvent &Event ) {
	if ( Sys::getTicks() - mLastTickMove > 100 ) {
		if ( Event.getKeyCode() == KEY_DOWN ) {
			mLastTickMove = Sys::getTicks();

			setValue( mValue + mClickStep );
		} else if ( Event.getKeyCode() == KEY_UP ) {
			mLastTickMove = Sys::getTicks();

			setValue( mValue - mClickStep );
		} else if ( Event.getKeyCode() == KEY_PAGEUP ) {
			mLastTickMove = Sys::getTicks();

			setValue( mMinValue );
		} else if ( Event.getKeyCode() == KEY_PAGEDOWN ) {
			mLastTickMove = Sys::getTicks();

			setValue( mMaxValue );
		}
	}

	return UIWidget::onKeyDown( Event );
}

void UISlider::manageClick( const Uint32& Flags ) {
	if ( Flags && NULL != getEventDispatcher() ) {
		Vector2f ControlPos = getEventDispatcher()->getMousePosf();
		mSlider->worldToNode( ControlPos );

		if ( Flags & EE_BUTTON_LMASK && !mSlider->isMouseOver()  ) {
			if ( UI_HORIZONTAL == mOrientation ) {
				if ( ControlPos.x < 0 )
					setValue( mValue - mClickStep );
				else
					setValue( mValue + mClickStep );
			} else {
				if ( ControlPos.y < 0 )
					setValue( mValue - mClickStep );
				else
					setValue( mValue + mClickStep );
			}
		} else if ( Flags & EE_BUTTONS_WUWD ) {
			if ( Flags & EE_BUTTON_WUMASK )
				setValue( mValue - mClickStep );
			else
				setValue( mValue + mClickStep );
		}
	}
}

UI_ORIENTATION UISlider::getOrientation() const {
	return mOrientation;
}

UISlider * UISlider::setOrientation( const UI_ORIENTATION & orientation ) {
	if ( orientation != mOrientation ) {
		mOrientation = orientation;

		applyDefaultTheme();
	}

	return this;
}

bool UISlider::getAllowHalfSliderOut() const {
	return mAllowHalfSliderOut;
}

void UISlider::setAllowHalfSliderOut( bool allowHalfSliderOut ) {
	if ( mAllowHalfSliderOut != allowHalfSliderOut ) {
		mAllowHalfSliderOut = allowHalfSliderOut;

		adjustChilds();

		setValue( mValue );
	}
}

bool UISlider::getExpandBackground() const {
	return mExpandBackground;
}

void UISlider::setExpandBackground( bool expandBackground ) {
	if ( mExpandBackground != expandBackground ) {
		mExpandBackground = expandBackground;

		adjustChilds();

		setValue( mValue );
	}
}

Float UISlider::getPageStep() const {
	return mPageStep;
}

void UISlider::setPageStep(const Float & pageStep) {
	mPageStep = eemin( eemax( pageStep, mMinValue ), mMaxValue );

	adjustChilds();

	setValue( mValue );
}

UINode * UISlider::getBackSlider() const {
	return mBackSlider;
}

UINode * UISlider::getSliderButton() const {
	return mSlider;
}

void UISlider::onAlphaChange() {
	UINode::onAlphaChange();
	
	mBackSlider->setAlpha( mAlpha );
	mSlider->setAlpha( mAlpha );
}

Uint32 UISlider::onMessage(const NodeMessage * Msg) {
	switch ( Msg->getMsg() ) {
		case NodeMessage::MouseUp:
		{
			manageClick( Msg->getFlags() );
			return 1;
		}
	}

	return 0;
}

bool UISlider::setAttribute( const NodeAttribute& attribute, const Uint32& state ) {
	const std::string& name = attribute.getName();

	if ( "orientation" == name ) {
		std::string val = attribute.asString();
		String::toLowerInPlace( val );

		if ( "horizontal" == val )
			setOrientation( UI_HORIZONTAL );
		else if ( "vertical" == val )
			setOrientation( UI_VERTICAL );
	} else if ( "minvalue" == name ) {
		setMinValue( attribute.asFloat() );
	} else if ( "maxvalue" == name ) {
		setMaxValue( attribute.asFloat() );
	} else if ( "value" == name ) {
		setValue( attribute.asFloat() );
	} else if ( "clickstep" == name ) {
		setClickStep( attribute.asFloat() );
	} else if ( "pagestep" == name ) {
		setPageStep( attribute.asFloat() );
	} else if ( "halfslider" == name ) {
		setAllowHalfSliderOut( attribute.asBool() );
	} else if ( "expandbackground" == name ) {
		setExpandBackground( attribute.asBool() );
	} else {
		return UIWidget::setAttribute( attribute, state );
	}

	return true;
}

Sizef UISlider::getMinimumSize() {
	Float w = eemax( mBackSlider->getSkinSize().getWidth(), mSlider->getSkinSize().getWidth() );
	Float h = eemax( mBackSlider->getSkinSize().getHeight(), mSlider->getSkinSize().getHeight() );
	return Sizef( w + ( mAllowHalfSliderOut ? w : 0 ) + mPadding.Left + mPadding.Right, h + mPadding.Top + mPadding.Bottom );
}

void UISlider::onAutoSize() {
	if ( mLayoutWidthRules == WRAP_CONTENT || mLayoutHeightRules == WRAP_CONTENT ) {
		Sizef total( getMinimumSize() );

		total = PixelDensity::dpToPx( total );

		if ( mLayoutWidthRules == WRAP_CONTENT ) {
			setInternalPixelsWidth( total.getWidth() );
		}

		if ( mLayoutHeightRules == WRAP_CONTENT ) {
			setInternalPixelsHeight( total.getHeight() );
		}

		adjustChilds();
	}
}

}}
