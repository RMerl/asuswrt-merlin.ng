/* This file ensure that is at least one symbol in our compat
 * convenience library. Otherwise, at least the Solaris linker
 * bails out with an error message like this:
 *
 * ld: elf error: file ../compat/.libs/compat.a: elf_getarsym
 *
 * Copyright 2016 Rainer Gerhards and Adiscon
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "config.h"
#ifdef OS_SOLARIS
int SOLARIS_wants_a_symbol_inside_the_lib;
#endif
