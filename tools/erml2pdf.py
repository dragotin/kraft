#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# erml2pdf - An RML to PDF converter with extended features
# Copyright (C) 2003, Fabien Pinckaers, UCL, FSA
# Contributors
#     Richard Waid <richard@iopen.net>
#     Klaas Freitag <freitag@kde.org>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import sys
import StringIO
import xml.dom.minidom
import copy
import subprocess
import shlex
import tempfile
import getopt
import re

import reportlab
from reportlab.pdfgen import canvas
from reportlab import platypus
from reportlab.lib import colors

from pyPdf import PdfFileWriter, PdfFileReader


#
# Change this to UTF-8 if you plan tu use Reportlab's UTF-8 support
#
# encoding = 'latin1'
# use utf8 for default
encoding = 'UTF-8'

#
# from previous file util.py, imported for simplicity:
def text_get(node):
        rc = ''
        for node in node.childNodes:
                if node.nodeType == node.TEXT_NODE:
                        rc = rc + node.data
        return rc

units = [
        (re.compile('^(-?[0-9\.]+)\s*in$'), reportlab.lib.units.inch),
        (re.compile('^(-?[0-9\.]+)\s*cm$'), reportlab.lib.units.cm),
        (re.compile('^(-?[0-9\.]+)\s*mm$'), reportlab.lib.units.mm),
        (re.compile('^(-?[0-9\.]+)\s*$'), 1)
]

def unit_get(size):
        global units
        for unit in units:
                res = unit[0].search(size, 0)
                if res:
                        return unit[1]*float(res.group(1))
        return False

def tuple_int_get(node, attr_name, default=None):
        if not node.hasAttribute(attr_name):
                return default
        res = [int(x) for x in node.getAttribute(attr_name).split(',')]
        return res

def bool_get(value):
        return (str(value)=="1") or (value.lower()=='yes')

def attr_get(node, attrs, dict={}):
        res = {}
        for name in attrs:
                if node.hasAttribute(name):
                        res[name] =  unit_get(node.getAttribute(name))
        for key in dict:
                if node.hasAttribute(key):
                        if dict[key]=='str':
                                res[key] = str(node.getAttribute(key))
                        elif dict[key]=='bool':
                                res[key] = bool_get(node.getAttribute(key))
                        elif dict[key]=='int':
                                res[key] = int(node.getAttribute(key))
        return res
# from previous file color.py, imported for simplicity:

allcols = colors.getAllNamedColors()

regex_t = re.compile('\(([0-9\.]*),([0-9\.]*),([0-9\.]*)\)')
regex_h = re.compile('#([0-9a-zA-Z][0-9a-zA-Z])([0-9a-zA-Z][0-9a-zA-Z])([0-9a-zA-Z][0-9a-zA-Z])')

def color_get(col_str):
        global allcols
        if col_str in allcols.keys():
                return allcols[col_str]
        res = regex_t.search(col_str, 0)
        if res:
                return (float(res.group(1)),float(res.group(2)),float(res.group(3)))
        res = regex_h.search(col_str, 0)
        if res:
                return tuple([ float(int(res.group(i),16))/255 for i in range(1,4)])
        return colors.red
 
#
# original trml2pdf starts here:
#

def _child_get(node, childs):
	clds = []
	for n in node.childNodes:
		if (n.nodeType==n.ELEMENT_NODE) and (n.localName==childs):
			clds.append(n)
	return clds

class _rml_styles(object):
	def __init__(self, nodes):
		self.styles = {}
		self.names = {}
		self.table_styles = {}
		for node in nodes:
			for style in node.getElementsByTagName('blockTableStyle'):
				self.table_styles[style.getAttribute('id')] = self._table_style_get(style)
			for style in node.getElementsByTagName('paraStyle'):
				self.styles[style.getAttribute('name')] = self._para_style_get(style)
			for variable in node.getElementsByTagName('initialize'):
				for name in variable.getElementsByTagName('name'):
					self.names[ name.getAttribute('id')] = name.getAttribute('value')

	def _para_style_update(self, style, node):
		for attr in ['textColor', 'backColor', 'bulletColor']:
			if node.hasAttribute(attr):
				style.__dict__[attr] = color_get(node.getAttribute(attr))
		for attr in ['fontName', 'bulletFontName', 'bulletText']:
			if node.hasAttribute(attr):
				style.__dict__[attr] = node.getAttribute(attr)
		for attr in ['fontSize', 'leftIndent', 'rightIndent', 'spaceBefore', 'spaceAfter', 'firstLineIndent', 'bulletIndent', 'bulletFontSize', 'leading']:
			if node.hasAttribute(attr):
				style.__dict__[attr] = unit_get(node.getAttribute(attr))
		if node.hasAttribute('alignment'):
			align = {
				'right':reportlab.lib.enums.TA_RIGHT,
				'center':reportlab.lib.enums.TA_CENTER,
				'justify':reportlab.lib.enums.TA_JUSTIFY
			}
			style.alignment = align.get(node.getAttribute('alignment').lower(), reportlab.lib.enums.TA_LEFT)
		return style

	def _table_style_get(self, style_node):
		styles = []
		for node in style_node.childNodes:
			if node.nodeType==node.ELEMENT_NODE:
				start = tuple_int_get(node, 'start', (0,0) )
				stop = tuple_int_get(node, 'stop', (-1,-1) )
				if node.localName=='blockValign':
					styles.append(('VALIGN', start, stop, str(node.getAttribute('value'))))
				elif node.localName=='blockFont':
					styles.append(('FONT', start, stop, str(node.getAttribute('name'))))
                                elif node.localName=='blockSpan':
                                        styles.append(('SPAN', start, stop))
				elif node.localName=='blockTextColor':
					styles.append(('TEXTCOLOR', start, stop, color_get(str(node.getAttribute('colorName')))))
				elif node.localName=='blockLeading':
					styles.append(('LEADING', start, stop, unit_get(node.getAttribute('length'))))
				elif node.localName=='blockAlignment':
					styles.append(('ALIGNMENT', start, stop, str(node.getAttribute('value'))))
				elif node.localName=='blockLeftPadding':
					styles.append(('LEFTPADDING', start, stop, unit_get(node.getAttribute('length'))))
				elif node.localName=='blockRightPadding':
					styles.append(('RIGHTPADDING', start, stop, unit_get(node.getAttribute('length'))))
				elif node.localName=='blockTopPadding':
					styles.append(('TOPPADDING', start, stop, unit_get(node.getAttribute('length'))))
				elif node.localName=='blockBottomPadding':
					styles.append(('BOTTOMPADDING', start, stop, unit_get(node.getAttribute('length'))))
				elif node.localName=='blockBackground':
					styles.append(('BACKGROUND', start, stop, color_get(node.getAttribute('colorName'))))
				if node.hasAttribute('size'):
					styles.append(('FONTSIZE', start, stop, unit_get(node.getAttribute('size'))))
				elif node.localName=='lineStyle':
					kind = node.getAttribute('kind')
					kind_list = [ 'GRID', 'BOX', 'OUTLINE', 'INNERGRID', 'LINEBELOW', 'LINEABOVE','LINEBEFORE', 'LINEAFTER' ]
					assert kind in kind_list
					thick = 1
					if node.hasAttribute('thickness'):
						thick = float(node.getAttribute('thickness'))
					styles.append((kind, start, stop, thick,  color_get(node.getAttribute('colorName'))))
		return platypus.tables.TableStyle(styles)

	def _para_style_get(self, node):
		styles = reportlab.lib.styles.getSampleStyleSheet()
		style = copy.deepcopy(styles["Normal"])
		self._para_style_update(style, node)
		return style

	def para_style_get(self, node):
		style = False
		if node.hasAttribute('style'):
			if node.getAttribute('style') in self.styles:
				style = copy.deepcopy(self.styles[node.getAttribute('style')])
			else:
				sys.stderr.write('Warning: style not found, %s - setting default!\n' % (node.getAttribute('style'),) )
		if not style:
			styles = reportlab.lib.styles.getSampleStyleSheet()
			style = copy.deepcopy(styles['Normal'])
		return self._para_style_update(style, node)

class _rml_doc(object):
	def __init__(self, data):
		self.dom = xml.dom.minidom.parseString(data)
		self.filename = self.dom.documentElement.getAttribute('filename')

	def docinit(self, els):
		from reportlab.lib.fonts import addMapping
		from reportlab.pdfbase import pdfmetrics
		from reportlab.pdfbase.ttfonts import TTFont

		for node in els:
			for font in node.getElementsByTagName('registerFont'):
				name = font.getAttribute('fontName').encode('ascii')
				fname = font.getAttribute('fontFile').encode('ascii')
				pdfmetrics.registerFont(TTFont(name, fname ))
				addMapping(name, 0, 0, name)    #normal
				addMapping(name, 0, 1, name)    #italic
				addMapping(name, 1, 0, name)    #bold
				addMapping(name, 1, 1, name)    #italic and bold

	def render(self, out):
		el = self.dom.documentElement.getElementsByTagName('docinit')
		if el:
			self.docinit(el)

		el = self.dom.documentElement.getElementsByTagName('stylesheet')
		self.styles = _rml_styles(el)

		el = self.dom.documentElement.getElementsByTagName('template')
		if len(el):
			pt_obj = _rml_template(out, el[0], self)
			pt_obj.render(self.dom.documentElement.getElementsByTagName('story')[0])
		else:
			self.canvas = canvas.Canvas(out)
			pd = self.dom.documentElement.getElementsByTagName('pageDrawing')[0]
			pd_obj = _rml_canvas(self.canvas, None, self)
			pd_obj.render(pd)
			self.canvas.showPage()
			self.canvas.save()

class _rml_canvas(object):
	def __init__(self, canvas, doc_tmpl=None, doc=None):
		self.canvas = canvas
		self.styles = doc.styles
		self.doc_tmpl = doc_tmpl
		self.doc = doc

	def _textual(self, node):
		rc = ''
		for n in node.childNodes:
			if n.nodeType == n.ELEMENT_NODE:
				if n.localName=='pageNumber':
					rc += str(self.canvas.getPageNumber())
			elif (n.nodeType == node.CDATA_SECTION_NODE):
				rc += n.data
			elif (n.nodeType == node.TEXT_NODE):
				rc += n.data
		return rc.encode(encoding)

	def _drawString(self, node):
		self.canvas.drawString(text=self._textual(node), **attr_get(node, ['x','y']))
	def _drawCenteredString(self, node):
		self.canvas.drawCentredString(text=self._textual(node), **attr_get(node, ['x','y']))
	def _drawRightString(self, node):
		self.canvas.drawRightString(text=self._textual(node), **attr_get(node, ['x','y']))
	def _rect(self, node):
		if node.hasAttribute('round'):
			self.canvas.roundRect(radius=unit_get(node.getAttribute('round')), **attr_get(node, ['x','y','width','height'], {'fill':'bool','stroke':'bool'}))
		else:
			self.canvas.rect(**attr_get(node, ['x','y','width','height'], {'fill':'bool','stroke':'bool'}))
	def _ellipse(self, node):
		x1 = unit_get(node.getAttribute('x'))
		x2 = unit_get(node.getAttribute('width'))
		y1 = unit_get(node.getAttribute('y'))
		y2 = unit_get(node.getAttribute('height'))
		self.canvas.ellipse(x1,y1,x2,y2, **attr_get(node, [], {'fill':'bool','stroke':'bool'}))
	def _curves(self, node):
		line_str = text_get(node).split()
		lines = []
		while len(line_str)>7:
			self.canvas.bezier(*[unit_get(l) for l in line_str[0:8]])
			line_str = line_str[8:]
	def _lines(self, node):
		line_str = text_get(node).split()
		lines = []
		while len(line_str)>3:
			lines.append([unit_get(l) for l in line_str[0:4]])
			line_str = line_str[4:]
		self.canvas.lines(lines)
	def _grid(self, node):
		xlist = [unit_get(s) for s in node.getAttribute('xs').split(',')]
		ylist = [unit_get(s) for s in node.getAttribute('ys').split(',')]
		self.canvas.grid(xlist, ylist)
	def _translate(self, node):
		dx = 0
		dy = 0
		if node.hasAttribute('dx'):
			dx = unit_get(node.getAttribute('dx'))
		if node.hasAttribute('dy'):
			dy = unit_get(node.getAttribute('dy'))
		self.canvas.translate(dx,dy)

	def _circle(self, node):
		self.canvas.circle(x_cen=unit_get(node.getAttribute('x')), y_cen=unit_get(node.getAttribute('y')), r=unit_get(node.getAttribute('radius')), **attr_get(node, [], {'fill':'bool','stroke':'bool'}))

	def _place(self, node):
		flows = _rml_flowable(self.doc).render(node)
		infos = attr_get(node, ['x','y','width','height'])

		infos['y']+=infos['height']
		for flow in flows:
			w,h = flow.wrap(infos['width'], infos['height'])
			if w<=infos['width'] and h<=infos['height']:
				infos['y']-=h
				flow.drawOn(self.canvas,infos['x'],infos['y'])
				infos['height']-=h
			else:
				raise ValueError, "Not enough space"

	def _line_mode(self, node):
		ljoin = {'round':1, 'mitered':0, 'bevelled':2}
		lcap = {'default':0, 'round':1, 'square':2}
		if node.hasAttribute('width'):
			self.canvas.setLineWidth(unit_get(node.getAttribute('width')))
		if node.hasAttribute('join'):
			self.canvas.setLineJoin(ljoin[node.getAttribute('join')])
		if node.hasAttribute('cap'):
			self.canvas.setLineCap(lcap[node.getAttribute('cap')])
		if node.hasAttribute('miterLimit'):
			self.canvas.setDash(unit_get(node.getAttribute('miterLimit')))
		if node.hasAttribute('dash'):
			dashes = node.getAttribute('dash').split(',')
			for x in range(len(dashes)):
				dashes[x]=unit_get(dashes[x])
			self.canvas.setDash(node.getAttribute('dash').split(','))

	def _image(self, node):
		import urllib
		from reportlab.lib.utils import ImageReader
		u = urllib.urlopen(str(node.getAttribute('file')))
		s = StringIO.StringIO()
		s.write(u.read())
		s.seek(0)
		img = ImageReader(s)
		(sx,sy) = img.getSize()

		args = {}
		for tag in ('width','height','x','y'):
			if node.hasAttribute(tag):
				args[tag] = unit_get(node.getAttribute(tag))
		if ('width' in args) and (not 'height' in args):
			args['height'] = sy * args['width'] / sx
		elif ('height' in args) and (not 'width' in args):
			args['width'] = sx * args['height'] / sy
		elif ('width' in args) and ('height' in args):
			if (float(args['width'])/args['height'])>(float(sx)>sy):
				args['width'] = sx * args['height'] / sy
			else:
				args['height'] = sy * args['width'] / sx
		self.canvas.drawImage(img, **args)

	def _path(self, node):
		self.path = self.canvas.beginPath()
		self.path.moveTo(**attr_get(node, ['x','y']))
		for n in node.childNodes:
			if n.nodeType == node.ELEMENT_NODE:
				if n.localName=='moveto':
					vals = text_get(n).split()
					self.path.moveTo(unit_get(vals[0]), unit_get(vals[1]))
				elif n.localName=='curvesto':
					vals = text_get(n).split()
					while len(vals)>5:
						pos=[]
						while len(pos)<6:
							pos.append(unit_get(vals.pop(0)))
						self.path.curveTo(*pos)
			elif (n.nodeType == node.TEXT_NODE):
				data = n.data.split()               # Not sure if I must merge all TEXT_NODE ?
				while len(data)>1:
					x = unit_get(data.pop(0))
					y = unit_get(data.pop(0))
					self.path.lineTo(x,y)
		if (not node.hasAttribute('close')) or bool_get(node.getAttribute('close')):
			self.path.close()
		self.canvas.drawPath(self.path, **attr_get(node, [], {'fill':'bool','stroke':'bool'}))

	def render(self, node):
		tags = {
			'drawCentredString': self._drawCenteredString,
			'drawRightString': self._drawRightString,
			'drawString': self._drawString,
			'rect': self._rect,
			'ellipse': self._ellipse,
			'lines': self._lines,
			'grid': self._grid,
			'curves': self._curves,
			'fill': lambda node: self.canvas.setFillColor(color_get(node.getAttribute('color'))),
			'stroke': lambda node: self.canvas.setStrokeColor(color_get(node.getAttribute('color'))),
			'setFont': lambda node: self.canvas.setFont(node.getAttribute('name'), unit_get(node.getAttribute('size'))),
			'place': self._place,
			'circle': self._circle,
			'lineMode': self._line_mode,
			'path': self._path,
			'rotate': lambda node: self.canvas.rotate(float(node.getAttribute('degrees'))),
			'translate': self._translate,
			'image': self._image
		}
		for nd in node.childNodes:
			if nd.nodeType==nd.ELEMENT_NODE:
				for tag in tags:
					if nd.localName==tag:
						tags[tag](nd)
						break

class _rml_draw(object):
	def __init__(self, node, styles):
		self.node = node
		self.styles = styles
		self.canvas = None

	def render(self, canvas, doc):
		canvas.saveState()
		cnv = _rml_canvas(canvas, doc, self.styles)
		cnv.render(self.node)
		canvas.restoreState()

class _rml_flowable(object):
	def __init__(self, doc):
		self.doc = doc
		self.styles = doc.styles

	def _textual(self, node):
		rc = ''
		for n in node.childNodes:
			if n.nodeType == node.ELEMENT_NODE:
				if n.localName=='getName':
					newNode = self.doc.dom.createTextNode(self.styles.names.get(n.getAttribute('id'),'Unknown name'))
					node.insertBefore(newNode, n)
					node.removeChild(n)
				if n.localName=='pageNumber':
					rc+='<pageNumber/>'            # TODO: change this !
				else:
					self._textual(n)
				rc += n.toxml()
			elif (n.nodeType == node.CDATA_SECTION_NODE):
				rc += n.data
			elif (n.nodeType == node.TEXT_NODE):
				rc += n.toxml()
		return rc.encode(encoding)

	def _table(self, node):
		length = 0
		colwidths = None
		rowheights = None
		data = []
		for tr in _child_get(node,'tr'):
			data2 = []
			for td in _child_get(tr, 'td'):
				flow = []
				for n in td.childNodes:
					if n.nodeType==node.ELEMENT_NODE:
						flow.append( self._flowable(n) )
				if not len(flow):
					flow = self._textual(td)
				data2.append( flow )
			if len(data2)>length:
				length=len(data2)
				for ab in data:
					while len(ab)<length:
						ab.append('')
			while len(data2)<length:
				data2.append('')
			data.append( data2 )
		if node.hasAttribute('colWidths'):
			assert length == len(node.getAttribute('colWidths').split(','))
			colwidths = [unit_get(f.strip()) for f in node.getAttribute('colWidths').split(',')]
		if node.hasAttribute('rowHeights'):
			rowheights = [unit_get(f.strip()) for f in node.getAttribute('rowHeights').split(',')]
		table = platypus.Table(data = data, colWidths=colwidths, rowHeights=rowheights, **(attr_get(node, ['splitByRow'] ,{'repeatRows':'int','repeatCols':'int'})))
		if node.hasAttribute('style'):
			table.setStyle(self.styles.table_styles[node.getAttribute('style')])
		return table

	def _illustration(self, node):
		class Illustration(platypus.flowables.Flowable):
			def __init__(self, node, styles):
				self.node = node
				self.styles = styles
				self.width = unit_get(node.getAttribute('width'))
				self.height = unit_get(node.getAttribute('height'))
			def wrap(self, *args):
				return (self.width, self.height)
			def draw(self):
				canvas = self.canv
				drw = _rml_draw(self.node, self.styles)
				drw.render(self.canv, None)
		return Illustration(node, self.styles)

	def _flowable(self, node):
		if node.localName=='para':
			style = self.styles.para_style_get(node)
			return platypus.Paragraph(self._textual(node), style, **(attr_get(node, [], {'bulletText':'str'})))
		elif node.localName=='name':
			self.styles.names[ node.getAttribute('id')] = node.getAttribute('value')
			return None
		elif node.localName=='xpre':
			style = self.styles.para_style_get(node)
			return platypus.XPreformatted(self._textual(node), style, **(attr_get(node, [], {'bulletText':'str','dedent':'int','frags':'int'})))
		elif node.localName=='pre':
			style = self.styles.para_style_get(node)
			return platypus.Preformatted(self._textual(node), style, **(attr_get(node, [], {'bulletText':'str','dedent':'int'})))
		elif node.localName=='illustration':
			return  self._illustration(node)
		elif node.localName=='blockTable':
			return  self._table(node)
		elif node.localName=='title':
			styles = reportlab.lib.styles.getSampleStyleSheet()
			style = styles['Title']
			return platypus.Paragraph(self._textual(node), style, **(attr_get(node, [], {'bulletText':'str'})))
		elif node.localName=='h1':
			styles = reportlab.lib.styles.getSampleStyleSheet()
			style = styles['Heading1']
			return platypus.Paragraph(self._textual(node), style, **(attr_get(node, [], {'bulletText':'str'})))
		elif node.localName=='h2':
			styles = reportlab.lib.styles.getSampleStyleSheet()
			style = styles['Heading2']
			return platypus.Paragraph(self._textual(node), style, **(attr_get(node, [], {'bulletText':'str'})))
		elif node.localName=='h3':
			styles = reportlab.lib.styles.getSampleStyleSheet()
			style = styles['Heading3']
			return platypus.Paragraph(self._textual(node), style, **(attr_get(node, [], {'bulletText':'str'})))
		elif node.localName=='image':
			return platypus.Image(node.getAttribute('file'), mask=(250,255,250,255,250,255), **(attr_get(node, ['width','height'])))
		elif node.localName=='spacer':
			if node.hasAttribute('width'):
				width = unit_get(node.getAttribute('width'))
			else:
				width = unit_get('1cm')
			length = unit_get(node.getAttribute('length'))
			return platypus.Spacer(width=width, height=length)
		elif node.localName=='pageBreak':
			return platypus.PageBreak()
		elif node.localName=='condPageBreak':
			return platypus.CondPageBreak(**(attr_get(node, ['height'])))
		elif node.localName=='setNextTemplate':
			return platypus.NextPageTemplate(str(node.getAttribute('name')))
		elif node.localName=='nextFrame':
			return platypus.CondPageBreak(1000)           # TODO: change the 1000 !
		else:
			sys.stderr.write('Warning: flowable not yet implemented: %s !\n' % (node.localName,))
			return None

	def render(self, node_story):
		story = []
		node = node_story.firstChild
		while node:
			if node.nodeType == node.ELEMENT_NODE:
				flow = self._flowable(node) 
				if flow:
					story.append(flow)
			node = node.nextSibling
		return story

class _rml_template(object):
	def __init__(self, out, node, doc):
		if not node.hasAttribute('pageSize'):
			pageSize = (unit_get('21cm'), unit_get('29.7cm'))
		else:
			ps = map(lambda x:x.strip(), node.getAttribute('pageSize').replace(')', '').replace('(', '').split(','))
			pageSize = ( unit_get(ps[0]),unit_get(ps[1]) )
		cm = reportlab.lib.units.cm
		self.doc_tmpl = platypus.BaseDocTemplate(out, pagesize=pageSize, **attr_get(node, ['leftMargin','rightMargin','topMargin','bottomMargin'], {'allowSplitting':'int','showBoundary':'bool','title':'str','author':'str'}))
		self.page_templates = []
		self.styles = doc.styles
		self.doc = doc
		pts = node.getElementsByTagName('pageTemplate')
		for pt in pts:
			frames = []
			for frame_el in pt.getElementsByTagName('frame'):
				frame = platypus.Frame( **(attr_get(frame_el, ['x1','y1', 'width','height', 'leftPadding', 'rightPadding', 'bottomPadding', 'topPadding'], {'id':'text', 'showBoundary':'bool'})) )
				frames.append( frame )
			gr = pt.getElementsByTagName('pageGraphics')
			if len(gr):
				drw = _rml_draw(gr[0], self.doc)
				self.page_templates.append( platypus.PageTemplate(frames=frames, onPage=drw.render, **attr_get(pt, [], {'id':'str'}) ))
			else:
				self.page_templates.append( platypus.PageTemplate(frames=frames, **attr_get(pt, [], {'id':'str'}) ))
		self.doc_tmpl.addPageTemplates(self.page_templates)

	def render(self, node_story):
		r = _rml_flowable(self.doc)
		fis = r.render(node_story)
		self.doc_tmpl.build(fis)

class Mark:
    "Enum Values for the watermark style: No watermark, first page, all pages."
    NOTHING    = "0"
    FIRST_PAGE = "1"
    ALL_PAGES  = "2"
    

class PdfWatermark:
    "A class that converts a rml file to pdf"
    
    def __init__( self, outFile = None ):
        self.outputFile = outFile

    def watermark( self, pdfStr, watermarkFile, spec ):
        # Read the watermark- and document pdf file
        inputWatermark = PdfFileReader( file( watermarkFile, "rb" ) )
        generatedPdf = PdfFileReader( pdfStr )
        outputPdf = PdfFileWriter()
        
        # flag for the first page of the source file
     	firstPage = True
     	
     	# Loop over source document pages and merge with the first page of the watermark
     	# file.
     	watermarkPage = inputWatermark.getPage(0)
     	for page in generatedPdf.pages:
	    if (spec == Mark.FIRST_PAGE and firstPage) or spec == Mark.ALL_PAGES:
		# deep copy the watermark page here, otherwise the watermark page
		# gets merged over and over because p would only be a reference
		p = copy.copy( watermarkPage )
		p.mergePage( page )
		outputPdf.addPage( p )
		firstPage = False
	    else:
                outputPdf.addPage(page)
     	
     	if self.outputFile:
     	    # Write to outputfile
     	    outputStream = file( self.outputFile, "wb" )
     	    outputPdf.write( outputStream )
     	    outputStream.close()
     	    return self.outputFile
     	else: 
     	    stringIO = StringIO.StringIO();
     	    outputPdf.write( stringIO )
     	    return stringIO.getvalue()
     

def parseString(data, fout=None):
	r = _rml_doc(data)
	if fout:
		fp = file(fout,'wb')
		r.render(fp)
		fp.close()
		return fout
	else:
		fp = StringIO.StringIO()
		r.render(fp)
		return fp.getvalue()

def erml2pdf_help():
	print 'Usage: erml2pdf [options] input.rml > output.pdf'
	print ''
	print 'Tool to render a file of the xml based markup language RML to PDF'
	print 'with option to merge another PDF file as watermark.'
	print ''
	print 'Options:'
	print '-o, --output <file>           output file, instead of standard out'
	print '-m, --watermark-mode <mode>   set the watermark mode with '
	print '                              0 = no watermark (default)'
	print '                              1 = watermark on first page'
	print '                              2 = watermark on all pages'
	print '                              Note: a watermark file must be specified for 1, 2'
	print '-w, --watermark-file <file>   watermark file, the first page is used.'
	print ''
	sys.exit(0)

if __name__=="__main__":
        
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ho:w:m:", ["help", "output=", "watermark-file=", "watermark-mode="])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        erml2pdf_help()
        sys.exit(2)
    output = None
    watermarkFile = None
    watermarkMode = Mark.NOTHING
    
    for o, a in opts:
        if o in ("-h", "--help"):
            erml2pdf_help()
            sys.exit()
        elif o in ("-o", "--output"):
            output = a
        elif o in ("-w", "--watermark-file"):
            watermarkFile = a
        elif o in ("-m", "--watermark-mode"):
            watermarkMode = a
        else:
            assert False, "unhandled option"
    # 
    if len(args) == 0:
	# a input file needs to be there
	erml2pdf_help()
    else:
	# print "Args:" + args[0]
	infile = args[0]
	# create the PDF with the help of reportlab
	pdf = parseString( file( infile, 'r' ).read() )
	
	# apply the watermark if required
	# print "############ Watermark-Mode: " + watermarkMode
	if watermarkMode != Mark.NOTHING:
	    wm = PdfWatermark()
	    pdfStringFile = StringIO.StringIO()
	    pdfStringFile.write( pdf )
	    pdf = wm.watermark( pdfStringFile, watermarkFile, watermarkMode )
	    
	# handle output option, either file or stdout
	if output:
	    outfile = open(output, 'w')
	    outfile.write( pdf )
	    outfile.close()
	else:
            print pdf
