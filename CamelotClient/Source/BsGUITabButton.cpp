#include "BsGUITabButton.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUILayoutOptions.h"
#include "BsGUIMouseEvent.h"
#include "BsGUITabbedTitleBar.h"

using namespace CamelotFramework;
using namespace BansheeEngine;

namespace BansheeEditor
{
	const CM::UINT32 GUITabButton::DRAG_MIN_DISTANCE = 3;

	const String& GUITabButton::getGUITypeName()
	{
		static String name = "TabButton";
		return name;
	}

	GUITabButton::GUITabButton(GUIWidget& parent, const GUIElementStyle* style, GUITabbedTitleBar* titleBar, CM::UINT32 index, const GUIContent& content, const GUILayoutOptions& layoutOptions)
		:GUIToggle(parent, style, content, nullptr, layoutOptions), mTitleBar(titleBar), mIndex(index)
	{

	}

	GUITabButton* GUITabButton::create(GUIWidget& parent, GUITabbedTitleBar* titleBar, CM::UINT32 index, const HString& text, const GUIElementStyle* style)
	{
		if(style == nullptr)
		{
			const GUISkin& skin = parent.getSkin();
			style = skin.getStyle(getGUITypeName());
		}

		return new (cm_alloc<GUITabButton, PoolAlloc>()) GUITabButton(parent, style, titleBar, index, GUIContent(text), getDefaultLayoutOptions(style));
	}

	GUITabButton* GUITabButton::create(GUIWidget& parent, const GUILayoutOptions& layoutOptions, GUITabbedTitleBar* titleBar, CM::UINT32 index, const HString& text, const GUIElementStyle* style)
	{
		if(style == nullptr)
		{
			const GUISkin& skin = parent.getSkin();
			style = skin.getStyle(getGUITypeName());
		}

		return new (cm_alloc<GUITabButton, PoolAlloc>()) GUITabButton(parent, style, titleBar, index, GUIContent(text), layoutOptions);
	}

	GUITabButton* GUITabButton::create(GUIWidget& parent, GUITabbedTitleBar* titleBar, CM::UINT32 index, const GUIContent& content, const GUIElementStyle* style)
	{
		if(style == nullptr)
		{
			const GUISkin& skin = parent.getSkin();
			style = skin.getStyle(getGUITypeName());
		}

		return new (cm_alloc<GUITabButton, PoolAlloc>()) GUITabButton(parent, style, titleBar, index, content, getDefaultLayoutOptions(style));
	}

	GUITabButton* GUITabButton::create(GUIWidget& parent, const GUILayoutOptions& layoutOptions, GUITabbedTitleBar* titleBar, CM::UINT32 index, const GUIContent& content, const GUIElementStyle* style)
	{
		if(style == nullptr)
		{
			const GUISkin& skin = parent.getSkin();
			style = skin.getStyle(getGUITypeName());
		}

		return new (cm_alloc<GUITabButton, PoolAlloc>()) GUITabButton(parent, style, titleBar, index, content, layoutOptions);
	}

	bool GUITabButton::mouseEvent(const GUIMouseEvent& ev)
	{
		bool eventProcessed = GUIToggle::mouseEvent(ev);

		if(ev.getType() == GUIMouseEventType::MouseDragStart)
		{
			mDragStartPosition = ev.getPosition();

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDrag)
		{
			UINT32 dist = mDragStartPosition.manhattanDist(ev.getPosition());

			if(dist > DRAG_MIN_DISTANCE)
			{
				if(!onDragged.empty())
					onDragged(mIndex);
			}
		}

		return eventProcessed;
	}
}