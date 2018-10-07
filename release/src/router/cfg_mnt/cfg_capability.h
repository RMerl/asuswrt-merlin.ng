#ifndef __CFG_CAPABILITY_H__
#define __CFG_CAPABILITY_H__
 
typedef struct _capability_ss {
	unsigned int type;
	unsigned int subtype;
} capability_s;

extern json_object *cm_generateCapability(capability_s *capablity);
	
/* type */
enum capabilityType {
	LED_CONTROL = 1
};

/* subtype */
/* for LED_CONTROL */
#define CENTRAL_LED		BIT(0)

#endif /* __CFG_CAPABILITY_H__ */
/* End of cfg_capability.h */
