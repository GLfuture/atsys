# API

#### login

uri:/api/login

need:
{
    "username":"",
    "password":"",
    "token":""
}

return:

{
    "code":status
    "token":
}

status: 0 //Login success
status: 1 //password error
status: 2 //no user(maybe need to register?)
status: 3 //json has errors