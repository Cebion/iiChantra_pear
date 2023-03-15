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
	Vector2			p;			//	позиция частицы
	Vector2			old_p;		//  старая позиция частицы, нужна для следа
	Vector2			v;			//	скорость 
	RGBAf			c;			//	текущий цвет
	RGBAf			dc;			//	приращение цвета на каждом шаге
	float			size;		//	текущий размер
	float			dsize;		//	приращение размера
	float			life;		//	время жизни частицы; -1 значит бесконечность
	float			age;		//	возраст частицы, то есть сколько она уже просуществовала
	float			parameter;	//  Необходим для поддержания сложных траекторий.
	float			trace;		//  Длина "хвоста" частицы
	bool			is_on_plane;
};

struct CPsysteminfo
{
	Vector2			p;						//	позиция системы частиц

	RGBAf			sc;						//	начальный цвет каждой частицы
	RGBAf			ec;						//	конечный цвет каждой частицы
	RGBAf			vc;						//	фактор случайности при выборе цвета

	int				MaxParticles;			//	максимально возможно число частиц
	int				ParticlesActive;		//	сколько частиц активно сейчас
	int				lifemin;				//	минимальное время жизни частицы
	int				lifemax;				//	максимальное время жизни частицы

	float			startsize;				//	начальный размер каждой частицы
	float			endsize;				//	конечный размер
	float			sizevar;				//	фактор случайности при выборе размера частицы

	float			plife;					//	авотхуйегознает
	int				plifevar;				//	авотхуйегознает

	float			life;					//	время жизни системы; -1 значит бесонечность
	float			age;					//	текущий возраст системы

	int				emission;				//	фактор, определяющий сколько частиц вылетит за раз
	int				notcreated;				//	количество частиц, которые мы не смогли выпустить в этом кадре по каким-то причинам
	Vector2			*geom;					//	массив точек, для генерации частиц
	int				GeomNumPoints;			//	число этих точек

	float					max_speed;		// Максимальная стартовая скорость частицы
	float					min_speed;		// Минимальная стартовая скорость частицы
	float					max_angle;		// Максимальный угол вылета (для точечных систем)
	float					min_angle;		// Минимальный угол вылета (для точечных систем)
	float					min_param;		// Минимальный стартовый параметр траектории частицы
	float					max_param;		// Максимальный стартовый параметр трактории частицы
	float					min_trace;		// Минмальная длина "хвоста" частицы.
	float					max_trace;		// Максимальная длина "хвоста" частицы.
	Vector2					gravity;		// Гравитация, действующая на частицы.
	bool					physical;		// Пока никак не используется
	bool					wind_affected;	// Подвержены ли частицы воздействию ветра
	TrajectoryType	trajectory;		// Тип поведения частиц
	float					t_param1;		// Параметр траектории 1
	float					t_param2;		// Параметр траектории 2
	bool					fancy_blending; // Аддитивный блендинг
};

typedef void (*FCreateFunc)(CParticle *);
typedef void (*FUpdateFunc)(CParticle *, float);

/**
*	TODO:
*		1. Добавить интерполяцию покруче, окоромя линейной
*		2. Добавить установку экзмепляра объекта для рендера
*		3. Добавить ещё что-нибуть
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
