#ifndef DL_SDL3_USERINPUT_H
#define DL_SDL3_USERINPUT_H

// Helps one to find out if a button was recently pressed
// One of these structs needs to be allocated for each button tracked thus
struct ButtonTimer {
	static constexpr char ACTIVE{0x01};
	static constexpr char RECENTLYPRESSED{0x02};
    
	float recently {0.0f};
	float timeSincePressed {0.0f};
	char flags {};

	ButtonTimer() = default;
	explicit ButtonTimer(float g_recently = 0.1f)
		: recently {g_recently}
	{}

	void Press () {
		timeSincePressed = 0.0f;
		flags = ACTIVE | RECENTLYPRESSED;
	}
	// Needs to be called at the start of the update loop
	void Update(float deltaTime) {
		if ((flags & ACTIVE) && (timeSincePressed < recently)) {
			timeSincePressed += deltaTime;
		}
		else {
			flags = 0;
		}
	}
};

#endif