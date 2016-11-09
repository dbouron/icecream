#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

#include "file_util.h"

namespace icecream
{
    namespace daemon
    {
        /**
         * Adapted from an answer by "Evan Teran" from this stack overflow question:
         * http://stackoverflow.com/questions/236129/split-a-string-in-c
         */
        std::vector<std::string> split(const std::string &s, char delim)
        {
            std::vector<std::string> elems;
            std::stringstream ss(s);
            std::string item;
            while (getline(ss, item, delim)) {
                if (!item.empty()) {
                    elems.push_back(item);
                }
            }
            return elems;
        }

        /**
         * Adapted from an answer by "dash-tom-bang" from this stack overflow question:
         * http://stackoverflow.com/questions/5772992/get-relative-path-from-two-absolute-paths
         */
        std::string get_relative_path(const std::string &to, const std::string &from) {
            std::vector<std::string> to_dirs = split(to, '/');
            std::vector<std::string> from_dirs = split(from, '/');

            std::string output;
            output.reserve(to.size());

            auto to_cit = to_dirs.cbegin();
            auto to_cend = to_dirs.cend();
            auto from_cit = from_dirs.cbegin();
            auto from_cend = from_dirs.cend();

            while ((to_cit != to_cend)
                   && (from_cit != from_cend)
                   && *to_cit == *from_cit)
            {
                ++to_cit;
                ++from_cit;
            }

            while (from_cit != from_cend)
            {
                output += "../";
                ++from_cit;
            }

            while (to_cit != to_cend)
            {
                output += *to_cit;
                ++to_cit;

                if (to_cit != to_cend)
                {
                    output += "/";
                }
            }

            return output;
        }

        /**
         * Returns a std::string without '..' and '.'
         *
         * Preconditions:  path must be an absolute path
         * Postconditions: if path is empty or not an absolute path, return original
         *                 path, otherwise, return path after resolving '..' and '.'
         */
        std::string get_canonicalized_path(const std::string &path) {
            if (path.empty() || path[0] != '/') {
                return path;
            }

            std::vector<std::string> parts = split(path, '/');
            std::vector<std::string> canonicalized_path;

            auto parts_cit = parts.cbegin();
            auto parts_cend = parts.cend();

            while (parts_cit != parts_cend) {
                if (*parts_cit == ".." && !canonicalized_path.empty()) {
                    canonicalized_path.pop_back();
                }
                else if (*parts_cit != "." && *parts_cit != "..") {
                    canonicalized_path.push_back(*parts_cit);
                }

                ++parts_cit;
            }

            auto path_cit = canonicalized_path.cbegin();
            auto path_cend = canonicalized_path.cend();

            std::string output;
            output.reserve(path.size());
            output += "/";
            while (path_cit != path_cend)
            {
                output += *path_cit;

                ++path_cit;
                if (path_cit != path_cend)
                {
                    output += "/";
                }
            }

            return output;
        }

        /**
         * Adapted from an answer by "Mark" from this stack overflow question:
         * http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
         */
        bool mkpath(const std::string &path) {
            bool success = false;
            int ret = mkdir(path.c_str(), 0775);
            if(ret == -1) {
                switch(errno) {
                case ENOENT:
                    if(mkpath(path.substr(0, path.find_last_of('/'))))
                        success = 0 == mkdir(path.c_str(), 0775);
                    else
                        success = false;
                    break;
                case EEXIST:
                    success = true;
                    break;
                default:
                    success = false;
                    break;
                }
            }
            else {
                success = true;
            }

            return success;
        }

        /**
         * Adapted from an answer by "asveikau" from this stack overflow question:
         * http://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
         */
        bool rmpath(const char* path) {
            DIR *d = opendir(path);
            size_t path_len = strlen(path);
            int r = -1;

            if (d) {
                struct dirent *p;

                r = 0;

                while (!r && (p=readdir(d))) {
                    int r2 = -1;
                    char *buf;
                    size_t len;

                    /* Skip the names "." and ".." as we don't want to recurse on them. */
                    if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                        continue;
                    }

                    len = path_len + strlen(p->d_name) + 2;
                    buf = (char*)malloc(len);

                    if (buf) {
                        struct stat statbuf;

                        snprintf(buf, len, "%s/%s", path, p->d_name);

                        if (!stat(buf, &statbuf)) {
                            if (S_ISDIR(statbuf.st_mode)) {
                                r2 = rmpath(buf);
                            }
                            else {
                                r2 = unlink(buf);
                            }
                        }

                        free(buf);
                    }

                    r = r2;
                }

                closedir(d);
            }

            if (!r) {
                r = rmdir(path);
            }

            return r;
        }
    } // daemon
} // icecream
