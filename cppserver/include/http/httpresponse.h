/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:38:53
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-06 01:21:18
 */

#ifndef _HTTPRESPONSE_H_
#define _HTTPRESPONSE_H_

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "buffer/buffer.h"
#include "log/log.h"

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string &srcDir, std::string &path, bool isKeepAlive = false, int code = -1);
    void errorContent(Buffer &buf, std::string message);
    void makeResponse(Buffer &buf);
    char *file();
    size_t fileSize() const;
    int code() const;

private:
    void addStateLine(Buffer &buf);
    void addHeader(Buffer &buf);
    void addContent(Buffer &buf);
    void errorHtml();
    std::string getFileType();

private:
    int m_code;
    bool m_keep_alive;
    char *m_file;
    std::string m_path;
    std::string m_src_dir;
    struct stat m_stat;

private:
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif // _HTTPRESPONSE_H_