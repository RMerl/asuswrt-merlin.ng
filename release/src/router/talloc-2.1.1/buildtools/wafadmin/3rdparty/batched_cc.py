#!/usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006 (ita)

"""
Batched builds - compile faster
instead of compiling object files one by one, c/c++ compilers are often able to compile at once:
cc -c ../file1.c ../file2.c ../file3.c

Files are output on the directory where the compiler is called, and dependencies are more difficult
to track (do not run the command on all source files if only one file changes)

As such, we do as if the files were compiled one by one, but no command is actually run:
replace each cc/cpp Task by a TaskSlave
A new task called TaskMaster collects the signatures from each slave and finds out the command-line
to run.

To set this up, the method ccroot::create_task is replaced by a new version, to enable batched builds
it is only necessary to import this module in the configuration (no other change required)
"""

MAX_BATCH = 50
MAXPARALLEL = False

EXT_C = ['.c', '.cc', '.cpp', '.cxx']

import os, threading
import TaskGen, Task, ccroot, Build, Logs
from TaskGen import extension, feature, before
from Constants import *

cc_str = '${CC} ${CCFLAGS} ${CPPFLAGS} ${_CCINCFLAGS} ${_CCDEFFLAGS} -c ${SRCLST}'
cc_fun = Task.compile_fun_noshell('batched_cc', cc_str)[0]

cxx_str = '${CXX} ${CXXFLAGS} ${CPPFLAGS} ${_CXXINCFLAGS} ${_CXXDEFFLAGS} -c ${SRCLST}'
cxx_fun = Task.compile_fun_noshell('batched_cxx', cxx_str)[0]

count = 70000
class batch_task(Task.Task):
	color = 'RED'

	after = 'cc cxx'
	before = 'cc_link cxx_link static_link'

	def __str__(self):
		return '(batch compilation for %d slaves)\n' % len(self.slaves)

	def __init__(self, *k, **kw):
		Task.Task.__init__(self, *k, **kw)
		self.slaves = []
		self.inputs = []
		self.hasrun = 0

		global count
		count += 1
		self.idx = count

	def add_slave(self, slave):
		self.slaves.append(slave)
		self.set_run_after(slave)

	def runnable_status(self):
		for t in self.run_after:
			if not t.hasrun:
				return ASK_LATER

		for t in self.slaves:
			#if t.executed:
			if t.hasrun != SKIPPED:
				return RUN_ME

		return SKIP_ME

	def run(self):
		outputs = []
		self.outputs = []

		srclst = []
		slaves = []
		for t in self.slaves:
			if t.hasrun != SKIPPED:
				slaves.append(t)
				srclst.append(t.inputs[0].abspath(self.env))

		self.env.SRCLST = srclst
		self.cwd = slaves[0].inputs[0].parent.abspath(self.env)

		env = self.env
		app = env.append_unique
		cpppath_st = env['CPPPATH_ST']
		env._CCINCFLAGS = env.CXXINCFLAGS = []

		# local flags come first
		# set the user-defined includes paths
		for i in env['INC_PATHS']:
			app('_CCINCFLAGS', cpppath_st % i.abspath())
			app('_CXXINCFLAGS', cpppath_st % i.abspath())
			app('_CCINCFLAGS', cpppath_st % i.abspath(env))
			app('_CXXINCFLAGS', cpppath_st % i.abspath(env))

		# set the library include paths
		for i in env['CPPPATH']:
			app('_CCINCFLAGS', cpppath_st % i)
			app('_CXXINCFLAGS', cpppath_st % i)

		if self.slaves[0].__class__.__name__ == 'cc':
			ret = cc_fun(self)
		else:
			ret = cxx_fun(self)

		if ret:
			return ret

		for t in slaves:
			t.old_post_run()

from TaskGen import extension, feature, after

import cc, cxx
def wrap(fun):
	def foo(self, node):
		# we cannot control the extension, this sucks
		self.obj_ext = '.o'

		task = fun(self, node)
		if not getattr(self, 'masters', None):
			self.masters = {}
			self.allmasters = []

		if not node.parent.id in self.masters:
			m = self.masters[node.parent.id] = self.master = self.create_task('batch')
			self.allmasters.append(m)
		else:
			m = self.masters[node.parent.id]
			if len(m.slaves) > MAX_BATCH:
				m = self.masters[node.parent.id] = self.master = self.create_task('batch')
				self.allmasters.append(m)

		m.add_slave(task)
		return task
	return foo

c_hook = wrap(cc.c_hook)
extension(cc.EXT_CC)(c_hook)

cxx_hook = wrap(cxx.cxx_hook)
extension(cxx.EXT_CXX)(cxx_hook)


@feature('cprogram', 'cshlib', 'cstaticlib')
@after('apply_link')
def link_after_masters(self):
	if getattr(self, 'allmasters', None):
		for m in self.allmasters:
			self.link_task.set_run_after(m)

for c in ['cc', 'cxx']:
	t = Task.TaskBase.classes[c]
	def run(self):
		pass

	def post_run(self):
		#self.executed=1
		pass

	def can_retrieve_cache(self):
		if self.old_can_retrieve_cache():
			for m in self.generator.allmasters:
				try:
					m.slaves.remove(self)
				except ValueError:
					pass	#this task wasn't included in that master
			return 1
		else:
			return None

	setattr(t, 'oldrun', t.__dict__['run'])
	setattr(t, 'run', run)
	setattr(t, 'old_post_run', t.post_run)
	setattr(t, 'post_run', post_run)
	setattr(t, 'old_can_retrieve_cache', t.can_retrieve_cache)
	setattr(t, 'can_retrieve_cache', can_retrieve_cache)

