#ifndef EE_UICUIRADIOBUTTON_H
#define EE_UICUIRADIOBUTTON_H

#include "cuitextbox.hpp"
#include "cuipushbutton.hpp"

namespace EE { namespace UI {

class EE_API cUIRadioButton : public cUITextBox {
	public:
		cUIRadioButton( const cUITextBox::CreateParams& Params );

		~cUIRadioButton();

		virtual void SetTheme( cUITheme * Theme );

		virtual Uint32 OnMessage( const cUIMessage * Msg );

		const bool& IsActive() const;

		void Active( const bool& active );

		const bool& Active() const;

		virtual void Padding( const eeRecti& padding );

		cUIControlAnim * ActiveButton() const;

		cUIControlAnim * InactiveButton() const;

		virtual Uint32 OnKeyDown( const cUIEventKey& Event );
	protected:
		cUIControlAnim *	mActiveButton;
		cUIControlAnim *	mInactiveButton;
		bool				mActive;
		Uint32				mLastTick;

		virtual void OnSizeChange();

		void SwitchState();

		void AutoActivate();

		bool CheckActives();
};

}}

#endif

