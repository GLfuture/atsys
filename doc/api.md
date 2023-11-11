# API

return code type:

code: 0   //Login success
code: 1   //password error
code: 2   //no user(maybe need to register?)
code: 3   //register success
code: 4   //has repeated username
code: 5   //register failed
code: 6   //logout success
code: 7   //logout fail
code: 8   //has no token in head
code: 9   //token expire 
code: 10  //json has errors
code: 11  //json doesn't has necessary member
code: 12  //method operates successfully
code: 13  //method operates fail
code: 14  //method error
code: 15  //privilidge error
code: 16  //clok reaptedly(already clocked)
code: 17  //didn't clock in(not ready to clock)
#### login

uri:/api/login

need:
{
    "role":1
    "username":"",
    "password":"",
}

role:
0   //manager
1   //simple user

return:
head: Token
{
    "code":code
}

uri:/api/register

need:
{
    "username":  varchar(15)
    "password":  varchar(20)
    "email":     varchar(20)
    "telphone":  varchar(20)
    "address":   varchar(20)
    "sex": int   //0 女 1 男
}

return :
{
    "code": code 
}



uri:/api/card


method: 0 //new card need card's name
method: 1 //delete need uid and cid(simple user del conn record(use uid and cid)     manager del card record(use cid))
method: 2 //update need uid and cid and num

need:
{
    method:0
    cname:""
}
return:
{
    "code":
}
or
need:
{
    method:1
    cid:
}
return :
{
    "code":
}
or
need:
simple user:
{
    method:2
    cid:
    num:        //+为加,-为减
}
manager:
{
    method:2
    uid:
    cid:
    num:
}

uri /api/user
need:
{
    //需要改哪个字段传哪个字段，也可以全传
    "username":  varchar(15)
    "password":  varchar(20)
    "email":     varchar(20)
    "telphone":  varchar(20)
    "address":   varchar(20)
    "sex": int   //0 女 1 男
}
return:
{
    code:
}

/api/time
simple user
need:
{
    method:1 //1为打卡开始,0为结束
}
return:
{
    code:
}
need:
{
    method:0
}
return:
{
    time:   //时间戳
    code:
}
manager:



// GET 
uri /api/data/card
return :
simple:
{
    "cardname1":{
        "cid":
        "num":
    },
    "cardname2":{
        "cid":
        "num":
    }
    ...
}
no card return null
manager:
{
    "username1":
        "uid":
        "cardname1":{
            "cid" : 
            "num" : num
        },
        "cardname2"：{
            "cid":
            "num":num
        },
        ...
    "username2":
        "uid":
        "cardname1":{
            "cid" : 
            "num" : num
        }
        "cardname2"：{
            "cid":
            "num":num
        }
        ...
    ...
}
no card return null

uri /api/data/user
need:
manager:/api/data/user?page=0
return:
{
    "uid1":
    {
        "username":
        "email":
        "telphone":
        "address":
        "sex":  //0 女  1 男
    }
    ...
}

user:api/data/user
return:
{
    "uid":
    {
        "username":
        "email":
        "telphone":
        "address":
        "sex":  //0 女  1 男
    }
}

