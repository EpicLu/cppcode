/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:40:00
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 18:58:16
 */

#include "http/httprequest.h"

const std::unordered_set<std::string>
    HttpRequest::DEFAULT_HTML = {
        "/index",
        "/register",
        "/login",
        "/welcome",
        "/video",
        "/picture",
};

const std::unordered_map<std::string, int>
    HttpRequest::DEFAULT_HTML_TAG = {
        {"/register.html", 0},
        {"/login.html", 1},
};

HttpRequest::HttpRequest()
{
    init();
}

void HttpRequest::init()
{
    m_version = "";
    m_method = "";
    m_path = "";
    m_body = "";
    m_state = HEADERS;
    m_header.reserve(16);
    m_post.reserve(16);
}

bool HttpRequest::parse(std::shared_ptr<Buffer> buf)
{
    const char CRLF[] = "\r\n";
    if (buf->readableBytes() <= 0)
    {
        return false;
    }
    while (buf->readableBytes() && m_state != FINISH)
    {
        // 获取Http请求头的行
        char *lineEnd = std::search(buf->peek(), buf->beginWrite(), CRLF, CRLF + 2);
        std::string line(buf->peek(), lineEnd);

        switch (m_state)
        {
        case HEADERS:
            if (!parseHeader(line))
                return false;

            parsePath();
            break;

        case REQUEST_LINE:
            parseRequestLine(line);

            if (buf->readableBytes() <= 2) // "\r\n"
                m_state = FINISH;

            break;

        case BODY:
            parseBody(line);
            break;

        default:
            break;
        }

        if (lineEnd == buf->beginWrite()) // 读完了
            break;

        buf->retrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", m_method.c_str(), m_path.c_str(), m_version.c_str());

    return true;
}

void HttpRequest::parsePath()
{
    if (m_path == "/")
    {
        m_path = "/index.html";
    }
    else
    {
        for (const auto &item : DEFAULT_HTML)
        {
            if (item == m_path)
            {
                m_path += ".html";
                break;
            }
        }
    }
}

bool HttpRequest::parseHeader(const std::string &line)
{
    // 正则表达式获取请求头首行的信息
    // 例如GET /index.html HTTP/1.1
    // 获取的就是GET /index.html 1.1
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;

    if (std::regex_match(line, subMatch, patten))
    {
        m_method = subMatch[1];
        m_path = subMatch[2];
        m_version = subMatch[3];
        m_state = REQUEST_LINE;
        return true;
    }
    LOG_ERROR("header error arg:\"line\" is %s", line.c_str());

    return false;
}

void HttpRequest::parseRequestLine(const std::string &line)
{
    // 正则表达式获取除首行以外的信息
    // 例如Connection: close
    // 则获取的就是Connection和close
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;

    if (std::regex_match(line, subMatch, patten))
    {
        m_header[subMatch[1]] = subMatch[2];
    }
    else
    {
        m_state = BODY;
    }
}

void HttpRequest::parseBody(const std::string &line)
{
    m_body = line;
    parsePost();
    m_state = FINISH;

    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::parsePost()
{
    // 处理POST请求
    if (m_method == "POST" && m_header["Content-Type"] == "application/x-www-form-urlencoded")
    {
        parseFormUrlEncoded();

        if (DEFAULT_HTML_TAG.count(m_path))
        {
            int tag = DEFAULT_HTML_TAG.find(m_path)->second;
            LOG_DEBUG("Tag:%d", tag);
            if (tag == 0 || tag == 1)
            {
                bool isLogin = (tag == 1);
                if (verifyUser(m_post["username"], m_post["password"], isLogin))
                {
                    m_path = "/welcome.html";
                } // !(userVerify(m_post["username"], m_post["password"], isLogin))
                else
                {
                    m_path = "/error.html";
                }

            } // (tag == 0 || tag == 1)

        } // !(DEFAULT_HTML_TAG.count(m_path))

    } // !(m_method == "POST" && m_header["Content-Type"] == "application/x-www-form-urlencoded")
}

void HttpRequest::parseFormUrlEncoded()
{
    // 一个简单的表单信息形式如下
    // key1=value1&key2=value2
    // 本函数用于获取上述的key和value
    if (m_body.size() == 0)
        return;

    std::string key, value;
    int num = 0;
    int n = m_body.size();
    int i = 0, j = 0;

    for (; i < n; i++)
    {
        char ch = m_body[i];

        switch (ch)
        {
        case '=':
            key = m_body.substr(j, i - j);
            j = i + 1;
            break;

        case '+': // 空格
            m_body[i] = ' ';
            break;

        case '%': // 十六进制ASCII编码
            num = converHex(m_body[i + 1]) * 16 + converHex(m_body[i + 2]);
            m_body[i + 2] = num % 10 + '0';
            m_body[i + 1] = num / 10 + '0';
            i += 2;
            break;

        case '&':
            value = m_body.substr(j, i - j);
            j = i + 1;
            m_post[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;

        default:
            break;
        }
    }
    assert(j <= i);

    if (m_post.count(key) == 0 && j < i)
    {
        value = m_body.substr(j, i - j);
        m_post[key] = value;
    }
}

bool HttpRequest::verifyUser(const std::string &name, const std::string &pwd, bool isLogin)
{
    if (name == "" || pwd == "")
        return false;

    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());

    MYSQL *sql = nullptr;
    SqlRAII(&sql, SqlPool::getInstance());
    assert(sql);

    bool flag = false;
    char order[256] = {0};

    // 注册
    if (!isLogin)
    {
        flag = registerUser(name, pwd, sql);
        return flag;
    }

    // 查询用户
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);

    if (mysql_query(sql, order))
    {
        LOG_DEBUG("sql search error: %s", mysql_error(sql));
        return false;
    }
    flag = verifyPassword(sql, pwd);

    if (flag)
        LOG_DEBUG("userVerify success");

    return flag;
}

bool HttpRequest::verifyPassword(MYSQL *sql, const std::string pwd)
{
    MYSQL_RES *res = nullptr;
    res = mysql_store_result(sql);

    while (MYSQL_ROW row = mysql_fetch_row(res))
    {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string password(row[1]);

        if (pwd == password)
        {
            mysql_free_result(res);
            return true;
        }
    }
    mysql_free_result(res);
    LOG_DEBUG("password error");

    return false;
}

bool HttpRequest::registerUser(const std::string &name, const std::string &pwd, MYSQL *sql)
{
    char order[256] = {0};

    LOG_DEBUG("regirster now");
    bzero(order, 256);
    snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
    LOG_DEBUG("%s", order);

    if (mysql_query(sql, order))
    {
        LOG_DEBUG("regirster error");
        return false;
    }

    return true;
}

int HttpRequest::converHex(char ch)
{
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return ch;
}

std::string HttpRequest::getPost(const std::string &key) const
{
    assert(key != "");
    if (m_post.count(key) == 1)
        return m_post.find(key)->second;

    return "";
}

std::string HttpRequest::getPost(const char *key) const
{
    assert(key != nullptr);
    if (m_post.count(key) == 1)
        return m_post.find(key)->second;

    return "";
}

bool HttpRequest::isKeepAlive() const
{
    if (m_header.count("Connection") == 1)
    {
        return m_header.find("Connection")->second == "keep-alive" && m_version == "1.1";
    }
    return false;
}