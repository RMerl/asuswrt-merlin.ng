/* im-helper.h
 * This file contains helper constructs that save time writing input modules. It
 * assumes some common field names and plumbing. It is intended to be used together
 * with module-template.h
 *
 * File begun on 2011-05-04 by RGerhards
 *
 * Copyright 2011-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#ifndef	IM_HELPER_H_INCLUDED
#define	IM_HELPER_H_INCLUDED 1


/* The following function provides a complete implementation to check a
 * ruleset and set the actual ruleset pointer. The macro assumes that
 * standard field names are used. A functon std_checkRuleset_genErrMsg()
 * must be defined to generate error messages in case the ruleset cannot
 * be found.
 */
static inline void std_checkRuleset_genErrMsg(modConfData_t *modConf, instanceConf_t *inst);
static inline rsRetVal
std_checkRuleset(modConfData_t *modConf, instanceConf_t *inst)
{
	ruleset_t *pRuleset;
	rsRetVal localRet;
	DEFiRet;

	inst->pBindRuleset = NULL;	/* assume default ruleset */

	if(inst->pszBindRuleset == NULL)
		FINALIZE;

	localRet = ruleset.GetRuleset(modConf->pConf, &pRuleset, inst->pszBindRuleset);
	if(localRet == RS_RET_NOT_FOUND) {
		std_checkRuleset_genErrMsg(modConf, inst);
	}
	CHKiRet(localRet);
	inst->pBindRuleset = pRuleset;

finalize_it:
	RETiRet;
}

#endif /* #ifndef IM_HELPER_H_INCLUDED */
