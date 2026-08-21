## XML Example Document and Schema

This directory contains the example doc and the XML schema to validate Kraft
documents.

Validation example:
  xmllint --noout --schema kraftdoc.xsd kraftdoc.xml

The example document `kraftdoc.xml` is not only documentation, it is test data:
`tests/t_xmlsaver.cpp` loads it and checks the parsed values, and
`tests/t_alltemplvars.cpp` uses it to generate the table of template variables
for the manual. Keep it complete, so that changing it means changing the tests
as well:

- it uses every element the schema defines,
- it has an item of each type (Normal, Alternative, Text, Demand and
  ExtraDiscount),
- it has one document attribute per attribute type, except `color` which is not
  implemented yet,
- the time of supply carries the time of day explicitly. Date-only values, as
  written by Kraft before version 2.1, are still read: the start becomes the
  beginning, the end the last second of the given day,
- the values in the `totals` block match what Kraft calculates from the items.
  Alternative, Text and Demand items do not count into the sums, an
  ExtraDiscount item counts with its negative price.

Open Questions:

- both docAttrib and customValues are name-value-type entities. Both needed?
- Add a document state history with date instead of only the current state?
