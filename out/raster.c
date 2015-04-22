#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "user32")
/* link: -lm -luser32  */
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

typedef int bool;
#define true 1
#define false 0
#define snprintf sprintf_s

struct raster_point {
    short x;
    short y;
    char dir;
    unsigned char value;
};

void raster_start();
void raster_new_contour();
void raster_end_contour();
void raster_line(float tf_x0, float tf_y0, float tf_x1, float tf_y1);
void raster_curve(float tf_x0, float tf_y0, float tf_x1, float tf_y1, float tf_x2, float tf_y2);
void raster_fill(unsigned char * buf, int n_w);
void raster_stroke(unsigned char * buf, int n_w);
static void add_raster_pair_steep(int n_x, int n_y, float f);
static void add_raster_pair_flat(int n_x, int n_y, float f);
static void raster_sort();

extern HINSTANCE cur_instance;
extern HWND hwnd_main;
char s_debug_buffer[1024];
struct raster_point * p_raster_points = NULL;
int n_raster_size = 0;
int n_raster_count = 0;
char * s_raster_dirs = NULL;
int n_raster_ymax = 0;
int i_contour_start;
int i_contour_seg;
int n_raster_dir;
bool b_dir_changed;
int n_last_point;

void raster_start(){
    n_raster_count = 0;
}

void raster_new_contour(){
    i_contour_start = n_raster_count;
    i_contour_seg = 0;
    n_raster_dir = 0;
    b_dir_changed = 0;
    if(n_raster_ymax > 0){
        memset(s_raster_dirs, 0, n_raster_ymax + 1);
    }
}

void raster_end_contour(){
    int k;
    int j;

    k = i_contour_start;
    if(p_raster_points[k].dir == n_raster_dir){
        int i;
        for(i=0; i<4; i++){
            if(p_raster_points[k].dir == s_raster_dirs[p_raster_points[k].y]){
                p_raster_points[k].dir = 0;
            }
            k++;
        }
    }
    else if(p_raster_points[k].dir == (-n_raster_dir)){
        if(n_raster_dir > 0){
            k++;
            j = n_raster_count - 1;
            if(p_raster_points[k].y > p_raster_points[j].y){
                p_raster_points[k].dir++;
            }
            else if(p_raster_points[k].y < p_raster_points[j].y){
                p_raster_points[j].dir--;
            }
        }
        else{
            j = n_raster_count - 2;
            if(p_raster_points[k].y < p_raster_points[j].y){
                p_raster_points[k].dir--;
            }
            else if(p_raster_points[k].y > p_raster_points[j].y){
                p_raster_points[j].dir++;
            }
        }
    }
}

void raster_line(float tf_x0, float tf_y0, float tf_x1, float tf_y1){
    bool tb_steep;
    bool tb_swapd;
    float tf_delta_x;
    float tf_delta_y;
    float tf_gradient;
    float tf_xend0;
    float tf_yend0;
    int tn_x0;
    int tn_y0;
    float tf_xend1;
    float tf_yend1;
    int tn_x1;
    int tn_y1;
    float tf_y;
    float tf_gap;
    int k;

    i_contour_seg++;
    if(tf_y1 > tf_y0){
        if(n_raster_dir == -1){
            b_dir_changed = 1;
            n_last_point = n_raster_count;
        }
        n_raster_dir = 1;
    }
    else if(tf_y1 < tf_y0){
        if(n_raster_dir == 1){
            b_dir_changed = 1;
            n_last_point = n_raster_count;
        }
        n_raster_dir = -1;
    }
    else{
        if(!n_raster_dir){
            n_raster_dir = 1;
        }
    }
    tb_steep = fabs(tf_y1 - tf_y0) > fabs(tf_x1 - tf_x0);
    if(tb_steep){
        float temp;
        temp = tf_x0;
        tf_x0 = tf_y0;
        tf_y0 = temp;
        temp = tf_x1;
        tf_x1 = tf_y1;
        tf_y1 = temp;
    }
    tb_swapd = tf_x0 > tf_x1;
    if(tb_swapd){
        float temp;
        temp = tf_x0;
        tf_x0 = tf_x1;
        tf_x1 = temp;
        temp = tf_y0;
        tf_y0 = tf_y1;
        tf_y1 = temp;
    }
    tf_delta_x = tf_x1 - tf_x0;
    tf_delta_y = tf_y1 - tf_y0;
    tf_gradient = tf_delta_y / tf_delta_x;
    tf_xend0 = (float)(floor(tf_x0 + 0.5));
    tf_yend0 = tf_y0 + tf_gradient * (tf_xend0 - tf_x0);
    if(tf_yend0 < 0){
        tf_yend0 = 0;
    }
    tn_x0 = (int)tf_xend0;
    tn_y0 = (int)tf_yend0;
    tf_xend1 = (float)(floor(tf_x1 + 0.5));
    tf_yend1 = tf_y0 + tf_gradient * (tf_xend1 - tf_x0);
    if(tf_yend1 < 0){
        tf_yend1 = 0;
    }
    tn_x1 = (int)tf_xend1;
    tn_y1 = (int)tf_yend1;
    if(tn_x0 == tn_x1){
        tf_y = tf_yend0;
        if(tb_steep){
            add_raster_pair_steep(tn_y0, tn_x0, (float)(floor(tf_y) + 1 - (tf_y)));
        }
        else{
            add_raster_pair_flat(tn_x0, tn_y0, (float)(floor(tf_y) + 1 - (tf_y)));
        }
        tf_gap = tf_x1 - tf_x0;
        k = n_raster_count - 2;
        p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
        k++;
        p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
    }
    else{
        if(tb_steep){
            int tn_x;
            if(tb_swapd){
                tf_y = tf_yend1;
                add_raster_pair_steep(tn_y1, tn_x1, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(tf_x1 + 0.5 - floor(tf_x1 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
            else{
                tf_y = tf_yend0;
                add_raster_pair_steep(tn_y0, tn_x0, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(floor(tf_x0 + 0.5) + 1 - (tf_x0 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
            tf_y = tf_yend0;
            for(tn_x=tn_x0+1; tn_x<tn_x1; tn_x++){
                tf_y += tf_gradient;
                add_raster_pair_steep((int)(tf_y), tn_x, (float)(floor(tf_y) + 1 - (tf_y)));
            }
            if(tb_swapd){
                tf_y = tf_yend0;
                add_raster_pair_steep(tn_y0, tn_x0, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(floor(tf_x0 + 0.5) + 1 - (tf_x0 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
            else{
                tf_y = tf_yend1;
                add_raster_pair_steep(tn_y1, tn_x1, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(tf_x1 + 0.5 - floor(tf_x1 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
        }
        else{
            int tn_x;
            if(tb_swapd){
                tf_y = tf_yend1;
                add_raster_pair_flat(tn_x1, tn_y1, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(tf_x1 + 0.5 - floor(tf_x1 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
            else{
                tf_y = tf_yend0;
                add_raster_pair_flat(tn_x0, tn_y0, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(floor(tf_x0 + 0.5) + 1 - (tf_x0 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
            tf_y = tf_yend0;
            for(tn_x=tn_x0+1; tn_x<tn_x1; tn_x++){
                tf_y += tf_gradient;
                add_raster_pair_flat(tn_x, (int)(tf_y), (float)(floor(tf_y) + 1 - (tf_y)));
            }
            if(tb_swapd){
                tf_y = tf_yend0;
                add_raster_pair_flat(tn_x0, tn_y0, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(floor(tf_x0 + 0.5) + 1 - (tf_x0 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
            else{
                tf_y = tf_yend1;
                add_raster_pair_flat(tn_x1, tn_y1, (float)(floor(tf_y) + 1 - (tf_y)));
                tf_gap = (float)(tf_x1 + 0.5 - floor(tf_x1 + 0.5));
                k = n_raster_count - 2;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
                k++;
                p_raster_points[k].value = (unsigned char)(p_raster_points[k].value * tf_gap);
            }
        }
    }
}

void raster_curve(float tf_x0, float tf_y0, float tf_x1, float tf_y1, float tf_x2, float tf_y2){
    float tf_x11;
    float tf_y11;
    float tf_x22;
    float tf_y22;
    float tf_x;
    float tf_y;

    if(fabs(tf_x2 - tf_x0) <= 1 && fabs(tf_y2 - tf_y0) <= 1){
        raster_line(tf_x0, tf_y0, tf_x2, tf_y2);
    }
    else{
        tf_x11 = (tf_x0 + tf_x1) / 2;
        tf_y11 = (tf_y0 + tf_y1) / 2;
        tf_x22 = (tf_x2 + tf_x1) / 2;
        tf_y22 = (tf_y2 + tf_y1) / 2;
        tf_x = (tf_x11 + tf_x22) / 2;
        tf_y = (tf_y11 + tf_y22) / 2;
        raster_curve(tf_x0, tf_y0, tf_x11, tf_y11, tf_x, tf_y);
        raster_curve(tf_x, tf_y, tf_x22, tf_y22, tf_x2, tf_y2);
    }
}

void raster_fill(unsigned char * buf, int n_w){
    int n_check;
    int tn_top_y;
    int tn_dir;
    int tn_y;
    int k;
    int tn_x;
    int k_start;
    bool b_fill;

    int i = 0;
    raster_sort();
    n_check = 0;
    tn_top_y = p_raster_points[n_raster_count-1].y;
    tn_dir = 0;
    tn_y = p_raster_points[i].y;
    while(i < n_raster_count){
        k = p_raster_points[i].y * n_w + p_raster_points[i].x;
        tn_x = p_raster_points[i].x - 1;
        k_start = k;
        b_fill = tn_dir;
        while(i<n_raster_count){
            if(p_raster_points[i].y != tn_y || p_raster_points[i].x != tn_x + 1){
                break;
            }
            tn_x = p_raster_points[i].x;
            tn_dir += p_raster_points[i].dir;
            buf[k] = p_raster_points[i].value;
            if(!buf[k]){
                buf[k] = 1;
            }
            k++;
            i++;
        }
        if(tn_y > 0 && tn_y < tn_top_y){
            if(b_fill){
                p_raster_points[n_check].x = k_start;
            }
            else{
                p_raster_points[n_check].x = k_start + 1;
            }
            if(tn_dir){
                p_raster_points[n_check].y = k;
            }
            else{
                p_raster_points[n_check].y = k - 1;
            }
            n_check++;
        }
        if(p_raster_points[i].y != tn_y){
            tn_dir = 0;
            tn_y = p_raster_points[i].y;
        }
        else if(tn_dir != 0){
            int j;
            for(j=0; j<p_raster_points[i].x-tn_x-1; j++){
                buf[k] = 255;
                k++;
            }
        }
    }
    for(i=0; i<n_check; i++){
        int j;
        tn_y = p_raster_points[i].x / n_w;
        tn_x = p_raster_points[i].x % n_w;
        for(j=p_raster_points[i].x; j<p_raster_points[i].y; j++){
            if(buf[j-n_w] && buf[j+n_w]){
                buf[j] = 255;
            }
        }
    }
}

void raster_stroke(unsigned char * buf, int n_w){
    int i;
    int k;

    raster_sort();
    i = 0;
    while(i<n_raster_count){
        k = p_raster_points[i].y * n_w + p_raster_points[i].x;
        buf[k] = p_raster_points[i].value;
        i++;
    }
}

void add_raster_pair_steep(int n_x, int n_y, float f){
    int tn_size;
    unsigned char value;
    char dir;

    if(n_raster_count + 2 > n_raster_size){
        if(n_raster_size == 0){
            n_raster_size = 10;
        }
        else{
            n_raster_size *= 2;
        }
        p_raster_points=(struct raster_point*)realloc(p_raster_points, n_raster_size*sizeof(struct raster_point));
    }
    if(n_raster_ymax < n_y + 1){
        int i;
        tn_size = (n_y + 10) * 2;
        s_raster_dirs=(char*)realloc(s_raster_dirs, tn_size*sizeof(char));
        for(i=n_raster_ymax; i<tn_size; i++){
            s_raster_dirs[i] = 0;
        }
        n_raster_ymax = tn_size - 1;
    }
    value = (unsigned char)(f * 255);
    if(b_dir_changed){
        int k;
        if(s_raster_dirs[n_y] == 0){
            dir = 0;
        }
        else{
            dir = n_raster_dir;
        }
        for(k=n_last_point-2; k<n_last_point; k++){
            if(((n_raster_dir == 1 && p_raster_points[k].y < n_y) || (n_raster_dir == -1 && p_raster_points[k].y > n_y)) && (s_raster_dirs[p_raster_points[k].y] == -n_raster_dir)){
                if(p_raster_points[k].dir){
                    p_raster_points[k].dir = 0;
                }
                else{
                    p_raster_points[k].dir = n_raster_dir;
                }
                break;
            }
        }
        memset(s_raster_dirs, 0, n_raster_ymax + 1);
        b_dir_changed = 0;
        if(n_raster_count > n_last_point){
            s_raster_dirs[p_raster_points[n_raster_count-1].y] = n_raster_dir;
        }
        s_raster_dirs[n_y] = n_raster_dir;
    }
    else{
        if(n_raster_dir == s_raster_dirs[n_y]){
            dir = 0;
        }
        else{
            s_raster_dirs[n_y] = n_raster_dir;
            dir = n_raster_dir;
        }
    }
    p_raster_points[n_raster_count].x = n_x;
    p_raster_points[n_raster_count].y = n_y;
    p_raster_points[n_raster_count].dir = dir;
    p_raster_points[n_raster_count].value = value;
    n_raster_count++;
    value = 255 - value;
    n_x++;
    dir = 0;
    p_raster_points[n_raster_count].x = n_x;
    p_raster_points[n_raster_count].y = n_y;
    p_raster_points[n_raster_count].dir = dir;
    p_raster_points[n_raster_count].value = value;
    n_raster_count++;
}

void add_raster_pair_flat(int n_x, int n_y, float f){
    int tn_size;
    unsigned char value;
    char dir;

    if(n_raster_count + 2 > n_raster_size){
        if(n_raster_size == 0){
            n_raster_size = 10;
        }
        else{
            n_raster_size *= 2;
        }
        p_raster_points=(struct raster_point*)realloc(p_raster_points, n_raster_size*sizeof(struct raster_point));
    }
    if(n_raster_ymax < n_y + 1){
        int i;
        tn_size = (n_y + 10) * 2;
        s_raster_dirs=(char*)realloc(s_raster_dirs, tn_size*sizeof(char));
        for(i=n_raster_ymax; i<tn_size; i++){
            s_raster_dirs[i] = 0;
        }
        n_raster_ymax = tn_size - 1;
    }
    value = (unsigned char)(f * 255);
    if(b_dir_changed){
        if(s_raster_dirs[n_y] == 0){
            dir = 0;
        }
        else{
            dir = n_raster_dir;
        }
        s_raster_dirs[n_y] = n_raster_dir;
    }
    else{
        if(n_raster_dir == s_raster_dirs[n_y]){
            dir = 0;
        }
        else{
            s_raster_dirs[n_y] = n_raster_dir;
            dir = n_raster_dir;
        }
    }
    p_raster_points[n_raster_count].x = n_x;
    p_raster_points[n_raster_count].y = n_y;
    p_raster_points[n_raster_count].dir = dir;
    p_raster_points[n_raster_count].value = value;
    n_raster_count++;
    value = 255 - value;
    n_y++;
    if(b_dir_changed){
        int k;
        if(s_raster_dirs[n_y] == 0){
            dir = 0;
        }
        else{
            dir = n_raster_dir;
        }
        for(k=n_last_point-2; k<n_last_point; k++){
            if(((n_raster_dir == 1 && p_raster_points[k].y < n_y) || (n_raster_dir == -1 && p_raster_points[k].y > n_y)) && (s_raster_dirs[p_raster_points[k].y] == -n_raster_dir)){
                if(p_raster_points[k].dir){
                    p_raster_points[k].dir = 0;
                }
                else{
                    p_raster_points[k].dir = n_raster_dir;
                }
                break;
            }
        }
        memset(s_raster_dirs, 0, n_raster_ymax + 1);
        b_dir_changed = 0;
        if(n_raster_count > n_last_point){
            s_raster_dirs[p_raster_points[n_raster_count-1].y] = n_raster_dir;
        }
        s_raster_dirs[n_y] = n_raster_dir;
    }
    else{
        if(n_raster_dir == s_raster_dirs[n_y]){
            dir = 0;
        }
        else{
            s_raster_dirs[n_y] = n_raster_dir;
            dir = n_raster_dir;
        }
    }
    p_raster_points[n_raster_count].x = n_x;
    p_raster_points[n_raster_count].y = n_y;
    p_raster_points[n_raster_count].dir = dir;
    p_raster_points[n_raster_count].value = value;
    n_raster_count++;
}

void raster_sort(){
    int tn_v;

    int i;
    for(i=1; i<n_raster_count; i++){
        struct raster_point t;
        int j = i-1;
        t = p_raster_points[i];
        while(j>=0){
            if(p_raster_points[i].y > p_raster_points[j].y){
                break;
            }
            else if(p_raster_points[i].y < p_raster_points[j].y){
                j--;
                continue;
            }
            else{
                if(p_raster_points[i].x > p_raster_points[j].x){
                    break;
                }
                else if(p_raster_points[i].x < p_raster_points[j].x){
                    j--;
                    continue;
                }
                else{
                    tn_v = p_raster_points[i].value + p_raster_points[j].value;
                    p_raster_points[j].value = tn_v < 256 ? tn_v: 255;
                    p_raster_points[j].dir += p_raster_points[i].dir;
                    p_raster_points[i].y = SHRT_MAX;
                    j = i;
                    break;
                }
            }
            j--;
        }
        j++;
        if(i > j){
            memmove(&p_raster_points[j+1], &p_raster_points[j], sizeof(struct raster_point) * (i - j));
            p_raster_points[j] = t;
        }
    }
    i = n_raster_count - 1;
    while(p_raster_points[i].y == SHRT_MAX){
        i--;
    }
    n_raster_count = i + 1;
}

