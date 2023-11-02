#ifndef CONFIG_H
#define CONFIG_H
#include "parser.h"

namespace Configer_NSP
{
    class Configer_Base
    {
    public:
        virtual int Load_Conf(const std::string &filepath) = 0;

    protected:
        int Read_Conf(const std::string &filepath, std::string &result);
    };

    class Json_Configer:public Configer_Base
    {
    public:
        using Ptr = std::shared_ptr<Json_Configer>;
        using Json_Parser_Ptr = Parser_NSP::Parser_Json::Ptr;
        Json_Configer()
        {
            Parser= std::make_shared<Parser_NSP::Parser_Json>();
        }
        
        Json_Parser_Ptr Get_Parser()
        {
            return Parser;
        }

        int Load_Conf(const std::string& filepath) override;
    private:
        Json_Parser_Ptr Parser;
    };

    class Yaml_Configer:public Configer_Base
    {
    public:
        using Ptr = std::shared_ptr<Yaml_Configer>;
        using Yaml_Parser_Ptr = Parser_NSP::Parser_Yaml::Ptr;
        Yaml_Configer(){
            Parser = std::make_shared<Parser_NSP::Parser_Yaml>();
        }

        Yaml_Parser_Ptr Get_Parser()
        {
            return Parser;
        }
        

        int Load_Conf(const std::string& filepath) override;
    private:
        Yaml_Parser_Ptr Parser;
    };

    class Config_Configer:public Configer_Base
    {
    public:
        Config_Configer()
        {
            Parser = std::make_shared<Parser_NSP::Parser_Config>();
        }
        using Ptr = std::shared_ptr<Config_Configer>;
        using Config_Parser_Ptr = Parser_NSP::Parser_Config::Ptr;

        int Load_Conf(const std::string& filepath) override;

        Config_Parser_Ptr Get_Parser()
        {
            return Parser;
        }

    private:
        Config_Parser_Ptr Parser;
    };
}


#endif