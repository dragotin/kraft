
CREATE TABLE DocTypeRelations (
  typeId INT NOT NULL,
  followerId INT NOT NULL,
  sequence   INT NOT NULL,

  PRIMARY KEY( typeId, followerId )
);

# Acceptance of Order follows Offer
SELECT @item := docTypeID FROM DocTypes WHERE name="Offer";
SELECT @follower := docTypeID FROM DocTypes WHERE name="Acceptance of Order";
INSERT INTO DocTypeRelations VALUES( @item, @follower, 1 );

# Invoice follorws Offer
SELECT @item := docTypeID FROM DocTypes WHERE name="Offer";
SELECT @follower := docTypeID FROM DocTypes WHERE name="Invoice";
INSERT INTO DocTypeRelations VALUES( @item, @follower, 2 );

# Invoice follows Acceptance of Order
SELECT @item := docTypeID FROM DocTypes WHERE name="Acceptance of Order";
SELECT @follower := docTypeID FROM DocTypes WHERE name="Invoice";
INSERT INTO DocTypeRelations VALUES( @item, @follower, 3 );


# Acceptance of Order follows Offer
SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
SELECT @follower := docTypeID FROM DocTypes WHERE name="Auftragsbestätigung";
INSERT INTO DocTypeRelations VALUES( @item, @follower, 4 );

# Invoice follorws Offer
SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
SELECT @follower := docTypeID FROM DocTypes WHERE name="Rechnung";
INSERT INTO DocTypeRelations VALUES( @item, @follower, 5 );

# Invoice follows Acceptance of Order
SELECT @item := docTypeID FROM DocTypes WHERE name like "Auftragsbest%";
SELECT @follower := docTypeID FROM DocTypes WHERE name="Rechnung";
INSERT INTO DocTypeRelations VALUES( @item, @follower, 6 );
