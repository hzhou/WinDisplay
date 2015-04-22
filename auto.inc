S string, font, ttf_head, ttf_hhea, raster_point
struct glyph{
    unsigned char * buf;
    int x;
    int y;
    int w;
    int h;
    float advance;
};
F void font_set_size(struct font * pf, int n_pt_size, float f_pt_size)
F struct glyph * font_render(struct font * pf, int c)
F struct font * load_font(char * s_fontfile)
F void raster_start()
F void raster_new_contour()
F void raster_curve(float tf_x0, float tf_y0, float tf_x1, float tf_y1, float tf_x2, float tf_y2)
F void raster_line(float tf_x0, float tf_y0, float tf_x1, float tf_y1)
F void raster_fill(unsigned char * buf, int n_w)
F void raster_end_contour()
F void raster_stroke(unsigned char * buf, int n_w)
