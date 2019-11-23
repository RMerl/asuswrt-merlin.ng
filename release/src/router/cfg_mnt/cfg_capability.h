#ifndef __CFG_CAPABILITY_H__
#define __CFG_CAPABILITY_H__
 
#if defined(BIT)
#undef BIT
#endif
#define BIT(x)	((1 << x))

typedef struct _capability_ss {
	unsigned int type;
	unsigned int subtype;
} capability_s;

extern json_object *cm_generateCapability(capability_s *capablity);
	
/* type */
enum capabilityType {
	LED_CONTROL = 1,
	WANS_CAP = 15,
	CAPABILITY_MAX
};

/* subtype */
/* for LED_CONTROL */
#define CENTRAL_LED		BIT(0)
#define LP55XX_LED		BIT(1)

/* for wan capability */
#define WANS_CAP_WAN        BIT(0)

#endif /* __CFG_CAPABILITY_H__ */
/* End of cfg_capability.h */
