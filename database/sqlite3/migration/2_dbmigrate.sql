-- message: Creating document position calulation tables ;

CREATE TABLE DocCalcTime (
	TCalcID       INTEGER PRIMARY KEY ASC autoincrement,
	TemplID       INT NOT NULL,

	name          VARCHAR(255),
	minutes	      INT default 0,
	percent       INT default 0,
	stdHourSet    INT default 0,
	allowGlobal   INT default 1,

	modDate	      TIMESTAMP(14)
);
CREATE INDEX calcTimeTemplIndx_2 ON DocCalcTime( TemplID );
CREATE TRIGGER update_docCalcTime_modDate AFTER UPDATE ON DocCalcTime
BEGIN
  UPDATE DocCalcTime SET modDate = DATETIME('NOW')  WHERE TCalcID = new.TCalcID;
END;


CREATE TABLE DocCalcFixed(
	FCalcID       INTEGER PRIMARY KEY ASC autoincrement,
	TemplID       INT NOT NULL,

	name          VARCHAR(255),
	amount        DECIMAL(10,2) default 1.0,
	price	      DECIMAL(10,2),
	percent       INT default 0,
	modDate	      TIMESTAMP(14)
);
CREATE INDEX CalcFixedTemplIndx_2 ON DocCalcFixed( TemplID );
CREATE TRIGGER update_docCalcFixed_modDate AFTER UPDATE ON DocCalcFixed
BEGIN
  UPDATE DocCalcFixed SET modDate = DATETIME('NOW')  WHERE FCalcID = new.FCalcID;
END;


CREATE TABLE DocCalcMaterials(
	MCalcID       INTEGER PRIMARY KEY ASC autoincrement,
	TemplID       INT NOT NULL,
	name          VARCHAR(255),
	percent       INT default 0,
	modDate	      TIMESTAMP(14)
);
CREATE INDEX CalcMaterialTemplIndx_2 ON DocCalcMaterials( TemplID );
CREATE TRIGGER update_docCalcMaterials_modDate AFTER UPDATE ON DocCalcMaterials
BEGIN
  UPDATE DocCalcMaterials SET modDate = DATETIME('NOW')  WHERE MCalcID = new.MCalcID;
END;

CREATE TABLE DocCalcMaterialDetails(
	MCalcDetailID INTEGER PRIMARY KEY ASC autoincrement,
	CalcID        INT NOT NULL,
	
	materialID    INT NOT NULL,
	amount 	      DECIMAL(10,2)
);
CREATE INDEX CalcMaterialDetailsCalcIDIndx_2 ON DocCalcMaterialDetails( CalcID );

