ALTER TABLE CalcMaterials RENAME TO CalcMaterialsOld;

CREATE TABLE CalcMaterials (
  MCalcID    INT NOT NULL AUTO_INCREMENT,
  TemplID    INT NOT NULL,
  materialID INT NOT NULL,
  percent    INT DEFAULT 0,
  amount     DECIMAL(10,2),
  modDate    TIMESTAMP,

  PRIMARY KEY(MCalcID)
);

INSERT INTO CalcMaterials (TemplID, materialID, amount, percent, modDate) 
SELECT CalcMaterialsOld.TemplID, CalcMaterialDetails.materialID, CalcMaterialDetails.amount, CalcMaterialsOld.percent, CalcMaterialsOld.modDate
FROM CalcMaterialDetails INNER JOIN CalcMaterialsOld ON CalcMaterialDetails.CalcID=CalcMaterialsOld.MCalcID;

DROP TABLE IF EXISTS CalcMaterialsOld;
DROP TABLE IF EXISTS CalcMaterialDetails;
DROP TABLE IF EXISTS attribute_old;