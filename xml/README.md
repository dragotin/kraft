## XML Example Document and Schema

This directory contains the example doc and the XML schema to validate Kraft
documents.

Validation example:
  xmllint --schema kraftdoc.xsd kraftdoc.xml


More features to be implemented later:

- Additional item type, for example "Text" that only contains a description.

Open Questions:

- add a document UUID which can be used as reference between docs
- both docAttrib and customValues are name-value-type entities. Both needed?
- Add a document state (Draft, SentOut, Invalid, Archived...) -> Implement as history with date?

