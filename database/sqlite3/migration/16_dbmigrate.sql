ALTER TABLE CalcMaterials RENAME TO CalcMaterialsOld;

CREATE TABLE CalcMaterials (
  MCalcID    INTEGER PRIMARY KEY ASC autoincrement,
  TemplID    INT NOT NULL,
  materialID INT NOT NULL,
  percent    INT DEFAULT 0,
  amount     DECIMAL(10,2),
  modDate    TIMESTAMP(14)
);

INSERT INTO CalcMaterials (TemplID, materialID, amount, percent, modDate) 
SELECT CalcMaterialsOld.TemplID, CalcMaterialDetails.materialID, CalcMaterialDetails.amount, CalcMaterialsOld.percent, CalcMaterialsOld.modDate
FROM CalcMaterialDetails INNER JOIN CalcMaterialsOld ON CalcMaterialDetails.CalcID=CalcMaterialsOld.MCalcID;

DROP TABLE IF EXISTS CalcMaterialsOld;
DROP TABLE IF EXISTS CalcMaterialDetails;