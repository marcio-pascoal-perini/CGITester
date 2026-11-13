/*
 * Windows
 *
 * gcc DBF-Viewer.c -o DBF-Viewer.exe -mwindows
 *
 * Linux
 *
 * gcc DBF-Viewer.c -o DBF-Viewer.exe
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#define SET_BINARY_MODE(handle) _setmode(_fileno(handle), _O_BINARY)
#else
#define SET_BINARY_MODE(handle) ((void)0)
#endif

#pragma pack(push, 1)
typedef struct
{
    uint8_t version;
    uint8_t last_update[3];
    uint32_t num_records;
    uint16_t header_size;
    uint16_t record_size;
    uint8_t reserved[20];
} DBFHeader;

typedef struct
{
    char name[11];
    char type;
    uint32_t data_address;
    uint8_t length;
    uint8_t decimal_count;
    uint8_t reserved[14];
} DBFField;
#pragma pack(pop)

unsigned char *find_bytes(unsigned char *haystack, size_t hlen, const char *needle)
{
    size_t nlen = strlen(needle);
    if (nlen == 0 || hlen < nlen)
        return NULL;
    for (size_t i = 0; i <= hlen - nlen; i++)
    {
        if (memcmp(haystack + i, needle, nlen) == 0)
            return haystack + i;
    }
    return NULL;
}

void trim_trailing_spaces(char *str, int len)
{
    int i = len - 1;
    while (i >= 0 && (str[i] == ' ' || str[i] == '\0' || str[i] == '\r' || str[i] == '\n'))
    {
        str[i] = '\0';
        i--;
    }
}

void print_html_header()
{
    printf("Content-type: text/html; charset=utf-8\n\n");
    printf("<!DOCTYPE html>\n<html>\n<head>\n<title>DBF Viewer</title>\n");
    printf("<style>\n");
    printf("  table { border-collapse: collapse; width: 100%%; font-family: sans-serif; }\n");
    printf("  th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
    printf("  th { background-color: #f2f2f2; }\n");
    printf("  tr:nth-child(even) { background-color: #f9f9f9; }\n");
    printf("  tr:hover { background-color: #ddd; }\n");
    printf("</style>\n</head>\n<body>\n");
}

void print_upload_form()
{
    print_html_header();
    // printf("<h1>Upload DBF File</h1>\n");
    // printf("<form method=\"POST\" enctype=\"multipart/form-data\">\n");
    // printf("  <input type=\"file\" name=\"dbffile\" accept=\".dbf\">\n");
    // printf("  <input type=\"submit\" value=\"Show Data\">\n");
    // printf("</form>\n");
    printf("<p>Error: The method must be POST, and don't forget: enctype=multipart/form-data</p>\n");
    printf("</body>\n</html>\n");
}

void process_dbf(unsigned char *buffer, size_t size)
{
    if (size < sizeof(DBFHeader))
    {
        printf("<p>Error: File too small to be a valid DBF.</p>\n");
        return;
    }
    DBFHeader *header = (DBFHeader *)buffer;
    if (header->header_size > size || header->record_size == 0)
    {
        printf("<p>Error: Invalid DBF header.</p>\n");
        return;
    }
    int num_fields = (header->header_size - 33) / 32;
    if (num_fields <= 0)
    {
        printf("<p>Error: No fields defined in DBF.</p>\n");
        return;
    }
    DBFField *fields = (DBFField *)(buffer + sizeof(DBFHeader));
    printf("<h4>DBF Records: %u</h4>\n", header->num_records);
    printf("<table>\n<thead><tr>\n");
    printf("<th>#</th>");
    for (int i = 0; i < num_fields; i++)
    {
        char name[12] = {0};
        memcpy(name, fields[i].name, 11);
        printf("<th>%s</th>", name);
    }
    printf("</tr></thead>\n<tbody>\n");
    unsigned char *record_ptr = buffer + header->header_size;
    for (uint32_t r = 0; r < header->num_records; r++)
    {
        if (record_ptr + header->record_size > buffer + size)
            break;
        char delete_flag = record_ptr[0];
        if (delete_flag == '*')
        {
            record_ptr += header->record_size;
            continue;
        }
        printf("<tr>\n");
        printf("<td>%u</td>", r + 1);
        int offset = 1;
        for (int f = 0; f < num_fields; f++)
        {
            int len = fields[f].length;
            char *val = (char *)malloc(len + 1);
            memcpy(val, record_ptr + offset, len);
            val[len] = '\0';
            trim_trailing_spaces(val, len);
            printf("<td>%s</td>", val);
            free(val);
            offset += len;
        }
        printf("</tr>\n");
        record_ptr += header->record_size;
    }
    printf("</tbody></table>\n");
}

int main()
{
    char *method = getenv("REQUEST_METHOD");
    if (method && strcmp(method, "POST") == 0)
    {
        char *len_str = getenv("CONTENT_LENGTH");
        char *type_str = getenv("CONTENT_TYPE");
        if (!len_str || !type_str || strstr(type_str, "multipart/form-data") == NULL)
        {
            print_upload_form();
            return 0;
        }
        long content_length = atol(len_str);
        if (content_length <= 0 || content_length > 10 * 1024 * 1024)
        { // 10MB limit for safety
            print_html_header();
            printf("<h1>Error</h1><p>File too large or invalid length.</p></body></html>");
            return 0;
        }
        unsigned char *buffer = (unsigned char *)malloc(content_length);
        if (!buffer)
        {
            print_html_header();
            printf("<h1>Error</h1><p>Memory allocation failed.</p></body></html>");
            return 0;
        }
        SET_BINARY_MODE(stdin);
        size_t read_bytes = fread(buffer, 1, content_length, stdin);
        print_html_header();
        char *boundary_ptr = strstr(type_str, "boundary=");
        if (boundary_ptr)
        {
            char boundary[256];
            snprintf(boundary, sizeof(boundary), "--%s", boundary_ptr + 9);
            unsigned char *data_start = find_bytes(buffer, read_bytes, "\r\n\r\n");
            if (data_start)
            {
                data_start += 4;
                unsigned char *data_end = find_bytes(data_start, read_bytes - (data_start - buffer), boundary);
                if (data_end)
                {
                    data_end -= 2;
                    process_dbf(data_start, data_end - data_start);
                }
                else
                {
                    printf("<p>Error: Could not find end of file data.</p>\n");
                }
            }
            else
            {
                printf("<p>Error: Could not find start of file data.</p>\n");
            }
        }
        else
        {
            printf("<p>Error: No boundary found in multipart request.</p>\n");
        }
        printf("</body></html>\n");
        free(buffer);
    }
    else
    {
        print_upload_form();
    }
    return 0;
}
