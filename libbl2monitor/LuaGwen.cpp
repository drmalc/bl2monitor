#include "stdafx.h"

#pragma warning(disable: 4251)

#include "lua.hpp"
#include "LuaGwen.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Renderers/DirectX9.h"
#include "Gwen/Controls/CheckBox.h"

#include "luabind/luabind.hpp"
#include "CGwen.h"

namespace LuaGwen
{
	using namespace Gwen::Controls;
	using namespace luabind;

	bool Initialize(lua_State* L)
	{
		//Gwen classes
		module(L)
			[
				class_<Base>("Base")
				.enum_("constants")
				[
					value("None", Gwen::Pos::None),
					value("Left", Gwen::Pos::Left),
					value("Right", Gwen::Pos::Right),
					value("Top", Gwen::Pos::Top),
					value("Bottom", Gwen::Pos::Bottom),
					value("CenterV", Gwen::Pos::CenterV),
					value("CenterH", Gwen::Pos::CenterH),
					value("Fill", Gwen::Pos::Fill),
					value("Center", Gwen::Pos::Center)
				]
				.def("Dock", &Base::Dock)
				.def(constructor<Base*, const Gwen::String &>()),

				class_<Gwen::TextObject>("TextObject")
				.def(constructor<const char*>())
				.def("c_str", &Gwen::TextObject::c_str),

				class_<Label, Base>("Label")
				.def("SetText", (void(Label::*)(const Gwen::TextObject&, bool))&Label::SetText)
				.def("SetText", (void(Label::*)(const Gwen::TextObject&))&Label::SetText)
				.def("Dock", &Label::Dock)
				.def(constructor<Base*>()),

				class_<Button, bases<Label, Base> >("Button"),
				class_<CheckBox, bases<Button, Label, Base> >("CheckBox"),
				class_<DockBase, Base>("DockBase"),
				class_<Canvas, Base>("Canvas"),
				class_<TabControl, Base>("TabControl"),
				class_<ScrollControl, Base>("ScrollControl"),
				class_<TabTitleBar, bases<Label, Base>>("TabTitleBar"),
				class_<ResizableControl, Base>("ResizableControl"),
				class_<WindowCloseButton, bases<Button, Label, Base>>("WindowCloseButton"),
				class_<BaseScrollBar, Base>("BaseScrollBar"),
				class_<TabStrip, Base>("TabStrip"),
				class_<Layout::TableRow, Base>("TableRow"),
				class_<StatusBar, bases<Label, Base>>("StatusBar"),
				class_<TabButton, bases<Button, Label, Base>>("TabButton"),
				class_<WindowControl, bases<ResizableControl, Base>>("WindowControl"),
				class_<ListBox, bases<ScrollControl, Base>>("ListBox")

				//crash
				/*class_<VerticalScrollBar, bases<BaseScrollBar, Base>>("VerticalScrollBar"),
				class_<HorizontalScrollBar, bases<BaseScrollBar, Base>>("HorizontalScrollBar"),*/
			];
		//Accessor for the root canvas created in CGwen.cpp
		module(L)
			[
				def("baseCanvas", &CGwen::baseCanvas)
			];

		return true;
	}
}
