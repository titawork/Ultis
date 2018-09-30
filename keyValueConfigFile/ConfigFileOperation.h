#ifndef _CONFIG_FILE_OPERATION_HPP_
#define _CONFIG_FILE_OPERATION_HPP_

/**
********************************************************************************
* @file         ConfigFileOperation
* @version      0.1
* @author       Big Bob
* @brief        
* @ingroup
*
* Revision History:
*
* Copyright (c) 2018. All rights reserved.
*******************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const int MAX_C_CONTENT_LEN = 1024;               // Max length of the config file
static const int MAX_C_RECORD_LEN = 50;
static const int MAX_C_FILE_NAME_LEN = 255;
    

class ConfigFileOperation
{
public:
    ConfigFileOperation(const char *path, int ver);

    ~ConfigFileOperation();

public:
	bool GetVersion(int &ver);
	bool SetVersion(int ver);
    bool GetValueByKeyword(const char *keyword, int &val, char *strVal);
    bool SetValueByKeyword(const char *keyword, int val, char *strVal);
    bool SetFileName(const char *filePath);
	bool Test();

private:
    bool WriteValue(const char *keyword, int val, char *strVal);
    bool ReadValue(const char *keyword, int &val, char *strVal);
    void ClearBuffer();
    void ClearRecordBuffer();
    void AddRecordToBuffer(const char *keyword, int val, char *strVal);
    bool GetRealKeyword(char *real, const char *keyword);
    bool WriteBufferToFile();
    FILE *OpenFile(char *operation);
    void CloseFile();
	bool CheckLineCommentted(char *start, bool &rst);

private:
    ConfigFileOperation(ConfigFileOperation const&);
    ConfigFileOperation &operator=(ConfigFileOperation const&);

private:
    static const char CONFIG_FILE_VERSION_KEYWORD[];
    static const int MAX_C_KEYWORD_LEN;        // length of keywords limited

    const int CONFIG_FILE_VERSION;
    
    int m_version;
    char m_filePath[MAX_C_FILE_NAME_LEN];
    FILE *m_fd;
    
    char m_record[MAX_C_RECORD_LEN];
    char m_fileBuf[MAX_C_CONTENT_LEN];
};

#endif


