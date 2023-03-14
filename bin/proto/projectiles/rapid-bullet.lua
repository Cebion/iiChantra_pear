--forbidden
name = "rapid-bullet";

push_force = 1.0;

-- �������� ����
bullet_damage = 10;
bullet_vel = 8;

-- �������� ������� ����
texture = "bullets";
overlay = {0};

z = -0.002;

image_width = 256;
image_height = 64;
frame_width = 64;
frame_height = 32;
frames_count = 7;

-- ������ ���������, ����� ���� �����
local sound_shoot = "blaster_shot"
local delay = 20

animations = 
{
	{
		-- ����, ������� �����
		name = "straight";
		frames = 
		{
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComControlledOverlayColor },
			{ com = constants.AnimComRealX; param = 38 },
			{ com = constants.AnimComRealY; param = 12 },
			{ com = constants.AnimComRealW; param = 11 },
			{ com = constants.AnimComRealH; param = 7 },
			{ dur = delay; num = 14 },
			{ dur = 100; num = 0; com = constants.AnimComRealH; param = 5 }
		}
	},
	{
		-- ����, ������� �� ��������� ����
		name = "diagdown";
		frames = 
		{
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComControlledOverlayColor },
			{ com = constants.AnimComRealX; param = 37 },
			{ com = constants.AnimComRealY; param = 21 },
			{ com = constants.AnimComRealW; param = 9 },
			{ com = constants.AnimComRealH; param = 7 },
			{ dur = delay; num = 14 },			
			{ dur = 100; num = 1; com = constants.AnimComRealH; param = 9 }
		}
	},
	{
		-- ����, ������� �� ��������� �����
		name = "diagup";
		frames = 
		{
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComControlledOverlayColor },
			{ com = constants.AnimComRealX; param = 37 },
			{ com = constants.AnimComRealY; param = 2 },
			{ com = constants.AnimComRealW; param = 9 },
			{ com = constants.AnimComRealH; param = 7 },
			{ dur = delay; num = 14 },
			{ dur = 100; num = 2; com = constants.AnimComRealH; param = 9 }
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
			{ com = constants.AnimComRealH; param = 7 },
			{ dur = 100; num = 4 },
			{ dur = 100; num = 5 },
			{ dur = 100; num = 6 },
			{ dur = 100; num = 7 },
			{ com = constants.AnimComDestroyObject }
		}
	}
}