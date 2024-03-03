#ifndef ENCODER_H
#define ENCODER_H
#if 0
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <memory>

namespace Encoder_NSP{
    class Encoder_Base
    {
    public:
        virtual std::string encode() = 0;
        virtual ~Encoder_Base() {}
    };

    class Encoder_Json:public Encoder_Base
    {
    public:
        using Ptr = std::shared_ptr<Encoder_Json>;
        void Init() {
            doc = rapidjson::Document();
            doc.SetObject();
        }
        std::string encode() override;

        void Add_Member(const std::string &key,rapidjson::Value &value);
    private:
        rapidjson::Document doc;
    };


}
#endif
#endif