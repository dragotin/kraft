# message Creating attributes table...
CREATE TABLE attributes (
  hostObject VARCHAR(255),
  hostId     INT,
  name       VARCHAR(255),
  value      MEDIUMTEXT,

  PRIMARY KEY( hostObject, hostId)
);
