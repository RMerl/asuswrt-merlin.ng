#!/usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006 (ita)

"TeX/LaTeX/PDFLaTeX support"

import os, re
import Utils, TaskGen, Task, Runner, Build
from TaskGen import feature, before
from Logs import error, warn, debug

re_tex = re.compile(r'\\(?P<type>include|input|import|bringin|lstinputlisting){(?P<file>[^{}]*)}', re.M)
def scan(self):
	node = self.inputs[0]
	env = self.env

	nodes = []
	names = []
	if not node: return (nodes, names)

	code = Utils.readf(node.abspath(env))

	curdirnode = self.curdirnode
	abs = curdirnode.abspath()
	for match in re_tex.finditer(code):
		path = match.group('file')
		if path:
			for k in ['', '.tex', '.ltx']:
				# add another loop for the tex include paths?
				debug('tex: trying %s%s' % (path, k))
				try:
					os.stat(abs+os.sep+path+k)
				except OSError:
					continue
				found = path+k
				node = curdirnode.find_resource(found)
				if node:
					nodes.append(node)
			else:
				debug('tex: could not find %s' % path)
				names.append(path)

	debug("tex: found the following : %s and names %s" % (nodes, names))
	return (nodes, names)

latex_fun, _ = Task.compile_fun('latex', '${LATEX} ${LATEXFLAGS} ${SRCFILE}', shell=False)
pdflatex_fun, _ = Task.compile_fun('pdflatex', '${PDFLATEX} ${PDFLATEXFLAGS} ${SRCFILE}', shell=False)
bibtex_fun, _ = Task.compile_fun('bibtex', '${BIBTEX} ${BIBTEXFLAGS} ${SRCFILE}', shell=False)
makeindex_fun, _ = Task.compile_fun('bibtex', '${MAKEINDEX} ${MAKEINDEXFLAGS} ${SRCFILE}', shell=False)

g_bibtex_re = re.compile('bibdata', re.M)
def tex_build(task, command='LATEX'):
	env = task.env
	bld = task.generator.bld

	if not env['PROMPT_LATEX']:
		env.append_value('LATEXFLAGS', '-interaction=batchmode')
		env.append_value('PDFLATEXFLAGS', '-interaction=batchmode')

	fun = latex_fun
	if command == 'PDFLATEX':
		fun = pdflatex_fun

	node = task.inputs[0]
	reldir  = node.bld_dir(env)

	#lst = []
	#for c in Utils.split_path(reldir):
	#	if c: lst.append('..')
	#srcfile = os.path.join(*(lst + [node.srcpath(env)]))
	#sr2 = os.path.join(*(lst + [node.parent.srcpath(env)]))
	srcfile = node.abspath(env)
	sr2 = node.parent.abspath() + os.pathsep + node.parent.abspath(env) + os.pathsep

	aux_node = node.change_ext('.aux')
	idx_node = node.change_ext('.idx')

	nm = aux_node.name
	docuname = nm[ : len(nm) - 4 ] # 4 is the size of ".aux"

	# important, set the cwd for everybody
	task.cwd = task.inputs[0].parent.abspath(task.env)


	warn('first pass on %s' % command)

	task.env.env = {'TEXINPUTS': sr2}
	task.env.SRCFILE = srcfile
	ret = fun(task)
	if ret:
		return ret

	# look in the .aux file if there is a bibfile to process
	try:
		ct = Utils.readf(aux_node.abspath(env))
	except (OSError, IOError):
		error('error bibtex scan')
	else:
		fo = g_bibtex_re.findall(ct)

		# there is a .aux file to process
		if fo:
			warn('calling bibtex')

			task.env.env = {'BIBINPUTS': sr2, 'BSTINPUTS': sr2}
			task.env.SRCFILE = docuname
			ret = bibtex_fun(task)
			if ret:
				error('error when calling bibtex %s' % docuname)
				return ret

	# look on the filesystem if there is a .idx file to process
	try:
		idx_path = idx_node.abspath(env)
		os.stat(idx_path)
	except OSError:
		error('error file.idx scan')
	else:
		warn('calling makeindex')

		task.env.SRCFILE = idx_node.name
		task.env.env = {}
		ret = makeindex_fun(task)
		if ret:
			error('error when calling makeindex %s' % idx_path)
			return ret


	hash = ''
	i = 0
	while i < 10:
		# prevent against infinite loops - one never knows
		i += 1

		# watch the contents of file.aux
		prev_hash = hash
		try:
			hash = Utils.h_file(aux_node.abspath(env))
		except KeyError:
			error('could not read aux.h -> %s' % aux_node.abspath(env))
			pass

		# debug
		#print "hash is, ", hash, " ", old_hash

		# stop if file.aux does not change anymore
		if hash and hash == prev_hash:
			break

		# run the command
		warn('calling %s' % command)

		task.env.env = {'TEXINPUTS': sr2 + os.pathsep}
		task.env.SRCFILE = srcfile
		ret = fun(task)
		if ret:
			error('error when calling %s %s' % (command, latex_compile_cmd))
			return ret

	return None # ok

latex_vardeps  = ['LATEX', 'LATEXFLAGS']
def latex_build(task):
	return tex_build(task, 'LATEX')

pdflatex_vardeps  = ['PDFLATEX', 'PDFLATEXFLAGS']
def pdflatex_build(task):
	return tex_build(task, 'PDFLATEX')

class tex_taskgen(TaskGen.task_gen):
	def __init__(self, *k, **kw):
		TaskGen.task_gen.__init__(self, *k, **kw)

@feature('tex')
@before('apply_core')
def apply_tex(self):
	if not getattr(self, 'type', None) in ['latex', 'pdflatex']:
		self.type = 'pdflatex'

	tree = self.bld
	outs = Utils.to_list(getattr(self, 'outs', []))

	# prompt for incomplete files (else the batchmode is used)
	self.env['PROMPT_LATEX'] = getattr(self, 'prompt', 1)

	deps_lst = []

	if getattr(self, 'deps', None):
		deps = self.to_list(self.deps)
		for filename in deps:
			n = self.path.find_resource(filename)
			if not n in deps_lst: deps_lst.append(n)

	self.source = self.to_list(self.source)
	for filename in self.source:
		base, ext = os.path.splitext(filename)

		node = self.path.find_resource(filename)
		if not node: raise Utils.WafError('cannot find %s' % filename)

		if self.type == 'latex':
			task = self.create_task('latex', node, node.change_ext('.dvi'))
		elif self.type == 'pdflatex':
			task = self.create_task('pdflatex', node, node.change_ext('.pdf'))

		task.env = self.env
		task.curdirnode = self.path

		# add the manual dependencies
		if deps_lst:
			variant = node.variant(self.env)
			try:
				lst = tree.node_deps[task.unique_id()]
				for n in deps_lst:
					if not n in lst:
						lst.append(n)
			except KeyError:
				tree.node_deps[task.unique_id()] = deps_lst

		if self.type == 'latex':
			if 'ps' in outs:
				tsk = self.create_task('dvips', task.outputs, node.change_ext('.ps'))
				tsk.env.env = {'TEXINPUTS' : node.parent.abspath() + os.pathsep + self.path.abspath() + os.pathsep + self.path.abspath(self.env)}
			if 'pdf' in outs:
				tsk = self.create_task('dvipdf', task.outputs, node.change_ext('.pdf'))
				tsk.env.env = {'TEXINPUTS' : node.parent.abspath() + os.pathsep + self.path.abspath() + os.pathsep + self.path.abspath(self.env)}
		elif self.type == 'pdflatex':
			if 'ps' in outs:
				self.create_task('pdf2ps', task.outputs, node.change_ext('.ps'))
	self.source = []

def detect(conf):
	v = conf.env
	for p in 'tex latex pdflatex bibtex dvips dvipdf ps2pdf makeindex pdf2ps'.split():
		conf.find_program(p, var=p.upper())
		v[p.upper()+'FLAGS'] = ''
	v['DVIPSFLAGS'] = '-Ppdf'

b = Task.simple_task_type
b('tex', '${TEX} ${TEXFLAGS} ${SRC}', color='BLUE', shell=False) # not used anywhere
b('bibtex', '${BIBTEX} ${BIBTEXFLAGS} ${SRC}', color='BLUE', shell=False) # not used anywhere
b('dvips', '${DVIPS} ${DVIPSFLAGS} ${SRC} -o ${TGT}', color='BLUE', after="latex pdflatex tex bibtex", shell=False)
b('dvipdf', '${DVIPDF} ${DVIPDFFLAGS} ${SRC} ${TGT}', color='BLUE', after="latex pdflatex tex bibtex", shell=False)
b('pdf2ps', '${PDF2PS} ${PDF2PSFLAGS} ${SRC} ${TGT}', color='BLUE', after="dvipdf pdflatex", shell=False)

b = Task.task_type_from_func
cls = b('latex', latex_build, vars=latex_vardeps)
cls.scan = scan
cls = b('pdflatex', pdflatex_build, vars=pdflatex_vardeps)
cls.scan = scan

