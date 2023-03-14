--forbidden
name = "spread";

reload_time = 400;
bullets_per_shot = 3;
damage_type = 2;

push_force = 2.0;

-- �������� ����
bullet_damage = 10;
bullet_vel = 2.5;

-- �������� ������� ����
texture = "bullets";

z = -0.002;

image_width = 256;
image_height = 64;
frame_width = 64;
frame_height = 32;
frames_count = 7;

-- ������ ���������, ����� ���� �����
local sound_shoot = "weapons/spread.ogg"

animations = 
{
	{
		-- ����, ������� �����
		name = "straight";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 38 },
			{ com = constants.AnimComRealY; param = 12 },
			{ com = constants.AnimComRealW; param = 11 },
			{ com = constants.AnimComPlaySound; txt = sound_shoot; param = 1 },
			{ dur = 1; num = 0; com = constants.AnimComRealH; param = 7 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = -8 },
			{ com = constants.AnimComCreateEffect; txt ="flash-straight-spread"; param = 13 },
			{ com = constants.AnimComDestroyObject }
		}
	},
	{
		-- ����, ������� �� ��������� ����
		name = "diagdown";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 37 },
			{ com = constants.AnimComRealY; param = 21 },
			{ com = constants.AnimComRealW; param = 9 },
			{ com = constants.AnimComPlaySound; txt = sound_shoot; param = 1 },
			{ dur = 1; num = 1; com = constants.AnimComRealH; param = 7 },
			{ com = constants.AnimComPushInt; param = -3 },
			{ com = constants.AnimComPushInt; param = -34 },
			{ com = constants.AnimComCreateEffect; txt ="flash-angle-down-spread"; param = 13 },
			{ com = constants.AnimComDestroyObject }
		}
	},
	{
		-- ����, ������� �� ��������� �����
		name = "diagup";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 37 },
			{ com = constants.AnimComRealY; param = 2 },
			{ com = constants.AnimComRealW; param = 9 },
			{ com = constants.AnimComPlaySound; txt = sound_shoot; param = 1 },
			{ dur = 1; num = 2; com = constants.AnimComRealH; param = 7 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = -10 },
			{ com = constants.AnimComCreateEffect; txt ="flash-angle-up-spread"; param = 13 },
			{ com = constants.AnimComPlaySound; txt = sound_shoot },
			{ com = constants.AnimComDestroyObject }
		}
	},
	{
		--���� ������ � ����������� ������.
		name = "miss";
		frames =
		{
			{ com = constants.AnimComStartDying },
			{ com = constants.AnimComSetAnim; txt = "die" }
		}
	},
	{
		-- ����������� ����
		name = "die";
		frames = 
		{
			{ com = constants.AnimComStop },
			{ com = constants.AnimComRealX; param = 38 },
			{ com = constants.AnimComRealY; param = 12 },
			{ com = constants.AnimComRealW; param = 11 },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComPushInt; param = 100 },
			{ dur = 100; num = 4 },
			{ dur = 100; num = 5 },
			{ dur = 100; num = 6 },
			{ dur = 100; num = 7 },
			{ com = constants.AnimComDestroyObject }
		}
	}
}