#ifndef __SPRITE_H_
#define __SPRITE_H_

#include "animation.h"
#include "../render/types.h"
#include "../render/texture.h"
#include "phys/phys_misc.h"

#include "proto.h"

typedef map<string, size_t> AnimationMap;

// ������ ���������� � �����.
// � ����� �������� ����� ����� ���� ����� ����� ��� ������.
typedef union
{
	int intData;
	const char* stringData;
} StackData;

enum StackElementType
{
	stInt, stString, stNone, stIntOrNone
};

// ������� �����
struct StackElement
{
	StackElement* next;
	StackData data;
	StackElementType type;

	StackElement()
	{
		next = NULL;
		type = stInt;
		data.intData = 0;
	}

	~StackElement()
	{
		if ( this->type == stString )
			DELETEARRAY(this->data.stringData);
	}
};

struct ParametersStack
{
	StackElement* top;
	bool locked;

	ParametersStack()
	{
		top = NULL;
		locked = false;
	}

	~ParametersStack()
	{
		locked = true;
		while ( top )
		{
			StackElement* next = top->next;
			DELETESINGLE(top);
			top = next;
		}
	}


	bool CheckParamTypes(int num, StackElementType first, ...);
	bool isEmpty();
	int PopInt();
	int GetInt(int depth = 0);
	const char* PopString();
	std::string PopStdString();
	const char* GetString(int depth = 0);
	void Push(const char* str);
	void Push(int num);
	StackElement* Pop();
	void DumpToLog();
};

//////////////////////////////////////////////////////////////////////////

struct Sprite
{
	const Texture* tex;

	RGBAf color;

	UINT overlayCount;
	UINT* overlayUse;
	RGBAf* ocolor;

	bool render_without_texture;

	// ��������, � �������� ����� ������������� ������.
	USHORT frameX;
	USHORT frameY;
	// ������ � ������ ����� ��������
	USHORT frameWidth;
	USHORT frameHeight;
	// ������ � ������ ����� ������� ������� ��������
	USHORT imageWidth;
	USHORT imageHeight;
	// ���������� "����� ���������"
	USHORT mpCount;
	// ���� "����� ���������"
	Vector2* mp;

	// ���������� ������ �������� ���� ������������ ��������������
	short realX;
	short realY;

	// ������ � ������ ������������ ��������������
	USHORT realWidth;
	USHORT realHeight;


	UINT currentFrame;				// ����� �������� ����� � ��������

	Animation* anims;
	size_t animsCount;
	AnimationMap const * animNames;

	size_t cur_anim_num;
	string cur_anim;				// ��� ������� ��������


	ParametersStack *stack;			// ���� ���������� ��� ������ ��������.
	UINT prevAnimTick;				// ���������� ���, �� ����� �������� �������� ����
	int offset;					// ��������� ������ �� ������ ����� (��� �������������� ���������� ��������)

	float z;

	char* proto_name;
	
	RenderSpriteMethod renderMethod;

	// �����
	// dead			1		������ �����, �� ������������
	// mirrored		2		������ ����������
	// fixed		4		������������ ���������� ����, � �� ����
	// visible		8		����� ����������
	// -----		16		�� ������������
	// animDone		32		�������� ���������
	BYTE flags;

	__INLINE bool IsDead()		{ return (flags & 1) != 0; }
	__INLINE bool IsMirrored()	{ return (flags & 2) != 0; }
	__INLINE bool IsFixed()		{ return (flags & 4) != 0; }
	__INLINE bool IsVisible()	{ return (flags & 8) != 0; }
	__INLINE bool IsAnimDone()	{ return (flags & 32) != 0; }

	__INLINE void SetDead()			{ flags |= 1; }
	__INLINE void SetMirrored()		{ flags |= 2; }
	__INLINE void SetFixed()		{ flags |= 4; }
	__INLINE void SetVisible()		{ flags |= 8; }
	__INLINE void SetAnimDone()		{ flags |= 32; }

	__INLINE void ClearDead()		{ flags &= ~1; }
	__INLINE void ClearMirrored()	{ flags &= ~2; }
	__INLINE void ClearFixed()		{ flags &= ~4; }
	__INLINE void ClearVisible()	{ flags &= ~8; }
	__INLINE void ClearAnimDone()	{ flags &= ~32; }


	bool SetAnimation(const string& anim_name);
	bool SetAnimation(size_t num);
	bool ChangeFrame(Animation* a);
	bool JumpFrame(int index);				// ������������� ������� ���� ��������

	void SetCurrentFrame(UINT cf);			// ������ ��� currentFrame

	Animation* GetAnimation(string name);
	Animation* GetAnimation(size_t num);
	//void SetLastAnimationFrame();

	void Draw(const CAABB& aabb);

	Sprite()
	{
		Init();
	}

	Sprite(const Proto* proto)
	{
		Init();
		LoadFromProto(proto);
	}

	void LoadFromProto(const Proto* proto);

	void MakeFrames();

	void Init()
	{
		tex = NULL;
		overlayCount = 0;
		overlayUse = NULL;
		ocolor = NULL;
		frameX = 0;
		frameY = 0;

		z = 0;

		frameHeight = 0;
		frameWidth = 0;
		flags = 0;
		imageHeight = 0;
		imageWidth = 0;
		prevAnimTick = 0;
		currentFrame = 0;

		cur_anim = "";
		cur_anim_num = MAX_ANIMATIONS_COUNT;
		animsCount = 0;
		anims = NULL;
		animNames = NULL;

		stack = new ParametersStack();
		mp = NULL;
		mpCount = 0;

		realX = 0;
		realY = 0;
		realWidth = 0;
		realHeight = 0;

		offset = 0;

		color = RGBAf(1.0f, 1.0f, 1.0f, 1.0f);

		render_without_texture = false;
		proto_name = NULL;

		renderMethod = rsmStandart;
	}

	~Sprite();
};

#endif // __SPRITE_H_
