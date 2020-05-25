create database weather_v2
create database status
create database voltage

-------------------------------------------------------------------------------
SOLAR POWER
-------------------------------------------------------------------------------
CREATE CONTINUOUS QUERY cq_power_1m on voltage BEGIN
    SELECT max(power) AS p_max, min(power) AS p_min,
        mean(power) as power, mean(voltage) AS voltage
    INTO voltage.monthly.mppt_aggregated
    FROM voltage.realtime.mppt
    GROUP BY time(1m),type
END
CREATE CONTINUOUS QUERY "cq_power_1h" ON "voltage" BEGIN
    SELECT max("power") AS p_max,min(power) AS p_min,
        mean(power) as power, mean(voltage) AS voltage
    INTO "yearly"."mppt_aggregated"
    FROM realtime.mppt
    GROUP BY time(1h), type
END
CREATE CONTINUOUS QUERY "cq_power_6h" ON "voltage" BEGIN
    SELECT max("power") AS p_max,min(power) AS p_min,
        mean(power) as power, mean(voltage) AS voltage
    INTO "infinite"."mppt_aggregated"
    FROM realtime.mppt
    GROUP BY time(6h), type
END

-------------------------------------------------------------------------------
DB STRUCTURE - Weather_v2
-------------------------------------------------------------------------------


create retention policy realtime on weather_v2 duration 168h replication 1 shard duration 1h
create retention policy monthly on weather_v2 duration 720h replication 1 shard duration 24h
create retention policy yearly on weather_v2 duration 8760h replication 1 shard duration 168h
create retention policy infinite on weather_v2 duration 0s replication 1 shard duration 720h

show retention policies on weather_v2
alter retention policy realtime on weather_v2 default

# if necesary copy the shite over to new retentions - especially if you are modifying an existing installation
select time,value into realtime.rain from autogen.rain where time > now()-1w group by *
select time,humidity,pressure,temperature into realtime.temphumi from autogen.temphumi where time > now()-1w group by *
select time,humidity,temperature into realtime.usense from autogen.usense where time > now()-1w group by *
select time,value into realtime.wind from autogen.wind where time > now()-1w group by *

select time,value into yearly.wind from realtime.wind group by time(1h), type
...


-------------------------------------------------------------------------------
WIND
-------------------------------------------------------------------------------
CREATE CONTINUOUS QUERY "cq_wind_10m" ON "weather_v2" BEGIN
    SELECT max(value) AS wind_max, mean(value) AS wind, min(value) AS wind_min
    INTO "monthly"."wind_aggregated"
    FROM realtime.wind
    GROUP BY time(10m), type
END
CREATE CONTINUOUS QUERY "cq_wind_1h" ON "weather_v2" BEGIN
    SELECT max(value) AS wind_max, mean(value) AS wind, min(value) AS wind_min
    INTO "yearly"."wind_aggregated"
    FROM realtime.wind
    GROUP BY time(1h), type
END
CREATE CONTINUOUS QUERY "cq_wind_6h" ON "weather_v2" BEGIN
    SELECT max(value) AS wind_max, mean(value) AS wind, min(value) AS wind_min
    INTO "infinite"."wind_aggregated"
    FROM realtime.wind
    GROUP BY time(6h), type
END

-------------------------------------------------------------------------------
RAIN
-------------------------------------------------------------------------------
CREATE CONTINUOUS QUERY "cq_rain_10m" ON "weather_v2" BEGIN
    SELECT max("value") AS val_max, mean(value) AS value
    INTO "monthly"."rainrate_aggregated"
    FROM realtime.rain
    GROUP BY type,time(10m)
END
CREATE CONTINUOUS QUERY "cq_rain_1h" ON "weather_v2" BEGIN
    SELECT max("value") AS val_max, mean(value) AS value
    INTO "yearly"."rainrate_aggregated"
    FROM realtime.rain
    GROUP BY type,time(1h)
END
CREATE CONTINUOUS QUERY "cq_rain_6h" ON "weather_v2" BEGIN
    SELECT max("value") AS val_max, mean(value) AS value
    INTO "infinite"."rainrate_aggregated"
    FROM realtime.rain
    GROUP BY type,time(6h)
END
-------------------------------------------------------------------------------
TEMPHUMI
-------------------------------------------------------------------------------
CREATE CONTINUOUS QUERY "cq_temphumi_10m" ON "weather_v2" BEGIN
    SELECT
        max("humidity") AS humidity_max,
        min("humidity") AS humidity_min,
        mean("humidity") AS humidity,
        max("temperature") AS temperature_max,
        min("temperature") AS temperature_min,
        mean("temperature") AS temperature
    INTO "monthly"."temphumi_aggregated"
    FROM realtime.temphumi
    GROUP BY type, time(10m)
END

CREATE CONTINUOUS QUERY "cq_temphumi_1h" ON "weather_v2" BEGIN
    SELECT
        max("humidity") AS humidity_max,
        min("humidity") AS humidity_min,
        mean("humidity") AS humidity,
        max("temperature") AS temperature_max,
        min("temperature") AS temperature_min,
        mean("temperature") AS temperature
    INTO "yearly"."temphumi_aggregated"
    FROM realtime.temphumi
    GROUP BY type, time(1h)
END

CREATE CONTINUOUS QUERY "cq_temphumi_6h" ON "weather_v2" BEGIN
    SELECT
        max("humidity") AS humidity_max,
        min("humidity") AS humidity_min,
        mean("humidity") AS humidity,
        max("temperature") AS temperature_max,
        min("temperature") AS temperature_min,
        mean("temperature") AS temperature
    INTO "infinite"."temphumi_aggregated"
    FROM realtime.temphumi
    GROUP BY type, time(6h)
END

-------------------------------------------------------------------------------
USENSE
-------------------------------------------------------------------------------
CREATE CONTINUOUS QUERY "cq_usense_6h" ON "weather_v2" BEGIN
    SELECT mean("battery") AS battery, mean(humidity) AS humidity, mean(temperature) AS temperature
    INTO "yearly"."usense_aggregated"
    FROM realtime.usense
    GROUP BY type,time(6h)
END

CREATE CONTINUOUS QUERY "cq_usense_12h" ON "weather_v2" BEGIN
    SELECT mean("battery") AS battery, mean(humidity) AS humidity, mean(temperature) AS temperature
    INTO "infinite"."usense_aggregated"
    FROM realtime.usense
    GROUP BY type,time(12h)
END


-------------------------------------------------------------------------------
STATUS
-------------------------------------------------------------------------------
    POLICIES
    --------
create retention policy realtime on status duration 168h replication 1 shard duration 1h
create retention policy monthly on status duration 720h replication 1 shard duration 24h
create retention policy yearly on status duration 8760h replication 1 shard duration 168h
create retention policy infinite on status duration 0s replication 1 shard duration 720h

select time,usage into realtime.RasPI_aggregated from autogen.RasPI where time > now()-1w group by *
select time,voltage into realtime.iss_aggregated from autogen.iss where time > now()-1w group by *

--
SELECT NON_NEGATIVE_DERIVATIVE(max(*)) as traffic INTO "monthly"."net_aggregated" FROM autogen.net WHERE time > now()-2w GROUP BY time(30s)
CQ
--
CREATE CONTINUOUS QUERY "cq_net_1m" ON "status" BEGIN
    SELECT NON_NEGATIVE_DERIVATIVE(max(*)) as traffic
    INTO "monthly"."net_aggregated"
    FROM realtime.net
    WHERE time > now()-1m
    GROUP BY time(30s)
END

alter retention policy realtime on status default
