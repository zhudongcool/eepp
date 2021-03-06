#ifndef EE_SCENEEVENTKEY_HPP
#define EE_SCENEEVENTKEY_HPP

#include <eepp/core/string.hpp>
#include <eepp/scene/event.hpp>
#include <eepp/window/keycodes.hpp>

using namespace EE::Window;

namespace EE { namespace Scene {

class EE_API KeyEvent : public Event {
  public:
	KeyEvent( Node* node, const Uint32& eventNum, const Keycode& keyCode, const Uint32& chr,
			  const Uint32& mod );

	KeyEvent( const KeyEvent& event );

	~KeyEvent();

	const Keycode& getKeyCode() const;

	const String::StringBaseType& getChar() const;

	const Uint32& getMod() const;

  protected:
	Keycode mKeyCode{Keycode::KEY_UNKNOWN};
	String::StringBaseType mChar{0};
	Uint32 mMod{0};
};

class EE_API TextInputEvent : public Event {
  public:
	TextInputEvent( Node* node, const Uint32& eventNum, const Uint32& chr,
					const Uint32& timestamp );

	const String::StringBaseType& getChar() const;

	const Uint32& getTimestamp() const;

	String getText() const;

  protected:
	String::StringBaseType mChar;
	Uint32 mTimestamp;
};

}} // namespace EE::Scene

#endif
