
:table-caption: Document Template:

The following tables list the available variables in a document template. The variables are
grouped under certain namespaces, depending on their meaning, ie. `doc` or `label`.

**Contacts**: For both the own identity with prefix `me` and for the customer contact with prefix `customer` the following variables are defined.

.Variables of a contact, both own identity (prefix `me`) and the customer contact (prefix `customer`)
[cols="1,2,1", width=99%]
|===
| Name of Variable |Description | Example Value

|me.COUNTRY |Country of the contact address |Germany
|me.EMAIL |Preferred email address of the contact |info@goofy.com
|me.EXTENDED |Extended address line of the contact address |
|me.FAX |Fax number of the contact |0912 232322
|me.LABEL |Formatted address label of the contact |
|me.LOCALITY |Locality (city) of the contact address |Spradsdorf
|me.MOBILE |Mobile phone number of the contact |
|me.NAME |Real name of the contact |Goofy Enterprises
|me.ORGANISATION |Organization of the contact, falls back to the real name |Goofy Enterprises
|me.PHONE |Work phone number of the contact |0912 211259
|me.POSTBOX |Post office box of the contact address |
|me.POSTCODE |Postal code of the contact address |92192
|me.REGION |Region (state) of the contact address |
|me.STREET |Street of the contact address |Steinstr. 34
|me.URL |Website URL of the contact |https://www.goofy.com

|===

**Document**: Document specific variables.

.Specific document variables
[cols="1,2,1", width=99%]
|===
| Name of Variable |Description | Example Value

|doc.address |Formatted address string of the customer |Goofy Stambulchicz
|doc.bruttoSumNum |Brutto sum as numerical value, not localized |535.84
|doc.bruttoSumStr |Brutto sum localized as user string |535,84 €
|doc.buyerReference |Buyer reference string, mainly for XRechnung |
|doc.clientUid |UID of the client/customer |BMhh9EhLwr
|doc.dateStr |Date as localized string according to user settings |27.01.2011
|doc.dateStrISO |date string ISO formatted, for XRechnung |2011-01-27
|doc.docIDStr |Document ID as string |f4deb784-6131-4e49-bd6d-92bd1355ea4d
|doc.docIdentifier |Human readable document identifier, e.g. &#39;Invoice 2026-001&#39; or a draft marker |Rechnung 20110127
|doc.docType |Localized document type, e.g. Invoice, Offer or Delivery Receipt |Rechnung
|doc.dueDateStrISO |Payment due date, ISO formatted, for XRechnung |
|doc.fullTaxPercentNum |Full tax rate in percent as numerical value, not localized |19.00
|doc.fullTaxPercentStr |Full tax rate in percent localized as user string |19
|doc.fullTaxSumNum |Tax amount of the full taxed items as numerical value, not localized |35.45
|doc.fullTaxSumStr |Tax amount of the full taxed items localized as user string |35,45 €
|doc.fullTaxesDocument |True if all items of the document are taxed with the full tax rate |false
|doc.goodbye |Closing greeting line of the document, e.g. &#39;Kind regards&#39; |mit den besten Grüssen,
|doc.hasIndividualTaxation |True if the document mixes different tax rates across its items |true
|doc.ident |Document identification number, &#39;draft&#39; for unsaved documents |20110127
|doc.individualTaxesDocument |True if the document mixes different tax rates across its items |true
|doc.isDraftState |True if the document is still in draft state |false
|doc.isInvoice |True if the document is an invoice |true
|doc.items |List of the document positions (line items) |
|doc.nettoSumNum |Netto sum as numerical value, not localized |479.86
|doc.nettoSumStr |Netto sum localized as user string |479,86 €
|doc.noTaxesDocument |True if none of the document items are taxed |false
|doc.objectName |General object name |
|doc.owner |Owner of the document |kf
|doc.postText |Text printed after the document positions, as plain text |Danke für dein Interesse,
|doc.postTextHtml |Text printed after the document positions, as HTML |Danke für dein Interesse,
|doc.preText |Text printed before the document positions, as plain text |Wir freuen uns, mit Dir Geschäfte machen zu können.
|doc.preTextHtml |Text printed before the document positions, as HTML |Wir freuen uns, mit Dir Geschäfte machen zu können.
|doc.predecessor |Identifier of the predecessor document this one was created from |id
|doc.projectLabel |Label of the project the document belongs to |hausgarten
|doc.reducedTaxPercentNum |Reduced tax rate in percent as numerical value, not localized |7.00
|doc.reducedTaxPercentStr |Reduced tax rate in percent localized as user string |7
|doc.reducedTaxSumNum |Tax amount of the reduced taxed items as numerical value, not localized |20.53
|doc.reducedTaxSumStr |Tax amount of the reduced taxed items localized as user string |20,53 €
|doc.reducedTaxesDocument |True if all items of the document are taxed with the reduced tax rate |false
|doc.salut |Salutation line of the document, e.g. &#39;Dear Mr. Smith&#39; |lieber goofy,
|doc.state |Document state as string, e.g. Draft, Final, Retracted or Invalid |Final
|doc.taxMarkerFull |Marker value identifying items taxed with the full tax rate |1
|doc.taxMarkerReduced |Marker value identifying items taxed with the reduced tax rate |2
|doc.taxPercentNum |Applicable tax rate in percent as numerical value, not localized |
|doc.taxPercentStr |Applicable tax rate in percent localized as user string |
|doc.taxSumNum |Total tax amount as numerical value, not localized |55.98
|doc.taxSumStr |Total tax amount localized as user string |55,98 €
|doc.timeOfSupplyEnd |End of the time of supply as localized string according to user settings |23.11.2019
|doc.timeOfSupplyEndISO |End of the time of supply, ISO formatted, for XRechnung |2019-11-23
|doc.timeOfSupplyMultiDay |True if the time of supply spans more than a single day |true
|doc.timeOfSupplyStart |Start of the time of supply as localized string according to user settings |21.11.2019
|doc.timeOfSupplyStartISO |Start of the time of supply, ISO formatted, for XRechnung |2019-11-21
|doc.timeOfSupplyValid |True if a valid time of supply is set on the document |true

|===

.Report items variables, from a loop over `doc.items`
[cols="1,2,1", width=99%]
|===
| Name of Variable |Description | Example Value

|item.amount |Quantity of the item localized as user string |8,4
|item.amountNum |Quantity of the item as numerical value, not localized |8.40
|item.htmlText |Text of the item, as HTML |first item
|item.itemNumber |Sequential number of the item within the document |1
|item.kind |Kind of the item, e.g. Position, Text, Demand or Alternative |Normal
|item.nettoPrice |Net total price of the item localized as user string |186,56 €
|item.nettoPriceNum |Net total price of the item as numerical value, not localized |186.56
|item.objectName |General object name |
|item.taxMarker |Marker value identifying the tax rate applied to the item |1
|item.text |Text of the item, as plain text |first item
|item.unit |Unit of the item, e.g. &#39;piece&#39; or &#39;hour&#39; |
|item.unitCode |UN/ECE Recommendation 20 unit code of the item, for XRechnung |
|item.unitPrice |Unit price of the item localized as user string |22,21 €
|item.unitPriceNum |Unit price of the item as numerical value, not localized |22.21

|===

.A generated EPC QR Code
[cols="1,2,1", width=99%]
|===
| Name of Variable |Description | Example Value

|epcqrcode.show |True if the EPC QR code should be shown on the document |true
|epcqrcode.svgfilename |File path of the generated EPC QR code SVG image |/tmp/jfRayr.svg
|epcqrcode.valid |True if a valid EPC QR code was generated for the document |true

|===

.Document Labels, translated
[cols="1,2,1", width=99%]
|===
| Name of Variable |Description | Value

|label.CURRENCY_SIGN |The configured currency symbol |€
|label.CUST_ID |Label for the customer ID, e.g. &#39;Customer ID&#39; |Customer ID
|label.DATE |Label for the document date, e.g. &#39;Date&#39; |Date
|label.DOC_NO |Label for the document number, e.g. &#39;Document No.&#39; |Document No.
|label.EMAIL |Label for the email address, e.g. &#39;Email&#39; |Email
|label.FAX |Label for the fax number, e.g. &#39;Fax&#39; |Fax
|label.ITEM |Label for the item column, e.g. &#39;Item&#39; |Item
|label.MOBILE |Label for the mobile number, e.g. &#39;Mobile&#39; |Mobile
|label.NET |Label for the net amount, e.g. &#39;Net&#39; |Net
|label.NO_SHORT |Label for the position sequence number column, e.g. &#39;No.&#39; |No.
|label.PAGE |Label for the page number, e.g. &#39;Page&#39; |Page
|label.PAGE_OF |The &#39;of&#39; word in &#39;page X of Y&#39; |of
|label.PHONE |Label for the phone number, e.g. &#39;Phone&#39; |Phone
|label.PREDECESSOR |Label for the predecessor document number |Predecessor-Doc
|label.PRICE |Label for the price column, e.g. &#39;Price&#39; |Price
|label.PROJECT |Label for the project, e.g. &#39;Project&#39; |Project
|label.QUANTITY_SHORT |Label for the quantity column, e.g. &#39;Qty.&#39; |Qty.
|label.SUM |Label for the sum column, e.g. &#39;Sum&#39; |Sum
|label.TIMEOFSUPPLY |Label for the time of supply, e.g. &#39;Time of supply&#39; |Time of supply
|label.TYPE |Label for the document type, e.g. &#39;Type&#39; |Type
|label.UNIT |Label for the unit column, e.g. &#39;Unit&#39; |Unit
|label.VAT |Label for the value added tax, e.g. &#39;VAT&#39; |VAT
|label.WEBSITE |Label for the website, e.g. &#39;Website&#39; |Website

|===

.Kraft Information Variables
[cols="1,2,1", width=99%]
|===
| Name of Variable |Description | Value

|kraft.DB_SCHEME |Database schema version of the running Kraft instance |DB-Scheme 24
|kraft.HOSTNAME |Host name of the machine running Kraft |localhost.localdomain
|kraft.SYS_USER |System user name running Kraft |klaas
|kraft.VERSION |Kraft version string including code name |Kraft 2.0.0 Cumulus

|===



