#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "user32")
/* link: -luser32  */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef int bool;
#define true 1
#define false 0
#define snprintf sprintf_s

struct glyph { /*public*/
    unsigned char * buf;
    int x;
    int y;
    int w;
    int h;
    float advance;
};

struct font {
    char * s;
    int n_pt_size;
    float f_px_size;
    unsigned char* pc_cache;
    struct glyph cur_glyph;
    struct ttf_head* p_head;
    struct ttf_hhea* p_hhea;
    char * s_loca;
    char * s_glyf;
    char * s_hmtx;
    char * s_cmap;
    int n_cmap_id0;
    int n_cmap_id1;
    int n_glyphs;
};

struct ttf_head {
    char s_ver[4];
    char s_rev[4];
    uint32_t u4_checksum;
    char s_magic[4];
    uint16_t u2_flag;
    uint16_t u2_em;
    char s_created[8];
    char s_modified[8];
    int16_t n2_xmin;
    int16_t n2_ymin;
    int16_t n2_xmax;
    int16_t n2_ymax;
    uint16_t u2_style;
    uint16_t u2_minsize;
    int16_t n2_direction;
    int16_t n2_index_format;
    int16_t n2_glyph_format;
};

struct ttf_hhea {
    char s_ver[4];
    int16_t n2_ascent;
    int16_t n2_descent;
    int16_t n2_linegap;
    uint16_t u2_advance_max;
    int16_t n2_left_min;
    int16_t n2_right_min;
    int16_t n2_extent_max;
    int16_t n2_caretSlopeRise;
    int16_t n2_caretSlopeRun;
    int16_t n2_caretOffset;
    char s_reserved[10];
    uint16_t u2_num_metrics;
};

struct string {
    char * s;
    int n;
    unsigned int size;
};

struct font * load_font(char * s_fontfile);
void font_set_size(struct font * pf, int n_pt_size, float f_pt_size);
struct glyph * font_render(struct font * pf, int c);
static struct string load_file(char * s_file);
static int f_get_short(char * s);
static int f_get_int(char * s);
static void swap_word(char * s, int n);
static int font_cmap(struct font * pf, int c);
void raster_start();
void raster_new_contour();
void raster_curve(float tf_x0, float tf_y0, float tf_x1, float tf_y1, float tf_x2, float tf_y2);
void raster_line(float tf_x0, float tf_y0, float tf_x1, float tf_y1);
void raster_end_contour();
void raster_fill(unsigned char * buf, int n_w);
static void get_error(char * s_prompt);

extern HINSTANCE cur_instance;
extern HWND hwnd_main;
char s_debug_buffer[1024];

struct font * load_font(char * s_fontfile){
    struct string str;
    struct font * pf;
    unsigned char * s;
    int tn_major;
    int tn_minor;
    int tn_tables;
    int tn_pos;
    int tn_off;
    int tn_size;
    int tn_cmap;
    int tn_id0;
    int tn_id1;
    int tn_off2;
    int tn_format;

    int i;
    str = load_file(s_fontfile);
    pf=(struct font*)malloc(sizeof(struct font));
    pf->s = NULL;
    pf->s_cmap = NULL;
    pf->n_pt_size = 12;
    pf->f_px_size = 0.012f;
    pf->pc_cache = NULL;
    pf->cur_glyph.buf = NULL;
    s = (unsigned char*)str.s;
    tn_major = f_get_short(s + 0);
    tn_minor = f_get_short(s + 2);
    tn_tables = f_get_short(s + 4);
    pf->s = s;
    for(i=0; i<tn_tables; i++){
        tn_pos = 12 + 16 * i;
        tn_off = f_get_int(s + tn_pos + 8);
        tn_size = f_get_int(s + tn_pos + 12);
        if(strncmp((s + tn_pos), "head", 4) == 0){
            pf->p_head = (struct ttf_head*)(s + tn_off);
            swap_word((char*)(&pf->p_head->u4_checksum), 4);
            swap_word((char*)(&pf->p_head->u2_flag), 2);
            swap_word((char*)(&pf->p_head->u2_em), 2);
            swap_word((char*)(&pf->p_head->n2_xmin), 2);
            swap_word((char*)(&pf->p_head->n2_ymin), 2);
            swap_word((char*)(&pf->p_head->n2_xmax), 2);
            swap_word((char*)(&pf->p_head->n2_ymax), 2);
            swap_word((char*)(&pf->p_head->u2_style), 2);
            swap_word((char*)(&pf->p_head->u2_minsize), 2);
            swap_word((char*)(&pf->p_head->n2_direction), 2);
            swap_word((char*)(&pf->p_head->n2_index_format), 2);
            swap_word((char*)(&pf->p_head->n2_glyph_format), 2);
        }
        else if(strncmp((s + tn_pos), "hhea", 4) == 0){
            pf->p_hhea = (struct ttf_hhea*)(s + tn_off);
            swap_word((char*)(&pf->p_hhea->n2_ascent), 2);
            swap_word((char*)(&pf->p_hhea->n2_descent), 2);
            swap_word((char*)(&pf->p_hhea->n2_linegap), 2);
            swap_word((char*)(&pf->p_hhea->u2_advance_max), 2);
            swap_word((char*)(&pf->p_hhea->n2_left_min), 2);
            swap_word((char*)(&pf->p_hhea->n2_right_min), 2);
            swap_word((char*)(&pf->p_hhea->n2_extent_max), 2);
            swap_word((char*)(&pf->p_hhea->n2_caretSlopeRise), 2);
            swap_word((char*)(&pf->p_hhea->n2_caretSlopeRun), 2);
            swap_word((char*)(&pf->p_hhea->n2_caretOffset), 2);
            swap_word((char*)(&pf->p_hhea->u2_num_metrics), 2);
        }
        else if(strncmp((s + tn_pos), "loca", 4) == 0){
            pf->s_loca = s + tn_off;
        }
        else if(strncmp((s + tn_pos), "glyf", 4) == 0){
            pf->s_glyf = s + tn_off;
        }
        else if(strncmp((s + tn_pos), "hmtx", 4) == 0){
            pf->s_hmtx = s + tn_off;
        }
        else if(strncmp((s + tn_pos), "cmap", 4) == 0){
            int j;
            tn_cmap = f_get_short(s + tn_off + 2);
            for(j=0; j<tn_cmap; j++){
                tn_pos = tn_off + 4 + j * 8;
                tn_id0 = f_get_short(s + tn_pos);
                tn_id1 = f_get_short(s + tn_pos + 2);
                tn_off2 = f_get_int(s + tn_pos + 4);
                tn_format = f_get_short(s + tn_off + tn_off2);
                if((tn_id0 == 0 || tn_id0 == 3) && (tn_format == 4 || tn_format == 6 || tn_format == 12)){
                    pf->s_cmap = s + tn_off + tn_off2;
                    pf->n_cmap_id0 = tn_id0;
                    pf->n_cmap_id1 = tn_id1;
                    break;
                }
            }
        }
        else if(strncmp((s + tn_pos), "maxp", 4) == 0){
            pf->n_glyphs = f_get_short(s + tn_off + 4);
        }
    }
    return pf;
}

void font_set_size(struct font * pf, int n_pt_size, float f_pt_size){
    int tn_w;
    int tn_h;
    int tn_size;

    pf->n_pt_size = n_pt_size;
    pf->f_px_size = f_pt_size * n_pt_size / pf->p_head->u2_em;
    tn_w = pf->p_head->n2_xmax - pf->p_head->n2_xmin + 1;
    tn_h = pf->p_head->n2_ymax - pf->p_head->n2_ymin + 1;
    tn_w = (int)(tn_w * pf->f_px_size) + 1;
    tn_h = (int)(tn_h * pf->f_px_size) + 1;
    tn_size = tn_w * tn_h;
    pf->pc_cache=(unsigned char*)realloc(pf->pc_cache, tn_size*sizeof(unsigned char));
}

struct glyph * font_render(struct font * pf, int c){
    struct glyph * glyph = NULL;
    int tn_idx;
    int tn_metrics;
    char * s;
    int tn_advance;
    int tn_left;
    char * ts_loca;
    int tn_off;
    int tn_off2;
    char * ts_glyph;
    int tn_contours;
    int tn_xmin;
    int tn_ymin;
    int tn_xmax;
    int tn_ymax;
    char * ts_contour;
    int tn_points;
    char * ts_instruction;
    int tn_instruction;
    char * ts_flags;
    int tn_xsize;
    int tn_unit;
    int tn_rep;
    char * ts_x;
    char * ts_y;
    int tn_x;
    int tn_y;
    int i_contour;
    int i_pt;
    int i_f;
    int i_x;
    int i_y;
    int tn_end;
    int tn_flag;
    bool tb_start;
    float tf_x_start;
    float tf_y_start;
    float tf_x0;
    float tf_y0;
    float tf_x1;
    float tf_y1;
    float tf_x;
    float tf_y;
    bool tb_curve;
    float tf_x_temp;
    float tf_y_temp;
    int tn_a;
    int tn_d;
    int tn_b;
    int tn_c;

    glyph = &(pf->cur_glyph);
    glyph->buf = pf->pc_cache;
    tn_idx = font_cmap(pf, c);
    tn_metrics = pf->p_hhea->u2_num_metrics;
    if(tn_idx < tn_metrics){
        s = pf->s_hmtx + 4 * tn_idx;
        tn_advance = f_get_short(s + 0);
        tn_left = f_get_short(s + 2);
    }
    else{
        s = pf->s_hmtx + 4 * tn_metrics;
        tn_advance = f_get_short(s - 2);
        tn_left = f_get_short(s + tn_idx * 2 + tn_metrics * 2);
    }
    glyph->advance = tn_advance * pf->f_px_size;
    if(pf->p_head->n2_index_format == 0){
        ts_loca = pf->s_loca + 2 * tn_idx;
        tn_off = f_get_short(ts_loca + 0);
        tn_off2 = f_get_short(ts_loca + 2);
    }
    else{
        ts_loca = pf->s_loca + 4 * tn_idx;
        tn_off = f_get_int(ts_loca + 0);
        tn_off2 = f_get_int(ts_loca + 4);
    }
    if(tn_off2 > tn_off){
        ts_glyph = pf->s_glyf + tn_off;
            tn_contours = f_get_short(ts_glyph + 0);
            tn_xmin = f_get_short(ts_glyph + 2);
            tn_ymin = f_get_short(ts_glyph + 4);
            tn_xmax = f_get_short(ts_glyph + 6);
            tn_ymax = f_get_short(ts_glyph + 8);
            glyph->w = (int)((tn_xmax - tn_xmin + 1) * pf->f_px_size) + 2;
            glyph->h = (int)((tn_ymax - tn_ymin + 1) * pf->f_px_size) + 2;
            glyph->x = (int)((-tn_left) * pf->f_px_size);
            glyph->y = (int)((-tn_ymin) * pf->f_px_size);
            s = ts_glyph + 10;
            raster_start();
            if(tn_contours > 0){
                int i = 0;
                ts_contour = s;
                tn_points = f_get_short(s + (tn_contours - 1) * 2) + 1;
                ts_instruction = s + tn_contours * 2 + 2;
                tn_instruction = f_get_short(ts_instruction - 2);
                ts_flags = ts_instruction + tn_instruction;
                tn_xsize = 0;
                s = ts_flags;
                while(i < tn_points){
                    if(s[0] & 0x2){
                        tn_unit = 1;
                    }
                    else if(s[0] & 0x10){
                        tn_unit = 0;
                    }
                    else{
                        tn_unit = 2;
                    }
                    if(s[0] & 0x8){
                        tn_rep = s[1] + 1;
                        tn_xsize += tn_unit * tn_rep;
                        i += tn_rep;
                        s += 2;
                    }
                    else{
                        tn_xsize += tn_unit;
                        i++;
                        s++;
                    }
                }
                ts_x = s;
                ts_y = ts_x + tn_xsize;
                tn_x = 0;
                tn_y = 0;
                i_contour = 0;
                i_pt = 0;
                i_f = 0;
                i_x = 0;
                i_y = 0;
                tn_end = -1;
                while(i_pt < tn_points){
                    tn_flag = ts_flags[i_f];
                    if(tn_flag & 0x08){
                        tn_rep = ts_flags[i_f+1] + 1;
                        i_f += 2;
                    }
                    else{
                        tn_rep = 1;
                        i_f++;
                    }
                    if(i_pt + tn_rep <= tn_points){
                        int j;
                        for(j=0; j<tn_rep; j++){
                            if(tn_flag & 0x02){
                                if(tn_flag & 0x10){
                                    tn_x += (unsigned char)ts_x[i_x];
                                }
                                else{
                                    tn_x += -(unsigned char)ts_x[i_x];
                                }
                                i_x++;
                            }
                            else{
                                if(tn_flag & 0x10){
                                }
                                else{
                                    tn_x += f_get_short(ts_x + i_x);
                                    i_x += 2;
                                }
                            }
                            if(tn_flag & 0x04){
                                if(tn_flag & 0x20){
                                    tn_y += (unsigned char)ts_y[i_y];
                                }
                                else{
                                    tn_y += -(unsigned char)ts_y[i_y];
                                }
                                i_y++;
                            }
                            else{
                                if(tn_flag & 0x20){
                                }
                                else{
                                    tn_y += f_get_short(ts_y + i_y);
                                    i_y += 2;
                                }
                            }
                            if(i_pt > tn_end){
                                tn_end = f_get_short(ts_contour + i_contour * 2);
                                i_contour++;
                                tb_start = 1;
                                raster_new_contour();
                            }
                            tf_x = (tn_x - tn_xmin) * pf->f_px_size;
                            tf_y = (tn_y - tn_ymin) * pf->f_px_size;
                            if(tb_start){
                                tb_start = 0;
                                tb_curve = 0;
                                tf_x0 = tf_x;
                                tf_y0 = tf_y;
                                tf_x_start = tf_x;
                                tf_y_start = tf_y;
                            }
                            else if(tn_flag & 0x1){
                                if(tb_curve){
                                    raster_curve(tf_x0, tf_y0, tf_x1, tf_y1, tf_x, tf_y);
                                }
                                else{
                                    raster_line(tf_x0, tf_y0, tf_x, tf_y);
                                }
                                tf_x0 = tf_x;
                                tf_y0 = tf_y;
                                tb_curve = 0;
                            }
                            else{
                                if(tb_curve){
                                    tf_x_temp = tf_x;
                                    tf_y_temp = tf_y;
                                    tf_x = (tf_x1 + tf_x) / 2;
                                    tf_y = (tf_y1 + tf_y) / 2;
                                    raster_curve(tf_x0, tf_y0, tf_x1, tf_y1, tf_x, tf_y);
                                    tf_x0 = tf_x;
                                    tf_y0 = tf_y;
                                    tf_x1 = tf_x_temp;
                                    tf_y1 = tf_y_temp;
                                }
                                else{
                                    tb_curve = 1;
                                    tf_x1 = tf_x;
                                    tf_y1 = tf_y;
                                }
                            }
                            if(i_pt >= tn_end){
                                tf_x = tf_x_start;
                                tf_y = tf_y_start;
                                if(tb_curve){
                                    raster_curve(tf_x0, tf_y0, tf_x1, tf_y1, tf_x, tf_y);
                                }
                                else{
                                    raster_line(tf_x0, tf_y0, tf_x, tf_y);
                                }
                                raster_end_contour();
                            }
                            i_pt++;
                        }
                    }
                    else{
                        snprintf(s_debug_buffer, 1024, "Error: i_pt: %d, tn_rep=%d, tn_points=%d\n", i_pt, tn_rep, tn_points);
                        OutputDebugString(s_debug_buffer);
                        break;
                    }
                }
            }
            else{
                do{
                    tn_flag = f_get_short(s);
                    tn_idx = f_get_short(s + 2);
                    if(tn_flag & 0x1){
                        tn_x = f_get_short(s + 4);
                        tn_y = f_get_short(s + 6);
                        s += 8;
                    }
                    else{
                        tn_x = s[4];
                        tn_y = s[5];
                        s += 6;
                    }
                    if(tn_flag & 0x8){
                        tn_a = f_get_short(s);
                        s += 2;
                    }
                    else if(tn_flag & 0x20){
                        tn_a = f_get_short(s);
                        tn_d = f_get_short(s + 2);
                        s += 4;
                    }
                    else if(tn_flag & 0x40){
                        tn_a = f_get_short(s);
                        tn_b = f_get_short(s + 2);
                        tn_c = f_get_short(s + 4);
                        tn_d = f_get_short(s + 6);
                        s += 8;
                    }
                }while(tn_flag & 0x20);
            }
            memset(glyph->buf, 0, glyph->w * glyph->h);
            raster_fill(glyph->buf, glyph->w);
    }
    return glyph;
}

struct string load_file(char * s_file){
    HANDLE hf;
    __int64 size;
    struct string str;

    hf = CreateFile(s_file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hf == INVALID_HANDLE_VALUE){
        get_error("CreateFile");
    }
    GetFileSizeEx(hf, (PLARGE_INTEGER) & size);
    str.size = (unsigned int)size;
    str.s=(char*)malloc(str.size*sizeof(char));
    if(!ReadFile(hf, str.s, str.size, &str.n, NULL)){
        get_error("ReadFile");
    }
    CloseHandle(hf);
    return str;
}

int f_get_short(char * s){
    char s_t[2] = {s[1], s[0]};

    return *(short*)s_t;
}

int f_get_int(char * s){
    char s_t[4] = {s[3],s[2],s[1],s[0]};

    return *(int*)s_t;
}

void swap_word(char * s, int n){
    int j;
    unsigned char c;

    int i;
    for(i=0; i<n/2; i++){
        j = n - 1 - i;
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int font_cmap(struct font * pf, int c){
    int tn_idx;
    char * s;
    int tn_format;
    int tn_segs;
    char * ts_endcodes;
    char * ts_startcodes;
    char * ts_deltas;
    char * ts_rangeoffs;
    int i;
    int tn_end;
    int tn_start;
    int tn_delta;
    int tn_rangeoff;
    char * ts_index;
    int tn_count;
    char * ts_group;
    int tn_code;

    tn_idx = 0;
    if(pf->s_cmap == NULL){
        return 0;
    }
    s = pf->s_cmap;
    tn_format = f_get_short(s);
    if(tn_format == 4){
        tn_segs = f_get_short(s + 6);
        ts_endcodes = s + 14;
        ts_startcodes = ts_endcodes + tn_segs + 2;
        ts_deltas = ts_startcodes + tn_segs;
        ts_rangeoffs = ts_deltas + tn_segs;
        i = 0;
        while(i<tn_segs){
            tn_end = f_get_short(ts_endcodes + i);
            if(c <= tn_end){
                break;
            }
            i+=2;
        }
        tn_start = f_get_short(ts_startcodes + i);
        tn_delta = f_get_short(ts_deltas + i);
        tn_rangeoff = f_get_short(ts_rangeoffs + i);
        if(c >= tn_start){
            if(tn_rangeoff == 0){
                tn_idx = (c + tn_delta) & 0xffff;
            }
            else{
                ts_index = ts_rangeoffs + i + tn_rangeoff;
                tn_idx = f_get_short(ts_index + (c - tn_start) * 2);
            }
        }
        else{
            return 0;
        }
    }
    else if(tn_format == 6){
        tn_start = f_get_short(s + 6);
        tn_count = f_get_short(s + 8);
        ts_index = s + 10;
        if(c >= tn_start){
            tn_idx = f_get_short(ts_index + (c - tn_start) * 2);
        }
        else{
            return 0;
        }
    }
    else if(tn_format == 12){
        int i;
        tn_segs = f_get_int(s + 12);
        ts_group = s + 16;
        for(i=0; i<tn_segs; i++){
            tn_end = f_get_int(ts_group + 4);
            if(c <= tn_end){
                break;
            }
            ts_group += 12;
        }
        tn_start = f_get_int(ts_group);
        tn_code = f_get_int(ts_group + 8);
        if(c >= tn_start){
            tn_idx = tn_code + (c - tn_start);
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
    if(tn_idx < pf->n_glyphs){
        return tn_idx;
    }
    else{
        return 0;
    }
}

void get_error(char * s_prompt){
    char * s = "";
    DWORD dw;

    dw = GetLastError();
    switch(dw){
        case 0:
            s = "ERROR_SUCCESS";
            break;
        case 1:
            s = "ERROR_INVALID_FUNCTION";
            break;
        case 2:
            s = "ERROR_FILE_NOT_FOUND";
            break;
        case 3:
            s = "ERROR_PATH_NOT_FOUND";
            break;
        case 4:
            s = "ERROR_TOO_MANY_OPEN_FILES";
            break;
        case 5:
            s = "ERROR_ACCESS_DENIED";
            break;
        case 6:
            s = "ERROR_INVALID_HANDLE";
            break;
        case 7:
            s = "ERROR_ARENA_TRASHED";
            break;
        case 8:
            s = "ERROR_NOT_ENOUGH_MEMORY";
            break;
        case 9:
            s = "ERROR_INVALID_BLOCK";
            break;
        case 10:
            s = "ERROR_BAD_ENVIRONMENT";
            break;
        case 11:
            s = "ERROR_BAD_FORMAT";
            break;
        case 12:
            s = "ERROR_INVALID_ACCESS";
            break;
        case 13:
            s = "ERROR_INVALID_DATA";
            break;
        case 14:
            s = "ERROR_OUTOFMEMORY";
            break;
        case 15:
            s = "ERROR_INVALID_DRIVE";
            break;
        case 16:
            s = "ERROR_CURRENT_DIRECTORY";
            break;
        case 17:
            s = "ERROR_NOT_SAME_DEVICE";
            break;
        case 18:
            s = "ERROR_NO_MORE_FILES";
            break;
        case 19:
            s = "ERROR_WRITE_PROTECT";
            break;
        case 20:
            s = "ERROR_BAD_UNIT";
            break;
        case 21:
            s = "ERROR_NOT_READY";
            break;
        case 22:
            s = "ERROR_BAD_COMMAND";
            break;
        case 23:
            s = "ERROR_CRC";
            break;
        case 24:
            s = "ERROR_BAD_LENGTH";
            break;
        case 25:
            s = "ERROR_SEEK";
            break;
        case 26:
            s = "ERROR_NOT_DOS_DISK";
            break;
        case 27:
            s = "ERROR_SECTOR_NOT_FOUND";
            break;
        case 28:
            s = "ERROR_OUT_OF_PAPER";
            break;
        case 29:
            s = "ERROR_WRITE_FAULT";
            break;
        case 30:
            s = "ERROR_READ_FAULT";
            break;
        case 31:
            s = "ERROR_GEN_FAILURE";
            break;
        case 32:
            s = "ERROR_SHARING_VIOLATION";
            break;
        case 33:
            s = "ERROR_LOCK_VIOLATION";
            break;
        case 34:
            s = "ERROR_WRONG_DISK";
            break;
        case 36:
            s = "ERROR_SHARING_BUFFER_EXCEEDED";
            break;
        case 38:
            s = "ERROR_HANDLE_EOF";
            break;
        case 39:
            s = "ERROR_HANDLE_DISK_FULL";
            break;
        case 50:
            s = "ERROR_NOT_SUPPORTED";
            break;
        case 51:
            s = "ERROR_REM_NOT_LIST";
            break;
        case 52:
            s = "ERROR_DUP_NAME";
            break;
        case 53:
            s = "ERROR_BAD_NETPATH";
            break;
        case 54:
            s = "ERROR_NETWORK_BUSY";
            break;
        case 55:
            s = "ERROR_DEV_NOT_EXIST";
            break;
        case 56:
            s = "ERROR_TOO_MANY_CMDS";
            break;
        case 57:
            s = "ERROR_ADAP_HDW_ERR";
            break;
        case 58:
            s = "ERROR_BAD_NET_RESP";
            break;
        case 59:
            s = "ERROR_UNEXP_NET_ERR";
            break;
        case 60:
            s = "ERROR_BAD_REM_ADAP";
            break;
        case 61:
            s = "ERROR_PRINTQ_FULL";
            break;
        case 62:
            s = "ERROR_NO_SPOOL_SPACE";
            break;
        case 63:
            s = "ERROR_PRINT_CANCELLED";
            break;
        case 64:
            s = "ERROR_NETNAME_DELETED";
            break;
        case 65:
            s = "ERROR_NETWORK_ACCESS_DENIED";
            break;
        case 66:
            s = "ERROR_BAD_DEV_TYPE";
            break;
        case 67:
            s = "ERROR_BAD_NET_NAME";
            break;
        case 68:
            s = "ERROR_TOO_MANY_NAMES";
            break;
        case 69:
            s = "ERROR_TOO_MANY_SESS";
            break;
        case 70:
            s = "ERROR_SHARING_PAUSED";
            break;
        case 71:
            s = "ERROR_REQ_NOT_ACCEP";
            break;
        case 72:
            s = "ERROR_REDIR_PAUSED";
            break;
        case 80:
            s = "ERROR_FILE_EXISTS";
            break;
        case 82:
            s = "ERROR_CANNOT_MAKE";
            break;
        case 83:
            s = "ERROR_FAIL_I24";
            break;
        case 84:
            s = "ERROR_OUT_OF_STRUCTURES";
            break;
        case 85:
            s = "ERROR_ALREADY_ASSIGNED";
            break;
        case 86:
            s = "ERROR_INVALID_PASSWORD";
            break;
        case 87:
            s = "ERROR_INVALID_PARAMETER";
            break;
        case 88:
            s = "ERROR_NET_WRITE_FAULT";
            break;
        case 89:
            s = "ERROR_NO_PROC_SLOTS";
            break;
        case 100:
            s = "ERROR_TOO_MANY_SEMAPHORES";
            break;
        case 101:
            s = "ERROR_EXCL_SEM_ALREADY_OWNED";
            break;
        case 102:
            s = "ERROR_SEM_IS_SET";
            break;
        case 103:
            s = "ERROR_TOO_MANY_SEM_REQUESTS";
            break;
        case 104:
            s = "ERROR_INVALID_AT_INTERRUPT_TIME";
            break;
        case 105:
            s = "ERROR_SEM_OWNER_DIED";
            break;
        case 106:
            s = "ERROR_SEM_USER_LIMIT";
            break;
        case 107:
            s = "ERROR_DISK_CHANGE";
            break;
        case 108:
            s = "ERROR_DRIVE_LOCKED";
            break;
        case 109:
            s = "ERROR_BROKEN_PIPE";
            break;
        case 110:
            s = "ERROR_OPEN_FAILED";
            break;
        case 111:
            s = "ERROR_BUFFER_OVERFLOW";
            break;
        case 112:
            s = "ERROR_DISK_FULL";
            break;
        case 113:
            s = "ERROR_NO_MORE_SEARCH_HANDLES";
            break;
        case 114:
            s = "ERROR_INVALID_TARGET_HANDLE";
            break;
        case 117:
            s = "ERROR_INVALID_CATEGORY";
            break;
        case 118:
            s = "ERROR_INVALID_VERIFY_SWITCH";
            break;
        case 119:
            s = "ERROR_BAD_DRIVER_LEVEL";
            break;
        case 120:
            s = "ERROR_CALL_NOT_IMPLEMENTED";
            break;
        case 121:
            s = "ERROR_SEM_TIMEOUT";
            break;
        case 122:
            s = "ERROR_INSUFFICIENT_BUFFER";
            break;
        case 123:
            s = "ERROR_INVALID_NAME";
            break;
        case 124:
            s = "ERROR_INVALID_LEVEL";
            break;
        case 125:
            s = "ERROR_NO_VOLUME_LABEL";
            break;
        case 126:
            s = "ERROR_MOD_NOT_FOUND";
            break;
        case 127:
            s = "ERROR_PROC_NOT_FOUND";
            break;
        case 128:
            s = "ERROR_WAIT_NO_CHILDREN";
            break;
        case 129:
            s = "ERROR_CHILD_NOT_COMPLETE";
            break;
        case 130:
            s = "ERROR_DIRECT_ACCESS_HANDLE";
            break;
        case 131:
            s = "ERROR_NEGATIVE_SEEK";
            break;
        case 132:
            s = "ERROR_SEEK_ON_DEVICE";
            break;
        case 133:
            s = "ERROR_IS_JOIN_TARGET";
            break;
        case 134:
            s = "ERROR_IS_JOINED";
            break;
        case 135:
            s = "ERROR_IS_SUBSTED";
            break;
        case 136:
            s = "ERROR_NOT_JOINED";
            break;
        case 137:
            s = "ERROR_NOT_SUBSTED";
            break;
        case 138:
            s = "ERROR_JOIN_TO_JOIN";
            break;
        case 139:
            s = "ERROR_SUBST_TO_SUBST";
            break;
        case 140:
            s = "ERROR_JOIN_TO_SUBST";
            break;
        case 141:
            s = "ERROR_SUBST_TO_JOIN";
            break;
        case 142:
            s = "ERROR_BUSY_DRIVE";
            break;
        case 143:
            s = "ERROR_SAME_DRIVE";
            break;
        case 144:
            s = "ERROR_DIR_NOT_ROOT";
            break;
        case 145:
            s = "ERROR_DIR_NOT_EMPTY";
            break;
        case 146:
            s = "ERROR_IS_SUBST_PATH";
            break;
        case 147:
            s = "ERROR_IS_JOIN_PATH";
            break;
        case 148:
            s = "ERROR_PATH_BUSY";
            break;
        case 149:
            s = "ERROR_IS_SUBST_TARGET";
            break;
        case 150:
            s = "ERROR_SYSTEM_TRACE";
            break;
        case 151:
            s = "ERROR_INVALID_EVENT_COUNT";
            break;
        case 152:
            s = "ERROR_TOO_MANY_MUXWAITERS";
            break;
        case 153:
            s = "ERROR_INVALID_LIST_FORMAT";
            break;
        case 154:
            s = "ERROR_LABEL_TOO_LONG";
            break;
        case 155:
            s = "ERROR_TOO_MANY_TCBS";
            break;
        case 156:
            s = "ERROR_SIGNAL_REFUSED";
            break;
        case 157:
            s = "ERROR_DISCARDED";
            break;
        case 158:
            s = "ERROR_NOT_LOCKED";
            break;
        case 159:
            s = "ERROR_BAD_THREADID_ADDR";
            break;
        case 160:
            s = "ERROR_BAD_ARGUMENTS";
            break;
        case 161:
            s = "ERROR_BAD_PATHNAME";
            break;
        case 162:
            s = "ERROR_SIGNAL_PENDING";
            break;
        case 164:
            s = "ERROR_MAX_THRDS_REACHED";
            break;
        case 167:
            s = "ERROR_LOCK_FAILED";
            break;
        case 170:
            s = "ERROR_BUSY";
            break;
        case 171:
            s = "ERROR_DEVICE_SUPPORT_IN_PROGRESS";
            break;
        case 173:
            s = "ERROR_CANCEL_VIOLATION";
            break;
        case 174:
            s = "ERROR_ATOMIC_LOCKS_NOT_SUPPORTED";
            break;
        case 180:
            s = "ERROR_INVALID_SEGMENT_NUMBER";
            break;
        case 182:
            s = "ERROR_INVALID_ORDINAL";
            break;
        case 183:
            s = "ERROR_ALREADY_EXISTS";
            break;
        case 186:
            s = "ERROR_INVALID_FLAG_NUMBER";
            break;
        case 187:
            s = "ERROR_SEM_NOT_FOUND";
            break;
        case 188:
            s = "ERROR_INVALID_STARTING_CODESEG";
            break;
        case 189:
            s = "ERROR_INVALID_STACKSEG";
            break;
        case 190:
            s = "ERROR_INVALID_MODULETYPE";
            break;
        case 191:
            s = "ERROR_INVALID_EXE_SIGNATURE";
            break;
        case 192:
            s = "ERROR_EXE_MARKED_INVALID";
            break;
        case 193:
            s = "ERROR_BAD_EXE_FORMAT";
            break;
        case 194:
            s = "ERROR_ITERATED_DATA_EXCEEDS_64k";
            break;
        case 195:
            s = "ERROR_INVALID_MINALLOCSIZE";
            break;
        case 196:
            s = "ERROR_DYNLINK_FROM_INVALID_RING";
            break;
        case 197:
            s = "ERROR_IOPL_NOT_ENABLED";
            break;
        case 198:
            s = "ERROR_INVALID_SEGDPL";
            break;
        case 199:
            s = "ERROR_AUTODATASEG_EXCEEDS_64k";
            break;
        case 200:
            s = "ERROR_RING2SEG_MUST_BE_MOVABLE";
            break;
        case 201:
            s = "ERROR_RELOC_CHAIN_XEEDS_SEGLIM";
            break;
        case 202:
            s = "ERROR_INFLOOP_IN_RELOC_CHAIN";
            break;
        case 203:
            s = "ERROR_ENVVAR_NOT_FOUND";
            break;
        case 205:
            s = "ERROR_NO_SIGNAL_SENT";
            break;
        case 206:
            s = "ERROR_FILENAME_EXCED_RANGE";
            break;
        case 207:
            s = "ERROR_RING2_STACK_IN_USE";
            break;
        case 208:
            s = "ERROR_META_EXPANSION_TOO_LONG";
            break;
        case 209:
            s = "ERROR_INVALID_SIGNAL_NUMBER";
            break;
        case 210:
            s = "ERROR_THREAD_1_INACTIVE";
            break;
        case 212:
            s = "ERROR_LOCKED";
            break;
        case 214:
            s = "ERROR_TOO_MANY_MODULES";
            break;
        case 215:
            s = "ERROR_NESTING_NOT_ALLOWED";
            break;
        case 216:
            s = "ERROR_EXE_MACHINE_TYPE_MISMATCH";
            break;
        case 217:
            s = "ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY";
            break;
        case 218:
            s = "ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY";
            break;
        case 220:
            s = "ERROR_FILE_CHECKED_OUT";
            break;
        case 221:
            s = "ERROR_CHECKOUT_REQUIRED";
            break;
        case 222:
            s = "ERROR_BAD_FILE_TYPE";
            break;
        case 223:
            s = "ERROR_FILE_TOO_LARGE";
            break;
        case 224:
            s = "ERROR_FORMS_AUTH_REQUIRED";
            break;
        case 225:
            s = "ERROR_VIRUS_INFECTED";
            break;
        case 226:
            s = "ERROR_VIRUS_DELETED";
            break;
        case 229:
            s = "ERROR_PIPE_LOCAL";
            break;
        case 230:
            s = "ERROR_BAD_PIPE";
            break;
        case 231:
            s = "ERROR_PIPE_BUSY";
            break;
        case 232:
            s = "ERROR_NO_DATA";
            break;
        case 233:
            s = "ERROR_PIPE_NOT_CONNECTED";
            break;
        case 234:
            s = "ERROR_MORE_DATA";
            break;
        case 240:
            s = "ERROR_VC_DISCONNECTED";
            break;
        case 254:
            s = "ERROR_INVALID_EA_NAME";
            break;
        case 255:
            s = "ERROR_EA_LIST_INCONSISTENT";
            break;
        case 258:
            s = "WAIT_TIMEOUT";
            break;
        case 259:
            s = "ERROR_NO_MORE_ITEMS";
            break;
        case 266:
            s = "ERROR_CANNOT_COPY";
            break;
        case 267:
            s = "ERROR_DIRECTORY";
            break;
        case 275:
            s = "ERROR_EAS_DIDNT_FIT";
            break;
        case 276:
            s = "ERROR_EA_FILE_CORRUPT";
            break;
        case 277:
            s = "ERROR_EA_TABLE_FULL";
            break;
        case 278:
            s = "ERROR_INVALID_EA_HANDLE";
            break;
        case 282:
            s = "ERROR_EAS_NOT_SUPPORTED";
            break;
        case 288:
            s = "ERROR_NOT_OWNER";
            break;
        case 298:
            s = "ERROR_TOO_MANY_POSTS";
            break;
        case 299:
            s = "ERROR_PARTIAL_COPY";
            break;
        case 300:
            s = "ERROR_OPLOCK_NOT_GRANTED";
            break;
        case 301:
            s = "ERROR_INVALID_OPLOCK_PROTOCOL";
            break;
        case 302:
            s = "ERROR_DISK_TOO_FRAGMENTED";
            break;
        case 303:
            s = "ERROR_DELETE_PENDING";
            break;
        case 304:
            s = "ERROR_INCOMPATIBLE_WITH_GLOBAL_SHORT_NAME_REGISTRY_SETTING";
            break;
        case 305:
            s = "ERROR_SHORT_NAMES_NOT_ENABLED_ON_VOLUME";
            break;
        case 306:
            s = "ERROR_SECURITY_STREAM_IS_INCONSISTENT";
            break;
        case 307:
            s = "ERROR_INVALID_LOCK_RANGE";
            break;
        case 308:
            s = "ERROR_IMAGE_SUBSYSTEM_NOT_PRESENT";
            break;
        case 309:
            s = "ERROR_NOTIFICATION_GUID_ALREADY_DEFINED";
            break;
        case 310:
            s = "ERROR_INVALID_EXCEPTION_HANDLER";
            break;
        case 311:
            s = "ERROR_DUPLICATE_PRIVILEGES";
            break;
        case 312:
            s = "ERROR_NO_RANGES_PROCESSED";
            break;
        case 313:
            s = "ERROR_NOT_ALLOWED_ON_SYSTEM_FILE";
            break;
        case 314:
            s = "ERROR_DISK_RESOURCES_EXHAUSTED";
            break;
        case 315:
            s = "ERROR_INVALID_TOKEN";
            break;
        case 316:
            s = "ERROR_DEVICE_FEATURE_NOT_SUPPORTED";
            break;
        case 317:
            s = "ERROR_MR_MID_NOT_FOUND";
            break;
        case 318:
            s = "ERROR_SCOPE_NOT_FOUND";
            break;
        case 319:
            s = "ERROR_UNDEFINED_SCOPE";
            break;
        case 320:
            s = "ERROR_INVALID_CAP";
            break;
        case 321:
            s = "ERROR_DEVICE_UNREACHABLE";
            break;
        case 322:
            s = "ERROR_DEVICE_NO_RESOURCES";
            break;
        case 323:
            s = "ERROR_DATA_CHECKSUM_ERROR";
            break;
        case 324:
            s = "ERROR_INTERMIXED_KERNEL_EA_OPERATION";
            break;
        case 326:
            s = "ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED";
            break;
        case 327:
            s = "ERROR_OFFSET_ALIGNMENT_VIOLATION";
            break;
        case 328:
            s = "ERROR_INVALID_FIELD_IN_PARAMETER_LIST";
            break;
        case 329:
            s = "ERROR_OPERATION_IN_PROGRESS";
            break;
        case 330:
            s = "ERROR_BAD_DEVICE_PATH";
            break;
        case 331:
            s = "ERROR_TOO_MANY_DESCRIPTORS";
            break;
        case 332:
            s = "ERROR_SCRUB_DATA_DISABLED";
            break;
        case 333:
            s = "ERROR_NOT_REDUNDANT_STORAGE";
            break;
        case 334:
            s = "ERROR_RESIDENT_FILE_NOT_SUPPORTED";
            break;
        case 335:
            s = "ERROR_COMPRESSED_FILE_NOT_SUPPORTED";
            break;
        case 336:
            s = "ERROR_DIRECTORY_NOT_SUPPORTED";
            break;
        case 337:
            s = "ERROR_NOT_READ_FROM_COPY";
            break;
        case 350:
            s = "ERROR_FAIL_NOACTION_REBOOT";
            break;
        case 351:
            s = "ERROR_FAIL_SHUTDOWN";
            break;
        case 352:
            s = "ERROR_FAIL_RESTART";
            break;
        case 353:
            s = "ERROR_MAX_SESSIONS_REACHED";
            break;
        case 400:
            s = "ERROR_THREAD_MODE_ALREADY_BACKGROUND";
            break;
        case 401:
            s = "ERROR_THREAD_MODE_NOT_BACKGROUND";
            break;
        case 402:
            s = "ERROR_PROCESS_MODE_ALREADY_BACKGROUND";
            break;
        case 403:
            s = "ERROR_PROCESS_MODE_NOT_BACKGROUND";
            break;
        case 487:
            s = "ERROR_INVALID_ADDRESS";
            break;
    }
    snprintf(s_debug_buffer, 1024, "%s error %d: %s\n", s_prompt, dw, s);
    OutputDebugString(s_debug_buffer);
}

