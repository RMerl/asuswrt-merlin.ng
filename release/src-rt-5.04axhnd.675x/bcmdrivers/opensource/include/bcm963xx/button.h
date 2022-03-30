#ifndef __BUTTON_H__
#define __BUTTON_H__

typedef void (* buttonNotifyHook_t)(unsigned long timeInMs, void* param);

int register_button_action(const char *button_name, char *action_name, buttonNotifyHook_t hook);

#endif
