#ifndef EE_UIHELPER_HPP
#define EE_UIHELPER_HPP

#include "base.hpp"

namespace EE { namespace UI {

Uint32 EE_API HAlignGet( Uint32 Flags );

Uint32 EE_API VAlignGet( Uint32 Flags );

enum UI_CONTROL_FLAGS_POS {
	UI_CTRL_FLAG_CLOSE_POS							= 0,
	UI_CTRL_FLAG_CLOSE_FO_POS						= 1,
	UI_CTRL_FLAG_ANIM_POS							= 2,
	UI_CTRL_FLAG_MOUSEOVER_POS						= 3,
	UI_CTRL_FLAG_HAS_FOCUS_POS						= 4,
	UI_CTRL_FLAG_SELECTED_POS						= 5,
	UI_CTRL_FLAG_DISABLE_CHECK_CLOSE_CHILDS_POS		= 6,
	UI_CTRL_FLAG_DISABLE_FADE_OUT_POS				= 7,
	UI_CTRL_FLAG_MOUSEOVER_ME_OR_CHILD_POS			= 8,
	UI_CTRL_FLAG_DRAGGING_POS						= 9,
	UI_CTRL_FLAG_FREE_USE_POS						= 31
};

enum UI_CONTROL_FLAGS_VALUES {
	UI_CTRL_FLAG_CLOSE								= (1<<UI_CTRL_FLAG_CLOSE_POS),
	UI_CTRL_FLAG_CLOSE_FO							= (1<<UI_CTRL_FLAG_CLOSE_FO_POS),
	UI_CTRL_FLAG_ANIM								= (1<<UI_CTRL_FLAG_ANIM_POS),
	UI_CTRL_FLAG_MOUSEOVER							= (1<<UI_CTRL_FLAG_MOUSEOVER_POS),
	UI_CTRL_FLAG_HAS_FOCUS							= (1<<UI_CTRL_FLAG_HAS_FOCUS_POS),
	UI_CTRL_FLAG_SELECTED							= (1<<UI_CTRL_FLAG_SELECTED_POS),
	UI_CTRL_FLAG_DISABLE_CHECK_CLOSE_CHILDS			= (1<<UI_CTRL_FLAG_DISABLE_CHECK_CLOSE_CHILDS_POS),
	UI_CTRL_FLAG_DISABLE_FADE_OUT					= (1<<UI_CTRL_FLAG_DISABLE_FADE_OUT_POS),
	UI_CTRL_FLAG_MOUSEOVER_ME_OR_CHILD				= (1<<UI_CTRL_FLAG_MOUSEOVER_ME_OR_CHILD_POS),
	UI_CTRL_FLAG_DRAGGING							= (1<<UI_CTRL_FLAG_DRAGGING_POS),
	UI_CTRL_FLAG_FREE_USE							= (1<<UI_CTRL_FLAG_FREE_USE_POS)
};

#define UI_HALIGN_LEFT		FONT_DRAW_LEFT
#define UI_HALIGN_MASK		FONT_DRAW_HALIGN_MASK
#define UI_VALIGN_TOP		FONT_DRAW_TOP
#define UI_VALIGN_MASK		FONT_DRAW_VALIGN_MASK

enum UI_FLAGS {
	UI_HALIGN_RIGHT					= FONT_DRAW_RIGHT,
	UI_HALIGN_CENTER				= FONT_DRAW_CENTER,
	UI_VALIGN_BOTTOM				= FONT_DRAW_BOTTOM,
	UI_VALIGN_CENTER				= FONT_DRAW_MIDDLE,
	UI_DRAW_SHADOW					= FONT_DRAW_SHADOW,
	UI_AUTO_SIZE					= (1 << 6),
	UI_INGORE_FOCUS					= (1 << 7),
	UI_FILL_BACKGROUND				= (1 << 8),
	UI_BORDER						= (1 << 9),
	UI_TAB_STOP						= (1 << 10),
	UI_FIT_TO_CONTROL				= (1 << 11),
	UI_CLIP_ENABLE					= (1 << 12),
	UI_AUTO_SHRINK_TEXT				= (1 << 13),
	UI_MULTI_SELECT					= (1 << 14),
	UI_AUTO_PADDING					= (1 << 15),
	UI_DRAG_ENABLE					= (1 << 16),
	UI_REPORT_SIZE_CHANGE_TO_CHILDS = (1 << 17),
	UI_ANCHOR_TOP					= (1 << 18),
	UI_ANCHOR_BOTTOM				= (1 << 19),
	UI_ANCHOR_LEFT					= (1 << 20),
	UI_ANCHOR_RIGHT					= (1 << 21)
};

enum UI_CONTROL_TYPES {
	UI_TYPE_CONTROL_DRAGABLE	= 0,
	UI_TYPE_CONTROL_ANIM		= 1,
	UI_TYPE_CONTROL_COMPLEX		= 2,
	UI_TYPE_GFX					= 3,
	UI_TYPE_TEXTBOX				= 4,
	UI_TYPE_TEXTINPUT			= 5,
	UI_TYPE_PUSHBUTTON			= 6,
	UI_TYPE_CHECKBOX			= 7,
	UI_TYPE_RADIOBUTTON			= 8,
	UI_TYPE_SLIDER				= 9,
	UI_TYPE_SPINBOX				= 10,
	UI_TYPE_SCROLLBAR			= 11,
	UI_TYPE_PROGRESSBAR			= 12,
	UI_TYPE_LISTBOX				= 13,
	UI_TYPE_DROPDOWNLIST		= 14,
	UI_TYPE_SEPARATOR			= 15,
	UI_TYPE_COMBOBOX			= 16,
	UI_TYPE_MENU				= 17,
	UI_TYPE_MENUITEM			= 18,
	UI_TYPE_MENUCHECKBOX		= 19,
	UI_TYPE_MENUSUBMENU			= 20,
	UI_TYPE_SPRITE				= 21,
	UI_TYPE_TEXTEDIT			= 22,
	UI_TYPE_TOOLTIP				= 23,
	UI_TYPE_GENERICGRID			= 24,
	UI_TYPE_WINDOW				= 25,
	UI_TYPE_WINMENU				= 26,
	UI_TYPE_SELECTBUTTON		= 27,
	UI_TYPE_POPUPMENU			= 28
};

#define UI_TYPE_GET(X) 				( 1 << (X) )

enum UI_SCROLLBAR_MODE {
	UI_SCROLLBAR_AUTO,
	UI_SCROLLBAR_ALWAYS_ON,
	UI_SCROLLBAR_ALWAYS_OFF
};

}}

#endif
