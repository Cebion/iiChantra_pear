#ifndef _PARTICLE_SYSTEM_H_
#define _PARTICLE_SYSTEM_H_

#include "../script/script.h"

#include "phys/phys_misc.h"
#include "sprite.h"

#include "proto.h"
#include "../render/renderer.h"

enum TrajectoryType { pttLine, pttRipple, pttRandom, pttSine, pttCosine, pttPseudoDepth, pttTwist, pttOrbit, pttGlobalSine };

void SetParticlesWind( Vector2 new_wind );
Vector2 GetParticlesWind();

struct CParticle
{
	Vector2			p;			//	������� �������
	Vector2			old_p;		//  ������ ������� �������, ����� ��� �����
	Vector2			v;			//	�������� 
	RGBAf			c;			//	������� ����
	RGBAf			dc;			//	���������� ����� �� ������ ����
	float			size;		//	������� ������
	float			dsize;		//	���������� �������
	float			life;		//	����� ����� �������; -1 ������ �������������
	float			age;		//	������� �������, �� ���� ������� ��� ��� ���������������
	float			parameter;	//  ��������� ��� ����������� ������� ����������.
	float			trace;		//  ����� "������" �������
	bool			is_on_plane;
};

struct CPsysteminfo
{
	Vector2			p;						//	������� ������� ������

	RGBAf			sc;						//	��������� ���� ������ �������
	RGBAf			ec;						//	�������� ���� ������ �������
	RGBAf			vc;						//	������ ����������� ��� ������ �����

	int				MaxParticles;			//	����������� �������� ����� ������
	int				ParticlesActive;		//	������� ������ ������� ������
	int				lifemin;				//	����������� ����� ����� �������
	int				lifemax;				//	������������ ����� ����� �������

	float			startsize;				//	��������� ������ ������ �������
	float			endsize;				//	�������� ������
	float			sizevar;				//	������ ����������� ��� ������ ������� �������

	float			plife;					//	���������������
	int				plifevar;				//	���������������

	float			life;					//	����� ����� �������; -1 ������ ������������
	float			age;					//	������� ������� �������

	int				emission;				//	������, ������������ ������� ������ ������� �� ���
	int				notcreated;				//	���������� ������, ������� �� �� ������ ��������� � ���� ����� �� �����-�� ��������
	Vector2			*geom;					//	������ �����, ��� ��������� ������
	int				GeomNumPoints;			//	����� ���� �����

	float					max_speed;		// ������������ ��������� �������� �������
	float					min_speed;		// ����������� ��������� �������� �������
	float					max_angle;		// ������������ ���� ������ (��� �������� ������)
	float					min_angle;		// ����������� ���� ������ (��� �������� ������)
	float					min_param;		// ����������� ��������� �������� ���������� �������
	float					max_param;		// ������������ ��������� �������� ��������� �������
	float					min_trace;		// ���������� ����� "������" �������.
	float					max_trace;		// ������������ ����� "������" �������.
	Vector2					gravity;		// ����������, ����������� �� �������.
	bool					physical;		// ���� ����� �� ������������
	bool					wind_affected;	// ���������� �� ������� ����������� �����
	TrajectoryType	trajectory;		// ��� ��������� ������
	float					t_param1;		// �������� ���������� 1
	float					t_param2;		// �������� ���������� 2
	bool					fancy_blending; // ���������� ��������
};

typedef void (*FCreateFunc)(CParticle *);
typedef void (*FUpdateFunc)(CParticle *, float);

/**
*	TODO:
*		1. �������� ������������ �������, ������� ��������
*		2. �������� ��������� ���������� ������� ��� �������
*		3. �������� ��� ���-������
*/


class CParticleSystem
{
public:
	CPsysteminfo			Info;
	CParticle*				particles;
	const Texture*			texture;
	UINT					emitter;
	int						mode;
	//CRenderObject*			UserRenderSample;

	vector<CAABB>*				area;
	vector<CAABB>*				drop_area;

	float					z;

	bool					dead;

	CParticleSystem();
	~CParticleSystem();
	void					Init();	
	bool					Update();
	bool					Render();

	CParticle*				CreateParticle();
	void					SetGeometry(Vector2 * points, int numPoints);

	bool					SaveToFile();
	bool					LoadFromFile();

protected:
	void					_swap(int i, int j);
};

#endif //_PARTICLE_SYSTEM_H_
