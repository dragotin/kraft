
ALTER TABLE CalcTime ADD COLUMN timeUnit INT default 0; -- Add a unit id 
ALTER TABLE DocCalcTime ADD COLUMN timeUnit INT default 0; -- Add a unit id 

UPDATE CalcTime set timeUnit=0; -- Update existing CalcTime entries.
UPDATE DocCalcTime set timeUnit=0;

