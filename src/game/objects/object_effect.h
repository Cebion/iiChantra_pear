/*		
 		Подвижные спрайты, являются, по сути, необязательными декорациями.
 	имеет смысл оставить их client-side.
 */

#ifndef __OBJECT_EFFECT_H_
#define __OBJECT_EFFECT_H_
#include "object_dynamic.h"
#include "object.h"


class ObjEffect: public ObjDynamic
{
	public:
		USHORT origin_point;
		Vector2 disp;
		bool old_orig_mirror;

		int health;
		RGBAf flash_target;
		int flash_speed;
		int flash_stage;

		virtual void Process();

		void ReceiveDamage( UINT ammount );
		void ParentEvent( ObjectEventInfo event_info );

		ObjEffect()
		{
			this->parentConnection = new ObjectConnection( NULL );
			origin_point = 0;
			disp = Vector2(0, 0);
			old_orig_mirror = false;
			flash_target = Vector4(1.0, 1.0, 1.0, 1.0);
			flash_speed = 0;
			health = 1;
			flash_stage = 0;
		}

		ObjEffect( ObjDynamic* origin, USHORT origin_point, const Vector2& disp )
		{
			ObjectConnection::addChild( origin, this );
			this->origin_point = origin_point;
			this->disp = disp;
			this->old_orig_mirror = false;
			this->flash_target = Vector4(1.0, 1.0, 1.0, 1.0);
			this->flash_speed = 0;
			this->health = 1;
			this->flash_stage = 0;
		}

};

ObjEffect* CreateEffect(const char* proto_name, bool fixed, ObjDynamic* origin, USHORT origin_point, const Vector2& disp);

#endif // __OBJECT_EFFECT_H_