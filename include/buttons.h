#pragma once
#include "config.h" // подключаем эффекты, там же их настройки
#ifdef ESP_USE_BUTTON
#include "misc.h"
#include <Ticker.h>
#include "../../include/LList.h"

typedef enum _button_action {
	BA_NONE,
	BA_BRIGHT,
	BA_SPEED,
	BA_SCALE,
	BA_ON,
	BA_OFF,
	BA_DEMO,
	BA_AUX_TOGLE,
	BA_OTA,
	BA_EFF_NEXT,
	BA_EFF_PREV,
	BA_SEND_TIME,
	BA_SEND_IP,
	BA_WHITE_HI,
	BA_WHITE_LO,
	BA_WIFI_REC,
	BA_EFFECT,
	BA_END			// признак конца enum
} BA;

const char *btn_get_desc(BA action);

class Button{
	typedef union _bflags {
		uint8_t mask;
		struct {
			uint8_t on:1;
			uint8_t hold:1;
			uint8_t click:3;
			uint8_t onetime:2; // признак однократного срабатывания при удержании и старший бит - то что срабатывание уже было
			uint8_t direction:1; // направление изменения
		};
	} btnflags;

	friend bool operator== (const Button &f1, const Button &f2) { return ((f1.flags.mask&0x1F) == (f2.flags.mask&0x1F)); }
	friend bool operator!= (const Button &f1, const Button &f2) { return ((f1.flags.mask&0x1F) != (f2.flags.mask&0x1F)); }

	public:
		Button(bool on, bool hold, uint8_t click, bool onetime, BA act = BA_NONE, const String& _param=String()) { flags.direction = false; flags.mask = 0; flags.on = on; flags.hold = hold; flags.click = click; flags.onetime=onetime; action = act; param=_param; }
		Button(uint8_t mask, BA act = BA_NONE, const String& _param=String()) { flags.direction = false; flags.mask = mask; action = act; param=_param; }

		bool activate(btnflags& flg, bool reverse);
		String getName();
		const String& getParam() {return param;}
		void setParam(const String&_param) {param=_param;}

		BA action;
		btnflags flags;
	private:
		String param;
};

class Buttons {
	private:
#pragma pack(push,1)
	union {
		struct {
			bool buttonEnabled:1; // кнопка обрабатывается если true
			bool holding:1; // кнопка удерживается
			bool holded:1; // кнопка удерживалась (touch.isHolded() можно проверить только однократно!!!)
			bool pinTransition:1;  // ловим "нажатие" кнопки
		};
		uint8_t btnflags = 0; // очистим флаги
	};
 #pragma pack(pop)
	uint8_t pin; // пин
	uint8_t pullmode; // подтяжка
	uint8_t state; // тип (нормально открытый/закрытый)

	byte clicks = 0;
	Ticker _buttonTicker; // планировщик кнопки
	LList<Button*> buttons;
	timerMinim holdtm; // таймаут удержания кнопки в мс

	void resetStates() {holding=false; holded=false; touch.resetStates();}

	public:
	bool getpinTransition() { return pinTransition; }
	void setpinTransition(bool val) { pinTransition = val; }
	int getPressTransitionType() {return pullmode==LOW_PULL ? RISING : FALLING;}
	int getReleaseTransitionType() {return pullmode!=LOW_PULL ? RISING : FALLING;}
	void setButtonOn(bool flag) { buttonEnabled = flag; }
	bool isButtonOn() { return buttonEnabled; }

	inline Button* operator[](int i) { return buttons[i]; }

	int size(){ return buttons.size(); }
	void add(Button *btn) { buttons.add(btn); }
	void remove(int i) { buttons.remove(i); }
	void clear();

	/*
	 * крючёк для обработки нажатия кнопки по прерываниям
	 * вызов метода, готоврит о том что состояние пина изменилось, нужно его перечитать
	 * @param bool state - true, кнопку "нажали", false - "отпустили"
	 */
	void buttonPress(bool state);
	Buttons(uint8_t _pin=BTN_PIN, uint8_t _pullmode=PULL_MODE, uint8_t _state=NORM_OPEN);

	int loadConfig(const char *cfg = nullptr);
	void saveConfig(const char *cfg = nullptr);

	GButton touch;
	void buttonTick(); // обработчик кнопки
	timerMinim tmChangeDirectionTimer;     // таймаут смены направления увеличение-уменьшение при удержании кнопки
};
#endif
