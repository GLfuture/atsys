#if 0
#include "base/encoder/encoder.h"

std::string Encoder_NSP::Encoder_Json::encode()
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}


void Encoder_NSP::Encoder_Json::Add_Member(const std::string &key,rapidjson::Value &value)
{
    doc.AddMember(rapidjson::StringRef(key.c_str()), value, doc.GetAllocator());
}
#endif