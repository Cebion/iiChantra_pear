#include "StdAfx.h"
#include "particle_system.h"
#include "../misc.h"
#include "phys/phys_misc.h"
#include "objects/object.h"
#include "objects/object_physic.h"
#include "objects/object_player.h"
#include "objects/object_dynamic.h"
#include "player.h"
#include "camera.h"

extern GameObject* attached_object;

/*
Режимы:
1 - создание по всему aabb объекта.
2 - применять гравитацию.
3 - "снег", создание по отрезку ширины стартовой позиции объекта вниз.
*/

Vector2 wind;

void SetParticlesWind( Vector2 new_wind )
{
	wind = new_wind;
}

Vector2 GetParticlesWind()
{
	return wind;
}

void CParticleSystem::_swap(int i, int j)
{
	CParticle t		= particles[i];
	particles[i]	= particles[j];
	particles[j]	= t;
}


void CParticleSystem::Init()
{
	memset(&Info, 0, sizeof(Info));

	this->Info.ParticlesActive = 0;
	this->Info.MaxParticles = 10000;
	this->particles = new CParticle [Info.MaxParticles];
	Info.emission = 500;
	Info.age = 0;
	Info.sizevar = 1;
	Info.plifevar = 1;
	Info.life = -1;
	Info.plife = 1;
	Info.trajectory = pttLine;
	Info.min_speed = 1;
	Info.max_speed = 1;
	Info.min_angle = 0;
	Info.max_angle = 360;
	Info.fancy_blending = false;

	texture = NULL;
	//TextureByName(name);

	Info.sc = RGBAf(1.0,0.0,0.0,0.5);
	Info.ec = RGBAf(0.0,0.0,1.0,0.5);
	Info.vc = RGBAf(0.0f,0.0f,0.0f,0.0f);
}

bool AreaMovement( Vector2& coord, Vector2 vel, vector<CAABB>* area )
{
#ifndef _DEBUG
	if ( area == NULL || area->size() == 0 )
#endif
	{
		coord += vel;
		return true;
	}
	Vector2 pnt = coord + vel;
	for ( vector<CAABB>::iterator it = area->begin(); it != area->end(); it++ )
	{
		if ( it->PointInCAABB(pnt.x, pnt.y) )
		{
			coord += vel;
			return true;
		}
	}
	return false;
}

bool CParticleSystem::Update()
{
#ifdef _DEBUG_DISABLE_PARTICLES_UPDATE
	return false;
#endif
	// Here integrating and updating values of active particles
	for(int i=0; i < Info.ParticlesActive; i++)
	{
		
		particles[i].age += 0.2f;
		if (particles[i].age >= particles[i].life)
		{
			_swap(i, Info.ParticlesActive-1);
			Info.ParticlesActive--;
			i--;
			continue;
		}
		particles[i].v += Info.gravity;
		particles[i].old_p = particles[i].p;

#ifndef _DEBUG
		if ( drop_area != NULL && !particles[i].is_on_plane && drop_area->size() > 0 )
#else
		if ( false )
#endif
		{
			for ( vector<CAABB>::iterator it = drop_area->begin(); it != drop_area->end(); it++ )
			{
				if ( it->PointInCAABB(particles[i].p.x, particles[i].p.y) )
				{
					if ( rand() % 100 > 90 )
						particles[i].is_on_plane = true;
					break;
				}
			}
		}

		if ( particles[i].is_on_plane )
		{
			particles[i].c += (particles[i].dc*0.2f);
			particles[i].size += particles[i].dsize;
			continue;
		}

		switch ( Info.trajectory )
		{
			case pttRipple:
				{
					particles[i].parameter += 0.2f;
					float tr_x = Info.t_param1*(-particles[i].v.x / particles[i].v.Length())*sin(particles[i].parameter*Info.t_param2);
					float tr_y = Info.t_param1*(particles[i].v.y / particles[i].v.Length())*sin(particles[i].parameter*Info.t_param2);
					if (!AreaMovement( particles[i].p, (particles[i].v + Vector2( tr_x, tr_y ))*0.2f, area ))
						AreaMovement( particles[i].p, Info.gravity, area );
					particles[i].c += (particles[i].dc*0.2f);
					particles[i].size += particles[i].dsize;
					break;
				}
			case pttTwist:
				{
					Vector2 center_point = particles[i].p;
					Vector2 old_center_point = center_point;
					if ( !dead ) 
						center_point = old_center_point = Info.p;
					if ( mode & 2 )
					{
						GameObject* go = GetGameObject( emitter );
						if ( go )
						{
							center_point = go->aabb.p;
							if ( go->IsPhysic() && ((ObjPhysic*)go)->IsDynamic() )
							old_center_point = center_point - ((ObjDynamic*)go)->vel;
						}
					}
					particles[i].parameter += 0.2f*Info.t_param1;
					if (!AreaMovement( particles[i].p, center_point + (0.2f*particles[i].v.Length() + (particles[i].p - center_point).Length()) * Vector2( cos( particles[i].parameter ), sin( particles[i].parameter) ) - particles[i].p, area))
						AreaMovement( particles[i].p, Info.gravity, area );
					particles[i].c += (particles[i].dc*0.2f);
					particles[i].size += particles[i].dsize;
					break;
				}
			case pttSine:
				{
					particles[i].parameter += 0.2f;
					Vector2 tr = particles[i].v.GetPerp().Normalized() * Info.t_param1 * sin(particles[i].parameter*Info.t_param2);
					if (!AreaMovement( particles[i].p, (particles[i].v + tr)*0.2f, area ))
						AreaMovement( particles[i].p, Info.gravity, area );
					particles[i].c += (particles[i].dc*0.2f);
					particles[i].size += particles[i].dsize;
					break;
				}
			case pttCosine:
				{
					particles[i].parameter += 0.2f;
					Vector2 tr = particles[i].v.GetPerp().Normalized() * Info.t_param1 * sin(particles[i].parameter*Info.t_param2);
					if (!AreaMovement( particles[i].p, (particles[i].v + tr)*0.2f, area ))
						AreaMovement( particles[i].p, Info.gravity, area );
					particles[i].c += (particles[i].dc*0.2f);
					particles[i].size += particles[i].dsize;
					break;
				}
			case pttPseudoDepth:
				{
					particles[i].size += particles[i].parameter*0.2f;
					if (particles[i].size > Info.t_param2 || particles[i].size < -Info.t_param2) particles[i].parameter = 0;
					if (!AreaMovement( particles[i].p, particles[i].v*0.2f, area))
						AreaMovement( particles[i].p, Info.gravity, area );
					particles[i].c += (particles[i].dc*0.2f);
					break;
				}
			case pttRandom:
				{
					//Vector2 tr = particles[i].v.GetPerp().Normalized() * Random_Float(-particles[i].parameter, particles[i].parameter);
					Vector2 tr = particles[i].v.GetPerp().Normalized() * Random_Float(-Info.t_param1, Info.t_param1);
					Vector2 tr2 = particles[i].v.GetPerp().GetPerp().Normalized() * Random_Float(-Info.t_param2, Info.t_param2);;
					particles[i].v += tr;
				}
			default:
				{
					if (!AreaMovement( particles[i].p,  particles[i].v*0.2f, area ))
						AreaMovement( particles[i].p, Info.gravity, area );
					particles[i].c += (particles[i].dc*0.2f);
					particles[i].size += particles[i].dsize;
					break;
				}
		}
		if (Info.wind_affected)
		{
			AreaMovement( particles[i].p, wind*0.2f, area );
		}
	}


	float np = Info.emission * 0.2f;	// + uncreated;

	if (Info.life != -1)
	{
		Info.age += 0.2f;
		if (Info.age >= Info.life)
		{
			this->dead = true;
			return true;
		}
	}

	int t = Info.ParticlesActive;
	for (int i = t; i < t + np; i++)
	{
		CreateParticle();
	}
	return true;
}

bool CParticleSystem::Render()
{
#ifdef _DEBUG_DISABLE_PARTICLES_DRAW
	return false;
#endif
	for(int i=0;i<Info.ParticlesActive;i++)
	{
		Vector2 trace = particles[i].p + ((particles[i].old_p - particles[i].p).Normalized() * particles[i].trace);
		float halfsize = particles[i].size * 0.5f;
		if ( trace != particles[i].p )
		{
			RenderLine( particles[i].p.x + halfsize, particles[i].p.y + halfsize,
						trace.x + halfsize, trace.y + halfsize,
						this->z-0.0001f, particles[i].c );
		}
	}
	if (!texture)
	{
		glPushAttrib(GL_TEXTURE_BIT | GL_POINTS);
		glEnable(GL_POINTS);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE);
		glPointSize(particles[0].size);
		glBegin(GL_POINTS);
		for(int i=0;i<Info.ParticlesActive;i++)
		{
			particles[i].c.glSet();
			glVertex3f(particles[i].p.x, particles[i].p.y, this->z);
		}
		glEnd();
		glPopAttrib();
	}
	else
	{
#ifdef RENDER_PARTICLES_WITH_STD_RENDER
		coord2f_t fsize;
		FrameInfo* fi = this->texture->frame;
		ASSERT(fi);
		size_t frameNum = 0;
		for(int i=0;i<Info.ParticlesActive;i++)
		{
			const CParticle& pp = particles[i];
			fsize.x = fsize.y = particles[i].size;
			if (this->texture->framesCount > 1)
			{
				// Выбор кадра анимации
				frameNum = (size_t)floor((pp.age / pp.life) * this->texture->framesCount);
				if (frameNum >= this->texture->framesCount) frameNum = this->texture->framesCount-1;
			}
			RenderSprite(particles[i].p.x, particles[i].p.y, (particles[i].is_on_plane ? SHADOW_Z : z), &fsize, fi[frameNum].coord, texture, particles[i].c);
		}
#else

		glPushAttrib(GL_TEXTURE_BIT | GL_POINTS);
		if ( Info.fancy_blending )
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		glDisable(GL_POINTS);
		glEnable(GL_TEXTURE_2D);
		
		glTranslatef(0.0f, 0.0f, 0.0f);
		glBindTexture(GL_TEXTURE_2D, texture->tex);

		glBegin(GL_QUADS);


		FrameInfo* fi = this->texture->frame;
		ASSERT(fi);
		size_t frameNum = 0;
				
		for(int i=0;i<Info.ParticlesActive;i++)
		{
			const CParticle& pp = particles[i];
			particles[i].c.glSet();
			
			if (this->texture->framesCount > 1)
			{
				// Выбор кадра анимации
				frameNum = (size_t)floor((pp.age / pp.life) * this->texture->framesCount);
				if (frameNum >= this->texture->framesCount) frameNum = this->texture->framesCount-1;
			}

			glTexCoord2fv((const GLfloat*)(fi[frameNum].coord + 0)); glVertex3f(floor(pp.p.x - 0.5f*pp.size + 0.5f),	floor(pp.p.y - 0.5f*pp.size + 0.5f), this->z);
			glTexCoord2fv((const GLfloat*)(fi[frameNum].coord + 1)); glVertex3f(floor(pp.p.x + 0.5f*pp.size + 0.5f),	floor(pp.p.y - 0.5f*pp.size + 0.5f), this->z);
			glTexCoord2fv((const GLfloat*)(fi[frameNum].coord + 2)); glVertex3f(floor(pp.p.x + 0.5f*pp.size + 0.5f),	floor(pp.p.y + 0.5f*pp.size + 0.5f), this->z);
			glTexCoord2fv((const GLfloat*)(fi[frameNum].coord + 3)); glVertex3f(floor(pp.p.x - 0.5f*pp.size + 0.5f),	floor(pp.p.y + 0.5f*pp.size + 0.5f), this->z);
		}
		glEnd();
		glPopAttrib();

		if ( Info.fancy_blending )
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif	// RENDER_PARTICLES_WITH_STD_RENDER
	}


	return true;
}

void CParticleSystem::SetGeometry( Vector2 * points, int numPoints )
{
	Info.geom = points;
	Info.GeomNumPoints = numPoints;
}

extern float CAMERA_TOP;
extern float CAMERA_BOTTOM;
extern float CAMERA_LEFT;
extern float CAMERA_RIGHT;

CParticle	*CParticleSystem::CreateParticle()
{
	if ( dead ) return NULL;

	int t = Info.ParticlesActive;
	int i = t;
	{
		if (i >= Info.MaxParticles)
			return NULL;
		if (Info.ParticlesActive >= Info.MaxParticles)
			return NULL;
		Info.ParticlesActive++;
	
		Vector2 emit_point = Info.p;
		if ( emitter )
		{
			GameObject * emit_obj = GetGameObject( emitter );
			if ( emit_obj )
			{
				if ( mode & 8 )
				{
					emit_point = emit_obj->aabb.p + this->Info.p;
				}
				else
				{
					float dx = (rand()%((int)(emit_obj->aabb.W*100)))/100.0f;
					float dy = (rand()%((int)(emit_obj->aabb.H*100)))/100.0f;
					emit_point = Vector2(emit_obj->aabb.p.x-(emit_obj->aabb.W/2)+dx,
								         emit_obj->aabb.p.y-(emit_obj->aabb.H/2)+dy);
				}
			}
			else
			{
				this->dead = true;
				return NULL;
			}
		}
		if ( mode & 16 )
		{
			ObjDynamic* op = (ObjDynamic*)attached_object;
			float vx = 0;
			float vy = 0;
			if ( op )
			{
				vx += op->vel.x;
				vy += op->vel.y;
			}
			emit_point = Vector2( Random_Float( CAMERA_LEFT+vx-320, CAMERA_RIGHT+vx+320 ), CAMERA_TOP-Info.startsize+Info.sizevar+vy-240 );
			if ( emit_point.x > CAMERA_RIGHT || emit_point.x < CAMERA_LEFT )
				emit_point.y += Random_Float( -100.0f, (CAMERA_BOTTOM - CAMERA_TOP)+100.0f );
			else
				emit_point.y -= Random_Float( 0.0f, 100.0f );
		}
		particles[i].life  = Random_Float(Info.plife ,Info.plife + Info.plifevar);
		if (particles[i].life < 0 )
			particles[i].life = Info.plife;
		particles[i].age = 0;
		particles[i].parameter = Random_Float( Info.min_param, Info.max_param );

		if ( area != NULL && area->size() > 0 )
		{
			bool in_area = false;
			for ( vector<CAABB>::iterator it = area->begin(); it != area->end(); it++ )
			{
				if ( it->PointInCAABB( emit_point.x, emit_point.y ) )
				{
					in_area = true;
					break;
				}
			}
			if (!in_area)
				return NULL;
		}

		if (Info.geom != NULL && Info.GeomNumPoints > 1)
		{
			int sr = Random_Int(0, Info.GeomNumPoints-2);
			particles[i].p = Info.geom[sr] + (Info.geom[sr+1]-Info.geom[sr])*Random_Float(0.0f, 1.0f);
			float angle = PI * Random_Float(Info.min_angle, Info.max_angle) / 180;
			float speed = Random_Float(Info.max_speed, Info.max_speed);
			particles[i].v = ((Info.geom[sr]-Info.geom[sr+1]).GetPerp().Normalized()) * speed;
			particles[i].v.x *= cos(angle);
			particles[i].v.y *= sin(angle);
		}
		else
		{
			particles[i].p = emit_point;
			float angle = PI * Random_Float(Info.min_angle, Info.max_angle) / 180;
			float speed = Random_Float(Info.max_speed, Info.max_speed);
			if ( Info.trajectory == pttPseudoDepth )
			{
				float angle2 = PI * Random_Float( -Info.t_param1, Info.t_param1 ) / 180;
				particles[i].parameter = speed * sin(angle2);
				particles[i].v = Vector2( speed * cos(angle) * cos(angle2), speed * sin(angle) * cos(angle2) );				
			}
			else
			{
				particles[i].v = Vector2( speed * cos(angle), speed*sin(angle) );
			}
		}
		particles[i].old_p = particles[i].p;

		particles[i].size = Random_Float(Info.startsize, Info.startsize + Info.sizevar);
		particles[i].dsize = (Info.endsize - Info.startsize) / particles[i].life;

		particles[i].c.a = Random_Float(Info.sc.a ,Info.sc.a + Info.vc.a);
		particles[i].c.r = Random_Float(Info.sc.r ,Info.sc.r + Info.vc.r);
		particles[i].c.g = Random_Float(Info.sc.g ,Info.sc.g + Info.vc.g);
		particles[i].c.b = Random_Float(Info.sc.b ,Info.sc.b + Info.vc.b);

		particles[i].dc = (Info.ec - particles[i].c)/particles[i].life;
		particles[i].trace = Random_Float(Info.min_trace, Info.max_trace);
		particles[i].is_on_plane = false;
}
	return &particles[i];
}

CParticleSystem::CParticleSystem()
{
	texture = NULL;
	particles = NULL;
	emitter = 0;
	dead = false;
	area = drop_area = NULL;
	memset(&this->Info, 0, sizeof(CPsysteminfo));
}


CParticleSystem::~CParticleSystem()
{
	if ( area )	DELETESINGLE(area);
	if ( drop_area ) DELETESINGLE(drop_area);
	DELETEARRAY(particles);
	DELETEARRAY(this->Info.geom);
}
