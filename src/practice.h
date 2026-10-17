#ifndef PRACTICE_H
#define PRACTICE_H

void practice_render(void);
void check_save_state(void);

struct buttonRender {
	u16 button;
	u32 color[3];
	s16 xPos;
	s16 yPos;
};

#endif
