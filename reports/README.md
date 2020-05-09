# Report Templates

From the templates in this directory, Kraft is generating the final
documents in PDF format.

## Customizing ReportLab Templates

Please refer to
http://volle-kraft-voraus.de/Main/Documenttemplate
about customizing the output document with the old ReportLab based system.

## Weasyprint

WeasyPrint is a modern, HTML and CSS based way of creating PDF documents.
The project homepage is [Weasyprint Project](https://weasyprint.org/).

WeasyPrint will replace the so far used ReportLab based system after a
deprecation period.

### Try it!

WeasyPrint can be tested from Kraft Version 0.95 on. Just create a template
and give it the file extension .gtmpl and Kraft will automatically use
the Grantlee templating and WeasyPrint.

The appearance of the printed page is mostly influenced by the CSS (Cascading Style Sheet)
in file `invoice.css`.

## Internationalization

All "human readable" strings in the doc templates are translated to the
target language in the code, similar to the normal user interface.

Instead of changing the template to the word in a non English language, one
of the following template variables could be used:

| Template Var. | Meaning                                | English Default|
|---------------|----------------------------------------|----------------|
|LAB_NO_SHORT   |Sequence number printed on the document |No. |
|LAB_ITEM       |Document item printed on the document   |Item|
|LAB_QUANTITY_SHORT|Abbrev. of Quantity printed on the document|Qty.|
|LAB_UNIT       |Unit printed on the document|Unit|
|LAB_PRICE      |Price of an item printed on the document|Price|
|LAB_SUM        |Printed on the document                 |Sum |
|LAB_NET        |printed on the document                 |Net |
|LAB_VAT        |Printed on the document                 |VAT |
|LAB_PHONE      |Printed on the document                 |Phone|
|LAB_FAX        |Printed on the document                 |FAX |
|LAB_MOBILE     |Printed on the document                 |Mobile|
|LAB_EMAIL      |Printed on the document                 |Email|
|LAB_WEBSITE    |Printed on the document                 |Website|
|LAB_SPECIAL_ITEMS|Text underneath the list of items to sign out special items like Demand or Alternative items|Please note: This offer contains %1 alternative or demand positions, printed in italic font..|
|LAB_TAX_FREE_ITEMS|Label for the amount of tax free items|tax free items (%1 pcs.)|
|LAB_TAX_REDUCED_ITEMS|Label for the amount of tax reduced items|items with reduced tax of %1% (%2 pcs.)|
|LAB_TAX_FULL_ITEMS|Label for the amount of full tax items|No label: items with full tax of %1% (%2 pcs.)|
|LAB_PAGE       |Printed on the document                 |Page|
|LAB_PAGE_OF    |Printed on the document                 |of|
|LAB_DOC_NO     |Printed on the document                 |Document No.|
|LAB_DATE       |Printed on the document                 |Date|
|LAB_PROJECT    |Printed on the document                 |Project|
|LAB_CUST_ID    |Printed on the document                 |Customer Id|
|LAB_CURRENCY_SIGN |Printed on the document                 |the currency symbol|

