
INSERT INTO DocTypes (name) VALUES ('Lieferschein');
SET @lsId := LAST_INSERT_ID();
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', @lsId, 'HidePrices', 'true');

INSERT INTO DocTypes (name) VALUES ('Note of Delivery');
SET @nodId := LAST_INSERT_ID();
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', @nodId, 'HidePrices', 'true');

# Lieferschein follows Angebot
SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
INSERT INTO DocTypeRelations VALUES( @item, @lsId, 10 );

# Rechnung follows Lieferschein
SELECT @follower := docTypeID FROM DocTypes WHERE name="Rechnung";
INSERT INTO DocTypeRelations VALUES( @lsId, @follower, 11 );

# Done.
