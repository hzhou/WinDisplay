# WinDisplay

A playground for graphics programming.

At this point, it is a font rasterizer (a freetype alternative).

```
subcode: page_content
    $(if:1)
        page.$set_size 100
        page.$moveto 10, 680
        page.$text_line "abcdefghijklmn"
        page.$text_line "opqrstuvwxyz"
        page.$text_line "ABCDEFGH"
        page.$text_line "IJKLMNOPQ"
        page.$text_line "RSTUVWXYZ"
        page.$set_size 16
        page.$text_line "abcdefghijklmn"
        page.$text_line "opqrstuvwxyz"
        page.$text_line "ABCDEFGH"
        page.$text_line "IJKLMNOPQ"
        page.$text_line "RSTUVWXYZ"
    $(else)
        page.$set_size 100
        page.$moveto 10, 10
        page.$text "a"

```

![windisplay.png](extra/windisplay.png)
