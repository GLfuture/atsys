create database if not exists web;

use web;

create table user (
    uid         bigint primary key AUTO_INCREMENT,
    sex         tinyint,
    username    varchar(20),
    password    varchar(20),
    email       varchar(20),
    phone       varchar(20),
    address     varchar(20)
);

create table card (
    cid         tinyint primary key AUTO_INCREMENT,
    name        varchar(20)
);


create table conn (
    uid         bigint,
    cid         tinyint,
    num         int,
    foreign key(uid) references user(uid),
    foreign key(cid) references card(cid)
);

create table manager (
    mid         int primary key AUTO_INCREMENT,
    username    varchar(20),
    password    varchar(20)
);

create user 'gong'@'%' identified by '123456';

grant all privileges on web.* to 'gong'@'%' ;