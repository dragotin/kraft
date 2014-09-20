
# message Create new document type delivery note
INSERT INTO DocTypes (name) VALUES ('Lieferschein');
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', 
(SELECT docTypeID FROM DocTypes WHERE name='Lieferschein'), 'HidePrices', 'true');

INSERT INTO DocTypes (name) VALUES ('Delivery Receipt');
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', 
(SELECT docTypeID FROM DocTypes WHERE name='Delivery Receipt'), 'HidePrices', 'true');

# Delivery Receipt follows Offer

# mayfail
# message Add more document relation settings
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Offer"),
	(SELECT docTypeID FROM DocTypes WHERE name="Delivery Receipt"), 12 );

# mayfail
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Delivery Receipt"),
	(SELECT docTypeID FROM DocTypes WHERE name="Invoice"), 13 );

# Lieferschein follows Angebot
# mayfail
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Angebot"),
(SELECT docTypeID FROM DocTypes WHERE name="Lieferschein"), 10 );

# Rechnung follows Lieferschein
# mayfail
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Lieferschein"),
(SELECT docTypeID FROM DocTypes WHERE name="Rechnung"), 11 );

# Done.
