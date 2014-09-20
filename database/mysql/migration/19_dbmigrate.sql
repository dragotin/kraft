# message Create new document type delivery note
INSERT INTO DocTypes (name) VALUES ('Lieferschein');
SET @lsId := LAST_INSERT_ID();
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', @lsId, 'HidePrices', 1);

INSERT INTO DocTypes (name) VALUES ('Delivery Receipt');
SET @nodId := LAST_INSERT_ID();
INSERT INTO attributes (hostObject, hostId, name, valueIsList) VALUES ('DocType', @nodId, 'HidePrices', 1);

# message Add more document relation settings
# Lieferschein follows Angebot
SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
# mayfail
INSERT INTO DocTypeRelations VALUES( @item, @lsId, 10 );

# Rechnung follows Lieferschein
SELECT @follower := docTypeID FROM DocTypes WHERE name="Rechnung";
# mayfail
INSERT INTO DocTypeRelations VALUES( @lsId, @follower, 11 );

# Delivery Receipt follows Offer
SELECT @item := docTypeID FROM DocTypes WHERE name="Offer";
# mayfail
INSERT INTO DocTypeRelations VALUES( @item, @nodId, 12 );

# Invoice follows Delivery Receipt
SELECT @follower := docTypeID FROM DocTypes WHERE name="Invoice";
# mayfail
INSERT INTO DocTypeRelations VALUES( @nodId, @follower, 13 );


# enhance the clientID col in document because the ids can be larger.
ALTER TABLE document CHANGE COLUMN clientID clientID VARCHAR(255);
ALTER TABLE archdoc CHANGE COLUMN clientUid clientUid VARCHAR(255);

# Done.
