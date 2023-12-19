#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy 2011

import os, shutil, re
import Options, Build, Logs

"""
Apply a least recently used policy to the Waf cache.

For performance reasons, it is called after the build is complete.

We assume that the the folders are written atomically

Do export WAFCACHE=/tmp/foo-xyz where xyz represents the cache size in megabytes
If missing, the default cache size will be set to 10GB
"""

re_num = re.compile('[a-zA-Z_]+(\d+)')

CACHESIZE = 10*1024*1024*1024 # in bytes
CLEANRATIO = 0.8
DIRSIZE = 4096

def compile(self):
	if Options.cache_global and not Options.options.nocache:
		try:
			os.makedirs(Options.cache_global)
		except:
			pass

	try:
		self.raw_compile()
	finally:
		if Options.cache_global and not Options.options.nocache:
			self.sweep()

def sweep(self):
	global CACHESIZE
	CACHEDIR = Options.cache_global

	# get the cache max size from the WAFCACHE filename
	re_num = re.compile('[a-zA-Z_]+(\d+)')
	val = re_num.sub('\\1', os.path.basename(Options.cache_global))
	try:
		CACHESIZE = int(val)
	except:
		pass

	# map folder names to timestamps
	flist = {}
	for x in os.listdir(CACHEDIR):
		j = os.path.join(CACHEDIR, x)
		if os.path.isdir(j) and len(x) == 32: # dir names are md5 hexdigests
			flist[x] = [os.stat(j).st_mtime, 0]

	for (x, v) in flist.items():
		cnt = DIRSIZE # each entry takes 4kB
		d = os.path.join(CACHEDIR, x)
		for k in os.listdir(d):
			cnt += os.stat(os.path.join(d, k)).st_size
		flist[x][1] = cnt

	total = sum([x[1] for x in flist.values()])
	Logs.debug('lru: Cache size is %r' % total)

	if total >= CACHESIZE:
		Logs.debug('lru: Trimming the cache since %r > %r' % (total, CACHESIZE))

		# make a list to sort the folders by timestamp
		lst = [(p, v[0], v[1]) for (p, v) in flist.items()]
		lst.sort(key=lambda x: x[1]) # sort by timestamp
		lst.reverse()

		while total >= CACHESIZE * CLEANRATIO:
			(k, t, s) = lst.pop()
			p = os.path.join(CACHEDIR, k)
			v = p + '.del'
			try:
				os.rename(p, v)
			except:
				# someone already did it
				pass
			else:
				try:
					shutil.rmtree(v)
				except:
					# this should not happen, but who knows?
					Logs.warn('If you ever see this message, report it (%r)' % v)
			total -= s
			del flist[k]
	Logs.debug('lru: Total at the end %r' % total)

Build.BuildContext.raw_compile = Build.BuildContext.compile
Build.BuildContext.compile = compile
Build.BuildContext.sweep = sweep

