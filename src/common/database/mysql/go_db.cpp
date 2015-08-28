#include "database/mysql/go_db.h"

#include "logger/logger.h"
#include "utils/str_util.h"

using namespace ::google::protobuf;

namespace {
const char* s_data_container_item = "items";

void set_proto_data(ProtoData* pd, const FieldDescriptor* fdes, const char* val) {
    const Reflection* ref = pd->GetReflection();
    FieldDescriptor::CppType type = fdes->cpp_type();

    switch (type) {
        case FieldDescriptor::CPPTYPE_INT32:
            ref->SetInt32(pd, fdes, atoi(val));
            break;
        case FieldDescriptor::CPPTYPE_INT64:
            ref->SetInt64(pd, fdes, atoll(val));
            break;
        case FieldDescriptor::CPPTYPE_UINT32:
            ref->SetUInt32(pd, fdes, atoi(val));
            break;
        case FieldDescriptor::CPPTYPE_UINT64:
            ref->SetUInt64(pd, fdes, atoll(val));
            break;
        case FieldDescriptor::CPPTYPE_FLOAT:
            ref->SetFloat(pd, fdes, atof(val));
            break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
            ref->SetDouble(pd, fdes, atof(val));
            break;
        case FieldDescriptor::CPPTYPE_BOOL:
            ref->SetBool(pd, fdes, (atoi(val) > 0) ? true : false);
            break;
        case FieldDescriptor::CPPTYPE_STRING:
            ref->SetString(pd, fdes, val);
            break;
        default:
            ERROR_LOG("[db]不支持的ProtoData类型%d,值为%s", (int)type, val);
    }
}
} // end annoymous namespace

namespace go {

bool db_init() {
    // 
    return true;
}

bool db_deinit() {
    return true;
}

MYSQL* db_connect(const char* ip, unsigned int port, const char* user,
    const char* passwd, const char* db) {
    MYSQL* ret = mysql_init(NULL);

    do {
        char value = 1;
        mysql_options(ret, MYSQL_OPT_RECONNECT, (char*)&value);
        mysql_options(ret, MYSQL_SET_CHARSET_NAME, "utf8");
        MYSQL* return_val = mysql_real_connect(ret, ip, user, passwd, 
            db, port, NULL, CLIENT_MULTI_STATEMENTS);    
        
        if (return_val == NULL) {
            ERROR_LOG("连接mysql失败,连接信息 ip:%s port:%d user:%s passwd:%s db:%s 失败原因%s",
                ip, port, user, passwd, db, mysql_error(ret));
            break;
        }

        return ret;
    } while (false);

    mysql_close(ret);
    return NULL;
}

void db_close(MYSQL* handle) {
    if (handle == NULL) {
        return;
    }

    mysql_close(handle);
}

bool db_exe(MYSQL* handle, const char* cmd) {
    if (handle == NULL || cmd == NULL) {
        return false;
    }

    do {
        int ret = mysql_query(handle, cmd);
        if (ret != 0) {
            ERROR_LOG("[db]执行数据库命令失败,命令%s,失败原因%s", cmd, mysql_error(handle));
            break;
        }

        do {
            MYSQL_RES* res = mysql_store_result(handle);
            if (res != NULL) {
                mysql_free_result(res);
            }
        } while (!mysql_next_result(handle));

        return true;

    } while (false);

    return false;
}

bool db_get_data(MYSQL* handle, const char* cmd, ProtoDataContainer* data) {
    if (handle == NULL || cmd == NULL || data == NULL) {
        return false;
    }
    const Descriptor* des = data->GetDescriptor();
    const FieldDescriptor* fd_des = des->FindFieldByName(s_data_container_item);
    if (fd_des == NULL) {
        ERROR_LOG("[db]提供的数据库数据容器没有定义,数据项名称%s", s_data_container_item);
        return false;
    }

    do {
        int ret = mysql_query(handle, cmd);
        if (ret != 0) {
            ERROR_LOG("[db]获取数据库数据错误,命令%s,失败原因%s", cmd, mysql_error(handle));
        }

        MYSQL_RES* res = mysql_store_result(handle);
        if (res != NULL) {
            const int row_num = mysql_num_rows(res);
            const int field_num = mysql_num_fields(res);

            const Reflection* ref = data->GetReflection();
            if (ref == NULL) {
                goto free_result;
            }

            for (int i = 0; i < row_num; ++i) {
                ProtoData* item = ref->AddMessage(data, fd_des);
                const MYSQL_ROW row = mysql_fetch_row(res);
                
                const Descriptor* item_des = item->GetDescriptor();

                for (int j = 0; j < field_num; ++j) {
                    const MYSQL_FIELD *ifd = mysql_fetch_field_direct(res, j);
                    const FieldDescriptor* item_field_des = item_des->FindFieldByName(ifd->name);
                    if (item_field_des == NULL) {
                        goto free_result;
                    }

                    if (row[j] == NULL || row[j] == util::str::Blank()) {
                        set_proto_data(item, item_field_des, "");
                    } else {
                        set_proto_data(item, item_field_des, row[j]);
                    }
                }
            }

            free_result:
                mysql_free_result(res);
        }

        while (!mysql_next_result(handle)) {
            res =  mysql_store_result(handle);
            if (!res) {
                mysql_free_result(res);
            }
        }

        return true;

    } while (false);

    return false;
}

} // end namespace go
