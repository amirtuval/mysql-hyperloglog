# This is a SQL script that demonstrates the various features of the hyperloglog functions

drop database if exists hll_test;

create database hll_test;

use hll_test;

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
where visit_time > date_sub(now(),INTERVAL 3 DAY)
group by url;

