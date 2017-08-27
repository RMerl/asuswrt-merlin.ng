/*======================================================================
 FILE: testvcal.c
 CREATOR: eric 26 May 2000

 (C) COPYRIGHT 2000 Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 The contents of this file are subject to the Mozilla Public License
 Version 1.0 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 the License for the specific language governing rights and
 limitations under the License.

 The Original Code is eric. The Initial Developer of the Original
 Code is Eric Busboom
======================================================================*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libicalvcal/icalvcal.h"
#include "libicalvcal/vcc.h"

#include <assert.h>

/* Given a vCal data file as its first argument, this program will
   print out an equivalent iCal component.

   For instance:

       ./testvcal ../../test-data/user-cal.vcf

*/

int main(int argc, char *argv[])
{
    VObject *vcal = 0;
    icalcomponent *comp;
    char *file;

    if (argc != 2) {
        file = "../../test-data/user-cal.vcf";
    } else {
        file = argv[1];
    }

    vcal = Parse_MIME_FromFileName(file);

    assert(vcal != 0);

    comp = icalvcal_convert(vcal);

    printf("%s\n", icalcomponent_as_ical_string(comp));

    return 0;
}
