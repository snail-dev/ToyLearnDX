#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "Mouse.h"

#include <windows.h>
#include <memory>

namespace DirectX {

    class Mouse {
    public:
		Mouse() noexcept(false);
		Mouse(Mouse&& moveFrom) noexcept;
		Mouse& operator= (Mouse&& moveFrom) noexcept;

		Mouse(Mouse const&) = delete;
		Mouse& operator=(Mouse const&) = delete;

		virtual ~Mouse();

        enum Mode {
            MODE_ABSOLUTE = 0,
            MODE_RELATIVE,
        };

        struct State {
            bool leftButton;
            bool middleButton;
            bool rightButton;
            bool xButton1;
            bool xButton2;
            int x;
            int y;
            int scrollWheelValue;
            Mode positionMode;
        };

        class ButtonStateTracker {
        public:
            enum ButtonState {
                UP = 0,
                HELD = 1,
                RELEASED = 2,
                PRESSED = 3,
            };

            ButtonState leftButton;
            ButtonState middleButton;
            ButtonState rightButton;
            ButtonState xButton1;
            ButtonState xButton2;

            ButtonStateTracker() noexcept { Reset();}

            void __cdecl Update(const State& state);
            void __cdecl Reset() noexcept;
            State __cdecl GetLastState() const { return lastState;}

        private:
            State lastState;
        };

        State __cdecl GetState() const;
        void __cdecl ResetScrollWheelValue();
        void __cdecl SetMode(Mode mode);
        bool __cdecl IsConnected() const;
        bool __cdecl IsVisible() const;
        void __cdecl SetVisible(bool visible);
        
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) && defined(WM_USER)
        void __cdecl SetWindow(HWND window);
        static void __cdecl ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);
#endif

        static Mouse& __cdecl Get();

    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;

    };
}

#endif // _MOUSE_H_
