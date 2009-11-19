-- message Create Document Relations Table
CREATE TABLE DocTypeRelations (
  typeId INT NOT NULL,
  followerId INT NOT NULL,
  sequence   INT NOT NULL,

  PRIMARY KEY( typeId, followerId )
);

-- Acceptance of Order follows Offer
-- SELECT @item := docTypeID FROM DocTypes WHERE name="Offer";
-- SELECT @follower := docTypeID FROM DocTypes WHERE name="Acceptance of Order";
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Offer"), 
                                     (SELECT docTypeID FROM DocTypes WHERE name="Acceptance of Order"), 1 );

-- Invoice follorws Offer
-- SELECT @item := docTypeID FROM DocTypes WHERE name="Offer";
-- SELECT @follower := docTypeID FROM DocTypes WHERE name="Invoice";
-- INSERT INTO DocTypeRelations VALUES( @item, @follower, 2 );
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Offer"), 
                                     (SELECT docTypeID FROM DocTypes WHERE name="Invoice"), 2 );

-- Invoice follows Acceptance of Order
-- SELECT @item := docTypeID FROM DocTypes WHERE name="Acceptance of Order";
-- SELECT @follower := docTypeID FROM DocTypes WHERE name="Invoice";
-- INSERT INTO DocTypeRelations VALUES( @item, @follower, 3 );
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Acceptance of Order"), 
                                     (SELECT docTypeID FROM DocTypes WHERE name="Invoice"), 3 );


-- Acceptance of Order follows Offer
-- SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
-- SELECT @follower := docTypeID FROM DocTypes WHERE name="Auftragsbestätigung";
-- INSERT INTO DocTypeRelations VALUES( @item, @follower, 4 );
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Angebot"), 
                                     (SELECT docTypeID FROM DocTypes WHERE name="Auftragsbestätigung"), 4 );

-- Invoice follorws Offer
-- SELECT @item := docTypeID FROM DocTypes WHERE name="Angebot";
-- SELECT @follower := docTypeID FROM DocTypes WHERE name="Rechnung";
-- INSERT INTO DocTypeRelations VALUES( @item, @follower, 5 );
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Angebot"), 
                                     (SELECT docTypeID FROM DocTypes WHERE name="Rechnung"), 5 );

--  Invoice follows Acceptance of Order
-- SELECT @item := docTypeID FROM DocTypes WHERE name like "Auftragsbest%";
-- SELECT @follower := docTypeID FROM DocTypes WHERE name="Rechnung";
-- INSERT INTO DocTypeRelations VALUES( @item, @follower, 6 );
INSERT INTO DocTypeRelations VALUES( (SELECT docTypeID FROM DocTypes WHERE name="Auftragsbestätigung"), 
                                     (SELECT docTypeID FROM DocTypes WHERE name="Rechnung"), 6 );
