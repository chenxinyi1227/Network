#include <stdio.h>
#include <json-c/json.h>
#include <json-c/json_object.h>//对象

int main()
{

    /* { "way" : 1, "name" : "zhangsan", "passwd" : "abd123" } */
    /* 新建json对象  "age" : 32*/
    struct json_object * jsonObj = json_object_new_object();
    if(jsonObj == NULL)
    {
        perror("json object error");
        exit(-1);
    }

    struct json_object *wayVal = json_object_new_int64(32);
    if(wayVal == NULL)
    {
        perror("json object error");
        exit(-1);
    }
    int ret = json_object_object_add(jsonObj, "way", wayVal);
    if(ret < 0)
    {
        /* 释放以上两个 todo.. */
         perror("new way json object error");
        exit(-1);
    }
    
    struct json_object *nameVal = json_object_new_string("zhang");
    if(nameVal == NULL)
    {
        perror("new name json object error");
        exit(-1);
    }
    json_object_object_add(jsonObj, "name", nameVal);


    /* 将json对象转换成字符串 */
    const char *str = json_object_to_json_string(jsonObj);
    printf("str:%s\n", str);// {"age": 32 }

//服务器代码 =》
    printf("==============demo server===================\n");
    /* 将字符串转换成json对象 */
   // struct json_object * ageObj = json_object_new_string(str);
    //解析
    struct json_object * newObj = json_tokener_parse(str);
    struct json_object * wayValue = json_object_object_get(newObj, "way");
    //解析int类型的值
    int64_t value_way = json_object_get_int(wayValue);
    printf( "value_way:%ld\n", value_way);

    struct json_object * nameValue = json_object_object_get(newObj, "name");
    const char * value_name = json_object_get_string(nameValue);
    printf( "value_name:%s\n", value_name);

    json_object_put(jsonObj);
    return 0;
}
