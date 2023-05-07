/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:38:48
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 18:58:48
 */

#ifndef _HTTPREQUEST_H_
#define _HTTPREQUEST_H_

#include <map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>
#include <mysql/mysql.h>
#include "buffer/buffer.h"
#include "log/log.h"
#include "pool/sqlpool.h"
#include "pool/sqlraii.h"

class HttpRequest
{
public: // 枚举 表示操作各种不同的状态
    enum PARSE_STATE
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

public:
    HttpRequest();
    ~HttpRequest() = default;

    void init();                                       // 初始化成员变量
    bool parse(std::shared_ptr<Buffer> buf);           // 处理http请求
    bool isKeepAlive() const;                          // connection是否是keep-alive
    std::string getPost(const std::string &key) const; // 获取post表单内容
    std::string getPost(const char *key) const;        // 获取post表单内容

    inline std::string path() const { return m_path; };       // 返回m_path
    inline std::string &path() { return m_path; };            // 返回m_path
    inline std::string method() const { return m_method; };   // 返回m_method
    inline std::string version() const { return m_version; }; // 返回m_version

private:
    bool parseHeader(const std::string &line);      // 处理请求首行
    void parseRequestLine(const std::string &line); // 处理请求除首行以外的行
    void parseBody(const std::string &line);        // 处理格式不是 标题：内容 形式的行
    void parsePath();                               // 将文件信息补充完整
    void parsePost();                               // 处理post请求
    void parseFormUrlEncoded();                     // 处理表单内容

private:
    // 验证用户名
    static bool verifyUser(const std::string &name, const std::string &pwd, bool isLogin);
    // 验证密码
    static bool verifyPassword(MYSQL *sql, const std::string pwd);
    // 注册
    static bool registerUser(const std::string &name, const std::string &pwd, MYSQL *sql);
    // 16进制转10进制
    static int converHex(char ch);

private:
    PARSE_STATE m_state;   // 操作处于的步骤
    std::string m_version; // HTTP协议版本
    std::string m_method;  // 请求方法
    std::string m_path;    // 请求文件
    std::string m_body;    // 表单内容

    std::unordered_map<std::string, std::string> m_header; // http请求除了首行以外的内容
    std::unordered_map<std::string, std::string> m_post;   // 处理好的表单信息

private:
    static const std::unordered_set<std::string> DEFAULT_HTML;          // 默认HTML页面文件名前缀
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG; // 注册登陆的页面
};

#endif // _HTTPREQUEST_H_