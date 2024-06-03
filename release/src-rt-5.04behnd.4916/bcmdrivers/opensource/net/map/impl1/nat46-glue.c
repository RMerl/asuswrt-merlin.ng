/*
 * glue functions, candidates to go to -core
 *
 * Copyright (c) 2013-2014 Andrew Yourtchenko <ayourtch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */


#include "nat46-glue.h"
#include "nat46-core.h"

static DEFINE_MUTEX(ref_lock);
int is_valid_nat46(nat46_instance_t *nat46) {
  return (nat46 && (nat46->sig == NAT46_SIGNATURE));
}

nat46_instance_t *alloc_nat46_instance(int npairs, nat46_instance_t *old, int from_ipair, int to_ipair, int remove_ipair) {
  nat46_instance_t *nat46 = kzalloc(sizeof(nat46_instance_t) + npairs*sizeof(nat46_xlate_rulepair_t), GFP_KERNEL);
  if (!nat46) {
    printk("[nat46] make_nat46_instance: can not alloc a nat46 instance with %d pairs\n", npairs);
    return NULL;
  } else {
    printk("[nat46] make_nat46_instance: allocated nat46 instance with %d pairs\n", npairs);
  }
  nat46->sig = NAT46_SIGNATURE;
  nat46->npairs = npairs;
  nat46->refcount = 1; /* The caller gets the reference */
  if (old) {
    nat46->debug = old->debug;
    for(; (from_ipair >= 0) && (to_ipair >= 0) &&
          (from_ipair < old->npairs) && (to_ipair < nat46->npairs); from_ipair++) {
      if (from_ipair != remove_ipair) {
        nat46->pairs[to_ipair] = old->pairs[from_ipair];
        to_ipair++;
      }
    }
  }
  return nat46;
}


nat46_instance_t *get_nat46_instance(struct sk_buff *sk) {
  nat46_instance_t *nat46 = netdev_nat46_instance(sk->dev);
  mutex_lock(&ref_lock);
  if (is_valid_nat46(nat46)) {
    nat46->refcount++;
    mutex_unlock(&ref_lock);
    return nat46;
  } else {
    printk("[nat46] get_nat46_instance: Could not find a valid NAT46 instance!");
    mutex_unlock(&ref_lock);
    return NULL;
  }
}

void release_nat46_instance(nat46_instance_t *nat46) {
  mutex_lock(&ref_lock);
  nat46->refcount--;
  if(0 == nat46->refcount) {
    printk("[nat46] release_nat46_instance: freeing nat46 instance with %d pairs\n", nat46->npairs);
    nat46->sig = FREED_NAT46_SIGNATURE;
    kfree(nat46);
  }
  mutex_unlock(&ref_lock);
}
