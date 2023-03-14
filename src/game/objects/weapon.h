#ifndef __OBJECT_WEAPON_H_
#define __OBJECT_WEAPON_H_

// ��������������� ����������� ������ �������.
// ��� ��� �� ����� ��������, �� � ������ ������� ��� �����
// �� ���� ������� ���������� �� ����.
class ObjCharacter;


// ����������� ��������. ������������, ����� ���������� ����.
enum WeaponDirection { wdRight, wdLeft, wdUpLeft, wdUpRight, wdDownLeft, wdDownRight/*, wdUp, wdDown*/ };



//////////////////////////////////////////////////////////////////////////

class Weapon
{
public:
	UINT reload_time;			// �����, ������� ������� �� ����������� ����� ��������
	UINT clip_reload_time;
	UINT last_reload_tick;		// ����� ������ �����������

	const Proto* bullet_proto;		// ��������, �� �������� ��������� ����
	USHORT bullets_count;		// ���������� ���� �� �������.
	USHORT shots_per_clip;
	USHORT clip;
	bool is_infinite;			// ���������� �� ����

	bool is_ray;

	Weapon()
	{
		reload_time = 0;
		clip_reload_time = 0;
		shots_per_clip = 0;
		last_reload_tick = 0;

		bullet_proto = NULL;
		bullets_count = 0;
		is_infinite = false;
		is_ray = false;
	}

	void Fire(ObjCharacter* shooter, Vector2 coord);
	bool IsReloaded();
	bool ClipReloaded(bool hold);
	bool IsReady();
};

Weapon* CreateWeapon(const char* proto_name);

#endif // __OBJECT_BULLET_H_