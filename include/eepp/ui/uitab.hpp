#ifndef EE_UICUITAB_HPP
#define EE_UICUITAB_HPP

#include <eepp/ui/uiselectbutton.hpp>

namespace EE { namespace UI {

class UITabWidget;

class EE_API UITab : public UISelectButton {
  public:
	static UITab* New();

	UITab();

	Node* getControlOwned() const;

	void setControlOwned( Node* controlOwned );

	virtual ~UITab();

	virtual Uint32 getType() const;

	virtual bool isType( const Uint32& type ) const;

	virtual void setTheme( UITheme* Theme );

	virtual const String& getText();

	virtual UIPushButton* setText( const String& text );

	virtual bool applyProperty( const StyleSheetProperty& attribute );

	virtual std::string getPropertyString( const PropertyDefinition* propertyDef,
										   const Uint32& propertyIndex = 0 );

  protected:
	friend class UITabWidget;

	Node* mControlOwned;
	String mText;
	std::string mOwnedName;

	virtual Uint32 onMouseUp( const Vector2i& position, const Uint32& flags );

	virtual Uint32 onMouseClick( const Vector2i& position, const Uint32& flags );

	virtual void onStateChange();

	virtual void onAutoSize();

	UITabWidget* getTabWidget();

	virtual void onParentChange();

	void setOwnedControl();

	void updateTab();
};

}} // namespace EE::UI

#endif
