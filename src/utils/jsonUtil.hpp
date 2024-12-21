#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <string>


std::string jsonToString(rapidjson::Document& doc){
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

rapidjson::Document stringToJson(const std::string& str){
    rapidjson::Document doc;
    doc.Parse(str.c_str());
    return doc;
}