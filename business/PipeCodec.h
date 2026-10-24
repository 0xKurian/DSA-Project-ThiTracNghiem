#ifndef PIPE_CODEC_H
#define PIPE_CODEC_H

#include <string>

inline std::string escapePipe(const char* value) {
    std::string result;
    if (value == nullptr) return result;
    for (int i = 0; value[i] != '\0'; ++i) {
        if (value[i] == '|' || value[i] == '\\') result += '\\';
        result += value[i];
    }
    return result;
}

inline int splitEscapedPipe(const std::string& line, std::string fields[],
                            int capacity) {
    if (fields == nullptr || capacity < 1) return 0;
    int count = 0;
    std::string current;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '\\' && i + 1 < line.size() &&
            (line[i + 1] == '|' || line[i + 1] == '\\')) {
            current += line[i + 1];
            ++i;
        } else if (line[i] == '|') {
            if (count >= capacity - 1) return -1;
            fields[count++] = current;
            current.clear();
        } else {
            current += line[i];
        }
    }
    fields[count++] = current;
    return count;
}

#endif
