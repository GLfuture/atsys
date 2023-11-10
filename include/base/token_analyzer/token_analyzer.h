#ifndef TOKEN_ANALYZER_H
#define TOKEN_ANALYZER_H
//use cpp-jwt default json lib
#include <jwt/jwt.hpp>
#include <atomic>
#define TOKEN_EXPIRE_TIME   30*60
#define TOKEN_KEY           "token-secret-180580"
#define TOKEN_ALGORITHM     "HS256"
#define TOKEN_ISS           "gong"

using namespace jwt::params;

class Token_Analyzer
{
public:
    Token_Analyzer(){
    }

    static void Move(jwt::jwt_object& to ,jwt::jwt_object& from)
    {
        jwt::jwt_header head;
        head.algo(TOKEN_ALGORITHM);
        to.header(head);
        to.secret(TOKEN_KEY);
        to.payload(from.payload());
    }

    static std::error_code Decode_Token(std::string enc_str,jwt::jwt_object& obj)
    {
        std::error_code code;
        auto dec_obj = jwt::decode(enc_str,algorithms({TOKEN_ALGORITHM}),code,secret(TOKEN_KEY),verify(true));
        Move(obj,dec_obj);
        return code;
    }

    static std::string Encode_Token(const std::string& iss ,const jwt::system_time_t expire_time, const param_init_list_t& params)
    {
        jwt::jwt_object obj{algorithm(TOKEN_ALGORITHM),payload(params),secret(TOKEN_KEY)};
        obj.add_claim("iss",TOKEN_ISS).add_claim("exp",expire_time);
        return obj.signature();
    }

};


#endif