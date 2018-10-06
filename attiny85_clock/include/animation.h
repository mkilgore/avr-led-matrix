#ifndef INCLUDE_ANIMATION_H
#define INCLUDE_ANIMATION_H

#include "digits.h"

void animate_frame(struct disp_matrix *, struct disp_buf *start, struct disp_buf *end, const struct disp_loc *, int frame);

void animate_digit_switch(struct disp_matrix *, int old_digit, int new_digit, const struct disp_loc *, int frame);

#endif
