#include "CmPlatformWndProc.h"
#include "CmRenderWindow.h"
#include "CmApplication.h"
#include "CmInput.h"
#include "CmDebug.h"

namespace CamelotFramework
{
	bool PlatformWndProc::isShiftPressed = false;
	bool PlatformWndProc::isCtrlPressed = false;

	LRESULT CALLBACK PlatformWndProc::_win32WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_CREATE)
		{	// Store pointer to Win32Window in user data area
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams));
			return 0;
		}

		// look up window instance
		// note: it is possible to get a WM_SIZE before WM_CREATE
		RenderWindow* win = (RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (!win)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);

		switch( uMsg )
		{
		case WM_ACTIVATE:
			{
				bool active = (LOWORD(wParam) != WA_INACTIVE);
				if( active )
				{
					win->setActive(true);

					if(!win->hasFocus())
						windowFocusReceived(win);
				}
				else
				{
					if(win->hasFocus())
						windowFocusLost(win);
				}

				break;
			}
		case WM_SYSCHAR:
			// return zero to bypass defProc and signal we processed the message, unless it's an ALT-space
			if (wParam != VK_SPACE)
				return 0;
			break;
		case WM_MOVE:
			windowMovedOrResized(win);
			break;
		case WM_DISPLAYCHANGE:
			windowMovedOrResized(win);
			break;
		case WM_SIZE:
			windowMovedOrResized(win);
			break;
		case WM_SETCURSOR:
			if(isCursorHidden())
				win32HideCursor();
			else
			{
				switch (LOWORD(lParam))
				{
				case HTTOPLEFT:
					SetCursor(LoadCursor(0, IDC_SIZENWSE));
					return 0;
				case HTTOP:
					SetCursor(LoadCursor(0, IDC_SIZENS));
					return 0;
				case HTTOPRIGHT:
					SetCursor(LoadCursor(0, IDC_SIZENESW));
					return 0;
				case HTLEFT:
					SetCursor(LoadCursor(0, IDC_SIZEWE));
					return 0;
				case HTRIGHT:
					SetCursor(LoadCursor(0, IDC_SIZEWE));
					return 0;
				case HTBOTTOMLEFT:
					SetCursor(LoadCursor(0, IDC_SIZENESW));
					return 0;
				case HTBOTTOM:
					SetCursor(LoadCursor(0, IDC_SIZENS));
					return 0;
				case HTBOTTOMRIGHT:
					SetCursor(LoadCursor(0, IDC_SIZENWSE));
					return 0;
				}

				win32ShowCursor();
			}
			return true;
		case WM_GETMINMAXINFO:
			// Prevent the window from going smaller than some minimu size
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
			break;
		case WM_CLOSE:
			{
				// TODO - Only stop main loop if primary window is closed!!
				gApplication().stopMainLoop();

				return 0;
			}
		case WM_NCHITTEST:
			{
				auto iterFind = mNonClientAreas.find(win);
				if(iterFind == mNonClientAreas.end())
					break;

				POINT mousePos;
				mousePos.x = GET_X_LPARAM(lParam);
				mousePos.y = GET_Y_LPARAM(lParam); 

				ScreenToClient(hWnd, &mousePos);

				Int2 mousePosInt;
				mousePosInt.x = mousePos.x;
				mousePosInt.y = mousePos.y;

				Vector<NonClientResizeArea>::type& resizeAreasPerWindow = iterFind->second.resizeAreas;
				for(auto area : resizeAreasPerWindow)
				{
					if(area.area.contains(mousePosInt))
						return translateNonClientAreaType(area.type);
				}

				Vector<Rect>::type& moveAreasPerWindow = iterFind->second.moveAreas;
				for(auto area : moveAreasPerWindow)
				{
					if(area.contains(mousePosInt))
						return HTCAPTION;
				}

				return HTCLIENT;
			}
		case WM_MOUSELEAVE:
			{
				// Note: Right now I track only mouse leaving client area. So it's possible for the "mouse left window" callback
				// to trigger, while the mouse is still in the non-client area of the window.
				mIsTrackingMouse = false; // TrackMouseEvent ends when this message is received and needs to be re-applied

				CM_LOCK_MUTEX(mSync);

				mMouseLeftWindows.push_back(win);
			}
			break;
		case WM_LBUTTONUP:
			{
				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorButtonReleased.empty())
					onCursorButtonReleased(intMousePos, OSMouseButton::Left, btnStates);
			}
			break;
		case WM_MBUTTONUP:
			{
				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorButtonReleased.empty())
					onCursorButtonReleased(intMousePos, OSMouseButton::Middle, btnStates);
			}
			break;
		case WM_RBUTTONUP:
			{
				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorButtonReleased.empty())
					onCursorButtonReleased(intMousePos, OSMouseButton::Right, btnStates);
			}
			break;
		case WM_LBUTTONDOWN:
			{
				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorButtonPressed.empty())
					onCursorButtonPressed(intMousePos, OSMouseButton::Left, btnStates);
			}
			break;
		case WM_MBUTTONDOWN:
			{
				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorButtonPressed.empty())
					onCursorButtonPressed(intMousePos, OSMouseButton::Middle, btnStates);
			}
			break;
		case WM_RBUTTONDOWN:
			{
				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorButtonPressed.empty())
					onCursorButtonPressed(intMousePos, OSMouseButton::Right, btnStates);
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;

				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorDoubleClick.empty())
					onCursorDoubleClick(intMousePos, btnStates);
			}
			break;
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			{
				// Set up tracking so we get notified when mouse leaves the window
				if(!mIsTrackingMouse)
				{
					TRACKMOUSEEVENT tme = { sizeof(tme) };
					tme.dwFlags = TME_LEAVE;

					tme.hwndTrack = hWnd;
					TrackMouseEvent(&tme);

					mIsTrackingMouse = true;
				}

				if(uMsg == WM_NCMOUSEMOVE)
					return true;

				Int2 intMousePos;
				OSPositionalInputButtonStates btnStates;
				
				getMouseData(hWnd, wParam, lParam, intMousePos, btnStates);

				if(!onCursorMoved.empty())
					onCursorMoved(intMousePos, btnStates);

				return true;
			}
		case WM_MOUSEWHEEL:
			{
				INT16 wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

				float wheelDeltaFlt = wheelDelta / (float)WHEEL_DELTA;
				if(!onMouseWheelScrolled.empty())
					onMouseWheelScrolled(wheelDeltaFlt);

				return true;
			}
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			{
				if(wParam == VK_SHIFT)
				{
					isShiftPressed = true;
					break;
				}

				if(wParam == VK_CONTROL)
				{
					isCtrlPressed = true;
					break;
				}

				InputCommandType command = InputCommandType::Backspace;
				if(getCommand((unsigned int)wParam, command))
				{
					if(!onInputCommand.empty())
						onInputCommand(command);

					return 0;
				}

				break;
			}
		case WM_SYSKEYUP:
		case WM_KEYUP:
			{
				if(wParam == VK_SHIFT)
				{
					isShiftPressed = false;
				}

				if(wParam == VK_CONTROL)
				{
					isCtrlPressed = false;
				}

				break;
			}
		case WM_CHAR:
			{
				// TODO - Not handling IME input

				switch (wParam) 
				{ 
				case VK_BACK:
				case 0x0A:  // linefeed 
				case 0x0D:  // carriage return 
				case VK_ESCAPE:
				case VK_TAB: 
					break; 
				default:    // displayable character 
					{
						UINT8 scanCode = (lParam >> 16) & 0xFF;

						BYTE keyState[256];
						HKL layout = GetKeyboardLayout(0);
						if(GetKeyboardState(keyState) == 0)
							return 0;

						unsigned int vk = MapVirtualKeyEx(scanCode, MAPVK_VSC_TO_VK_EX, layout);
						if(vk == 0)
							return 0;

						InputCommandType command = InputCommandType::Backspace;
						if(getCommand(vk, command)) // We ignore character combinations that are special commands
							return 0;

						UINT32 finalChar = (UINT32)wParam;

						if(!onCharInput.empty())
							onCharInput(finalChar);

						return 0;
					}
				} 

				break;
			}
		case WM_CM_SETCAPTURE:
			SetCapture(hWnd);
			break;
		case WM_CM_RELEASECAPTURE:
			ReleaseCapture();
			break;
		case WM_CAPTURECHANGED:
			if(!onMouseCaptureChanged.empty())
				onMouseCaptureChanged();
			break;
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	LRESULT PlatformWndProc::translateNonClientAreaType(NonClientAreaBorderType type)
	{
		LRESULT dir = HTCLIENT;
		switch(type)
		{
		case NonClientAreaBorderType::Left:
			dir = HTLEFT;
			break;
		case NonClientAreaBorderType::TopLeft:
			dir = HTTOPLEFT;
			break;
		case NonClientAreaBorderType::Top:
			dir = HTTOP;
			break;
		case NonClientAreaBorderType::TopRight:
			dir = HTTOPRIGHT;
			break;
		case NonClientAreaBorderType::Right:
			dir = HTRIGHT;
			break;
		case NonClientAreaBorderType::BottomRight:
			dir = HTBOTTOMRIGHT;
			break;
		case NonClientAreaBorderType::Bottom:
			dir = HTBOTTOM;
			break;
		case NonClientAreaBorderType::BottomLeft:
			dir = HTBOTTOMLEFT;
			break;
		}

		return dir;
	}

	void PlatformWndProc::getMouseData(HWND hWnd, WPARAM wParam, LPARAM lParam, Int2& mousePos, OSPositionalInputButtonStates& btnStates)
	{
		POINT clientPoint;

		clientPoint.x = GET_X_LPARAM(lParam);
		clientPoint.y = GET_Y_LPARAM(lParam); 

		ClientToScreen(hWnd, &clientPoint);

		mousePos.x = clientPoint.x;
		mousePos.y = clientPoint.y;

		btnStates.mouseButtons[0] = (wParam & MK_LBUTTON) != 0;
		btnStates.mouseButtons[1] = (wParam & MK_MBUTTON) != 0;
		btnStates.mouseButtons[2] = (wParam & MK_RBUTTON) != 0;
		btnStates.shift = (wParam & MK_SHIFT) != 0;
		btnStates.ctrl = (wParam & MK_CONTROL) != 0;
	}

	bool PlatformWndProc::getCommand(unsigned int virtualKeyCode, InputCommandType& command)
	{
		switch (virtualKeyCode) 
		{ 
		case VK_LEFT:
			command = isShiftPressed ? InputCommandType::SelectLeft : InputCommandType::CursorMoveLeft;
			return true;
		case VK_RIGHT:
			command = isShiftPressed ? InputCommandType::SelectRight : InputCommandType::CursorMoveRight;
			return true;
		case VK_UP:
			command = isShiftPressed ? InputCommandType::SelectUp : InputCommandType::CursorMoveUp;
			return true;
		case VK_DOWN:
			command = isShiftPressed ? InputCommandType::SelectDown : InputCommandType::CursorMoveDown;
			return true;
		case VK_ESCAPE:
			command = InputCommandType::Escape;
			return true;
		case VK_RETURN:
			command = InputCommandType::Return;
			return true;
		case VK_BACK:
			command = InputCommandType::Backspace;
			return true;
		case VK_DELETE:
			command = InputCommandType::Delete;
			return true;
		case VK_TAB:
			command = InputCommandType::Tab;
			return true;
		case 0x41: // A
			if(isCtrlPressed)
			{
				command = InputCommandType::SelectAll;
				return true;
			}
		case 0x43: // C
			if(isCtrlPressed)
			{
				command = InputCommandType::Copy;
				return true;
			}
		case 0x56: // V
			if(isCtrlPressed)
			{
				command = InputCommandType::Paste;
				return true;
			}
		case 0x58: // X
			if(isCtrlPressed)
			{
				command = InputCommandType::Cut;
				return true;
			}
		case 0x5A: // Z
			if(isCtrlPressed)
			{
				command = InputCommandType::Undo;
				return true;
			}
		case 0x59: // Y
			if(isCtrlPressed)
			{
				command = InputCommandType::Redo;
				return true;
			}
		}

		return false;
	}
}