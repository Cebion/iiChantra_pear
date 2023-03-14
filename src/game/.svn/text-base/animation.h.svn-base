#ifndef __ANIMATION_H_
#define  __ANIMATION_H_


#define MAX_ANIMFRAMES_COUNT USHRT_MAX
#define MAX_ANIMATIONS_COUNT SIZE_MAX

#include "animation_frame.h"


class Animation
{
public:
	char* name;

	USHORT frameCount;			// Количество кадров в анимации
	USHORT current;				// Текущий кадр.

	AnimationFrame* frames;

	Animation()
	{
		frameCount = 0;
		current = MAX_ANIMFRAMES_COUNT; //Остаётся только надеяться, что столько кадров никому не понадобится. Особенно с учётом SetAnim.
		frames = NULL;
		name = NULL;
	}

	Animation(const Animation& src);
	Animation& operator=(const Animation& src);

	// Деструкторы вызываются только для первых анимаций, хранящихся в прототипе.
	// Копии, хранящиеся в спрайтах выделяются с помощью malloc/free, так что для 
	// них деструктор не вызывается.
	~Animation()
	{
		
		DELETEARRAY(frames);
		DELETEARRAY(name);
	}
};




#endif // __ANIMATION_H_
