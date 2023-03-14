#ifndef __ANIMATION_H_
#define  __ANIMATION_H_


#define MAX_ANIMFRAMES_COUNT USHRT_MAX
#define MAX_ANIMATIONS_COUNT SIZE_MAX

#include "animation_frame.h"


class Animation
{
public:
	char* name;

	USHORT frameCount;			// ���������� ������ � ��������
	USHORT current;				// ������� ����.

	AnimationFrame* frames;

	Animation()
	{
		frameCount = 0;
		current = MAX_ANIMFRAMES_COUNT; //������� ������ ���������, ��� ������� ������ ������ �� �����������. �������� � ������ SetAnim.
		frames = NULL;
		name = NULL;
	}

	Animation(const Animation& src);
	Animation& operator=(const Animation& src);

	// ����������� ���������� ������ ��� ������ ��������, ���������� � ���������.
	// �����, ���������� � �������� ���������� � ������� malloc/free, ��� ��� ��� 
	// ��� ���������� �� ����������.
	~Animation()
	{
		
		DELETEARRAY(frames);
		DELETEARRAY(name);
	}
};




#endif // __ANIMATION_H_
