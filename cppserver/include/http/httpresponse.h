/*
 * @Author: EpicLu
 * @Date: 2023-04-22 18:38:53
 * @Last Modified by: EpicLu
 * @Last Modified time: 2023-05-07 18:56:56
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
    void errorContent(std::shared_ptr<Buffer> buf, std::string message);
    void makeResponse(std::shared_ptr<Buffer> buf);

    inline char *file() { return m_file; };
    inline size_t fileSize() const { return m_stat.st_size; };
    inline int code() const { return m_code; };

private:
    void addStateLine(std::shared_ptr<Buffer> buf);
    void addHeader(std::shared_ptr<Buffer> buf);
    void addContent(std::shared_ptr<Buffer> buf);
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