// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Broadcom Ltd.
 */

#include <common.h>
#include <linux/compat.h>
#include <dm.h>
#include <asm/gpio.h>
#include <dt-bindings/gpio/gpio.h>
#include "bcmbca_button.h"

DECLARE_GLOBAL_DATA_PTR;

#define BCA_GPIO_ACTIVE_HIGH    (0x0 << 1)
#define BCA_GPIO_ACTIVE_LOW     (0x1 << 1)

#define CONSUMER_NAME           "ext_irq"
#define BTN_EVENT_NUM           3

#define PRINT_ACT_NAME          "print"
#define PRINT_ACT_NAME_UBOOT    "print_uboot"

#define PRESS_EVENT             "press"
#define HOLD_EVENT              "hold"
#define RELEASE_EVENT           "release"

#define MAX_BTN_HOOKS_PER_TRIG  5
#define MAX_BTN_HOOKS_PER_BTN   (MAX_BTN_HOOKS_PER_TRIG * 3)

typedef enum {
	PB_BUTTON_0,
	PB_BUTTON_1,
	PB_BUTTON_2,
	PB_BUTTON_MAX
} PB_BUTTON_ID;

#define BTN_EV_PRESSED      0x1
#define BTN_EV_HOLD         0x2
#define BTN_EV_RELEASED     0x4
#define BTN_POLLFREQ        100 /* in ms */

// Main button structure:
typedef struct _BtnInfo {
	PB_BUTTON_ID btnId;
	struct gpio_desc gd;
	int active;
	u32 lastPressTime;
	u32 lastHoldTime;
	u32 lastReleaseTime;
	spinlock_t lock;
	u32 events;
	u32 act_registered;
	u32 longest_holdevt;
	void *poll;
	char name[32];
	bool (*isDown)(struct _BtnInfo *btnInfo);
} BtnInfo;

typedef struct {
	buttonNotifyHook_t hook;
	unsigned long timeout; // in ms;
	void *param;
	int done;
} pushButtonHookInfo_t;

typedef int (*registerBtnHook)(PB_BUTTON_ID btn,
				buttonNotifyHook_t hook,
				unsigned long timeInMs,
				void *param);

struct button_action {
	char *action_name;
	buttonNotifyHook_t btn_hook;
};

struct button_events {
	char *btn_event;
	registerBtnHook reg_hook;
};

static pushButtonHookInfo_t btnPressedInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
static pushButtonHookInfo_t btnHeldInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
static pushButtonHookInfo_t btnReleasedInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
static int btnPressTime[PB_BUTTON_MAX] = {};

static BtnInfo btnInfo[PB_BUTTON_MAX] = {};

static void reset_hooks_array(PB_BUTTON_ID btn)
{
	int j;

	for (j = 0; j < MAX_BTN_HOOKS_PER_TRIG; j++) {
		btnPressedInfo[btn][j].done = 0;
		btnHeldInfo[btn][j].done = 0;
		btnReleasedInfo[btn][j].done = 0;
	}
}

static BtnInfo *find_btn_info(const char *name)
{
	int i;

	for (i = 0 ; i < PB_BUTTON_MAX; i++) {
		if (!strcmp(name, btnInfo[i].name))
			return &btnInfo[i];
	}
	return NULL;
}

static BtnInfo *find_btn_info_by_id(PB_BUTTON_ID btn)
{
	int i;

	for (i = 0; i < PB_BUTTON_MAX; i++) {
		if (btnInfo[i].btnId == btn)
			return &btnInfo[i];
	}
	return NULL;
}

static void do_button(PB_BUTTON_ID btn,
				unsigned long currentTime,
				pushButtonHookInfo_t (*actions_arr)[MAX_BTN_HOOKS_PER_TRIG])
{
	unsigned long timeInMs;
	pushButtonHookInfo_t *pInfo;
	int callIdx;
	int callInfoIdx = 0;
	pushButtonHookInfo_t callInfo[MAX_BTN_HOOKS_PER_TRIG] = {};
	unsigned long flags;
	int idx;

	if (unlikely(btn >= PB_BUTTON_MAX)) {
		printk(KERN_ERR "%s: unrecognized button id (%d)\n",
			__func__, btn);
		return;
	}

	timeInMs = currentTime - btnPressTime[btn];

	spin_lock_irqsave(&lock, flags);
	for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
		pInfo = &actions_arr[btn][idx];
		if (pInfo->done)
			continue;

		if (pInfo->hook) {
			if (pInfo->timeout) {
				if (pInfo->timeout > timeInMs)
					continue;
			}
			callInfo[callInfoIdx] = *pInfo;
			pInfo->done = 1;
			callInfoIdx++;
		}
	}
	spin_unlock_irqrestore(&lock, flags);

	for (callIdx = 0; callIdx < callInfoIdx; callIdx++)
		callInfo[callIdx].hook(timeInMs, callInfo[callIdx].param);

}

void do_button_release(PB_BUTTON_ID btn, unsigned long currentTime)
{
	int idx;
	unsigned long timeInMs;
	unsigned long flags;
	int callIdx;
	int callInfoIdx = 0;
	pushButtonHookInfo_t callInfo[MAX_BTN_HOOKS_PER_TRIG] = {};

	if (unlikely(btn >= PB_BUTTON_MAX)) {
		printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
		return;
	}

	timeInMs = currentTime - btnPressTime[btn];

	spin_lock_irqsave(&lock, flags);
	for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
		pushButtonHookInfo_t *pNewInfo = &btnReleasedInfo[btn][idx];

		if (!pNewInfo->hook || pNewInfo->timeout > timeInMs)
			continue;
		if (callInfoIdx == 0 || pNewInfo->timeout == callInfo[0].timeout) {
			callInfo[callInfoIdx] = *pNewInfo;
			callInfoIdx++;
		} else if (pNewInfo->timeout > callInfo[0].timeout) {
			callInfo[0] = *pNewInfo;
			callInfoIdx = 1;
		}
	}
	spin_unlock_irqrestore(&lock, flags);

	for (callIdx = 0; callIdx < callInfoIdx; callIdx++)
		callInfo[callIdx].hook(timeInMs, callInfo[callIdx].param);
}

static int insert_to_array(buttonNotifyHook_t hook,
			unsigned long timeInMs,
			void *param,
			pushButtonHookInfo_t *pHookArray)
{
	int idx;
	pushButtonHookInfo_t *pInfo;

	if (pHookArray[MAX_BTN_HOOKS_PER_TRIG - 1].hook) {
		printk(KERN_ERR "%s: to many entries\n", __func__);
		return -1;
	}
	for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
		pInfo = &pHookArray[idx];
		if (!pInfo->hook) {  // inserting at end
			pInfo->hook = hook;
			pInfo->timeout = timeInMs;
			pInfo->param = param;
			return idx;
		}
	}
	return -1;
}

static int register_button_press_notify_hook(PB_BUTTON_ID btn,
				buttonNotifyHook_t hook,
				unsigned long timeInMs,
				void *param)
{
	unsigned long flags;
	int idx;

	if (unlikely(btn >= PB_BUTTON_MAX)) {
		printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
		return -1;
	}
	if (unlikely(!hook)) {
		printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
		return -1;
	}

	spin_lock_irqsave(&lock, flags);
	idx = insert_to_array(hook, timeInMs, param, btnPressedInfo[btn]);
	spin_unlock_irqrestore(&lock, flags);
	if (unlikely(idx < 0)) {
		printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n",
			__func__, hook);
		return -1;
	}

	return 0;
}

static int register_button_hold_notify_hook(PB_BUTTON_ID btn,
				buttonNotifyHook_t hook,
				unsigned long timeInMs,
				void *param)
{
	int idx;
	unsigned long flags;
	BtnInfo *button = NULL;
  
	if (unlikely(btn >= PB_BUTTON_MAX)) {
		printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
		return -1;
	}
	if (unlikely(!hook)) {
		printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
		return -1;
	}
	button = find_btn_info_by_id(btn);
	if (unlikely(!button)) {
		printk(KERN_ERR "%s: cannot find the button info for id %d\n", __func__, btn);
		return -1;
	}

	spin_lock_irqsave(&lock, flags);
	idx = insert_to_array(hook, timeInMs, param, btnHeldInfo[btn]);
	if (timeInMs > button->longest_holdevt)
		button->longest_holdevt = timeInMs;
	spin_unlock_irqrestore(&lock, flags);
	if (unlikely(idx < 0)) {
		printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
		return -1;
	}

	return 0;
}

static int register_button_release_notify_hook(PB_BUTTON_ID btn,
				buttonNotifyHook_t hook,
				unsigned long timeInMs,
				void *param)
{
	int idx;
	unsigned long flags;
	BtnInfo *button = NULL;

	if (unlikely(btn >= PB_BUTTON_MAX)) {
		printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
		return -1;
	}
	if (unlikely(!hook)) {
		printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
		return -1;
	}
	button = find_btn_info_by_id(btn);
	if (unlikely(!button)) {
		printk(KERN_ERR "%s: cannot find the button info for id %d\n", __func__, btn);
		return -1;
	}
	
	spin_lock_irqsave(&lock, flags);
	idx = insert_to_array(hook, timeInMs, param, btnReleasedInfo[btn]);
	if (timeInMs > button->longest_holdevt)
		button->longest_holdevt = timeInMs;
	spin_unlock_irqrestore(&lock, flags);
	if (unlikely(idx < 0)) {
		printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
		return -1;
	}

	return 0;
}

/***************************************************************************/
// BP_BTN_ACTION_PRINT
static void btn_hook_print(unsigned long timeInMs, void *param)
{
	printk("%s\n", (char *)param);
}

/***************************************************************************
 * Function Name: btn_do_press
 * Description  : This is called when a press has been detected.
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btn_do_press(BtnInfo *btn, unsigned long currentTime)
{
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);

	reset_hooks_array(btn->btnId);
	btnPressTime[btn->btnId] = currentTime;
	btn->events &= ~BTN_EV_PRESSED;

	spin_unlock_irqrestore(&lock, flags);

	do_button(btn->btnId, currentTime, btnPressedInfo);
}

/***************************************************************************
 * Function Name: btn_do_release
 * Description  : This is called when a release has been detected
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btn_do_release(BtnInfo *btn, unsigned long currentTime)
{
	btn->events &= ~(BTN_EV_RELEASED | BTN_EV_HOLD);
	do_button_release(btn->btnId, currentTime);
}

/***************************************************************************
 * Function Name: btn_do_hold
 * Description  : This is called when a button hold is detected
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btn_do_hold(BtnInfo *btn, unsigned long currentTime)
{
	btn->events &= ~BTN_EV_HOLD;
	do_button(btn->btnId, currentTime, btnHeldInfo);
}

static int __btn_poll(BtnInfo *btn)
{
	unsigned long currentTime = get_timer(0);
	unsigned long flags;

	spin_lock_irqsave(&btn->lock, flags);
	if (btn->active) {
		if (btn->isDown(btn)) {
			btn->lastHoldTime = currentTime;
			btn->events |= BTN_EV_HOLD;
			btn_do_hold(btn, btn->lastHoldTime);
		} else {
			btn->lastReleaseTime = currentTime;
			btn->active = 0;
			btn->events |= BTN_EV_RELEASED;
			btn_do_release(btn, btn->lastReleaseTime);
		}
	} else {
		if (btn->isDown(btn)) {
			btn->active = 1;
			btn->lastPressTime = currentTime;
			btn->events |= BTN_EV_PRESSED;
			btn_do_press(btn, btn->lastPressTime);
		}
	}

	spin_unlock_irqrestore(&btn->lock, flags);

	return btn->active;
}

/***************************************************************************
 * Function Name: btn_poll
 * Description  : This is the polling function. It checks each button
		  and act on if event timeout occurs. It should be called by
		  u-boot periodic function such as cli_loop
 * Return       : positive if any button is active
 ***************************************************************************/
int btn_poll(void)
{
	BtnInfo *btn;
	int i, rc = 0;

	for (i = PB_BUTTON_0; i < PB_BUTTON_MAX; i++) {
		btn = &btnInfo[i];
		if (btn->poll && btn->act_registered)
			rc |= __btn_poll(btn);
	}

	return rc;
}

/***************************************************************************
 * Function Name: btn_poll_block
 * Description  : This function keep polling the button until all button is
		  released. It can be called during the boot to any special
		  button event before entering the cli
 ***************************************************************************/
void btn_poll_block(void)
{
	int rc = 1;

	while (rc)
		rc = btn_poll();
}

/***************************************************************************
 * Function Name: btn_is_gpio_btn_down
 * Description  : This a the check to see if a gpio-based button is down
		  based on the gpio level
 * Parameters   : arg: a pointer to a BtnInfo structure
 * Returns      : 1 if the button is down
 ***************************************************************************/
static bool btn_is_gpio_btn_down(BtnInfo *btn)
{
	return dm_gpio_get_value(&btn->gd);
}

static struct button_events btn_events[BTN_EVENT_NUM] = {
	{PRESS_EVENT,   register_button_press_notify_hook},
	{HOLD_EVENT,    register_button_hold_notify_hook},
	{RELEASE_EVENT, register_button_release_notify_hook}
};

static registerBtnHook get_register_hook_fnc(const char *event_name)
{
	int i;

	for (i = 0; i < BTN_EVENT_NUM; i++) {
		if (!strcmp(event_name, btn_events[i].btn_event))
			return btn_events[i].reg_hook;
	}
	return NULL;
}

static int read_button_event_action_params(ofnode btn_event_np, const char *action_name,
	u32 *ptimeout, void** pparam)
{
	const char *print_string = NULL, *cmd_string = NULL;
	char *action_name_cmd;
	
	if (!strcmp(action_name, PRINT_ACT_NAME)
		|| !strcmp(action_name, PRINT_ACT_NAME_UBOOT)) {
			print_string = ofnode_read_string(btn_event_np, action_name);
			if (print_string == NULL)
				return -1;
			*pparam = strndup(print_string, strlen(print_string));
	} else {
		if (ofnode_read_u32(btn_event_np, action_name, ptimeout))
			return -1;
		*ptimeout *= 1000;

		/* check if <action_name>_cmd command string is defined */
		action_name_cmd = malloc(sizeof(action_name) + 8);
		sprintf(action_name_cmd, "%s_cmd", action_name);
		cmd_string = ofnode_read_string(btn_event_np, action_name_cmd);
		if (cmd_string)
			*pparam = strndup(cmd_string, strlen(cmd_string));
		free(action_name_cmd);
	}

	return 0;
}

static int register_button_event_action(const char *button_name, ofnode event_np,
	buttonNotifyHook_t hook, u32 timeout, void* param)
{
	registerBtnHook btn_hook_register = NULL;
	BtnInfo *btn_info = NULL;

	btn_hook_register =
		get_register_hook_fnc(ofnode_get_name(event_np));
	if (!btn_hook_register)
		return -1;
	
	btn_info = find_btn_info(button_name);
	if (!btn_info)
		return -1;
	
	if (btn_hook_register(btn_info->btnId, hook, timeout, param))
		return -1;
	else 
		btn_info->act_registered =1;

	return 0;
}
  

int register_button_action(const char *button_name, const char *action_name,
	buttonNotifyHook_t hook)
{
	ofnode btn_np, btn_event_np;
	u32 timeout = 0;
	void *param = NULL;
	int ret = -1;	

	btn_np = ofnode_by_compatible(ofnode_null(), "brcm,buttons");
	if (!ofnode_valid(btn_np))
		return -ENODEV;

	btn_np = ofnode_find_subnode(btn_np, button_name);
	if (!ofnode_valid(btn_np))
		return -ENODEV;

	ofnode_for_each_subnode(btn_event_np, btn_np) {
		if (read_button_event_action_params(btn_event_np, action_name,
			&timeout, &param))
			continue;

		if (ret = register_button_event_action(button_name, btn_event_np, hook,
				timeout, param))
			return -ENOENT;
	}

	return ret;
}

int register_button_action_for_event(const char *button_name, const char* event_name,
	const char *action_name, buttonNotifyHook_t hook)
{
	ofnode btn_np, btn_event_np;
	u32 timeout = 0;
	void *param = NULL;
	const void *fdt = gd->fdt_blob;
	int node_offset, prop_offset;
	const void *value;
	const char *propname;	
	int len, num_action = 0;

	btn_np = ofnode_by_compatible(ofnode_null(), "brcm,buttons");
	if (!ofnode_valid(btn_np))
		return -ENODEV;

	btn_np = ofnode_find_subnode(btn_np, button_name);
	if (!ofnode_valid(btn_np))
		return -ENODEV;

	btn_event_np = ofnode_find_subnode(btn_np, event_name);
	if (!ofnode_valid(btn_np))
		return -ENODEV;

	if (action_name) {
		if (!read_button_event_action_params(btn_event_np, action_name,
			&timeout, &param)) {
			if (register_button_event_action(button_name, btn_event_np, hook,
				timeout, param))
				return -ENOENT;
			else
				num_action++;
		}
	} else {
		/* no action_name specified, enumerate all the actions properties */
		node_offset = ofnode_to_offset(btn_event_np);
		for (prop_offset = fdt_first_property_offset(fdt, node_offset);
	    	prop_offset > 0;
			prop_offset = fdt_next_property_offset(fdt, prop_offset)) {
				value = fdt_getprop_by_offset(fdt, prop_offset,
					      &propname, &len);			
				if (!value)
					continue;
				/* skip if the property is the action cmd string */
				len = strlen(propname);
				if (strncmp(&propname[len-4], "_cmd", 4) == 0)
					continue;

			if (read_button_event_action_params(btn_event_np, propname,
				&timeout, &param))
				continue;

			if (register_button_event_action(button_name, btn_event_np, hook,
				timeout, param))
				return -ENOENT;
			else
				num_action++;
		}
	}

	return num_action;
}

static int bcmbca_button_probe(struct udevice *dev)
{
	ofnode btn_np;
	int ret;
	int i = 0;
	struct ofnode_phandle_args params;

	dev_for_each_subnode(btn_np, dev) {
		memset(&btnInfo[i], 0x0, sizeof(BtnInfo));

		spin_lock_init(&btnInfo[i].lock);
		strncpy(btnInfo[i].name, ofnode_get_name(btn_np), 32);

		btnInfo[i].btnId = i;
		btnInfo[i].active = 0;
		btnInfo[i].events = 0;

		btnInfo[i].poll = btn_poll;
		btnInfo[i].isDown = btn_is_gpio_btn_down;

		ret = ofnode_parse_phandle_with_args(btn_np, CONSUMER_NAME, NULL, 3, 0,
									&params);
		if (ret) {
			dev_err(dev, "%s property not found ret %d\n", CONSUMER_NAME, ret);
			continue;
		}

		ret = uclass_first_device(UCLASS_GPIO, &btnInfo[i].gd.dev);
		if (ret) {
			dev_err(dev, "Failed to get GPIO device ret %d\n", ret);
			return ret;
		}

		if (params.args[1] & BCA_GPIO_ACTIVE_LOW) {
			params.args[1] &= ~BCA_GPIO_ACTIVE_LOW;
			params.args[1] |= GPIO_ACTIVE_LOW;
		}
		ret = gpio_xlate_offs_flags(btnInfo[i].gd.dev, &btnInfo[i].gd,
			&params);
		if (ret) {
			dev_err(dev, "gpio_xlate_offs_flags returned failure %d\n", ret);
			continue;
		}
		btnInfo[i].gd.flags |= GPIOD_IS_IN;

		ret = dm_gpio_request(&btnInfo[i].gd, CONSUMER_NAME);
		if (ret) {
			dev_err(dev, "Failed to request GPIO %d ret %d\n",
				btnInfo[i].gd.offset, ret);
			continue;
		}

		dm_gpio_set_dir(&btnInfo[i].gd);
		if (ret) {
			dev_err(dev, "Failed to set GPIO %d direction ret %d\n",
				btnInfo[i].gd.offset, ret);
			continue;
		}

		ret = register_button_action(ofnode_get_name(btn_np), PRINT_ACT_NAME_UBOOT,
			btn_hook_print);
		if (ret < 0) {
			register_button_action(ofnode_get_name(btn_np), PRINT_ACT_NAME,
				btn_hook_print);
		}

		i++;
		if (i == PB_BUTTON_MAX) {
			dev_err(dev, "max number %d of button reached!\n", i);
			break;
		}
	}

	return 0;
}

static struct udevice_id const bcmbca_button_of_match[] = {
	{ .compatible = "brcm,buttons" },
	{}
};

U_BOOT_DRIVER(bcmbca_button) = {
	.name = "bcm-bca-button",
	.id = UCLASS_NOP,
	.of_match = bcmbca_button_of_match,
	.probe = bcmbca_button_probe,
};

void bcmbca_button_init(void)
{
	struct udevice *dev;

	uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bcmbca_button),
						&dev);
}
