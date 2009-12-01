-- message Creating attributes table...
CREATE TABLE attributes (
  hostObject VARCHAR(64),
  hostId     INT NOT NULL,
  name       VARCHAR(64),
  value      MEDIUMTEXT,

  PRIMARY KEY( hostObject, hostId, name )
);

-- message Creating attributes for archived documents
CREATE TABLE archPosAttribs (
  archPosAttribId INTEGER PRIMARY KEY ASC autoincrement,
  archDocID  INT NOT NULL,
  name       VARCHAR(64),
  value      VARCHAR(64)
);

-- message Adding position type and overall price ot archdocpositions
ALTER TABLE archdocpos ADD COLUMN kind VARCHAR(64); -- AFTER ordNumber;
ALTER TABLE archdocpos ADD COLUMN overallPrice DECIMAL(10,2); -- AFTER price;

-- message Changing old kinds to Normal
UPDATE archdocpos SET kind = "Normal";

-- message Calculating archive position price
UPDATE archdocpos SET overallPrice = ROUND( price * amount, 2);

-- message Creating Document Type table
CREATE TABLE DocTypes (
  docTypeID INTEGER PRIMARY KEY ASC autoincrement,
  name      VARCHAR(255)
);

-- message Filling doc type attributes
INSERT INTO DocTypes (name) VALUES ( 'Offer' );

INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Offer"), 'AllowDemand', 'true');
INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Offer"), 'AllowAlternative', 'true');

INSERT INTO DocTypes (name) VALUES ( 'Acceptance of Order' );

INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Acceptance of Order"), 'AllowDemand', 'true');
INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Acceptance of Order"), 'AllowAlternative', 'true');

INSERT INTO DocTypes (name) VALUES ( 'Invoice' );

-- message Filling doc type attributes
INSERT INTO DocTypes (name) VALUES ( 'Angebot' );
INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Angebot"), 'AllowDemand', 'true');
INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Angebot"), 'AllowAlternative', 'true');

INSERT INTO DocTypes (name) VALUES ( 'Auftragsbestätigung' );
INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Auftragsbestätigung"), 'AllowDemand', 'true');
INSERT INTO attributes VALUES ('DocType', (SELECT docTypeID FROM DocTypes WHERE name="Auftragsbestätigung"), 'AllowAlternative', 'true');

INSERT INTO DocTypes (name) VALUES ( 'Rechnung' );

-- message Drop an unused table archdocStates
-- DROP TABLE IF EXISTS archdocStates;
ALTER TABLE archdocStates RENAME TO archdocStates_unused;

