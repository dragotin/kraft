# DROP DATABASE IF EXISTS kraft; 
# CREATE DATABASE kraft DEFAULT CHARACTER SET "utf8";

# use kraft;


CREATE TABLE preisArten (
	preisArtID    INT NOT NULL,
	preisArt      VARCHAR(64) NOT NULL,

	PRIMARY KEY( preisArtID )
);

CREATE TABLE wordLists(
  category     VARCHAR(64),
  word         VARCHAR(255),

  PRIMARY KEY( category, word )
);

CREATE TABLE CatalogSet(
  catalogSetID INT NOT NULL AUTO_INCREMENT,
  name         VARCHAR(255),
  description  VARCHAR(255),
  catalogType  VARCHAR(64),
  sortKey      INT NOT NULL,
  
  PRIMARY KEY(catalogSetID)
);

CREATE TABLE CatalogChapters(
  chapterID    INT NOT NULL AUTO_INCREMENT,
  catalogSetID INT NOT NULL, 
  chapter      VARCHAR(255),
  sortKey      INT NOT NULL,

  PRIMARY KEY(chapterID),
  INDEX(chapter)
);

CREATE TABLE Catalog (
	TemplID      INT NOT NULL AUTO_INCREMENT,
	chapterID    INT NOT NULL  default 1,
	unitID       INT NOT NULL,
	Floskel      TEXT,
	Gewinn	     DECIMAL(6,2)        default 0,
	zeitbeitrag  TINYINT             default 1,
	enterDatum   DATETIME,
	modifyDatum  TIMESTAMP,
	Preisart     INT NOT NULL  default 1,
	EPreis       DECIMAL(10,2)       default 0,
	PRIMARY KEY( TemplID ),
	INDEX ( chapterID )
);

UPDATE Catalog SET modifyDatum=enterDatum;

CREATE TABLE CalcTime (
	TCalcID       INT NOT NULL AUTO_INCREMENT,
	TemplID       INT NOT NULL,

	name          VARCHAR(255),
	minutes	      INT default 0,
	percent       INT default 0,
	stdHourSet    INT default 0,
	allowGlobal   INT default 1,

	modDate	      TIMESTAMP,

	PRIMARY KEY( TCalcID),
	INDEX( TemplID )
);

CREATE TABLE CalcFixed(
	FCalcID       INT NOT NULL AUTO_INCREMENT,
	TemplID       INT NOT NULL,

	name          VARCHAR(255),
	amount        DECIMAL(10,2) default 1.0,
	price	      DECIMAL(10,2),
	percent       INT default 0,
	modDate	      TIMESTAMP,

	PRIMARY KEY(FCalcID),
	INDEX(TemplID)
);

CREATE TABLE CalcMaterials(
	MCalcID       INT NOT NULL AUTO_INCREMENT,
	TemplID       INT NOT NULL,
	name          VARCHAR(255),
	percent       INT default 0,
	modDate	      TIMESTAMP,

	PRIMARY KEY(MCalcID),
	INDEX(TemplID)
);

CREATE TABLE CalcMaterialDetails(
	MCalcDetailID INT NOT NULL AUTO_INCREMENT,
	CalcID        INT NOT NULL,
	
	materialID    INT NOT NULL,
	amount 	      DECIMAL(10,2),

	PRIMARY KEY(MCalcDetailID),
	INDEX(CalcID)
);

CREATE TABLE units(
	unitID       INT NOT NULL,
	unitShort    VARCHAR(255),
	unitLong     VARCHAR(255),
	unitPluShort VARCHAR(255),
	unitPluLong  VARCHAR(255),

	PRIMARY KEY(unitID),
	INDEX(unitShort)
);



CREATE TABLE stockMaterial (
	matID        INT NOT NULL AUTO_INCREMENT,
	chapterID    INT NOT NULL default 1,
	material     mediumtext,
	unitID       INT NOT NULL,
	perPack	     DECIMAL(10,2),
	priceIn	     DECIMAL(10,2),
	priceOut     DECIMAL(10,2),
	enterDate    DATETIME,
	modifyDate   TIMESTAMP,

	PRIMARY KEY(matID),
	INDEX(chapterID)
);

CREATE TABLE stdSaetze( 
	stdSaetzeID	      INT NOT NULL AUTO_INCREMENT,
	name                  VARCHAR(255),
	price                 DECIMAL(10,2),
	sortKey               int,

	PRIMARY KEY(stdSaetzeID)
);

CREATE TABLE document(
    docID             INT NOT NULL AUTO_INCREMENT,
    ident             VARCHAR(32),
    docType           VARCHAR(255),
    clientID          VARCHAR(32),
    clientAddress     TEXT,
    salut             VARCHAR(255),
    goodbye           VARCHAR(128),
    lastModified      TIMESTAMP,
    date              DATE,

    pretext           TEXT,
    posttext          TEXT,

    PRIMARY KEY( docID ),
    INDEX(ident),
    INDEX(clientID)
);

CREATE TABLE docposition(
    positionID        INT NOT NULL AUTO_INCREMENT,
    docID             INT NOT NULL,
    ordNumber         INT NOT NULL,
    text              TEXT,
    amount            DECIMAL(10,2),
    unit              INT,
    price             DECIMAL(10,2),
    
    PRIMARY KEY( positionID ),
    INDEX(docID),
    UNIQUE( docID, ordNumber)
);

CREATE TABLE archdocStates(
    stateID          INT NOT NULL AUTO_INCREMENT,
    state            VARCHAR(32),

    PRIMARY KEY( stateID )
);

CREATE TABLE archdoc(
    archDocID         INT NOT NULL AUTO_INCREMENT,
    ident             VARCHAR(32),
    docType           VARCHAR(255),
    docDescription    TEXT,
    clientAddress     TEXT,
    salut             VARCHAR(255),
    goodbye           VARCHAR(128),
    printDate         TIMESTAMP,
    date              DATE,

    pretext           TEXT,
    posttext          TEXT,

    state             int,

    PRIMARY KEY( archDocID ),
    INDEX(ident)
);

CREATE TABLE archdocpos(
    archPosID        INT NOT NULL AUTO_INCREMENT,
    archDocID         INT NOT NULL,
    ordNumber         INT NOT NULL,
    text              TEXT,
    amount            DECIMAL(10,2),
    unit              VARCHAR(64),
    price             DECIMAL(10,2),
    vat               DECIMAL(4,1),

    PRIMARY KEY( archPosID ),
    INDEX(archDocID),
    UNIQUE( archDocID, ordNumber)
);

CREATE TABLE kraftsystem(
    dbschemaversion  INT NOT NULL,
    updateUser       VARCHAR(256)
);

INSERT INTO kraftsystem ( dbschemaversion ) VALUES ( 1 );

# message Database created.

