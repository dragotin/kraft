# message: Creating document position calulation tables ;

CREATE TABLE DocCalcTime (
	TCalcID       INT NOT NULL AUTO_INCREMENT,
	TemplID       INT NOT NULL,

	name          VARCHAR(255),
	minutes	      INT default 0,
	percent       INT default 0,
	stdHourSet    INT default 0,
	allowGlobal   INT default 1,

	modDate	      TIMESTAMP,

	PRIMARY KEY( TCalcID),
	INDEX(TemplID)
);

CREATE TABLE DocCalcFixed(
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

CREATE TABLE DocCalcMaterials(
	MCalcID       INT NOT NULL AUTO_INCREMENT,
	TemplID       INT NOT NULL,
	name          VARCHAR(255),
	percent       INT default 0,
	modDate	      TIMESTAMP,

	PRIMARY KEY(MCalcID),
	INDEX(TemplID)
);

CREATE TABLE DocCalcMaterialDetails(
	MCalcDetailID INT NOT NULL AUTO_INCREMENT,
	CalcID        INT NOT NULL,
	
	materialID    INT NOT NULL,
	amount 	      DECIMAL(10,2),

	PRIMARY KEY(MCalcDetailID),
	INDEX(CalcID)
);
