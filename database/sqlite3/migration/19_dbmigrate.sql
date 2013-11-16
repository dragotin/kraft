
INSERT INTO DocTypes (name) VALUES ('Lieferschein');
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', 
(SELECT docTypeID FROM DocTypes WHERE name="Lieferschein"), 'HidePrices', 'true');

INSERT INTO DocTypes (name) VALUES ('Note of Delivery');
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', 
(SELECT docTypeID FROM DocTypes WHERE name="Note of Delivery"), 'HidePrices', 'true');

# Lieferschein follows Angebot
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Angebot"),
(SELECT docTypeID FROM DocTypes WHERE name="Lieferschein"), 10 );

# Rechnung follows Lieferschein
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Lieferschein"),
(SELECT docTypeID FROM DocTypes WHERE name="Rechnung"), 11 );

# Done.
