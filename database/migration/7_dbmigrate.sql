ALTER TABLE archdoc ADD clientUid VARCHAR(32) AFTER clientAddress;

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



