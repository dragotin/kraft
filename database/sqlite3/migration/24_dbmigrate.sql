
ALTER TABLE units ADD COLUMN ec20 VARCHAR(10);

UPDATE units set ec20 = "MTR" WHERE unitShort = "m";
UPDATE units set ec20 = "MTK" WHERE unitShort = "qm" or unitShort = "sm";
UPDATE units set ec20 = "MTQ" WHERE unitShort = "cbm";
UPDATE units set ec20 = "XSA" WHERE unitLong = "Sack" or unitLong = "Bag";
UPDATE units set ec20 = "LTR" WHERE unitLong = "Liter";
UPDATE units set ec20 = "KGM" WHERE unitLong = "Kilogramm";
UPDATE units set ec20 = "XPP" WHERE unitShort = "Stck." or unitShort ="Pcs.";
UPDATE units set ec20 = "TNE" WHERE unitShort = "t";
UPDATE units set ec20 = "HUR" WHERE unitLong = "Stunde" or unitLong ="Hour";




