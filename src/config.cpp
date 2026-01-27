#include "config.hpp"

void Config::parseCmd(std::vector<String> &tokens, JsonDocument &_res_doc)
{
    int _tokenCount = tokens.size();

    if (_tokenCount > 1)
    {
        String subCmd = tokens[1];
        
        if (subCmd == "load")
        {
            load();
            _res_doc["result"] = "ok";
            _res_doc["ms"] = "config loaded";
        }
        else if (subCmd == "save")
        {
            save();
            _res_doc["result"] = "ok";
            _res_doc["ms"] = "config saved";
        }
        else if (subCmd == "dump")
        {
            String jsonStr = dump();
            DeserializationError error = deserializeJson(_res_doc["ms"], jsonStr);
            if (error)
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "json parse error";
            }
            else
            {
                _res_doc["result"] = "ok";
            }
        }
        else if (subCmd == "clear")
        {
            clear();
            _res_doc["result"] = "ok";
            _res_doc["ms"] = "config cleared";
        }
        else if (subCmd == "set")
        {
            if (_tokenCount > 3)
            {
                String key = tokens[2];
                String value = tokens[3];

                if (isNumber(value))
                {
                    set(key.c_str(), value.toInt());
                }
                else
                {
                    set(key.c_str(), value);
                }

                _res_doc["result"] = "ok";
                _res_doc["ms"] = "config set";
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "need key and value";
            }
        }
        else if (subCmd == "get")
        {
            if (_tokenCount > 2)
            {
                String key = tokens[2];

                if (!hasKey(key.c_str()))
                {
                    _res_doc["result"] = "fail";
                    _res_doc["ms"] = "key not exist";
                }
                else
                {
                    _res_doc["result"] = "ok";
                    _res_doc["value"] = get<String>(key.c_str());
                }
            }
            else
            {
                _res_doc["result"] = "fail";
                _res_doc["ms"] = "need key";
            }
        }
        else
        {
            _res_doc["result"] = "fail";
            _res_doc["ms"] = "unknown sub command";
        }
    }
    else
    {
        _res_doc["result"] = "fail";
        _res_doc["ms"] = "need sub command";
    }
}
