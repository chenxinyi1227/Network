#include <stdio.h>
#include <json-c/json.h>
#include <json-c/json_object.h>//对象

int main()
{
    /* 新建json对象  "age" : 32*/
    struct json_object * jsonObj = json_object_new_object();

    struct json_object *val = json_object_new_int64(32);
    
    json_object_object_add(jsonObj, "age", val);

    /* 将json对象转换成字符串 */
    const char *str = json_object_to_json_string(jsonObj);
    printf("str:%s\n", str);// {"age": 32 }

    /* 将字符串转换成json对象 */
   // struct json_object * ageObj = json_object_new_string(str);
    //解析
    struct json_object * ageObj = json_tokener_parse(str);
    struct json_object * keyValue = json_object_object_get(ageObj, "age");
    int64_t value = json_object_get_int(keyValue);
    printf( "value:%ld\n", value);
    return 0;
}
