/**
**/

#ifndef LOVE_KEYBOARD_PPAPI_KEYBOARD_H
#define LOVE_KEYBOARD_PPAPI_KEYBOARD_H

// LOVE
#include <keyboard/Keyboard.h>
#include <common/EnumMap.h>
#include <window/ppapi/Input.h>

namespace love
{
namespace keyboard
{
namespace ppapi
{
	class Keyboard : public love::keyboard::Keyboard
	{
	public:
                typedef Keyboard::Key LoveKey;
                typedef love::window::ppapi::KeyCode PPAPIKey;

		// Implements Module.
		const char * getName() const;
		bool isDown(Key * keylist) const;
		void setKeyRepeat(int delay, int interval) const;
		int getKeyRepeatDelay() const;
		int getKeyRepeatInterval() const;

                static bool Convert(LoveKey in_key, PPAPIKey* out_key);
                static bool Convert(PPAPIKey in_key, LoveKey* out_key);

        private:
                typedef EnumMap<LoveKey, PPAPIKey, Keyboard::KEY_MAX_ENUM> KeyEnumMap;
		static KeyEnumMap::Entry keyEntries[];
		static KeyEnumMap keys;
	}; // Keyboard

} // ppapi
} // keyboard
} // love

#endif // LOVE_KEYBOARD_PPAPI_KEYBOARD_H
