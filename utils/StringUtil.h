//
// Created by Alexandre Lemos on 01/07/2019.
//

#ifndef PROJECT_STRINGUTIL_H
#define PROJECT_STRINGUTIL_H

#include <string>

inline int isFirst(std::string s1, std::string s2, int size) {
    for (int i = 0; i < size; ++i) {
        if (s1[i] == '0' && s2[i] == '1')
            return 1;
        if (s1[i] == '1' && s2[i] == '0')
            return -1;
        if (s1[i] == '1' && s2[i] == '1')
            return 0;
    }
    return 0;
}

inline bool stringcompare(std::string s1, std::string s2, int size, bool same) {
    for (int i = 0; i < size; ++i) {
        if (same && s2[i] != s1[i])
            return false;
        else if (!same && s2[i] == s1[i] && '1' == s1[i])
            return false;
    }
    return true;

}

inline bool stringcontains(std::string s1, std::string s2, int size) {
    bool s2B = false, s1B = false;
    for (int i = 0; i < size; ++i) {
        if (s2[i] == '1' && s1[i] == '0')
            s2B = true;
        if (s1[i] == '1' && s2[i] == '0')
            s1B = true;
    }
    return !(s1B && s2B);

}

inline std::string itos(int i) {
    std::stringstream s;
    s << i;
    return s.str();
}




#endif //PROJECT_STRINGUTIL_H
