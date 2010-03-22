#!/bin/bash

#/***************************************************************************
# trml2pdf_kraft.sh - script to render the pdf files for kraft
#                             -------------------
#    begin                : Feb 2009
#    copyright            : (C) 2009 by Andreas Wuest
#                                    original version by Klaas Freitag                         
#    email                : andreaswuest@gmx.de
# ***************************************************************************/

#/***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# ***************************************************************************/

# uncomment for debug messages
# set -x

# print the usage
usage() {
  echo "Usage: $0 {0|1|2} trml-file watermark-pdf-file"
}

# check the infile exists
checkInfile() {
  if [ ! -e $1 ]; then
    echo "Trml file \"$1\" does not exist!" >> /dev/stderr
    usage
    exit 1;
  fi
}

# check if watermark exists
checkWatermarkFile() {
  if [ -z $1 ]; then
    echo "Watermark file \"$1\" does not exist!" >> /dev/stderr
    exit 1;
  fi

  if [ ! -e $1 ]; then
    echo "Watermark file \"$1\" does not exist!" >> /dev/stderr
    usage
    exit 1;
  fi
}

# we need at least 2 arguments (or 3 for watermark)
if [ "$#" -lt 2 ]; then
  usage
  exit 1;
fi

pdftk="`which pdftk`"

if [ -z $pdftk ]; then
  echo "pdftk (pdf toolkit) not found. Please install!" >> /dev/stderr
  exit 2;
fi

if [ ! -x $pdftk ]; then
  echo "pdftk is not an executable. Please fix!" >> /dev/stderr
  exit 3;
fi

# cleanup the resources and create new directory
tmpDir=`mktemp -d trml2pdf.XXXXXXXX`

type=$1
infile=$2

case "$type" in
  0)
     checkInfile $infile
     # render the document
     trml2pdf $infile > $tmpDir/rendered.pdf
     # print pdf to stdout
     $pdftk $tmpDir/rendered.pdf cat output -
     ;;
  1) 
     watermark=$3
     checkInfile $infile
     checkWatermarkFile $watermark
     # render the document
     trml2pdf $infile > $tmpDir/rendered.pdf
     # burst the document
     $pdftk $tmpDir/rendered.pdf burst output $tmpDir/pg_%04d.pdf
     # apply the watermark
     $pdftk $tmpDir/pg_0001.pdf background $watermark output $tmpDir/firstback.pdf
     # remove the tmp file
     rm $tmpDir/pg_0001.pdf
     # merge all together
     if [ -e $tmpDir/pg_0002.pdf ]; then
       $pdftk $tmpDir/firstback.pdf pg_*.pdf cat output -
     else
       $pdftk $tmpDir/firstback.pdf cat output -
     fi
     ;;
  2)
     watermark=$3
     checkInfile $infile
     checkWatermarkFile $watermark
     # render the document
     trml2pdf $infile > $tmpDir/rendered.pdf
     # apply the watermark
     $pdftk $tmpDir/rendered.pdf background $watermark output $tmpDir/renderedFinal.pdf
     # print pdf to stdout
     $pdftk $tmpDir/renderedFinal.pdf cat output -
     ;;
  *) 
     usage
     exit 1
     ;;
esac

rm -rf $tmpDir

exit 0;

