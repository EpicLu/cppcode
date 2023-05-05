/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:36:39
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-06 01:38:42
 */

#include "http/httpresponse.h"

const std::unordered_map<std::string, std::string>
    HttpResponse::SUFFIX_TYPE = {
        {".html", "text/html; charset=utf-8"},
        {".htm", "text/html; charset=utf-8"},
        {".xml", "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt", "text/plain; charset=utf-8"},
        {".rtf", "application/rtf"},
        {".pdf", "application/pdf"},
        {".word", "application/nsword"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".au", "audio/basic"},
        {".mpeg", "video/mpeg"},
        {".mpg", "video/mpeg"},
        {".avi", "video/x-msvideo"},
        {".gz", "application/x-gzip"},
        {".tar", "application/x-tar"},
        {".css", "text/css "},
        {".js", "text/javascript "},
        {".ico", "image/ico"},
        {".mp3", "audio/mpeg"},
        {".ogg", "application/ogg"},
        {".pac", "application/x-ns-proxy-autoconfig"},
        {".svg", "image/svg+xml"},
        {".mp4", "video/mp4"},
        {".woff2", "application/octet-stream"},
        {".woff", "application/octet-stream"},
        {".ttf", "application/octet-stream"},
        {".tof", "application/octet-stream"},
};

const std::unordered_map<int, std::string>
    HttpResponse::CODE_STATUS = {
        {200, "OK"},
        {400, "Bad Request"},
        {403, "Forbidden"},
        {404, "Not Found"},
};

const std::unordered_map<int, std::string>
    HttpResponse::CODE_PATH = {
        {400, "/400.html"},
        {403, "/403.html"},
        {404, "/404.html"},
};

HttpResponse::HttpResponse()
    : m_code(-1), m_path(""), m_src_dir(""), m_keep_alive(false), m_file(nullptr)
{
    m_stat = {0};
}

HttpResponse::~HttpResponse()
{
    if (m_file)
    {
        munmap(m_file, m_stat.st_size);
        m_file = nullptr;
    }
}

void HttpResponse::init(const std::string &srcDir, std::string &path,
                        bool isKeepAlive, int code)
{
    assert(srcDir != "");

    m_code = code;
    m_keep_alive = isKeepAlive;
    m_path = path;
    m_src_dir = srcDir;
}

void HttpResponse::makeResponse(Buffer &buf)
{
    /* 判断请求的资源文件 */
    if (stat((m_src_dir + m_path).data(), &m_stat) < 0 || S_ISDIR(m_stat.st_mode))
    {
        m_code = 404;
    }
    else if (!(m_stat.st_mode & S_IROTH))
    {
        m_code = 403;
    }
    else if (m_code == -1)
    {
        m_code = 200;
    }

    errorHtml();
    addStateLine(buf);
    addHeader(buf);
    addContent(buf);
}

void HttpResponse::errorHtml()
{
    if (CODE_PATH.count(m_code) == 1)
    {
        m_path = CODE_PATH.find(m_code)->second;
        stat((m_src_dir + m_path).data(), &m_stat);
    }
}

void HttpResponse::addStateLine(Buffer &buf)
{
    std::string status;
    if (CODE_STATUS.count(m_code) == 1)
    {
        status = CODE_STATUS.find(m_code)->second;
    }
    else
    {
        m_code = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buf.append("HTTP/1.1 " + std::to_string(m_code) + " " + status + "\r\n");
}

void HttpResponse::addHeader(Buffer &buf)
{
    buf.append("Connection: ");
    if (m_keep_alive)
    {
        buf.append("keep-alive\r\n");
        buf.append("keep-alive: max=6, timeout=120\r\n");
    }
    else
    {
        buf.append("close\r\n");
    }
    buf.append("Content-type: " + getFileType() + "\r\n");
    // buf.append("Content-Encoding: " + "deflate" + "\r\n");
}

void HttpResponse::addContent(Buffer &buf)
{
    int srcFd = open((m_src_dir + m_path).data(), O_RDONLY);
    if (srcFd < 0)
    {
        errorContent(buf, "File NotFound!");
        return;
    }

    /* 将文件映射到内存提高文件的访问速度
        MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    LOG_DEBUG("file path %s", (m_src_dir + m_path).data());
    int *mmRet = (int *)mmap(0, m_stat.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (*mmRet == -1)
    {
        errorContent(buf, "File NotFound!");
        return;
    }
    m_file = (char *)mmRet;

    close(srcFd);
    buf.append("Content-length: " + std::to_string(m_stat.st_size) + "\r\n\r\n");
}

std::string HttpResponse::getFileType()
{
    /* 判断文件类型 */
    std::string::size_type idx = m_path.find_last_of('.');

    if (idx == std::string::npos)
        return "text/plain";

    std::string suffix = m_path.substr(idx);
    if (SUFFIX_TYPE.count(suffix) == 1)
        return SUFFIX_TYPE.find(suffix)->second;

    return "text/plain";
}

void HttpResponse::errorContent(Buffer &buf, std::string message)
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(m_code) == 1)
    {
        status = CODE_STATUS.find(m_code)->second;
    }
    else
    {
        status = "Bad Request";
    }
    body += std::to_string(m_code) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buf.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buf.append(body);
}

char *HttpResponse::file()
{
    return m_file;
}

size_t HttpResponse::fileSize() const
{
    return m_stat.st_size;
}

int HttpResponse::code() const
{
    return m_code;
}