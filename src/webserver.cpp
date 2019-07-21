#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"

#define PART_BOUNDARY "***-_-streaming-server-boundary-_-***"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static httpd_handle_t stream_httpd = NULL;

static esp_err_t jpeg_stream_httpd_handler(httpd_req_t *req)
{
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    char * part_buf[64];
    
    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }
    
    while(true){
        // Grab frame
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            return ESP_FAIL;
        }

        // If the frame is not JPEG we fail
        if(fb->format != PIXFORMAT_JPEG){
            esp_camera_fb_return(fb);
            ESP_LOGE(TAG, "Frame not JPEG");
            return res;
        }
        
        // Send Content-Length
        size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, fb->len);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        if (res != ESP_OK) {
            esp_camera_fb_return(fb);
            ESP_LOGE(TAG, "Sending content-length header failed");
            return res;
        }

        // Send the frame
        res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        if (res != ESP_OK) {
            esp_camera_fb_return(fb);
            ESP_LOGE(TAG, "Sending image failed %u", res);
            return res;
        }

        // Send part boundary
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        if (res != ESP_OK) {
            esp_camera_fb_return(fb);
            ESP_LOGE(TAG, "Sending boundary failed");
            return res;
        }

        esp_camera_fb_return(fb);
    }
    return res;
}

void start_camera_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t stream_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = jpeg_stream_httpd_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&stream_httpd, &config) != ESP_OK) {
        ESP_LOGE(TAG,"Unable to start HTTP server");
        return;
    }
    httpd_register_uri_handler(stream_httpd, &stream_uri);
}

void stop_camera_server()
{
    if (stream_httpd == NULL) {
        return;
    }

    httpd_stop(stream_httpd);
}
