// SPDX-License-Identifier: GPL-2.0-only
/// NULL check before some freeing functions is not needed.
///
/// Based on checkpatch warning
/// "kfree(NULL) is safe this check is probably not required"
/// and kfreeaddr.cocci by Julia Lawall.
///
// Copyright: (C) 2014 Fabian Frederick.
// Comments: -
// Options: --no-includes --include-headers

virtual patch
virtual org
virtual report
virtual context

@r2 depends on patch@
expression E;
@@
- if (E != NULL)
(
  free(E);
|
  kfree(E);
|
  vfree(E);
|
  vfree_recursive(E);
|
  kmem_cache_free(E);
|
  kmem_cache_destroy(E);
|
  gzfree(E);
)

@r depends on context || report || org @
expression E;
position p;
@@

* if (E != NULL)
*	\(free@p\|kfree@p\|vfree@p\|debugfs_remove_recursive@p\|
*         kmem_cache_free@p\|kmem_cache_destroy@p\|gzfree@p\)(E);

@script:python depends on org@
p << r.p;
@@

cocci.print_main("NULL check before that freeing function is not needed", p)

@script:python depends on report@
p << r.p;
@@

msg = "WARNING: NULL check before some freeing functions is not needed."
coccilib.report.print_report(p[0], msg)
