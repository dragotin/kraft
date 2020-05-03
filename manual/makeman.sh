#!/bin/bash

# set -euxo pipefail

# FIXME: Check if the tools ascidoctor and po4a-translate are installed
# FIXME: Be compatible with other ascii doc generators

# =====================================================================
showhelp() {

    echo "Usage: $0 <source directory>" >&2
    echo
    echo "   Creates the Kraft manual and its translations."
    echo "   asciidoctor and po4trans need to be installed."
    echo ""
    echo "   To read the source from elsewhere, the script"
    echo "   takes a source directory as argument"
    echo ""
    echo "   Output happens to the current directory."

    exit 1
}

# =====================================================================

srcdir="${1:-.}"
srcfile="${srcdir}/kraft.adoc"
version="1.0"
outfile="kraft-en.html"

if [  -n "$1" ] && [ "$1" == "-h" ]; then
    showhelp
fi
echo "Building $srcfile in ${srcdir}"

outdir=`pwd`
pushd "${srcdir}"

lang=en
asciidocargs="-D ${outdir} -a path=${srcdir} -a VERSION=${version} -a lang=${lang} -a stylesheet=kraftmanual.css"

# english master doc
asciidoctor ${asciidocargs} -o ${outfile} ${srcfile}
echo "built ${outfile}"

# build the internationalized versions
languages="de"

for lang in ${languages}
do
    transsrc="${srcdir}/po/kraft-${lang}.po"
    if [ -f "${transsrc}" ]; then
        po4a-translate -f asciidoc -M utf-8 -m ${srcfile} -p ${transsrc} -k 0 -l kraft-${lang}.adoc
        outfile="kraft-${lang}.html"
        asciidoctor ${asciidocargs} -o ${outfile} kraft-${lang}.adoc
	rm kraft-${lang}.adoc
        echo "built ${transsrc} to ${outfile}"
    else
        echo "File ${transsrc} does not exist!"
    fi
done
popd

