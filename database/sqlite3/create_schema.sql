
CREATE TABLE preisArten (
	preisArtID    INTEGER PRIMARY KEY ASC autoincrement,
	preisArt      VARCHAR(64) NOT NULL
);

CREATE TABLE wordLists(
  category     VARCHAR(64),
  word         VARCHAR(255),

  PRIMARY KEY( category, word )
);

CREATE TABLE CatalogSet(
  catalogSetID INTEGER PRIMARY KEY ASC autoincrement,
  name         VARCHAR(255),
  description  VARCHAR(255),
  catalogType  VARCHAR(64),
  sortKey      INT NOT NULL
);

CREATE TABLE CatalogChapters(
	chapterID INTEGER PRIMARY KEY ASC autoincrement,
        catalogSetID INT NOT NULL, 
	chapter      VARCHAR(255),
        sortKey      INT NOT NULL
);
CREATE INDEX chapterIndx ON CatalogChapters( chapter );

CREATE TABLE Catalog (
	TemplID      INTEGER PRIMARY KEY ASC autoincrement,
	chapterID    INT NOT NULL  default 1,
	unitID       INT NOT NULL,
	Floskel      TEXT,
	Gewinn	     DECIMAL(6,2)        default 0,
	zeitbeitrag  TINYINT             default 1,
	enterDatum   DATETIME,
	modifyDatum  TIMESTAMP(14),
	Preisart     INT NOT NULL  default 1,
	EPreis       DECIMAL(10,2)       default 0
);
CREATE INDEX chapterIdIndx ON Catalog( chapterID );

CREATE TRIGGER insert_catalog_timeEnter AFTER  INSERT ON Catalog
BEGIN
  UPDATE Catalog SET enterDatum = DATETIME('NOW')  WHERE TemplID = new.TemplID;
END;
CREATE TRIGGER update_catalog_timeEnter AFTER UPDATE ON Catalog
BEGIN
  UPDATE Catalog SET modifyDatum = DATETIME('NOW')  WHERE TemplID = new.TemplID;
END;

UPDATE Catalog SET modifyDatum=enterDatum;

CREATE TABLE CalcTime (
	TCalcID       INTEGER PRIMARY KEY ASC autoincrement,
	TemplID       INT NOT NULL,

	name          VARCHAR(255),
	minutes	      INT default 0,
	percent       INT default 0,
	stdHourSet    INT default 0,
	allowGlobal   INT default 1,

	modDate	      TIMESTAMP(14)
);
CREATE INDEX calcTimeIndx ON CalcTime( TemplID );
CREATE TRIGGER update_calcTime_modifyDate AFTER UPDATE ON CalcTime
BEGIN
  UPDATE CalcTime SET modDate = DATETIME('NOW')  WHERE TCalcID = new.TCalcID;
END;


CREATE TABLE CalcFixed(
	FCalcID       INTEGER PRIMARY KEY ASC autoincrement,
	TemplID       INT NOT NULL,

	name          VARCHAR(255),
	amount        DECIMAL(10,2) default 1.0,
	price	      DECIMAL(10,2),
	percent       INT default 0,
	modDate	      TIMESTAMP(14)
);
CREATE INDEX calcFixedIndx ON CalcFixed( TemplID );
CREATE TRIGGER update_calcFixed_modifyDate AFTER UPDATE ON CalcFixed
BEGIN
  UPDATE CalcFixed SET modDate = DATETIME('NOW')  WHERE FCalcID = new.FCalcID;
END;


CREATE TABLE CalcMaterials(
	MCalcID       INTEGER PRIMARY KEY ASC autoincrement,
	TemplID       INT NOT NULL,
	name          VARCHAR(255),
	percent       INT default 0,
	modDate	      TIMESTAMP(14)
);
CREATE INDEX calcMatIndx ON CalcMaterials( TemplID );
CREATE TRIGGER update_calcMaterials_modifyDate AFTER UPDATE ON CalcMaterials
BEGIN
  UPDATE CalcMaterials SET modDate = DATETIME('NOW')  WHERE MCalcID = new.MCalcID;
END;

CREATE TABLE CalcMaterialDetails(
	MCalcDetailID INTEGER PRIMARY KEY ASC autoincrement,
	CalcID        INT NOT NULL,
	
	materialID    INT NOT NULL,
	amount 	      DECIMAL(10,2)
);
CREATE INDEX calcIdIndx ON CalcMaterialDetails( CalcID );

CREATE TABLE units(
	unitID       INTEGER PRIMARY KEY ASC autoincrement,
	unitShort    VARCHAR(255),
	unitLong     VARCHAR(255),
	unitPluShort VARCHAR(255),
	unitPluLong  VARCHAR(255)
);
CREATE INDEX unitShortIndx ON units( unitShort );


CREATE TABLE stockMaterial (
	matID        INTEGER PRIMARY KEY ASC autoincrement,
	chapterID    INT NOT NULL default 1,
	material     mediumtext,
	unitID       INT NOT NULL,
	perPack	     DECIMAL(10,2),
	priceIn	     DECIMAL(10,2),
	priceOut     DECIMAL(10,2),
	enterDate    DATETIME,
	modifyDate   TIMESTAMP(14)
);
CREATE INDEX matChapterIndx ON stockMaterial( chapterID );

CREATE TRIGGER insert_material_enterDate AFTER  INSERT ON stockMaterial
BEGIN
  UPDATE stockMaterial SET enterDate = DATETIME('NOW')  WHERE matID = new.matID;
END;
CREATE TRIGGER update_material_modifyDate AFTER UPDATE ON stockMaterial
BEGIN
  UPDATE stockMaterial SET modifyDate = DATETIME('NOW')  WHERE matID = new.matID;
END;


CREATE TABLE stdSaetze( 
	stdSaetzeID	INTEGER PRIMARY KEY ASC autoincrement,
	name            VARCHAR(255),
	price           DECIMAL(10,2),
	sortKey         int
);

CREATE TABLE document(
    docID             INTEGER PRIMARY KEY ASC autoincrement,
    ident             VARCHAR(32),
    docType           VARCHAR(255),
    docDescription    TEXT,
    clientID          VARCHAR(32),
    clientAddress     TEXT,
    salut             VARCHAR(255),
    goodbye           VARCHAR(128),
    lastModified      TIMESTAMP,
    date              DATE,
    pretext           TEXT,
    posttext          TEXT,
    country           VARCHAR(32),
    language          VARCHAR(32),
    projectLabel      VARCHAR(255)
);
CREATE INDEX identIndx ON document( ident );
CREATE INDEX clientIndx ON document( clientID );
CREATE TRIGGER update_document AFTER UPDATE ON document
BEGIN
  UPDATE document SET lastModified = DATETIME('NOW')  WHERE docID = new.docID;
END;


CREATE TABLE docposition(
    positionID        INTEGER PRIMARY KEY ASC autoincrement,
    docID             INT NOT NULL,
    ordNumber         INT NOT NULL,
    text              TEXT,
    postype           VARCHAR(64),
    amount            DECIMAL(10,2),
    unit              INT,
    price             DECIMAL(10,2),
    taxType           INT default 3
);
CREATE INDEX docIdIndx ON docposition( docID );
CREATE UNIQUE INDEX ordIndx ON docposition( docID, ordNumber );

CREATE TABLE archdocStates(
    stateID          INTEGER PRIMARY KEY ASC autoincrement,
    state            VARCHAR(32)
);

CREATE TABLE archdoc(
    archDocID         INTEGER PRIMARY KEY ASC autoincrement,
    ident             VARCHAR(32),
    docType           VARCHAR(255),
    docDescription    TEXT,
    clientAddress     TEXT,
    clientUid         VARCHAR(32),
    salut             VARCHAR(255),
    goodbye           VARCHAR(128),
    printDate         TIMESTAMP,
    date              DATE,
    pretext           TEXT,
    posttext          TEXT,
    country           VARCHAR(32),
    language          VARCHAR(32),
    projectLabel      VARCHAR(255),
    tax               DECIMAL(5,1),
    reducedTax        DECIMAL(5,1),
    state             int
);
CREATE INDEX archIdentIndx ON archdoc( ident );
CREATE TRIGGER update_archdoc AFTER UPDATE ON archdoc
BEGIN
  UPDATE archDoc SET printDate = DATETIME('NOW')  WHERE archDocID = new.archDocID;
END;


CREATE TABLE archdocpos(
    archPosID         INTEGER PRIMARY KEY ASC autoincrement,
    archDocID         INT NOT NULL,
    ordNumber         INT NOT NULL,
    kind              VARCHAR(64),
    postype           VARCHAR(64),
    text              TEXT,
    amount            DECIMAL(10,2),
    unit              VARCHAR(64),
    price             DECIMAL(10,2),
    overallPrice      DECIMAL(10,2),
    taxType           INT default 0
);
CREATE INDEX archDocIdIndx ON archdocpos( archDocID );
CREATE UNIQUE INDEX archOrdIndx ON archdocpos( archDocID, ordNumber );

CREATE TABLE kraftsystem(
    dbschemaversion  INT NOT NULL,
    updateUser       VARCHAR(256)
);

INSERT INTO kraftsystem ( dbschemaversion ) VALUES ( 1 );



