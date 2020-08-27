#ifndef __SM_H__
#define __SM_H__

#include <shared.h>

#define SM_DEBUG_FILE	"/tmp/SM_DEBUG"

#define SM_DEBUG(fmt, args...) \
	if(f_exists(SM_DEBUG_FILE)) { \
		cprintf(fmt, ## args); \
	}

typedef struct _convType convType;
struct _convType {
        unsigned char   type;
        char            *signature;
	unsigned char	base;
};

typedef struct _ac_trans ac_trans;
struct _ac_trans {
	unsigned char		transChar;
	struct _ac_state	*nextState;
	struct _ac_trans	*nextTrans;
};

typedef struct _ac_state ac_state;
struct _ac_state {
	struct _ac_trans	*nextTrans;
	struct _ac_state	*prevState;
	struct _ac_state	*next;
	struct _match_rule	*matchRuleList;
};

typedef struct _match_rule match_rule;
struct _match_rule {
	unsigned char		ID;
	struct _match_rule	*next;
	unsigned char		baseID;
};

ac_state *construct_ac_trie(convType *type);
ac_state *find_next_state(ac_state *state, unsigned char transChar);
ac_state *create_ac_state();
void add_new_next_state(ac_state *curState, unsigned char pChar, ac_state *nextState);
void add_match_rule_to_state(ac_state *state, unsigned char type, unsigned char base);
unsigned char prefix_search(ac_state *sm, const char *text, unsigned char *baseID);
unsigned char full_search(ac_state *sm, const char *text, unsigned char *baseID);
unsigned int prefix_search_index(ac_state *sm, const char *text, unsigned char *baseID);
#endif
