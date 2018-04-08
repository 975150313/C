/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 *
 * This program fetches HTTP URLs.
 */
#include "mongoose.h"
#include <sstream>
static int s_exit_flag = 0;

#define POSTFIELDS1 "username=aitest&password=123456&manage=vip&rememberme=true&isApp=1"
#define POSTFIELDS2 "action=get_carer_psgs&lineId=2757&issueId=244&classesId=6006"
#define POSTFIELDS3 "imgName=/ossfs/cpn/user/psg_photo/20170904/05f9f6c9-5b72-41ff-a04e-5fda53fc994a_small.jpg"
#define CONTENTTYPE "Content-Type: application/x-www-form-urlencoded\r\n"
#define CONTENTTYPE1 "Content-Type: application/json;charset=utf-8\r\n"

#define  DEBUG  1



using namespace std;
string cookie;
struct mg_mgr mgr;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    switch (ev) {
        case MG_EV_CONNECT:
            if (*(int *) ev_data != 0) {
                fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
                s_exit_flag = 1;
            }
            break;
        case MG_EV_HTTP_REPLY://建立http服务器并收到数据
            switch(hm->resp_code){
                case 200://请求已成功
                {
#if DEBUG
//            fwrite(hm->message.p, 1, hm->message.len, stdout);
                    fwrite(hm->body.p, 1, hm->body.len, stdout);
                    //这是是接收到的数据
#endif
                    nc->flags |= MG_F_SEND_AND_CLOSE;
                    s_exit_flag = 1;
                    // 获取真正的cookie  必须使用
                    struct mg_str *cookie_header = mg_get_http_header(hm, "Set-Cookie");
                    if(cookie_header == nullptr){
                        return;
                    } else{
                        cookie.clear();
                        cookie.append(CONTENTTYPE);
                        cookie.append("Cookie:");
                        cookie.append(cookie_header->p,cookie_header->len);
                        cookie.append("\r\n");
                    }


                }
                    break;
                case 302://重定向 请求的资源临时从不同的 URI响应请求
                {
                    //获取设置Cookie头  可以不使用
//                    struct mg_str *cookie_header = mg_get_http_header(hm, "Set-Cookie");
//                    cookie.clear();
//                    cookie.append("Cookie:");
//                    cookie.append(cookie_header->p,cookie_header->len);
//                    cookie.append("\r\n");
                    struct mg_str *url = mg_get_http_header(hm, "Location");
                    //获取 Location
                    string strURL;
                    strURL.append(url->p,url->len);
                    mg_connect_http(&mgr, ev_handler,strURL.c_str(), NULL, NULL);
                }
                    break;
                default:
                    break;
            }
            break;
        case MG_EV_CLOSE:
            if (s_exit_flag == 0) {
                printf("Server closed connection\n");
                s_exit_flag = 1;
            }
            break;
        default:
            break;
    }
}
void connectHhttp(const char* url,const char* contentType,const char* params){
    mg_connect_http(&mgr, ev_handler, url, contentType,params );//POST模拟表单提交
    while (s_exit_flag == 0) {
        mg_mgr_poll(&mgr, 1000);
    }
    s_exit_flag=0;//重置标志位
}
int main(int argc, char *argv[]) {
    mg_mgr_init(&mgr, NULL);

    connectHhttp( "http://127.0.0.1/login", CONTENTTYPE,POSTFIELDS1 );
    connectHhttp( "http://127.0.0.1/auth/api", cookie.c_str(),POSTFIELDS2);
    connectHhttp( "http://127.0.0.1/getImg", cookie.c_str(),POSTFIELDS3 );

    mg_mgr_free(&mgr);
    return 0;
}