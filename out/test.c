#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "gdi32")
#pragma comment(lib, "user32")
/* link: -lgdi32 -luser32  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define snprintf sprintf_s

struct display_buffer {
    unsigned char * buffer;
    int width;
    int height;
    int offset_x;
    int offset_y;
    float pt;
    float x;
    float y;
    struct font * font;
    int size;
    float matrix[6];
};

struct glyph {
    unsigned char * buf;
    int x;
    int y;
    int w;
    int h;
    float advance;
};

LRESULT CALLBACK WndProc_main(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
struct font * load_font(char * s_fontfile);
void font_set_size(struct font * pf, int n_pt_size, float f_pt_size);
void f_disp_set_page(struct display_buffer * p_disp, int tn_w, int tn_h);
void f_disp_add_text(struct display_buffer * p_disp, char * s);
void f_disp_resize(struct display_buffer * p_disp, int tn_w, int tn_h);
struct glyph * font_render(struct font * pf, int c);
void f_disp_copy_gray(struct display_buffer * p_disp, int tn_x, int tn_y, unsigned char* pc_buf, int tn_w, int tn_h);

HINSTANCE cur_instance;
HWND hwnd_main;
char s_debug_buffer[1024];
struct display_buffer page;
struct display_buffer editor;

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR s_cmdline, int n_cmdshow){
    WNDCLASSEX wc;
    MSG msg;

    cur_instance=hInst;
    snprintf(s_debug_buffer, 1024, "main_init\n");
    OutputDebugString(s_debug_buffer);
    memset(&page, 0, sizeof(page));
    page.buffer = NULL;
    page.pt = 1.0;
    memset(&editor, 0, sizeof(editor));
    editor.buffer = NULL;
    editor.pt = 1.0;
    page.font = load_font("cyberbit.ttf");
    page.size = 100;
    if(page.font && page.size > 0){
        font_set_size(page.font, page.size, page.pt);
    }
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = cur_instance;
    wc.cbClsExtra = 0;
    wc.style = 0;
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WinDisplay";
    wc.lpfnWndProc = WndProc_main;
    wc.cbWndExtra = 0;
    RegisterClassEx(&wc);
    hwnd_main = CreateWindowEx(0, "WinDisplay", "WinDisplay", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 1000, NULL, NULL, cur_instance, NULL);

    ShowWindow(hwnd_main, n_cmdshow);
    UpdateWindow(hwnd_main);

    while(GetMessage(&msg, NULL, 0, 0) > 0){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WndProc_main(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    CREATESTRUCT * p_create;
    int tn_w;
    int tn_h;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect_client;
    HBRUSH temp_brush;
    BITMAPINFO bi;
    float tf_save_x;
    uint32_t* pu4_pixel;
    int k;

    switch(msg){
        case WM_CREATE:
            p_create = (CREATESTRUCT*)lparam;
            tn_w = p_create->cx;
            tn_h = p_create->cy;
            break;
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect_client);
            temp_brush = CreateSolidBrush(0x808080);
            FillRect(hdc, &rect_client, temp_brush);
            snprintf(s_debug_buffer, 1024, "on_draw page: %d x %d\n", page.width, page.height);
            OutputDebugString(s_debug_buffer);
            memset(&bi, 0, sizeof(bi));
            bi.bmiHeader.biSize = 40;
            bi.bmiHeader.biWidth = page.width;
            bi.bmiHeader.biHeight = page.height;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;
            StretchDIBits(hdc, page.offset_x, page.offset_y, page.width, page.height, 0, 0, page.width, page.height, page.buffer, &bi, DIB_RGB_COLORS, SRCCOPY);
            memset(&bi, 0, sizeof(bi));
            bi.bmiHeader.biSize = 40;
            bi.bmiHeader.biWidth = editor.width;
            bi.bmiHeader.biHeight = editor.height;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;
            StretchDIBits(hdc, editor.offset_x, editor.offset_y, editor.width, editor.height, 0, 0, editor.width, editor.height, editor.buffer, &bi, DIB_RGB_COLORS, SRCCOPY);
            EndPaint(hwnd, &ps);
            return 0;
            break;
        case WM_SIZE:
            if(wparam == SIZE_MAXIMIZED || wparam == SIZE_RESTORED){
                int i;
                int j;
                tn_w = LOWORD(lparam);
                tn_h = HIWORD(lparam);
                snprintf(s_debug_buffer, 1024, "on_size: %d x %d\n", tn_w, tn_h);
                OutputDebugString(s_debug_buffer);
                f_disp_set_page(&page, tn_w, tn_h - 45);
                page.size = 100;
                if(page.font && page.size > 0){
                    font_set_size(page.font, page.size, page.pt);
                }
                page.x = 10;
                page.y = 680;
                tf_save_x = page.x;
                f_disp_add_text(&page, "abcdefghijklmn");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "opqrstuvwxyz");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "ABCDEFGH");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "IJKLMNOPQ");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "RSTUVWXYZ");
                page.y -= page.size;
                page.x = tf_save_x;
                page.size = 16;
                if(page.font && page.size > 0){
                    font_set_size(page.font, page.size, page.pt);
                }
                tf_save_x = page.x;
                f_disp_add_text(&page, "abcdefghijklmn");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "opqrstuvwxyz");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "ABCDEFGH");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "IJKLMNOPQ");
                page.y -= page.size;
                page.x = tf_save_x;
                tf_save_x = page.x;
                f_disp_add_text(&page, "RSTUVWXYZ");
                page.y -= page.size;
                page.x = tf_save_x;
                f_disp_resize(&editor, tn_w, 45);
                editor.offset_x = 0;
                editor.offset_y = tn_h - 45;
                pu4_pixel = (uint32_t*)editor.buffer;
                tn_w = editor.width - 0 * 2;
                tn_h = editor.height - (0 + 5) * 2;
                for(i=0; i<5; i++){
                    int j;
                    k = (0 + i) * editor.width + 0;
                    for(j=0; j<tn_w; j++){
                        pu4_pixel[k] = 0xa0a0a0;
                        k++;
                    }
                    k = (editor.height - 0 - i - 1) * editor.width + 0;
                    for(j=0; j<tn_w; j++){
                        pu4_pixel[k] = 0xa0a0a0;
                        k++;
                    }
                }
                for(j=0; j<5; j++){
                    int i;
                    k = (0 + 5) * editor.width + 0 + j;
                    for(i=0; i<tn_h; i++){
                        pu4_pixel[k] = 0xa0a0a0;
                        k += editor.width;
                    }
                    k = (0 + 5) * editor.width + 0 + tn_w - j - 1;
                    for(i=0; i<tn_h; i++){
                        pu4_pixel[k] = 0xa0a0a0;
                        k += editor.width;
                    }
                }
                pu4_pixel = (uint32_t*)editor.buffer;
                tn_w = editor.width - 7 * 2;
                tn_h = editor.height - (7 + 2) * 2;
                for(i=0; i<2; i++){
                    int j;
                    k = (7 + i) * editor.width + 7;
                    for(j=0; j<tn_w; j++){
                        pu4_pixel[k] = 0x000000;
                        k++;
                    }
                    k = (editor.height - 7 - i - 1) * editor.width + 7;
                    for(j=0; j<tn_w; j++){
                        pu4_pixel[k] = 0x000000;
                        k++;
                    }
                }
                for(j=0; j<2; j++){
                    int i;
                    k = (7 + 2) * editor.width + 7 + j;
                    for(i=0; i<tn_h; i++){
                        pu4_pixel[k] = 0x000000;
                        k += editor.width;
                    }
                    k = (7 + 2) * editor.width + 7 + tn_w - j - 1;
                    for(i=0; i<tn_h; i++){
                        pu4_pixel[k] = 0x000000;
                        k += editor.width;
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 1;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
    DeleteObject(temp_brush);
}

void f_disp_set_page(struct display_buffer * p_disp, int tn_w, int tn_h){
    float tf_aspect;
    int tn_margin;
    int tn_w2;
    int tn_h2;

    tf_aspect = (float)(8.5 / 11);
    tn_margin = 20;
    tn_w2 = tn_w - tn_margin;
    tn_h2 = tn_h - tn_margin;
    if(tn_h2 * tf_aspect > tn_w2){
        tn_h2 = (int)(tn_w2 / tf_aspect);
    }
    else{
        tn_w2 = (int)(tn_h2 * tf_aspect);
    }
    f_disp_resize(p_disp, tn_w2, tn_h2);
    p_disp->pt = tn_w2 / (float)(8.5 * 72);
    p_disp->offset_x = (tn_w - tn_w2) / 2;
    p_disp->offset_y = (tn_h - tn_h2) / 2;
}

void f_disp_add_text(struct display_buffer * p_disp, char * s){
    struct glyph * p_glyf;
    int tn_x;
    int tn_y;

    while(*s){
        p_glyf = font_render(p_disp->font, *s);
        tn_x = (int)(p_disp->x * p_disp->pt);
        tn_y = (int)(p_disp->y * p_disp->pt);
        f_disp_copy_gray(p_disp, tn_x - p_glyf->x, tn_y - p_glyf->y, p_glyf->buf, p_glyf->w, p_glyf->h);
        p_disp->x += p_glyf->advance / p_disp->pt;
        s++;
    }
}

void f_disp_resize(struct display_buffer * p_disp, int tn_w, int tn_h){
    int tn_size;

    if(p_disp->buffer){
        free(p_disp->buffer);
    }
    p_disp->width = tn_w;
    p_disp->height = tn_h;
    tn_size = tn_w * tn_h * 4;
    p_disp->buffer = malloc(tn_size);
    memset(p_disp->buffer, 255, tn_size);
    if(p_disp->font && p_disp->size > 0){
        font_set_size(p_disp->font, p_disp->size, p_disp->pt);
    }
}

void f_disp_copy_gray(struct display_buffer * p_disp, int tn_x, int tn_y, unsigned char* pc_buf, int tn_w, int tn_h){
    int tn_width;
    int n;
    int m;
    unsigned char c;

    int i;
    tn_width = 0;
    if(tn_y + tn_h > p_disp->height){
        tn_h = p_disp->height - tn_y;
    }
    if(tn_x + tn_w > p_disp->width){
        tn_width = tn_w;
        tn_w = p_disp->width - tn_x;
    }
    n = 0;
    for(i=0; i<tn_h; i++){
        int j;
        m = ((tn_y + i) * p_disp->width + tn_x) * 4;
        if(tn_width){
            n = i * tn_width;
        }
        for(j=0; j<tn_w; j++){
            if(pc_buf[n]){
                c = 255 - pc_buf[n];
                p_disp->buffer[m] = c;
                p_disp->buffer[m+1] = c;
                p_disp->buffer[m+2] = c;
            }
            m += 4;
            n++;
        }
    }
}

