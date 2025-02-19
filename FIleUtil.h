#pragma once

#include<string>
#include<Shlwapi.h>


#pragma comment(lib,"shlwapi.lib")


bool SearchFilePathA(const char* filename, std::string& result);

bool SearchFilePathW(const wchar_t* file, std::wstring& result);

#if defined(UNICODE) || defined(_UNICODE)
    inline bool SearchFilePath(const wchar_t* filename, std::wstring& result)
    {
        return SearchFilePathW(filename, result);
    }
#else
    inline bool SearchFilePath(const char* filename, std::string& result)
    {
        return SearchFilePathA(filename, result);
    }
#endif//defined(UNICODE) || defined(_UNICODE)



