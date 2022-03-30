#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <bcm_bca_extintr.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/sched/types.h>
#include <linux/input.h>
#include <button.h>

#define CONSUMER_NAME           "ext_irq"
#define BTN_EVENT_NUM           3

#define PRINT_ACT_NAME          "print"
#define PRESS_ACT_NAME          "linux,press"
#define RELEASE_ACT_NAME        "linux,release"
#define KEY_CODE_NAME           "linux,code"

#define PRESS_EVENT             "press"
#define HOLD_EVENT              "hold"
#define RELEASE_EVENT           "release"

#define MAX_BTN_HOOKS_PER_TRIG  5
#define MAX_BTN_HOOKS_PER_BTN   (MAX_BTN_HOOKS_PER_TRIG * 3)

typedef int PB_BUTTON_ID;
#define PB_BUTTON_MAX 32

#define BTN_EV_PRESSED       0x1
#define BTN_EV_HOLD          0x2
#define BTN_EV_RELEASED      0x4
#define BTN_POLLFREQ         100         /* in ms */

// Main button structure:
typedef struct _BtnInfo {
    PB_BUTTON_ID         btnId;
    int                  extIrq;
    struct gpio_desc     *gd;
    int                  active;       //set to 1 if button is down, 0 otherwise
    
    uint32_t             lastPressJiffies;
    uint32_t             lastHoldJiffies;
    uint32_t             lastReleaseJiffies;
    
    struct timer_list    timer;        //used for polling
    
    spinlock_t            lock;
    unsigned long         events;       //must be protected by lock
    wait_queue_head_t     waitq;
    struct task_struct *  thread;
    
    unsigned short linux_key_code;
    //interrupt related functions
    irqreturn_t  (* pressIsr)(int irq, void *btnInfo);
    irqreturn_t  (* releaseIsr)(int irq, void *btnInfo);
    /*poll below is function pointer for linux-4.1 void (* poll)(unsigned long btnInfo); 
      or for linux-4.19 void (* poll)(struct timer_list*);*/
    void * poll;
    char   name[32];

    //functional related fuctions
    bool         (* isDown)(struct _BtnInfo *btnInfo);
    void         (* enableIrqs)(struct _BtnInfo *btnInfo);
    void         (* disableIrqs)(struct _BtnInfo *btnInfo);
} BtnInfo;


typedef struct {
    buttonNotifyHook_t  hook;
    unsigned long       timeout; // in ms;
    void*               param;
    int                 done;
} pushButtonHookInfo_t;

typedef int (*registerBtnHook)( PB_BUTTON_ID        btn, 
                            buttonNotifyHook_t      hook, 
                            unsigned long           timeInMs,
                            void*                   param);

struct button_action
{
    char                *action_name;
    buttonNotifyHook_t  btn_hook;
};

struct button_events
{
    char*           btn_event;
    registerBtnHook reg_hook;
};

static pushButtonHookInfo_t btnPressedInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
static pushButtonHookInfo_t btnHeldInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
static pushButtonHookInfo_t btnReleasedInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
static int btnPressJiffies[PB_BUTTON_MAX] = {};
static DEFINE_SPINLOCK(lock);

static BtnInfo btnInfo[PB_BUTTON_MAX] = {};
static unsigned short btn_keymap[PB_BUTTON_MAX] = {0};
static struct input_dev *btn_input_dev = NULL;

static void reset_hooks_array(PB_BUTTON_ID btn)
{
    int j;
    for(j=0; j<MAX_BTN_HOOKS_PER_TRIG; j++)
    {
        btnPressedInfo[btn][j].done = 0;
        btnHeldInfo[btn][j].done = 0;
        btnReleasedInfo[btn][j].done = 0;
    }
}

static BtnInfo* find_btn_info(const char *name)
{
    int i;
    for (i=0 ;i<PB_BUTTON_MAX; i++) 
    {
        if(!strcmp(name, btnInfo[i].name))
            return &btnInfo[i];
    }
    return NULL;
}

static void do_button(PB_BUTTON_ID btn, unsigned long currentJiffies, pushButtonHookInfo_t (*actions_arr)[MAX_BTN_HOOKS_PER_TRIG])
{
    unsigned long timeInMs;
    pushButtonHookInfo_t *pInfo;
    int callIdx;
    int callInfoIdx = 0;
    pushButtonHookInfo_t callInfo[MAX_BTN_HOOKS_PER_TRIG] = {};      
    unsigned long flags;
    int idx;
    
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return;
    }

    timeInMs = jiffies_to_msecs(currentJiffies-btnPressJiffies[btn]);

    spin_lock_irqsave(&lock,flags);
    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++)
    {
        pInfo = &actions_arr[btn][idx];
        if(pInfo->done)
            continue;

        if (pInfo->hook) {
            if(pInfo->timeout)
            {
                if(pInfo->timeout > timeInMs)
                    continue;
            }
            callInfo[callInfoIdx] = *pInfo;
            pInfo->done = 1;
            callInfoIdx++;
        }
    }
    spin_unlock_irqrestore(&lock, flags);

    for (callIdx = 0; callIdx < callInfoIdx; callIdx++) {
        callInfo[callIdx].hook(timeInMs, callInfo[callIdx].param);
    }
}

void do_button_release(PB_BUTTON_ID btn, unsigned long currentJiffies)
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

    timeInMs = jiffies_to_msecs(currentJiffies-btnPressJiffies[btn]);
    
    spin_lock_irqsave(&lock,flags);
    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
        pushButtonHookInfo_t *pNewInfo = &btnReleasedInfo[btn][idx];
        if ( (pNewInfo->hook == NULL) || (pNewInfo->timeout > timeInMs))
            continue;
        if ( callInfoIdx == 0 || pNewInfo->timeout == callInfo[0].timeout) {
            callInfo[callInfoIdx] = *pNewInfo;
            callInfoIdx++;
        } else if (pNewInfo->timeout > callInfo[0].timeout) {
            callInfo[0] = *pNewInfo;
            callInfoIdx = 1;
        }
    }
    spin_unlock_irqrestore(&lock, flags);
    
    for (callIdx = 0; callIdx < callInfoIdx; callIdx++) {
        callInfo[callIdx].hook(timeInMs, callInfo[callIdx].param);
    }
}

static int insert_to_array( 
    buttonNotifyHook_t          hook, 
    unsigned long                   timeInMs,
    void*                           param,
    pushButtonHookInfo_t            *pHookArray )
{
    int idx;
    pushButtonHookInfo_t *pInfo;

    if (pHookArray[MAX_BTN_HOOKS_PER_TRIG-1].hook != NULL) {
        printk(KERN_ERR "%s: to many entries\n", __func__);
        return -1;
    }
    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
        pInfo = &pHookArray[idx];
        if (pInfo->hook == NULL) {           // inserting at end
            pInfo->hook = hook;
            pInfo->timeout = timeInMs;
            pInfo->param = param;
            return idx;
        }
    }
    return -1;
}

static int register_button_press_notify_hook(
    PB_BUTTON_ID                        btn, 
    buttonNotifyHook_t                  hook,
    unsigned long                       timeInMs,
    void*                               param)
{
    // Note: this is used from ISR's, so different locking mechansim required here....
    unsigned long flags;
    int idx;

    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }
    if (unlikely(hook == NULL)) {        
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }
    
    spin_lock_irqsave(&lock,flags);
    idx = insert_to_array(hook,timeInMs,param,btnPressedInfo[btn]);
    spin_unlock_irqrestore(&lock,flags); 
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
        return -1;
    }
    
    return 0;
}

static int register_button_hold_notify_hook(
    PB_BUTTON_ID                    btn, 
    buttonNotifyHook_t              hook, 
    unsigned long                   timeInMs,
    void*                           param)
{
    int idx;
    unsigned long flags;
    
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }
    if (unlikely(hook == NULL)) {
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }
    
    spin_lock_irqsave(&lock,flags);
    idx = insert_to_array(hook,timeInMs,param,btnHeldInfo[btn]);
    spin_unlock_irqrestore(&lock,flags);    
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
        return -1;
    }

    return 0;
}

static int register_button_release_notify_hook(
    PB_BUTTON_ID                    btn, 
    buttonNotifyHook_t              hook, 
    unsigned long                   timeInMs,
    void*                           param)
{
    int idx;
    unsigned long flags;
    
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }    
    if (unlikely(hook == NULL)) {
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }    
    spin_lock_irqsave(&lock,flags);
    idx = insert_to_array(hook,timeInMs,param,btnReleasedInfo[btn]);
    spin_unlock_irqrestore(&lock,flags);  
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
        return -1;
    }
    return 0;
}

/***************************************************************************/
// BP_BTN_ACTION_PRINT
static void btn_hook_print(unsigned long timeInMs, void* param) {    
    printk("%s\n", (char *)param);
}
//INPUT action
static void btn_hook_linux_press(unsigned long timeInMs, void* param)
{    
    unsigned int key_code = (uintptr_t)param;
    input_event(btn_input_dev, EV_KEY, key_code, 1);
    input_sync(btn_input_dev);
}

static void btn_hook_linux_release(unsigned long timeInMs, void* param)
{    
    unsigned int key_code = (uintptr_t)param;
    input_event(btn_input_dev, EV_KEY, key_code, 0);
    input_sync(btn_input_dev);
}

/***************************************************************************
 * Function Name: btn_do_press
 * Description  : This is called when a press has been detected.
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btn_do_press (BtnInfo *btn, unsigned long currentJiffies) {
    unsigned long flags;
    // this is called from the kernel thread context.
    if (btn->releaseIsr) {
        // note: releaseIsr implies edge detect
            btn->enableIrqs(btn);
    } else {
        btn->disableIrqs(btn);
    }

    spin_lock_irqsave(&lock,flags);

    reset_hooks_array(btn->btnId);
    btnPressJiffies[btn->btnId] = currentJiffies;

    spin_unlock_irqrestore(&lock,flags);

    do_button(btn->btnId, currentJiffies, btnPressedInfo);
    return;
}

/***************************************************************************
 * Function Name: btn_do_release
 * Description  : This is called when a release has been detected
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btn_do_release (BtnInfo *btn, unsigned long currentJiffies) {
    // this is called from the kernel thread context.
    do_button_release(btn->btnId, currentJiffies);
    if (btn->pressIsr) {
            btn->enableIrqs(btn);
    }
    return;
}

/***************************************************************************
 * Function Name: btn_do_hold
 * Description  : This is called when a button hold is detected
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btn_do_hold(BtnInfo *btn, unsigned long currentJiffies) {
    // this is called from the kernel thread context.
    do_button(btn->btnId, currentJiffies, btnHeldInfo);
}

/***************************************************************************
 * Function Name: btn_thread
 * Description  : This is the thread function that takes care of a button.
                  It is repsonsible for invoking any registered call backs,
                  and doing polling. 
                  Assume it will never exit...
 * Parameters   : arg: pointer to the button structure
 ***************************************************************************/
static int btn_thread(void * arg) 
{
    BtnInfo *btn = (BtnInfo*)arg;
    unsigned long flags;
    struct sched_param sp = { .sched_priority = 20 };

    // set to be realtime thread with somewhat high priority:
    sched_setscheduler(current, SCHED_FIFO, &sp);

    while(1) {     

        // at this point the button is not pressed -- wait for press:
        wait_event_interruptible(btn->waitq, (btn->events & BTN_EV_PRESSED) != 0);
        
        spin_lock_irqsave(&btn->lock, flags);        
        btn->events &= ~ BTN_EV_PRESSED;
        spin_unlock_irqrestore(&btn->lock, flags);
        btn_do_press(btn, btn->lastPressJiffies);

        // at this point the button is down -- wait for release or until next hold event:
        while(1) {
            wait_event_interruptible_timeout(btn->waitq, btn->events != 0, msecs_to_jiffies(BTN_POLLFREQ));
            if (btn->events & BTN_EV_HOLD) {
                spin_lock_irqsave(&btn->lock, flags);        
                btn->events &= ~BTN_EV_HOLD;
                spin_unlock_irqrestore(&btn->lock, flags);
                btn_do_hold(btn, btn->lastHoldJiffies);
            }
            if (btn->events & BTN_EV_RELEASED) {
                spin_lock_irqsave(&btn->lock, flags);        
                btn->events &= ~ (BTN_EV_RELEASED | BTN_EV_HOLD);
                spin_unlock_irqrestore(&btn->lock, flags);
                btn_do_release(btn, btn->lastReleaseJiffies);
                break;
            }
        }
    }
    return 0;
}

/***************************************************************************
 * Function Name: btn_press_isr
 * Description  : This is the default btnPress interrupt handler.  It
                  assumes the button drives a gpio and is mapped to an
                  external interrupt.
                  This invokes the do_button_press, and starts the
                  polling timer.
 * Parameters   : irq: the irq number of the button
                  info: a pointer to a BtnInfo structure
 * Returns      : IRQ_HANDLED.
 ***************************************************************************/
static irqreturn_t btn_press_isr(int irq, void *info) {
    BtnInfo *btn = (BtnInfo*)info;
    unsigned long currentJiffies = jiffies;
    int wasTimerActive;
    unsigned long flags;
    
    spin_lock_irqsave(&btn->lock, flags);
    btn->active=1;
    btn->lastPressJiffies=currentJiffies;
    if ( btn->releaseIsr == NULL && btn->poll != NULL){
        wasTimerActive = mod_timer(&btn->timer, (currentJiffies + msecs_to_jiffies(BTN_POLLFREQ)));
    }
    btn->events |= BTN_EV_PRESSED;
    btn->disableIrqs(btn);
    wake_up(&btn->waitq);
    spin_unlock_irqrestore(&btn->lock, flags);
    return IRQ_HANDLED;
}

static void __btn_poll(BtnInfo *btn)
{
    unsigned long currentJiffies = jiffies;
    int wasTimerActive;
    unsigned long flags;

    spin_lock_irqsave(&btn->lock, flags);
    if (btn->active) {
        if ( btn->isDown(btn) ) {
            btn->lastHoldJiffies = currentJiffies;
            wasTimerActive=mod_timer(&btn->timer, currentJiffies + msecs_to_jiffies(BTN_POLLFREQ)); 
            btn->events |= BTN_EV_HOLD;
            wake_up(&btn->waitq);
        }
        else if (btn->releaseIsr == NULL) {
            btn->lastReleaseJiffies = currentJiffies;
            btn->active = 0;
            del_timer(&btn->timer);
            btn->events |= BTN_EV_RELEASED;
            wake_up(&btn->waitq);
        } 
        else {
            // hit race condition.  releaseIsr is pending (and it will 
            // stop the timer, etc.   Do nothing here)
        }
    } 
    else {
        // we should not get here
    }
    
    spin_unlock_irqrestore(&btn->lock, flags);
}

/***************************************************************************
 * Function Name: btn_poll
 * Description  : This is the polling function.  It is started when a 
                  button press is detected, and stopped when a button 
                  release is detected.
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btn_poll(struct timer_list * arg) {

    BtnInfo *btn = from_timer(btn, arg, timer);
    __btn_poll(btn);
}

/***************************************************************************
 * Function Name: btn_is_gpio_btn_down
 * Description  : This a the check to see if a gpio-based button is down
                  based on the gpio level
 * Parameters   : arg: a pointer to a BtnInfo structure
 * Returns      : 1 if the button is down
 ***************************************************************************/
static bool btn_is_gpio_btn_down(BtnInfo *btn) {

    return gpiod_get_value(btn->gd);
}

/***************************************************************************
 * Function Name: btn_enable_irq
 * Description  : enable a buttons Irqs
 ***************************************************************************/
static void btn_enable_irq(BtnInfo *btn) {
    bcm_bca_extintr_clear(btn->extIrq);
    bcm_bca_extintr_unmask(btn->extIrq);
}

/***************************************************************************
 * Function Name: btn_disable_irq
 * Description  : enable a buttons Irqs
 ***************************************************************************/
static void btn_disable_irq(BtnInfo *btn) {
    bcm_bca_extintr_mask(btn->extIrq);
    bcm_bca_extintr_clear(btn->extIrq);
}

static struct button_events btn_events[BTN_EVENT_NUM] =
{
    {PRESS_EVENT,   register_button_press_notify_hook},
    {HOLD_EVENT,    register_button_hold_notify_hook},
    {RELEASE_EVENT, register_button_release_notify_hook}
};

static registerBtnHook get_register_hook_fnc(const char *event_name)
{
    int i;
    for(i=0; i<BTN_EVENT_NUM; i++)
    {
        if(!strcmp(event_name, btn_events[i].btn_event))
            return btn_events[i].reg_hook;
    }
    return NULL;
}

int register_button_action(const char *button_name, char *action_name, buttonNotifyHook_t hook)
{
    struct device_node *btn_np = NULL, *btn_event_np = NULL;
    registerBtnHook btn_hook_register = NULL;
    
    btn_np = of_find_node_by_name(NULL, button_name);
    if(!btn_np)
        return -ENODEV;

    for (;;)
    {
        const char *print_string = NULL;
        int timeout = 0;
        BtnInfo* btn_info = NULL;
        void *param = NULL;

        btn_event_np = of_get_next_child(btn_np, btn_event_np);
        if (!btn_event_np) 
            break;
        
        btn_hook_register = get_register_hook_fnc(btn_event_np->name);
        if(!btn_hook_register)
            return -EINVAL;

        btn_info = find_btn_info(button_name);
        if(!btn_info)
            return -ENODEV;

        if(!strcmp(action_name, PRINT_ACT_NAME))
        {
            if(of_property_read_string(btn_event_np, PRINT_ACT_NAME, &print_string))
                continue;
            param = kstrdup(print_string, GFP_KERNEL);
        }
        else if(!strcmp(action_name, PRESS_ACT_NAME) || !strcmp(action_name, RELEASE_ACT_NAME))
        {
            if(of_property_read_u32(btn_event_np, action_name, &timeout))
                continue;
            param = (void *)(uintptr_t)(btn_info->linux_key_code); 
        }
        else
        {
            if(of_property_read_u32(btn_event_np, action_name, &timeout))
                continue;
            timeout *= 1000;
        }

        btn_hook_register(btn_info->btnId, hook, timeout, param);
    }
    return 0;
}

EXPORT_SYMBOL(register_button_action);

static int btn_keys_open(struct input_dev *input)
{
    return 0;
}

static void btn_keys_close(struct input_dev *input)
{
}

static int button_probe(struct platform_device *pdev)
{
    struct device_node *btn_np = NULL;
    int ret;
    int i;
    struct device *dev = &pdev->dev;

    btn_input_dev = devm_input_allocate_device(dev);
    if (!btn_input_dev) 
    {
        dev_err(dev, "failed to allocate input device\n");
        return -ENOMEM;
    }

    btn_input_dev->name = pdev->name;
    btn_input_dev->phys = "gpio-keys/input0";
    btn_input_dev->dev.parent = dev;
    btn_input_dev->open = btn_keys_open;
    btn_input_dev->close = btn_keys_close;

    btn_input_dev->id.bustype = BUS_HOST;
    btn_input_dev->id.vendor = 0x0001;
    btn_input_dev->id.product = 0x0001;
    btn_input_dev->id.version = 0x0100;

    btn_input_dev->keycode = &btn_keymap[0];
    btn_input_dev->keycodesize = sizeof(btn_keymap[0]);
    btn_input_dev->keycodemax = PB_BUTTON_MAX;

    for (i=0; i<PB_BUTTON_MAX; i++)
    {
        unsigned int key_code = 0;
        btn_np = of_get_next_child(pdev->dev.of_node, btn_np);
        if (!btn_np)
            break;

        spin_lock_init(&btnInfo[i].lock);

        strncpy(btnInfo[i].name, btn_np->name, 32);

        btnInfo[i].btnId = i;
        btnInfo[i].active = 0;
        btnInfo[i].events = 0;
        init_waitqueue_head(&btnInfo[i].waitq);

        /* The following is the default callbacks (assuming gpio to extIrq).  For any
        other type of button, simply replace the callbacks */
        btnInfo[i].pressIsr = btn_press_isr; 
        btnInfo[i].releaseIsr = NULL;
        btnInfo[i].poll = btn_poll;
        btnInfo[i].isDown = btn_is_gpio_btn_down;
        btnInfo[i].enableIrqs = btn_enable_irq;
        btnInfo[i].disableIrqs = btn_disable_irq;
        btnInfo[i].linux_key_code = KEY_UNKNOWN;

        // set up timer:
        if (btnInfo[i].releaseIsr == NULL) {
        // we're going to have to poll the button to see when it's released:
            timer_setup(&btnInfo[i].timer, btnInfo[i].poll,0);
            btnInfo[i].timer.expires  = jiffies + msecs_to_jiffies(BTN_POLLFREQ);
        }

        ret = bcm_bca_extintr_request(&pdev->dev, btn_np, CONSUMER_NAME, btnInfo[i].pressIsr, &btnInfo[i], btn_np->name , NULL);
        if (ret < 0)
        {
            dev_err(&pdev->dev, "bcm_bca_extintr_request for %s failed err %d\n", CONSUMER_NAME, ret);
            return ret;
        }
        btnInfo[i].extIrq = ret;

        btnInfo[i].gd = bcm_bca_extintr_get_gpiod(btnInfo[i].extIrq);
        if (btnInfo[i].gd == ERR_PTR(-EPROBE_DEFER))
            return -EPROBE_DEFER;

        if (!btnInfo[i].gd)
        {
            dev_err(&pdev->dev, "bcm_bca_extintr_get_gpiod for irq %d failed\n",  btnInfo[i].extIrq);
            continue;
        }

        if(!of_property_read_u32(btn_np, KEY_CODE_NAME, &key_code))
        {
            btn_keymap[i] = btnInfo[i].linux_key_code = key_code;
            input_set_capability(btn_input_dev, EV_KEY, key_code);
        }

        ret = register_button_action(btn_np->name, PRINT_ACT_NAME, btn_hook_print);
        if (ret < 0)
        {
            dev_err(&pdev->dev, "register_button_action for %s failed err %d\n", CONSUMER_NAME, ret);
            continue;
        }

        ret = register_button_action(btn_np->name, PRESS_ACT_NAME, btn_hook_linux_press);
        if (ret < 0)
        {
            dev_err(&pdev->dev, "register_button_action input for %s failed err %d\n", CONSUMER_NAME, ret);
            continue;
        }

        ret = register_button_action(btn_np->name, RELEASE_ACT_NAME, btn_hook_linux_release);
        if (ret < 0)
        {
            dev_err(&pdev->dev, "register_button_action input for %s failed err %d\n", CONSUMER_NAME, ret);
            continue;
        }


        btnInfo[i].thread = kthread_run(btn_thread, (void *)&btnInfo[i], "btnhandler%d", i);
        if (!btnInfo[i].thread) 
        {
            dev_err(&pdev->dev, "ERROR could not start kthread\n");   
            continue;  
        }
    }

    ret = input_register_device(btn_input_dev);
    if (ret) 
    {
        dev_err(dev, "Unable to register input device, error: %d\n", ret);
        return ret;
    }

    return 0;
}

static struct of_device_id const button_match[] = {
	{ .compatible = "brcm,buttons" },
	{}
};

MODULE_DEVICE_TABLE(of, button_match);

static struct platform_driver button_driver = {
	.driver = {
			.name = "brcm,buttons",
			.of_match_table = button_match,
	},
	.probe = button_probe,
};

static int __init button_init(void)
{
	return platform_driver_register(&button_driver);
}

subsys_initcall(button_init);

MODULE_AUTHOR("Vladimir Neyelov (vladimir.neyelov@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA Board Button driver");
MODULE_LICENSE("GPL v2");
