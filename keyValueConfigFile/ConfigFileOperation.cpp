#include "ConfigFileOperation.h"


const int ConfigFileOperation::MAX_C_KEYWORD_LEN = 20;
const char ConfigFileOperation::CONFIG_FILE_VERSION_KEYWORD[] = "version";

ConfigFileOperation::ConfigFileOperation(const char *path, int ver)
    : CONFIG_FILE_VERSION(0)
    , m_version(CONFIG_FILE_VERSION)
{
    m_fd = (FILE *)NULL;
    ClearBuffer();
    SetFileName(path);
}

ConfigFileOperation::~ConfigFileOperation()
{
    CloseFile();
}

bool ConfigFileOperation::GetVersion(int &ver)
{
	if (false == GetValueByKeyword(CONFIG_FILE_VERSION_KEYWORD, ver, NULL))
	{
		return false;
	}
	
	return true;
}

bool ConfigFileOperation::SetVersion(int ver)
{
	if (false == SetValueByKeyword(CONFIG_FILE_VERSION_KEYWORD, ver, NULL))
	{
		return false;
	}
	
	return true;
}

bool ConfigFileOperation::SetValueByKeyword(const char *keyword, int val, char *strVal)
{
    char realKeyword[MAX_C_KEYWORD_LEN] = {0};
	
    printf("ConfigFileOperation::%s, Request Set %s as %d/%s.", __func__, keyword, val, strVal);
	
    if (NULL == keyword || NULL == m_filePath)
	{
        return false;
	}

    if (!GetRealKeyword(realKeyword, keyword))
	{
        return false;
	}

    if (!m_fd)
    {
        bool flag = false;
        if (0 != access(m_filePath, F_OK))
		{
            flag = true;
		}

        if (!(m_fd = OpenFile((char *)"a+")))
		{
            return false;
		}

        if (flag)
        {
            ClearRecordBuffer();
            snprintf(m_record, sizeof(m_record), "%s=%d\n", CONFIG_FILE_VERSION_KEYWORD, m_version);
            fputs(m_record, m_fd);
            
            printf("ConfigFileOperation::%s, Add To %s with %s", __func__, m_filePath, m_record);
        }
    }

    WriteValue(keyword, val, strVal);

    CloseFile();

    return true;
    
}


bool ConfigFileOperation::GetValueByKeyword(const char *keyword, int &val, char *strVal)
{
	char realKeyword[MAX_C_KEYWORD_LEN] = {0};
	
    printf("ConfigFileOperation::%s: Get config value use %s.", __func__, keyword);

    if (!GetRealKeyword(realKeyword, keyword))
	{
        return false;
	}
		
    ClearBuffer();

    if (!(m_fd = OpenFile((char *)"r")))
    {
        printf("ConfigFileOperation::%s, no config file for %s.", 
                       __func__, keyword);
        return false;
    }

    if (false == ReadValue(realKeyword, val, strVal))
	{
		return false;
	}

    CloseFile();

    return true;

}

bool ConfigFileOperation::ReadValue(const char *keyword, int &val, char *strVal)
{
    int size;

    if (!m_fd)
	{
        return false;
	}

    fseek(m_fd, 0, SEEK_SET);

    if( 0 >= (size = (int)fread(m_fileBuf, sizeof(char), (size_t)(sizeof(m_fileBuf) - 1), m_fd)))
    {
        printf("ConfigFileOperation::%s: Read config file fail, size %d.",
            __func__, size);
        CloseFile();
        return false;
    }

    char *buffer = m_fileBuf;
    char *offset = strstr(buffer, keyword);
    if (NULL != offset)
    {
		// commentted
		bool rst;
		if (false == CheckLineCommentted(offset, rst) || rst == true)
		{
			return false;
		}

		offset += strlen(keyword);
		if (strVal == NULL)
		{
			val = atoi(offset);
			
			printf("ConfigFileOperation::%s: Get config value %s=%d.", __func__, keyword, val);
		}
		else
		{
			char *end = strchr(offset, '\n');
			if (end  == NULL)
			{
				return false;
			}
			
			int len = end - offset;
			if (len >= MAX_C_RECORD_LEN)
			{
				return false;
			}
			
			memcpy(strVal, offset, len);
			strVal[len] = 0;
			
			printf("ConfigFileOperation::%s: Get config value %s=%s.", __func__, keyword, strVal);
		}
    }
    else
    {
        printf("ConfigFileOperation::%s: (%s) not found.", __func__, keyword);
        return false;
    }

    return true;

}

bool ConfigFileOperation::WriteValue(const char *keyword, int val, char *strVal)
{
    bool alterSign = false;

    if ((FILE *)NULL == m_fd || NULL == keyword)
	{
        return false;
	}

    ClearBuffer();
    fseek(m_fd, 0, SEEK_SET);

    while(fgets(m_record, sizeof(m_record) - 1, m_fd))
    {
        if (3 > strlen(m_record))
            continue;

        if (NULL != (strstr(m_record, keyword)))
        {
            if (!alterSign)
            {
                alterSign = true;
                AddRecordToBuffer(keyword, val, strVal);
            }
        }
        else
        {
            strcat(m_fileBuf, m_record);
        }   

        if (EOF == fgetc(m_fd))
		{
            break;
		}

        fseek(m_fd, -1, SEEK_CUR);

        ClearRecordBuffer();

    }

    // if not found, add one
    if (!alterSign)
    {
        AddRecordToBuffer(keyword, val , strVal);
    }

    if (!WriteBufferToFile())
	{
        return false;
	}

    return true;
}

bool ConfigFileOperation::CheckLineCommentted(char *start, bool &rst)
{
	if (start == NULL)
	{
		return false;
	}
	
	int offset = start - m_fileBuf;
	if (offset < 0 || offset >= MAX_C_RECORD_LEN + MAX_C_KEYWORD_LEN)
	{
		return false;
	}
	
	for (; offset >= 0; offset--)
	{
		if (m_fileBuf[offset] == '\n')
		{
			rst = false;
			return true;
		}
		
		if (m_fileBuf[offset] == '#')
		{
			rst = true;
			return true;
		}
	}
	
	rst = false;
	return true;
}

bool ConfigFileOperation::WriteBufferToFile()
{
    CloseFile();

    if (! (m_fd = fopen(m_filePath, "w")))
    {
        printf("ConfigFileOperation::%s: Open (%s) config fail", __func__, m_filePath);
        return false;
    }

    fseek(m_fd, 0, SEEK_SET);
    fputs(m_fileBuf, m_fd);

    return true;
}

bool ConfigFileOperation::SetFileName(const char *filePath)
{
    if (NULL == filePath)
	{
        return false;
	}
    
	int size = strlen(filePath);
	
    int len = (size > MAX_C_FILE_NAME_LEN-1) ? MAX_C_FILE_NAME_LEN-1 : size;

    memset(m_filePath, 0, sizeof(m_filePath));
    memcpy(m_filePath, filePath, len);

    return true;
}


void ConfigFileOperation::CloseFile()
{
    if (m_fd)
    {
        fclose(m_fd);
    }

    m_fd = (FILE *)NULL;
}

FILE *ConfigFileOperation::OpenFile(char *operation)
{
    FILE *fd = (FILE *)NULL;

    if (! (fd = fopen(m_filePath, operation)))
    {
        printf("ConfigFileOperation::%s, open config file (%s) fail operation %s.", __func__, m_filePath, operation);
        return NULL;
    }

    return fd;
}

bool ConfigFileOperation::GetRealKeyword(char *real, const char *keyword)
{
    if (strlen(keyword) > MAX_C_KEYWORD_LEN-1)
    {
        printf("ConfigFileOperation::%s, Keyword is limited by %d, (%s) exceed!", __func__, MAX_C_KEYWORD_LEN-1, keyword);

        return false;
    }
	
    strcat(real, keyword);
    strcat(real, (char *)"=");

    return true;
}

void ConfigFileOperation::AddRecordToBuffer(const char *keyword, int val, char *strVal)
{
    ClearRecordBuffer();
	
	if (strVal == NULL)
	{
		snprintf(m_record, sizeof(m_record), "%s=%d\n", keyword, val);
	}
	else
	{
		snprintf(m_record, sizeof(m_record), "%s=%s\n", keyword, strVal);
	}
	
    strcat(m_fileBuf, m_record);

    printf("ConfigFileOperation::%s, Add record to buffer %s\n", __func__, m_record);
}

void ConfigFileOperation::ClearRecordBuffer()
{
    memset(m_record, 0, sizeof(m_record));
}

void ConfigFileOperation::ClearBuffer()
{
    memset(m_record, 0, sizeof(m_record));
    memset(m_fileBuf, 0, sizeof(m_fileBuf));
}

bool ConfigFileOperation::Test()
{
	int i = 0;
	char *key[] = 
	{
		(char *)"key1",
		(char *)"key2",
		(char *)"key3",
		(char *)"key4",
		(char *)"key5",
		(char *)"key6",
		(char *)"key7"
	};
	
	int val[] = 
	{
		1,
		1000,
		10000,
		-10000
	};
	
	char *strVal[] = 
	{
		(char *)"kxx1",
		(char *)"kxx2",
		(char *)"kxx3",
		(char *)"kxx4"
	};
	
	for (i = 0; i < 4; i++)
	{
		SetValueByKeyword(key[i], val[i], NULL);
	}
	
	int readVal = 0;
	
	for (i = 0; i < 4; i++)
	{
		if (true == GetValueByKeyword(key[i], readVal, NULL))
		{
			printf("read %s = %d\r\n", key[i], readVal);
			
			if (readVal != val[i])
			{
				printf("read error!\r\n");
				return false;
			}
		}
		else
		{
			printf("read %s failed!\r\n", key[i]);
			return false;
		}
	}
	
	for (i = 0; i < 4; i++)
	{
		SetValueByKeyword(key[i], 0, strVal[i]);
	}
	
	char readStrVal[MAX_C_RECORD_LEN] = {0};
	
	for (i = 0; i < 4; i++)
	{
		if (true == GetValueByKeyword(key[i], readVal, readStrVal))
		{
			printf("read %s=%s\r\n", key[i], readStrVal);
			
			if (strcmp(readStrVal, strVal[i]))
			{
				printf("read error!\r\n");
				return false;
			}
		}
		else
		{
			printf("read %s failed!\r\n", key[i]);
			return false;
		}
	}
	
	for (; i < 7; i++)
	{
		if (false == GetValueByKeyword(key[i], readVal, NULL))
		{
			printf("key %s commentted\r\n", key[i]);
		}
		else
		{
			printf("key %s don't commentted!\r\n", key[i]);
			return false;
		}
	}
	
	printf("Test SUCCESS!!!!!!!\r\n");
	
	return true;
}

#ifdef _TEST_

int main()
{
	ConfigFileOperation config("./xx.log", 1);
	config.Test();
	
}

#endif
