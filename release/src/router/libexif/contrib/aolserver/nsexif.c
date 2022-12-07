/*

Copyright (c) 2000 Curtis Galloway

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This is a loadable module for the AOLserver web server that provides
an interface to the EXIF tag parsing library.

It adds one Tcl command into the AOLserver Tcl interpreter:

  ns_exif <file>

It returns the ID of an ns_set that contains the tag names and values
from the EXIF file.

*/

#include "ns.h"

#include "exif.h"

#define BUFSIZE  1024

static void
rec_put(Ns_Set *set, exif_record_t *rec)
{
  char buf[BUFSIZE];
  char *str = buf;

  switch (rec->rec_type) {
  case 's':
    str = rec->rec_data.s;
    break;
  case 'f':
    snprintf(buf, BUFSIZE, "%f", rec->rec_data.f);
    break;
  case 'g':
    snprintf(buf, BUFSIZE, "%g", rec->rec_data.f);
    break;
  case 'l':
    snprintf(buf, BUFSIZE, "%ld", rec->rec_data.l);
    break;
  case 'r':
    snprintf(buf, BUFSIZE, "%d/%d", rec->rec_data.r.num,
	    rec->rec_data.r.denom);
    break;
  default:
    snprintf(buf, BUFSIZE, "<Unknown record type '%c'>", rec->rec_type);
    break;
  }
  Ns_SetPut(set, rec->rec_name, str);
}

static int
Tcl_ReadExifDataCmd (
		     ClientData    clientData,
		     Tcl_Interp   *interp,
		     int           argc,
		     char        **argv)
{
  exif_data_t *d;
  int i;
  Ns_Set *rset;

  d = exif_parse_file(argv[1]);
  if (d == NULL) {
    Tcl_AppendResult(interp, "Could not process file '", argv[1], "'", NULL);
    return TCL_ERROR;
  }
  rset = Ns_SetCreate("exif");

  for (i=0; i<d->n_recs; i++) {
    rec_put(rset, &d->recs[i]);
  }

  Ns_TclEnterSet(interp, rset, NS_TCL_SET_TEMPORARY | NS_TCL_SET_DYNAMIC);
  exif_free_data(d);
  return TCL_OK;
}


/*----------------------------------------------------------------------
 *
 *  Tcl_InitExif --
 *
 *  Initialize the Tcl command.
 *
 *----------------------------------------------------------------------
 */
void
Tcl_InitExif (interp)
    Tcl_Interp *interp;
{
  Tcl_CreateCommand (interp, "ns_exif", Tcl_ReadExifDataCmd, 
		     NULL, NULL);
}

static int
nsexif_interp_init (Tcl_Interp *interp, void *dummy)
{
  Tcl_InitExif(interp);
  return TCL_OK;
}


int
Ns_ModuleInit(char *hServer, char *hModule)
{
  char *configPath;

  Ns_Log( Notice, "%s module starting", hModule);

  exif_init((void *(*)(int))Ns_Malloc,
	    (void (*)(void *))Ns_Free,
	    (void *(*)(void *, int))Ns_Realloc);

  configPath = Ns_ConfigGetPath(hServer, hModule, NULL);
  /*  if (!Ns_ConfigGetBool (configPath, "Debug", &debug_p))
      debug_p = DEFAULT_DEBUG; */

  Ns_TclInitInterps (hServer, nsexif_interp_init, NULL);
  return NS_OK;
}

int Ns_ModuleVersion = 1;
