#include <stdio.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
/* { 
    "name":"runoob", 
    "alexa":10000,
    "shopping":[ "jingdong", "pingduoduo", "Taobao" ] 
    "sites": { 
        "site1":"www.runoob.com", 
        "site2":"m.runoob.com", 
    }
}
 */
int main()
{
    struct json_object *jsonObj = json_object_new_object();

    json_object_object_add(jsonObj, "name", json_object_new_string("runoob"));
    json_object_object_add(jsonObj, "alexa", json_object_new_int(10000));

    struct json_object *jsonArray = json_object_new_array();
    json_object_array_add(jsonArray, json_object_new_string("jingdong"));
    json_object_array_add(jsonArray, json_object_new_string("pingduoduo"));
    json_object_array_add(jsonArray, json_object_new_string("Taobao"));

    json_object_object_add(jsonObj, "shopping", jsonArray);

    struct json_object *jsonsites = json_object_new_object();
    json_object_object_add(jsonsites, "site1", json_object_new_string("www.runoob.com"));
    json_object_object_add(jsonsites, "site2", json_object_new_string("m.runoob.com"));

    json_object_object_add(jsonObj, "sites", jsonsites);

    const char * str = json_object_to_json_string(jsonObj);
    printf("josnObj:%s\n", str);
   

    struct json_object * obj = json_tokener_parse(str);
    struct json_object * name = json_object_object_get(obj, "name");
    printf("name:%s\n", json_object_get_string(name));

    json_object_put(jsonObj);
    return 0;
}
