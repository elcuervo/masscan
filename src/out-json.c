#include "output.h"
#include "masscan-app.h"
#include "masscan-status.h"
#include "string_s.h"
#include <ctype.h>


/****************************************************************************
 ****************************************************************************/
static void
json_out_open(struct Output *out, FILE *fp)
{
    UNUSEDPARM(out);
    fprintf(fp, "[\n"); // enclose the atomic {}'s into an []
}


/****************************************************************************
 ****************************************************************************/
static void
json_out_close(struct Output *out, FILE *fp)
{
    UNUSEDPARM(out);
    fprintf(fp, "]\n"); // enclose the atomic {}'s into an []
}

//{ ip: "124.53.139.201", ports: [ {port: 443, proto: "tcp", status: "open", reason: "syn-ack", ttl: 48} ] }
/****************************************************************************
 ****************************************************************************/
static void
json_out_status(struct Output *out, FILE *fp, time_t timestamp, int status,
               unsigned ip, unsigned ip_proto, unsigned port, unsigned reason, unsigned ttl, const unsigned char mac[6])
{
    char reason_buffer[128];
    UNUSEDPARM(out);
    //UNUSEDPARM(timestamp);

    /* Trailing comma breaks some JSON parsers. We don't know precisely when
     * we'll end, but we do know when we begin, so instead of appending
     * a command to the record, we prepend it -- but not before first record */
    if (out->is_first_record_seen)
        fprintf(fp, ",\n");
    else
        out->is_first_record_seen = 1;

    fprintf(fp, "{ ");
    fprintf(fp, "  \"ip\": \"%u.%u.%u.%u\", ",
            (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>> 8)&0xFF, (ip>> 0)&0xFF);
    fprintf(fp, "  \"mac\": \"%02x:%02x:%02x:%02x:%02x:%02x\", ",
            mac[0],mac[1], mac[2], mac[3], mac[4], mac[5]);
    fprintf(fp, "  \"timestamp\": \"%d\", \"ports\": [ {\"port\": %u, \"proto\": \"%s\", \"status\": \"%s\","
                " \"reason\": \"%s\", \"ttl\": %u} ] ",
                (int) timestamp,
                port,
                name_from_ip_proto(ip_proto),
                status_string(status),
                reason_string(reason, reason_buffer, sizeof(reason_buffer)),
                ttl
            );
    fprintf(fp, "}\n");


}

/*****************************************************************************
 * Remove bad characters from the banner, especially new lines and HTML
 * control codes.
 *****************************************************************************/
static const char *
normalize_json_string(const unsigned char *px, size_t length,
                 char *buf, size_t buf_len)
{
    size_t i=0;
    size_t offset = 0;


    for (i=0; i<length; i++) {
        unsigned char c = px[i];

        if (isprint(c) && c != '<' && c != '>' && c != '&' && c != '\\' && c != '\"' && c != '\'') {
            if (offset + 2 < buf_len)
                buf[offset++] = px[i];
        } else {
            if (offset + 7 < buf_len) {
                buf[offset++] = '\\';
                buf[offset++] = 'u';
                buf[offset++] = '0';
                buf[offset++] = '0';
                buf[offset++] = "0123456789abcdef"[px[i]>>4];
                buf[offset++] = "0123456789abcdef"[px[i]&0xF];
            }
        }
    }

    buf[offset] = '\0';

    return buf;
}

/******************************************************************************
 ******************************************************************************/
static void
json_out_banner(struct Output *out, FILE *fp, time_t timestamp,
               unsigned ip, unsigned ip_proto, unsigned port,
               enum ApplicationProtocol proto,
               unsigned ttl,
               const unsigned char *px, unsigned length)
{
    char banner_buffer[65536];

    UNUSEDPARM(ttl);
    //UNUSEDPARM(timestamp);

    /* Trailing comma breaks some JSON parsers. We don't know precisely when
     * we'll end, but we do know when we begin, so instead of appending
     * a command to the record, we prepend it -- but not before first record */
    if (out->is_first_record_seen)
        fprintf(fp, ",\n");
    else
        out->is_first_record_seen = 1;
    
    fprintf(fp, "{ ");
    fprintf(fp, "  \"ip\": \"%u.%u.%u.%u\", ",
            (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>> 8)&0xFF, (ip>> 0)&0xFF);
    fprintf(fp, "  \"timestamp\": \"%d\", \"ports\": [ {\"port\": %u, \"proto\": \"%s\", \"service\": {\"name\": \"%s\", \"banner\": \"%s\"} } ] ",
            (int) timestamp,
            port,
            name_from_ip_proto(ip_proto),
            masscan_app_to_string(proto),
            normalize_json_string(px, length, banner_buffer, sizeof(banner_buffer))
            );
    fprintf(fp, "}\n");

    UNUSEDPARM(out);

/*    fprintf(fp, "<host endtime=\"%u\">"
            "<address addr=\"%u.%u.%u.%u\" addrtype=\"ipv4\"/>"
            "<ports>"
            "<port protocol=\"%s\" portid=\"%u\">"
            "<state state=\"open\" reason=\"%s\" reason_ttl=\"%u\" />"
            "<service name=\"%s\" banner=\"%s\"></service>"
            "</port>"
            "</ports>"
            "</host>"
            "\r\n",
            (unsigned)timestamp,
            (ip>>24)&0xFF,
            (ip>>16)&0xFF,
            (ip>> 8)&0xFF,
            (ip>> 0)&0xFF,
            name_from_ip_proto(ip_proto),
            port,
            reason, ttl,
            masscan_app_to_string(proto),
            normalize_string(px, length, banner_buffer, sizeof(banner_buffer))
            );*/
}

/****************************************************************************
 ****************************************************************************/
const struct OutputType json_output = {
    "json",
    0,
    json_out_open,
    json_out_close,
    json_out_status,
    json_out_banner
};
