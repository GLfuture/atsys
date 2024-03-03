#pragma once
#ifndef HTTP_HPP
#define HTTP_HPP
#endif
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <memory>
namespace HTTP_NSP
{
    using std::string;
    using std::string_view;
    using std::unordered_map;
    using std::vector;
    using Http_kv_List = unordered_map<string, string>;
    using Http_String = string_view;
    
    class HTTP
    {
    public:
        using Ptr = std::shared_ptr<HTTP>;
        enum HTTP_RET_ERROR_CODE{
            INCOMPLETE = -100,         //不完整的包
            PROTO_ERROR,            //协议格式错误
            LENGTH_ERROR,           //长度错误
        };


        int Parse(const string &request)
        {
            int beg = 0;
            int end = request.find("\r\n\r\n");
            if(end == std::string::npos) return INCOMPLETE;
            string head = request.substr(beg, end);
            Decode_Head(head);
            if(Request_Get_Http_Type().compare("OPTIONS") == 0){
                return end + 4;
            }
            beg = end + 4;
            std::string_view len_str = Request_Get_Key_Value("Content-Length");
            if(len_str.empty()) return PROTO_ERROR;
            int content_len = atoi(len_str.cbegin());
            string other = request.substr(beg);
            if(content_len > other.length()) return INCOMPLETE;
            string body = request.substr(beg , content_len);
            Decode_Body(body);
            return beg + content_len + 2;
        }
        
        string Content_Head()
        {
            string response_head = "HTTP/" + this->version + " " + std::to_string(this->status) + " ";
            response_head = response_head + Status_Str() + "\r\n";
            for (auto &[k, v] : this->resopnse_list)
            {
                response_head = response_head + k + ": " + v + "\r\n";
            }
            response_head += "\r\n";
            return response_head;
        }

        Http_String Get_Version()
        {
            return this->version;
        }

        void Set_Version(Http_String version)
        {
            this->version.assign(version.cbegin(), version.cend());
        }

        Http_String Request_Get_Url() const
        {
            return this->url;
        }

        Http_String Request_Get_Http_Type() const
        {
            return this->http_type;
        }

        Http_String Request_Get_Key_Value(Http_String key) const
        {
            Http_kv_List::const_iterator it = filed_list.find(key.cbegin());
            if(it == filed_list.end()) return "";
            return it->second.c_str();
        }

        Http_kv_List Request_Get_kv_List() const
        {
            return this->filed_list;
        }

        Http_String Request_Get_Arg_Value(Http_String key) const 
        {
            Http_kv_List::const_iterator it = arg_list.find(key.cbegin());
            if(it == arg_list.end()) return "";
            return it->second.c_str();
        }
 
        Http_String Request_Get_Body() const 
        {
            return this->request_body;
        }

        uint16_t Response_Get_Status() const
        {
            return this->status;
        }

        void Response_Set_Status(const uint16_t status) 
        {
            this->status = status;
        }

        void Response_Set_Key_Value(Http_String key, Http_String value)
        {
            string k(key.cbegin(), key.cend()), v(value.cbegin(), value.cend());
            this->resopnse_list[k] = v;
        }

    private:
        // 未列出所有值
        string Status_Str()
        {
            switch (this->status)
            {
            case 100:
                return "Continue";
            case 101:
                return "Switching Protocols";
            case 200:
                return "OK";
            case 201:
                return "Created";
            case 202:
                return "Accepted";
            case 203:
                return "Non-Authoritative Information";
            case 204:
                return "No Content";
            case 205:
                return "Reset Content";
            case 206:
                return "Partial Content";
            case 300:
                return "Multiple Choices";
            case 301:
                return "Moved Permanently";
            case 302:
                return "Found";
            case 303:
                return "See Other";
            case 304:
                return "Not Modified";
            case 305:
                return "Use Proxy";
            case 306:
                return "Unused";
            case 307:
                return "Temporary Redirect";
            case 400:
                return "Bad Request";
            case 401:
                return "Unauthorized";
            case 402:
                return "Payment Required";
            case 403:
                return "Forbidden";
            case 404:
                return "Not Found";
            case 405:
                return "Method Not Allowed";
            case 406:
                return "Not Acceptable";
            case 407:
                return "Proxy Authentication Required";
            case 408:
                return "Request Time-out";
            case 409:
                return "Conflict";
            case 410:
                return "Gone";
            case 411:
                return "Length Required";
            case 412:
                return "Precondition Failed";
            case 413:
                return "Request Entity Too Large";
            case 414:
                return "Request-URI Too Large";
            case 415:
                return "Unsupported Media Type";
            case 416:
                return "Requested range not satisfiable";
            case 417:
                return "Expectation Failed";
            case 500:
                return "Internal Server Error";
            case 501:
                return "Not Implemented";
            case 502:
                return "Bad Gateway";
            case 503:
                return "Service Unavailable";
            case 504:
                return "Gateway Time-out";
            case 505:
                return "HTTP Version not supported";
            default:
                break;
            }
            return "";
        }

        vector<string> Get_Lines(string request)
        {
            int cur = 0, end = 0;
            vector<string> res;
            do
            {
                end = request.find("\r\n", cur);
                string temp = request.substr(cur, end - cur);
                if (!temp.empty())
                    res.emplace_back(temp);
                cur = end + 2;
            } while (end != string::npos);
            return res;
        }

        // 解析http头
        void Decode_Head(const string &request) // 解析
        {
            vector<string> lines = Get_Lines(request);
            for (int i = 0; i < lines.size(); i++)
            {
                int index = 0;
                if (i == 0)
                {
                    index = Decode_Request_Type(lines[i], index);
                    index = Decode_Url(lines[i], index);
                    index = Decode_Version(lines[i], index);
                }
                else
                {
                    Decode_Fields(lines[i], 0);
                }
            }
        }

        // 解析http的body
        void Decode_Body(const string &body)
        {
            this->request_body = body.c_str();
        }

        // 解析其他字段
        int Decode_Fields(string str, int index)
        {
            int beg = 0, end = 0;
            end = str.find_first_of(":");
            string key = str.substr(0, end);
            beg = end + 2;
            string value = str.substr(beg);
            this->filed_list[key] = value;
            return 0;
        }

        // 解析版本号
        int Decode_Version(string str, int index)
        {
            int beg = str.find('/', index);
            int end = str.find("\r\n");
            this->version = str.substr(beg + 1, end - beg - 1);
            return end + 1;
        }

        // 解析请求类型
        int Decode_Request_Type(string str, int index)
        {
            int end = str.find_first_of(' ');
            this->http_type = str.substr(index, end);
            return end + 1;
        }

        // 解析参数
        int Decode_Url(string str, int index)
        {
            int end = str.find_first_of(' ', index);
            string URL = str.substr(index, end - index);
            int argindx = URL.find('?', index);
            if (argindx != string::npos)
            {
                int cur = 0;
                this->url = URL.substr(cur, argindx - cur);
                string args = URL.substr(argindx + 1);
                string key = "", value = "";
                int status = 0;
                for (int i = 0; i < args.length(); i++)
                {
                    if (status == 0)
                    {
                        if (args[i] == '=')
                        {
                            status = 1;
                        }
                        else
                        {
                            key += args[i];
                        }
                    }
                    else
                    {
                        if (args[i] == '&')
                        {
                            this->arg_list[key] = value;
                            key = "", value = "";
                            status = 0;
                        }
                        else
                        {
                            value += args[i];
                            if (i == args.length() - 1)
                            {
                                this->arg_list[key] = value;
                            }
                        }
                    }
                }
                return end + 1;
            }
            this->url = URL;
            return end + 1;
        }

    private:
        string version;             // 版本号
        string http_type;           // http协议类型
        string url;                 // url
        Http_kv_List arg_list;      // 参数
        Http_kv_List filed_list;    // 其他字段,如Host：
        Http_kv_List resopnse_list; // 回应头
        string request_body;        // request消息体
        uint16_t status;            // 状态
    };

}
