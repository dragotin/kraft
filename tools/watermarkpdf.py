#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# Copyright 2020-2022 Klaas Freitag <kraft@freisturz.de>
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
import copy

import getopt

from PyPDF2 import PdfFileMerger, PdfFileWriter, PdfFileReader

# use utf8 for default
encoding = 'UTF-8'

class Mark:
    "Enum Values for the watermark style: No watermark, first page, all pages."
    NOTHING    = "0"
    FIRST_PAGE = "1"
    ALL_PAGES  = "2"
    ALTERNATING = "3"
    LASTPAGE_DIFFERENT = "4"

class PdfWatermark:
    "Class to put a watermark from a PDF file on a PDF file"

    def watermark( self, pdfFile, watermarkFile, spec ):
        # Read the watermark- and document pdf file
        watermark = PdfFileReader(open(watermarkFile, "rb"))
        watermark_page = watermark.getPage(0)

        inputPdf = PdfFileReader( open(pdfFile, "rb"))
        outputPdf = PdfFileWriter()

        # flag for the first page of the source file
        firstPage = True
        page_count = 1
        watermark_length = watermark.getNumPages ()
        input_length = inputPdf.getNumPages()

        if ((spec == Mark.ALTERNATING or spec == Mark.LASTPAGE_DIFFERENT) and watermark_length < 3):
            print("Watermark PDF has not enough pages. It needs three.")
            exit(1)

        # Loop over source document pages and merge with the watermark file.
        for page in range(inputPdf.getNumPages()):
            pdf_page = inputPdf.getPage(page)

            if ( firstPage or spec == Mark.ALL_PAGES):
                bg_page = copy.copy(watermark.getPage(0))
                bg_page.mergePage (pdf_page)
                outputPdf.addPage (bg_page)
                firstPage = False
            elif (spec == Mark.ALTERNATING):
                # add page 1 of watermark to first page. Page 2 and 3 alternate 
                bg_page = copy.copy(watermark.getPage(2 - page % 2))
                bg_page.mergePage (pdf_page)
                outputPdf.addPage (bg_page)
            elif (spec == Mark.LASTPAGE_DIFFERENT):
                # add page 1 of watermark to first page. Last page gets the last page of the watermark
                # pages in between get the middle page
                if (page == input_length-1):
                    # The last page of the input. Take the last page of the watermark
                    bg_page = copy.copy(watermark.getPage(watermark_length-1))
                else:
                    # pages in between 1..n-1
                    bg_page = copy.copy(watermark.getPage(1))
                bg_page.mergePage (pdf_page)
                outputPdf.addPage (bg_page)
            else:
                print ("Adding plain pdf page")
                outputPdf.addPage(pdf_page)

        bytesIO = io.BytesIO();
        outputPdf.write(bytesIO)
        return bytesIO

    def append(self, pdf, appendFile):
        merger = PdfFileMerger()
        merger.append(pdf)
        merger.append(appendFile)

        bytesIO = io.BytesIO();
        merger.write(bytesIO)
        return bytesIO

def file_exists(infile, perm):
    if os.path.exists(infile):
        # path exists
        if os.path.isfile(infile): # is it a file or a dir?
            # also works when file is a link and the target is writable
            return os.access(infile, perm)

def watermarkpdf_help():
    print( 'Usage: watermarkpdf [options] watermark.pdf file.pdf')
    print( '')
    print( 'Tool to put a watermark to a PDF file. The watermark is read from another PDF.')
    print( 'How the watermark is applied depends on parameter -m, see description below.')
    print( '')
    print( 'If no output is specified, the result is printed to stdout.')
    print( '')
    print( 'Options:')
    print( '-o, --output <file>           output file, instead of standard out')
    print( '-m, --watermark-mode <mode>   set the watermark mode with ')
    print( '    1 = watermark on first page (default)')
    print( '    2 = watermark on all pages. Only the first page of the watermarkfile is used.')
    print( '    3 = page 2 and 3 of the matermark file alternate on subsequent pages.')
    print( '    4 = page 1 of watermark goes to first page, 2 to all in between, 3 to the last page')
    print( '    For mode 3 and 4, the watermark pdf has to have three pages.')
    print( '-a, --append-file <file>      append this PDF to the resulting PDF')
    print( '')
    sys.exit(0)

if __name__=="__main__":

    try:
        opts, args = getopt.getopt(sys.argv[1:], "ho:m:a:", ["help", "output=", "watermark-mode=", "append-file="])
    except getopt.GetoptError as err:
        # print help information and exit:
        print( str(err)) # will print something like "option -a not recognized"
        watermarkpdf_help()
        sys.exit(2)
    output = None
    watermarkFile = None
    watermarkMode = Mark.FIRST_PAGE
    appendFile = ""

    for o, a in opts:
        if o in ("-h", "--help"):
            watermarkpdf_help()
            sys.exit()
        elif o in ("-o", "--output"):
            output = a
        elif o in ("-m", "--watermark-mode"):
            watermarkMode = a
        elif o in ("-a", "--append-file"):
            appendFile = a
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
        print("WATERMARK FILE" + watermarkFile)
        if (watermarkFile and not file_exists(watermarkFile, os.R_OK)):
            print( "Watermark file does not exist!");
            exit(1);

        # print "############ Watermark-Mode: " + watermarkMode
        pdfEngine = PdfWatermark()
        if watermarkMode != Mark.NOTHING:
            pdf = pdfEngine.watermark( infile, watermarkFile, watermarkMode )
        else:
            with open(infile, "rb") as fh:
                pdf = io.BytesIO(fh.read())

        # append a file if needed
        if appendFile:
            pdf = pdfEngine.append(pdf, appendFile)

        # handle output option, either file or stdout
        if output:
            outfile = open(output, 'wb')
            outfile.write( pdf.getvalue() )
            outfile.close()
        else:
            if sys.version_info[0] < 3:
                sys.stdout.write(pdf.getvalue())
            else:
                sys.stdout.buffer.write(pdf.getvalue())
