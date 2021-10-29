%module capi
%{
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/mngt.h>
%}

%include <stdint.i>
%include <cstring.i>

/* #include <netlink/genl/ctrl.h> */
extern int genl_ctrl_alloc_cache(struct nl_sock *, struct nl_cache **o_cache);
extern struct genl_family *genl_ctrl_search(struct nl_cache *, int);
extern struct genl_family *genl_ctrl_search_by_name(struct nl_cache *,
						    const char *);
extern int genl_ctrl_resolve(struct nl_sock *, const char *);
extern int genl_ctrl_resolve_grp(struct nl_sock *sk, const char *family,
				 const char *grp);

/* #include <netlink/genl/family.h> */
extern struct genl_family *genl_family_alloc(void);
extern void genl_family_put(struct genl_family *);

extern unsigned int genl_family_get_id(struct genl_family *);
extern void genl_family_set_id(struct genl_family *, unsigned int);
extern char *genl_family_get_name(struct genl_family *);
extern void genl_family_set_name(struct genl_family *, const char *name);
extern uint8_t genl_family_get_version(struct genl_family *);
extern void genl_family_set_version(struct genl_family *, uint8_t);
extern uint32_t genl_family_get_hdrsize(struct genl_family *);
extern void genl_family_set_hdrsize(struct genl_family *, uint32_t);
extern uint32_t genl_family_get_maxattr(struct genl_family *);
extern void genl_family_set_maxattr(struct genl_family *, uint32_t);

extern int genl_family_add_op(struct genl_family *, int, int);
extern int genl_family_add_grp(struct genl_family *, uint32_t , const char *);

/* #include <netlink/genl/genl.h> */
extern int genl_connect(struct nl_sock *);

extern void *genlmsg_put(struct nl_msg *, uint32_t, uint32_t,
			 int, int, int, uint8_t, uint8_t);

struct nlattr {
};

struct nla_policy {
	/** Type of attribute or NLA_UNSPEC */
	uint16_t	type;

	/** Minimal length of payload required */
	uint16_t	minlen;

	/** Maximal length of payload allowed */
	uint16_t	maxlen;
};

%inline %{
PyObject *py_genlmsg_parse(struct nlmsghdr *nlh, int uhl, int max,
			   PyObject *p)
{
	struct nlattr *tb_msg[max + 1];
	struct nla_policy *policy = NULL;
	void *pol;
	PyObject *attrs = Py_None;
	PyObject *k;
	PyObject *v;
	PyObject *resobj;
	int err;
	int i;

	if (p != Py_None) {
		PyObject *pobj;

		if (!PyList_Check(p)) {
			fprintf(stderr, "expected list object\n");
			err = -1;
			goto fail;
		}
		pobj = PyList_GetItem(p, 0);
		err = SWIG_ConvertPtr(pobj, &pol, SWIGTYPE_p_nla_policy, 0 |  0 );
		if (!SWIG_IsOK(err))
			goto fail;
		policy = pol;
	}
	err = genlmsg_parse(nlh, uhl, tb_msg, max, policy);
	if (err < 0) {
		fprintf(stderr, "Failed to parse response message\n");
	} else {
		attrs = PyDict_New();
		for (i = 0; i <= max; i++)
			if (tb_msg[i]) {
				k = PyInt_FromLong((long)i);
				v = SWIG_NewPointerObj(SWIG_as_voidptr(tb_msg[i]), SWIGTYPE_p_nlattr, 0 |  0 );
				PyDict_SetItem(attrs, k, v);
			}
	}
fail:
	if (attrs == Py_None)
		Py_INCREF(Py_None);
	resobj = Py_BuildValue("(iO)", err, attrs);
	return resobj;
}

%}
/* #include <netlink/genl/mngt.h> */
/* nothing yet */
