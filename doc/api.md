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
    "role":1,
    "username":"",
    "password":""
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


//change card's data

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

return:
{
    "code":
}

//change user's data
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

//clock in/off
uri /api/time

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

return:
{
    code:
}


//get one user's card

uri /api/data/card/self

return :
simple:
{
    "cid":{
        "cardname":
        "num":
    },
    "cid":{
        "cardname":
        "num":
    }
    ...
}

no card return null

manager:
{
    "uid":
        "username":
        "cid":{
            "cardname" : 
            "num" : num
        },
        "cid"：{
            "cardname":
            "num":num
        },
        ...
    "uid":
        "username":
        "cidA":{
            "cardname" : 
            "num" : num
        }
        "cid"：{
            "cardname":
            "num":num
        }
        ...
    ...
}
no card return null

//get card data

uri:/api/data/card?page=0

return:
{
    "cid1":
    {
        "cardname"
    }
    "cid2":
    {
        "cardname"
    }
    ...
}

//get  user data

manager:/api/data/user?page=0

need:
return:
{
    "uid1":
    {
        "username":username
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
        "username":username
        "email":
        "telphone":
        "address":
        "sex":  //0 女  1 男
    }
}

manager and user:

//get time data

api/data/time

return:
{
    "username1":
}

attention:
username+data       代表真实用户名
username不加data    代表“username”这个key
