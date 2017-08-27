#define ADTBW_TIMER 3   //sec
#define ADTBW_FREEZE_TIME 5 //sec
#define ADTBW_HITCOUNT 2

enum {
	ADTBW_STATE_ENTER = 0,
	ADTBW_STATE_LEAVE,
	ADTBW_STATE_MAX
};

extern int adtbw_enable();
extern int adtbw_config();
extern int adtbw_enter();
extern int adtbw_leave();
extern int adtbw_active();
extern int adtbw_restore();
