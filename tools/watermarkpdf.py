#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Modernized PDF watermarking tool (CLI + library)
# Original copyright 2020-2022 Klaas Freitag <kraft@freisturz.de>
# MIT License
#

import argparse
import io
import os
import sys
import copy
from pypdf import PdfMerger, PdfWriter, PdfReader

class Mark:
    """Watermark application modes."""
    NOTHING = "0"
    FIRST_PAGE = "1"
    ALL_PAGES = "2"
    ALTERNATING = "3"
    LASTPAGE_DIFFERENT = "4"


class PdfWatermark:
    """Class to put a watermark from a PDF file on another PDF file."""

    def watermark(self, pdf_file, watermark_file, spec):
        # Read PDFs
        watermark = PdfReader(open(watermark_file, "rb"))
        input_pdf = PdfReader(open(pdf_file, "rb"))
        output_pdf = PdfWriter()

        watermark_length = len(watermark.pages)
        input_length = len(input_pdf.pages)

        if ((spec == Mark.ALTERNATING or spec == Mark.LASTPAGE_DIFFERENT) and watermark_length < 3):
            raise ValueError("Watermark PDF must have at least 3 pages for mode 3 or 4.")

        for page_idx, pdf_page in enumerate(input_pdf.pages):
            if page_idx == 0 and spec in (Mark.FIRST_PAGE, Mark.ALL_PAGES, Mark.ALTERNATING, Mark.LASTPAGE_DIFFERENT):
                bg_page = copy.copy(watermark.pages[0])
            elif spec == Mark.ALL_PAGES:
                bg_page = copy.copy(watermark.pages[0])
            elif spec == Mark.ALTERNATING:
                bg_page = copy.copy(watermark.pages[2 - page_idx % 2])
            elif spec == Mark.LASTPAGE_DIFFERENT:
                if page_idx == input_length - 1:
                    bg_page = copy.copy(watermark.pages[watermark_length - 1])
                else:
                    bg_page = copy.copy(watermark.pages[1])
            else:
                output_pdf.add_page(pdf_page)
                continue

            bg_page.merge_page(pdf_page)
            output_pdf.add_page(bg_page)

        result = io.BytesIO()
        output_pdf.write(result)
        return result

    def append(self, pdf_bytes, append_file):
        merger = PdfMerger()
        merger.append(pdf_bytes)
        merger.append(append_file)
        result = io.BytesIO()
        merger.write(result)
        return result


def apply_watermark(input_pdf, watermark_pdf, mode=Mark.FIRST_PAGE, append_file=None):
    """
    Apply a watermark to a PDF file.

    :param input_pdf: Path to input PDF
    :param watermark_pdf: Path to watermark PDF
    :param mode: One of Mark.* constants
    :param append_file: Optional PDF file to append
    :return: BytesIO object with resulting PDF
    """
    if not os.path.isfile(input_pdf):
        raise FileNotFoundError(f"Input file not found: {input_pdf}")
    if not os.path.isfile(watermark_pdf):
        raise FileNotFoundError(f"Watermark file not found: {watermark_pdf}")
    if append_file and not os.path.isfile(append_file):
        raise FileNotFoundError(f"Append file not found: {append_file}")

    engine = PdfWatermark()

    if mode != Mark.NOTHING:
        pdf_data = engine.watermark(input_pdf, watermark_pdf, mode)
    else:
        with open(input_pdf, "rb") as fh:
            pdf_data = io.BytesIO(fh.read())

    if append_file:
        pdf_data = engine.append(pdf_data, append_file)

    return pdf_data


def main():
    parser = argparse.ArgumentParser(
        description="Apply a PDF watermark to another PDF file."
    )
    parser.add_argument("watermark", help="Watermark PDF file")
    parser.add_argument("input_pdf", help="Input PDF to watermark")
    parser.add_argument(
        "-o", "--output",
        help="Output file (default: stdout)",
    )
    parser.add_argument(
        "-m", "--mode",
        default=Mark.FIRST_PAGE,
        choices=[Mark.NOTHING, Mark.FIRST_PAGE, Mark.ALL_PAGES, Mark.ALTERNATING, Mark.LASTPAGE_DIFFERENT],
        help="Watermark mode: "
             "0=none, 1=first page, 2=all pages, "
             "3=alternate pages 2/3, 4=first/middle/last"
    )
    parser.add_argument(
        "-a", "--append-file",
        help="Append this PDF after watermarking"
    )

    args = parser.parse_args()

    try:
        pdf_data = apply_watermark(
            args.input_pdf,
            args.watermark,
            mode=args.mode,
            append_file=args.append_file
        )
    except (FileNotFoundError, ValueError) as e:
        sys.exit(f"Error: {e}")

    if args.output:
        with open(args.output, "wb") as out_f:
            out_f.write(pdf_data.getvalue())
    else:
        sys.stdout.buffer.write(pdf_data.getvalue())


if __name__ == "__main__":
    main()
