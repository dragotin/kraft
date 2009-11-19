ALTER TABLE document ADD COLUMN projectLabel VARCHAR(255) AFTER language;
ALTER TABLE archdoc ADD COLUMN projectLabel VARCHAR(255) AFTER language;

ALTER TABLE archdoc ADD COLUMN tax DECIMAL(5,1) AFTER projectLabel;
ALTER TABLE archdoc ADD COLUMN reducedTax DECIMAL(5,1) AFTER tax;

ALTER TABLE archdocpos DROP COLUMN vat;
ALTER TABLE archdocpos ADD COLUMN taxType INT DEFAULT 0 AFTER overallPrice;

