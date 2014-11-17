# This is a SQL script that demonstrates the various features of the hyperloglog functions

drop database if exists hll_test;

create database hll_test;

use hll_test;

# create raw data table
drop table if exists user_visits;
create table user_visits (
  id integer primary key auto_increment,
  url varchar(255),
  user_id integer,
  visit_time datetime,
  visit_length_in_minutes integer
);

# you must enable loading local files into mysql. See http://dev.mysql.com/doc/refman/5.1/en/load-data-local.html
load data local infile 'data.csv' into table user_visits columns terminated by ',' (url, user_id, visit_time, visit_length_in_minutes);

# get accurate and estimated counts for google visits per day
select date(visit_time), count(distinct user_id) accurate_user_count, hll_compute(user_id) estimated_user_count
from user_visits
where url like '%google%'
group by date(visit_time);

# get accurate and estimated counts for the last 3 days per url
select url, count(distinct user_id) accurate_user_count, hll_compute(user_id) estimated_user_count
from user_visits
where visit_time >= date_sub('2014-06-16',INTERVAL 3 DAY)
group by url;


# create aggregated table
drop table if exists daily_user_visits;
create table daily_user_visits (
  day date,
  url varchar(255),
  user_hll varchar(5468),
  visit_length_in_minutes integer,
  unique(day, url)
);

replace into daily_user_visits(day, url, user_hll, visit_length_in_minutes)
select date(visit_time), url, hll_create(user_id), sum(visit_length_in_minutes)
from user_visits
group by date(visit_time), url;


# get estimated counts for google visits per day
select day, hll_merge_compute(user_hll) estimated_user_count
from daily_user_visits
where url like '%google%'
group by day;

# get accurate and estimated counts for the last 3 days per url
select url, hll_merge_compute(user_hll) estimated_user_count
from daily_user_visits
where day >= date_sub('2014-06-16',INTERVAL 3 DAY)
group by url;


# build aggregation incrementally

truncate table daily_user_visits;

# first insert, no unique violation
insert into daily_user_visits(day, url, user_hll, visit_length_in_minutes)
select date(visit_time), url, hll_create(user_id), sum(visit_length_in_minutes)
from user_visits
where id < 10000
group by date(visit_time), url;

# second insert, on unique violation we update the existing row
insert into daily_user_visits(day, url, user_hll, visit_length_in_minutes)
select date(visit_time), url, hll_create(user_id), sum(visit_length_in_minutes)
from user_visits
where id >= 10000
group by date(visit_time), url
on duplicate key update user_hll=(select hll_merge(user_hll, values(user_hll))), visit_length_in_minutes=visit_length_in_minutes+values(visit_length_in_minutes); 
