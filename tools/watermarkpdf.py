#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Klaas Freitag <kraft@freisturz.de>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this
# software and associated documentation files (the "Software"), to deal in the Software
# without restriction, including without limitation the rights to use, copy, modify,
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be included in all copies
# or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
import io
import os
import sys

import getopt

# StringIO is not longer separate in python3, but in io
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

from six import text_type
from PyPDF2 import PdfFileWriter, PdfFileReader

# use utf8 for default
encoding = 'UTF-8'

class Mark:
    "Enum Values for the watermark style: No watermark, first page, all pages."
    NOTHING    = "0"
    FIRST_PAGE = "1"
    ALL_PAGES  = "2"


class PdfWatermark:
    "Class to put a watermark from a PDF file on a PDF file"

    def watermark( self, pdfFile, watermarkFile, spec ):
        # Read the watermark- and document pdf file
        watermark = PdfFileReader(watermarkFile)
        watermark_page = watermark.getPage(0)

        inputPdf = PdfFileReader( pdfFile )
        outputPdf = PdfFileWriter()

        # flag for the first page of the source file
        firstPage = True

        # Loop over source document pages and merge with the first page of the watermark
        # file.
        for page in range(inputPdf.getNumPages()):
            pdf_page = inputPdf.getPage(page)
            if (spec == Mark.FIRST_PAGE and firstPage) or spec == Mark.ALL_PAGES:
                # deep copy the watermark page here, otherwise the watermark page
                # gets merged over and over because p would only be a reference
                pdf_page.mergePage(watermark_page)
                outputPdf.addPage( pdf_page )
                firstPage = False
            else:
                outputPdf.addPage(pdf_page)

        bytesIO = io.BytesIO();
        outputPdf.write(bytesIO)
        return bytesIO.getvalue()

def file_exists(infile, perm):
    if os.path.exists(infile):
        # path exists
        if os.path.isfile(infile): # is it a file or a dir?
            # also works when file is a link and the target is writable
            return os.access(infile, perm)

def watermarkpdf_help():
    print( 'Usage: watermarkpdf [options] watermark.pdf file.pdf')
    print( '')
    print( 'Tool to put a watermark to a PDF file from another PDF.')
    print( 'It can either put the watermark only on first page or')
    print( 'on all pages, depending on parameter -m.')
    print( 'If no output is specified, the result is printed to stdout.')
    print( '')
    print( 'Options:')
    print( '-o, --output <file>           output file, instead of standard out')
    print( '-m, --watermark-mode <mode>   set the watermark mode with ')
    print( '                              1 = watermark on first page (default)')
    print( '                              2 = watermark on all pages')
    print( '')
    sys.exit(0)

if __name__=="__main__":

    try:
        opts, args = getopt.getopt(sys.argv[1:], "ho:m:", ["help", "output=", "watermark-mode="])
    except(getopt.GetoptError, err):
        # print help information and exit:
        print( str(err)) # will print something like "option -a not recognized"
        watermarkpdf_help()
        sys.exit(2)
    output = None
    watermarkFile = None
    watermarkMode = Mark.FIRST_PAGE

    for o, a in opts:
        if o in ("-h", "--help"):
            erml2pdf_help()
            sys.exit()
        elif o in ("-o", "--output"):
            output = a
        elif o in ("-m", "--watermark-mode"):
            watermarkMode = a
        else:
            assert False, "unhandled option"
    #
    if len(args) < 2:
        # a input file needs to be there
        watermarkpdf_help()
    else:
        # print ("Args:" + args[0])
        watermarkFile = args[0]
        infile = args[1]

        if not (infile and file_exists(infile, os.R_OK)):
            print( "Input file does not exist!");
            exit(1);
        if not (watermarkFile and file_exists(watermarkFile, os.R_OK)):
            print( "Watermark file does not exist!");
            exit(1);

        # print "############ Watermark-Mode: " + watermarkMode
        if watermarkMode != Mark.NOTHING:
            wm = PdfWatermark()
            pdf = wm.watermark( infile, watermarkFile, watermarkMode )
        else:
            inf = open(infile, 'rb')
            pdf = inf.read()

        # handle output option, either file or stdout
        if output:
            outfile = open(output, 'wb')
            outfile.write( pdf )
            outfile.close()
        else:
            if sys.version_info[0] < 3:
                sys.stdout.write(pdf)
            else:
                sys.stdout.buffer.write(pdf)
