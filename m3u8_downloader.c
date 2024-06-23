#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "curl_service.h"

typedef 
enum _m_error {
    m_success = 0,
    m_parameter,
    m_alloc,
    m_file,
	m_file_write
} m_error;

void print_help(char* argv[])
{
    printf(
        "Usage:\n"
        "\t%s <M3U8_URL> <OUTPUT_FILE>\n",
		argv[0]
    );
}

// the function does not have error handling 
// just because i am sure there should be some lib 
// to parse this kind of file but i am not going to find and use it just to find best quality video
// --
// this function tries to find best quality according to bandwith value
// here i think best quality means biggest bandwith value ( I may be wrong though )
m_error m3u8_get_best_quality_url(const char *data, size_t size, char **m3u8_url)
{
    m_error ret;

    size_t max_bandwith = 0;

    char *ptr = NULL;
    char *data_copied = NULL;

    bail_assert(data != NULL, m_parameter);
    bail_assert(size > 0, m_parameter);
    bail_assert(m3u8_url != NULL, m_parameter);
    bail_assert(*m3u8_url == NULL, m_parameter);

    data_copied = strndup(data, size);
    bail_assert(data_copied != NULL, m_alloc);

    ptr = data_copied;
  
    for (char *line; (line = strtok_r(ptr, "\n", &ptr)); ) {
        char *p;
        if (strstr(line, "#EXT-X-STREAM-INF:") == line &&
            (p = strstr(line, ",BANDWIDTH=")) != NULL) {
            size_t bandwith = 0;

            p += strlen(",BANDWIDTH=");
            *strstr(p, ",") = '\0';

            sscanf(p, "%zu", &bandwith);
            // if found max bandwith, take next line
            if (bandwith > max_bandwith) {
                if (*m3u8_url) {
                    free(*m3u8_url);
                }
                *m3u8_url = strdup(strtok_r(ptr, "\n", &ptr));
                max_bandwith = bandwith;
            }
        }
    }
    ret = m_success;

exit:
    if (data_copied) {
        free(data_copied);
    }
    return ret;
}

// dowloads all ts files into one file 
m_error m3u8_download(curl_service *cs, const char *m3u8_video_url, const char *path, const char *data, size_t size)
{
    m_error ret;

    char *url = NULL;
    char *ptr = NULL;
    char *data_copied = NULL;

    FILE *f = NULL;

    bail_assert(cs != NULL, m_parameter);
    bail_assert(path != NULL, m_parameter);
    bail_assert(data != NULL, m_parameter);
    bail_assert(size > 0, m_parameter);

    data_copied = strndup(data, size);
    bail_assert(data_copied != NULL, m_alloc);

    f = fopen(path, "wb");
    bail_assert(data_copied != NULL, m_file);

    ptr = data_copied;

    for (char *line; (line = strtok_r(ptr, "\n", &ptr)); ) {
        cs_error ce;
        char *d;
        size_t s;
        size_t swritten;

        if (*line == '#') {
            continue;
        }
        url = calloc((int)(strrchr(m3u8_video_url, '/') - m3u8_video_url) + 1 + strlen(line) + 1, sizeof(char));
        bail_assert(url != NULL, m_alloc);

        sprintf(url, "%.*s/%s", (int)(strrchr(m3u8_video_url, '/') - m3u8_video_url), m3u8_video_url, line);

        printf("%s downloading...\n", line);
        ce = cs_get(cs, url, &d, &s);
        bail_assert(ce == cs_success, ce);

        swritten = fwrite(d, sizeof(char), s, f);
		bail_assert(swritten == s, m_file_write);
    }

    ret = m_success;
exit:
    if (data_copied) {
        free(data_copied);
    }
    if (f) {
        fclose(f);
    }
    return ret;
}

int main(int argc, char* argv[])
{
    int ret;
    curl_service *cs = NULL;

    char *data = NULL;
    size_t size = 0;

    char *m3u8_video_url = NULL;
    char *m3u8_sub_video_url = NULL;

    bail_assert(argc == 3, m_parameter);

    ret = cs_create(&cs);
    bail_assert(ret == cs_success, ret);

    ret = cs_get(cs, argv[1], &data, &size);
    bail_assert(ret == cs_success, ret);

    ret = m3u8_get_best_quality_url(data, size, &m3u8_sub_video_url);
    bail_assert(ret == m_success, ret);
    
    m3u8_video_url = calloc((int)(strrchr(argv[1], '/') - argv[1]) + 1 + strlen(m3u8_sub_video_url) + 1, sizeof(char));
    bail_assert(m3u8_video_url != NULL, m_alloc);

    sprintf(m3u8_video_url, "%.*s/%s", (int)(strrchr(argv[1], '/') - argv[1]), argv[1], m3u8_sub_video_url);

    ret = cs_get(cs, m3u8_video_url, &data, &size);
    bail_assert(ret == cs_success, ret);

    ret = m3u8_download(cs, m3u8_video_url, argv[2], data, size);
    bail_assert(ret == m_success, ret);

exit:
    if (argc != 3) {
        print_help(argv);
    }
    if (m3u8_sub_video_url) {
        free(m3u8_sub_video_url);
    }
    if (m3u8_video_url) {
        free(m3u8_video_url);
    }
    if (cs) {
        cs_free(cs);
    }
    return ret;
}