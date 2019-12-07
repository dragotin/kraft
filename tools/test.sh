#!/bin/bash

rm test2_*.pdf
rm test3_*.pdf

# stdout output, no watermark
echo "Stdout output, no watermark:"
python2 erml2pdf.py test1.trml > test2_0.pdf
python3 erml2pdf.py test1.trml > test3_0.pdf

if((`stat -c%s "test2_0.pdf"`!=`stat -c%s "test3_0.pdf"`));then
  echo "File sizes differ!"
  exit 1
fi
md5sum test2_0.pdf
md5sum test3_0.pdf

# Only output file, no watermark
echo "Output to file, no watermark:"
python2 erml2pdf.py -o test2_1.pdf test1.trml
python3 erml2pdf.py -o test3_1.pdf test1.trml

if((`stat -c%s "test2_1.pdf"`!=`stat -c%s "test3_1.pdf"`));then
  echo "File sizes differ!"
  exit 1
fi
md5sum test2_1.pdf
md5sum test3_1.pdf

# Watermark on first page, stdout output
echo "Stdout output, watermark on first page:"
python2 erml2pdf.py -m 1 -w water.pdf  test1.trml > test2_2w.pdf
python3 erml2pdf.py -m 1 -w water.pdf  test1.trml > test3_2w.pdf

md5sum test2_2w.pdf
md5sum test3_2w.pdf

# Watermark on first page and output to file
echo "Output to file, watermark on first page:"
python2 erml2pdf.py -o test2_3w.pdf -m 1 -w water.pdf  test1.trml
python3 erml2pdf.py -o test3_3w.pdf -m 1 -w water.pdf  test1.trml

md5sum test2_3w.pdf
md5sum test3_3w.pdf

# Watermark on all pages, stdout output
echo "Stdout output, watermark on all pages:"
python2 erml2pdf.py -m 2 -w water.pdf  test1.trml > test2_4w.pdf
python3 erml2pdf.py -m 2 -w water.pdf  test1.trml > test3_4w.pdf

md5sum test2_4w.pdf
md5sum test3_4w.pdf

# Watermark on all page and output to file
echo "Output to file, watermark on all pages:"
python2 erml2pdf.py -o test2_5w.pdf -m 2 -w water.pdf  test1.trml
python3 erml2pdf.py -o test3_5w.pdf -m 2 -w water.pdf  test1.trml

md5sum test2_5w.pdf
md5sum test3_5w.pdf
