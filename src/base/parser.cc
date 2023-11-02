#include "base/parser/parser.h"

int Parser_NSP::Parser_Json::Prase(const std::string& content)
{
    rapidjson::Document doc;
    doc.Parse(content.c_str());
    if (doc.HasParseError())
    {
        spdlog::error("File:{} Line:{} Json Parse Error : ErrorCode is {}",__FILE__,__LINE__, std::to_string(doc.GetParseError()));
        return PARSE_ERROR;
    }
    m_root.CopyFrom(doc,doc.GetAllocator());
    return OK;
}


int Parser_NSP::Parser_Yaml::Prase(const std::string& content)
{
    m_root = YAML::Load(content);
    if(m_root.IsNull()) return PARSE_ERROR;
    return OK;
}

int Parser_NSP::Parser_Config::Prase(const std::string &content)
{
    std::stringstream iss(content);
    std::string line;
    while(std::getline(iss,line)){
        if(line[0] == '#') continue; //#为注释
        int indx = line.find('=');
        if(indx == std::string::npos || indx == 0 || indx == line.size()-1) return PARSE_ERROR;//没有等号或者没有key或value为错误
        m_root[line.substr(0,indx)] = line.substr(indx+1);
    }
    return OK;
}

int Parser_NSP::Parser_Config::Get_Value(const std::string &key, std::string &value)
{
    std::map<std::string, std::string>::iterator it = m_root.find(key);
    if (it == m_root.end())
        return NO_MEMBER;
    value = it->second;
    return OK;
}