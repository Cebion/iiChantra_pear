#include "StdAfx.h"
#include "animation.h"

#include "../misc.h"

AnimationFrame::AnimationFrame(const AnimationFrame& src) 
	: duration(src.duration), num(src.num), command(src.command), param(src.param),
	txt_param(StrDupl(src.txt_param))
{
}

AnimationFrame& AnimationFrame::operator=(const AnimationFrame& src)
{
	duration = src.duration;
	num = src.num;
	param = src.param;
	command = src.command;
	DELETEARRAY(txt_param);
	txt_param = StrDupl(src.txt_param);
	return *this;
}

Animation::Animation(const Animation& src)
	: name(StrDupl(src.name)), frameCount(src.frameCount),  
	current(MAX_ANIMFRAMES_COUNT)
{
	frames = new AnimationFrame[frameCount];
	for (size_t i = 0; i < frameCount; i++)
	{
		frames[i] = src.frames[i];
	}
}

Animation& Animation::operator=(const Animation& src)
{
	current = src.current;
	frameCount = src.frameCount;
	DELETEARRAY(name);
	name = StrDupl(src.name);
	if ( frames )
		DELETEARRAY(frames);
	frames = new AnimationFrame[frameCount];
	for (size_t i = 0; i < frameCount; i++)
	{
		frames[i] = src.frames[i];
	}
	return *this;
}
