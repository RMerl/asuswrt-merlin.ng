/*
** =============================================================================
**   FILE NAME        : fapi_wlan_tools.h
**   PROJECT          : UGW WLAN FAPI
**   DATE             : 02-Aug-2016
**   AUTHOR           : WLAN Subsystem
**   DESCRIPTION      : This file contains the WLAN FAPI tools prototypes.
**   REFERENCES       :
**   COPYRIGHT        : Copyright (c) 2015
**                      Intel
**
**   Any use of this software is subject to the conclusion of a respective
**   License agreement. Without such a License agreement no rights to the
**   software are granted

**   HISTORY          :
**   $Date   02-Aug-2016         $Author  yhanin                     $Comment
**
** ============================================================================
*/
/*! \file	fapi_wlan_tools.h
	\brief	This file provides prototype definitions for the WLAN FAPI tools:
			touchFile , copyFile, copyDir, removeDir, remove, rename, mkdir, 
			mvFile , file_exists, chmod.
	

*/

#ifndef _FAPI_WLAN_TOOLS_H
#define _FAPI_WLAN_TOOLS_H

/*! \brief Function for doing touch file.
		\param[in] pointer to file name
		\return UGW_SUCCESS or UGW_FAILURE
*/
int touchFile(char *file_name);

/*! \brief Function for doing mv file.
		\param[in] pointer to src file name
		\param[in] pointer to trg file name
		\return UGW_SUCCESS or UGW_FAILURE
*/
int mvFile(char *src_path, char *trg_path);

/*! \brief Function for doing copy file keeping file attributes.
		\param[in] pointer to full path and source file name
		\param[in] pointer to full path and target file name
		\return UGW_SUCCESS or UGW_FAILURE
*/
int copyFile(char *src_path, char *trg_path);

/*! \brief	Function for doing copy directory or selective files by
			file_name_str.same as doing cp -rf *<file_name_str>*
			if the string is there the file	will be copied if no
			then not.
		\param[in] pointer to full path source directory.
		\param[in] pointer to full path target directory.
		\param[in] pointer to file name string for selective files copying.
		\return UGW_SUCCESS or UGW_FAILURE
*/
int copyDir(const char *src_dir, const char *trg_dir, const char *file_name_str);

/*! \brief Function for doing remove directory .
		DIR_MAX_NUM - maximum number of directories depth.

		\param[in] pointer to directory.
		\return 0 SUCCESS
*/
#define DIR_MAX_NUM 16
int removeDir(const char *src_dir);

/*! \brief file_exists flags:
		R_OK	- test for read permission
		W_OK	- test for write permission
		X_OK	- test for execute or search permission
		F_OK	- test whether the directories leading to
					the file can be searched and the file exists.

		\param[in] fname - file name to check
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int file_exists(const char *fname);

char *paramValueFromObjGet(ObjList *objPtr, char *objName, char *paramName);

char *paramValueFromObjBufGet(ObjList *objPtr, char *objName, char *paramName);

char *paramValueFromSubObjGet(ObjList *subObjDB, char *paramName);

void addObjListIfNotExist(ObjList *objPtr, char *objName);

/*
 C library function
 ------------------
 int rename(const char *old_filename, const char *new_filename);
 int remove(const char *filename);
 int mkdir(const char *pathname, mode_t mode);
 int chmod(const char *path, mode_t mode);
*/

#endif //_FAPI_WLAN_TOOLS_H
