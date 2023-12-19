#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2007-2010 (ita)

"""
debugging helpers for parallel compilation, outputs
a svg file in the build directory
"""

import os, time, sys, threading
try: from Queue import Queue
except: from queue import Queue
import Runner, Options, Utils, Task, Logs
from Constants import *

#import random
#random.seed(100)

def set_options(opt):
	opt.add_option('--dtitle', action='store', default='Parallel build representation for %r' % ' '.join(sys.argv),
		help='title for the svg diagram', dest='dtitle')
	opt.add_option('--dwidth', action='store', type='int', help='diagram width', default=1000, dest='dwidth')
	opt.add_option('--dtime', action='store', type='float', help='recording interval in seconds', default=0.009, dest='dtime')
	opt.add_option('--dband', action='store', type='int', help='band width', default=22, dest='dband')
	opt.add_option('--dmaxtime', action='store', type='float', help='maximum time, for drawing fair comparisons', default=0, dest='dmaxtime')

# red   #ff4d4d
# green #4da74d
# lila  #a751ff

color2code = {
	'GREEN'  : '#4da74d',
	'YELLOW' : '#fefe44',
	'PINK'   : '#a751ff',
	'RED'    : '#cc1d1d',
	'BLUE'   : '#6687bb',
	'CYAN'   : '#34e2e2',

}

mp = {}
info = [] # list of (text,color)

def map_to_color(name):
	if name in mp:
		return mp[name]
	try:
		cls = Task.TaskBase.classes[name]
	except KeyError:
		return color2code['RED']
	if cls.color in mp:
		return mp[cls.color]
	if cls.color in color2code:
		return color2code[cls.color]
	return color2code['RED']

def loop(self):
	while 1:
		tsk=Runner.TaskConsumer.ready.get()
		tsk.master.set_running(1, id(threading.currentThread()), tsk)
		Runner.process_task(tsk)
		tsk.master.set_running(-1, id(threading.currentThread()), tsk)
Runner.TaskConsumer.loop = loop


old_start = Runner.Parallel.start
def do_start(self):
        print Options.options
	try:
		Options.options.dband
	except AttributeError:
		raise ValueError('use def options(opt): opt.load("parallel_debug")!')

	self.taskinfo = Queue()
	old_start(self)
	process_colors(self)
Runner.Parallel.start = do_start

def set_running(self, by, i, tsk):
	self.taskinfo.put( (i, id(tsk), time.time(), tsk.__class__.__name__, self.processed, self.count, by)  )
Runner.Parallel.set_running = set_running

def name2class(name):
	return name.replace(' ', '_').replace('.', '_')

def process_colors(producer):
	# first, cast the parameters
	tmp = []
	try:
		while True:
			tup = producer.taskinfo.get(False)
			tmp.append(list(tup))
	except:
		pass

	try:
		ini = float(tmp[0][2])
	except:
		return

	if not info:
		seen = []
		for x in tmp:
			name = x[3]
			if not name in seen:
				seen.append(name)
			else:
				continue

			info.append((name, map_to_color(name)))
		info.sort(key=lambda x: x[0])

	thread_count = 0
	acc = []
	for x in tmp:
		thread_count += x[6]
		acc.append("%d %d %f %r %d %d %d" % (x[0], x[1], x[2] - ini, x[3], x[4], x[5], thread_count))
	f = open('pdebug.dat', 'w')
	#Utils.write('\n'.join(acc))
	f.write('\n'.join(acc))

	tmp = [lst[:2] + [float(lst[2]) - ini] + lst[3:] for lst in tmp]

	st = {}
	for l in tmp:
		if not l[0] in st:
			st[l[0]] = len(st.keys())
	tmp = [  [st[lst[0]]] + lst[1:] for lst in tmp ]
	THREAD_AMOUNT = len(st.keys())

	st = {}
	for l in tmp:
		if not l[1] in st:
			st[l[1]] = len(st.keys())
	tmp = [  [lst[0]] + [st[lst[1]]] + lst[2:] for lst in tmp ]


	BAND = Options.options.dband

	seen = {}
	acc = []
	for x in range(len(tmp)):
		line = tmp[x]
		id = line[1]

		if id in seen:
			continue
		seen[id] = True

		begin = line[2]
		thread_id = line[0]
		for y in range(x + 1, len(tmp)):
			line = tmp[y]
			if line[1] == id:
				end = line[2]
				#print id, thread_id, begin, end
				#acc.append(  ( 10*thread_id, 10*(thread_id+1), 10*begin, 10*end ) )
				acc.append( (BAND * begin, BAND*thread_id, BAND*end - BAND*begin, BAND, line[3]) )
				break

	if Options.options.dmaxtime < 0.1:
		gwidth = 1
		for x in tmp:
			m = BAND * x[2]
			if m > gwidth:
				gwidth = m
	else:
		gwidth = BAND * Options.options.dmaxtime

	ratio = float(Options.options.dwidth) / gwidth
	gwidth = Options.options.dwidth

	gheight = BAND * (THREAD_AMOUNT + len(info) + 1.5)

	out = []

	out.append("""<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>
<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"
\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">
<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.0\"
   x=\"%r\" y=\"%r\" width=\"%r\" height=\"%r\"
   id=\"svg602\" xml:space=\"preserve\">

<style type='text/css' media='screen'>
    g.over rect  { stroke:#FF0000; fill-opacity:0.4 }
</style>

<script type='text/javascript'><![CDATA[
    var svg  = document.getElementsByTagName('svg')[0];
    var svgNS = svg.getAttribute('xmlns');
    svg.addEventListener('mouseover',function(e){
      var g = e.target.parentNode;
      var x = document.getElementById('r_'+g.id);
      if (x) {
         g.setAttribute('class', g.getAttribute('class')+' over');
         x.setAttribute('class', x.getAttribute('class')+' over');
         showInfo(e, g.id);
      }
    },false);
    svg.addEventListener('mouseout',function(e){
      var g = e.target.parentNode;
      var x = document.getElementById('r_'+g.id);
      if (x) {
         g.setAttribute('class',g.getAttribute('class').replace(' over',''));
         x.setAttribute('class',x.getAttribute('class').replace(' over',''));
         hideInfo(e);
      }
    },false);

function showInfo(evt, txt) {
    tooltip = document.getElementById('tooltip');

    var t = document.getElementById('tooltiptext');
    t.firstChild.data = txt;

    var x = evt.clientX+10;
    if (x > 200) { x -= t.getComputedTextLength() + 16; }
    var y = evt.clientY+30;
    tooltip.setAttribute("transform", "translate(" + x + "," + y + ")");
    tooltip.setAttributeNS(null,"visibility","visible");

    var r = document.getElementById('tooltiprect');
    r.setAttribute('width', t.getComputedTextLength()+6)
}


function hideInfo(evt) {
    tooltip = document.getElementById('tooltip');
    tooltip.setAttributeNS(null,"visibility","hidden");
}

]]></script>

<!-- inkscape requires a big rectangle or it will not export the pictures properly -->
<rect
   x='%r' y='%r'
   width='%r' height='%r' z-index='10'
   style=\"font-size:10;fill:#ffffff;fill-opacity:0.01;fill-rule:evenodd;stroke:#ffffff;\"
   />\n

""" % (0, 0, gwidth + 4, gheight + 4,   0, 0, gwidth + 4, gheight + 4))

	# main title
	if Options.options.dtitle:
		out.append("""<text x="%d" y="%d" style="font-size:15px; text-anchor:middle; font-style:normal;font-weight:normal;fill:#000000;fill-opacity:1;stroke:none;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1;font-family:Bitstream Vera Sans">%s</text>
""" % (gwidth/2, gheight - 5, Options.options.dtitle))

	# the rectangles
	groups = {}
	for (x, y, w, h, clsname) in acc:
		try:
			groups[clsname].append((x, y, w, h))
		except:
			groups[clsname] = [(x, y, w, h)]

	for cls in groups:

		out.append("<g id='%s'>\n" % name2class(cls))

		for (x, y, w, h) in groups[cls]:
			out.append("""   <rect
   x='%r' y='%r'
   width='%r' height='%r' z-index='11'
   style=\"font-size:10;fill:%s;fill-rule:evenodd;stroke:#000000;stroke-width:0.2px;\"
   />\n""" % (2 + x*ratio, 2 + y, w*ratio, h, map_to_color(cls)))

		out.append("</g>\n")

	# output the caption
	cnt = THREAD_AMOUNT

	for (text, color) in info:
		# caption box
		b = BAND/2
		out.append("""<g id='r_%s'><rect
		x='%r' y='%r'
		width='%r' height='%r'
		style=\"font-size:10;fill:%s;fill-rule:evenodd;stroke:#000000;stroke-width:0.2px;\"
  />\n""" %                       (name2class(text), 2 + BAND,     5 + (cnt + 0.5) * BAND, b, b, color))

		# caption text
		out.append("""<text
   style="font-size:12px;font-style:normal;font-weight:normal;fill:#000000;fill-opacity:1;stroke:none;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1;font-family:Bitstream Vera Sans"
   x="%r" y="%d">%s</text></g>\n""" % (2 + 2 * BAND, 5 + (cnt + 0.5) * BAND + 10, text))
		cnt += 1

	out.append("""
<g transform="translate(0,0)" visibility="hidden" id="tooltip">
  <rect id="tooltiprect" y="-15" x="-3" width="1" height="20" style="stroke:black;fill:#edefc2;stroke-width:1"/>
  <text id="tooltiptext" style="font-family:Arial; font-size:12;fill:black;"> </text>
</g>""")

	out.append("\n</svg>")

	#node = producer.bld.path.make_node('pdebug.svg')
	f = open('pdebug.svg', 'w')
	f.write("".join(out))


