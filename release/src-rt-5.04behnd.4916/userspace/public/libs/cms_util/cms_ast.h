/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
:>
 *
 ************************************************************************/


#ifndef __CMS_AST_H__
#define __CMS_AST_H__

/*!\file cms_ast.h
 * \brief Header file for assert functions.
 *  
 *  For production builds, define NDEBUG to disable asserts.
 */

/** Check that an assumed condition is TRUE.
 * 
 * This should be used during development only.  For production code,
 * compile with NDEBUG defined.
 */
#define cmsAst_assert(expr) cmsAst_assertFunc(__FILE__, __LINE__, __STRING(expr), (expr))

/** The actual assert function, don't call this directly, use the macro.
 *
 * @param filename (IN) file where the assert occured.
 * @param lineNumber (IN) Linenumber of the assert statement.
 * @param exprString (IN) The actual expression that is being asserted.
 * @param expr       (IN) The result of the evaluation of the expression.  0 is fail,
 *                        non-zero is pass.
 */
void cmsAst_assertFunc(const char *filename, UINT32 lineNumber, const char *exprString, SINT32 expr);

#endif /* __CMS_AST_H__ */
