#ifndef __EBT_VTAG_T_H__
#define __EBT_VTAG_T_H__


struct ebt_vtag_t_info
{
	int vtag; 
	/* EBT_ACCEPT, EBT_DROP, EBT_CONTINUE or EBT_RETURN */
	int target;
};

#endif //__EBT_VTAG_T_H__

