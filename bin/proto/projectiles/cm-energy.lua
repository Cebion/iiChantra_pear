--forbidden

reload_time = 200;
bullets_per_shot = 0;
damage_type = 2;

local diff = (difficulty-1)/5+1;

-- �������� ����
bullet_damage = 0;
bullet_vel = 3*diff;

-- �������� ������� ����
texture = "cm-energy";
hurts_same_type = 1;

z = -0.001;

image_width = 64;
image_height = 16;
frames_count = 4;

ghost_to = 2;

color = { 0.8, 1, 0.8, 1 }

-- ������ ���������, ����� ���� �����
local sound_shoot = "blaster_shot"

animations = 
{
	{
		-- ����, ������� �����
		name = "straight";
		frames = 
		{
			{ com = constants.AnimComRealW; param = 16 },
			{ com = constants.AnimComRealH; param = 16 },
			{ dur = 1 },
			{ com = constants.AnimComSetMaxVelX; param = 1000 },
			{ com = constants.AnimComSetMaxVelY; param = 1000 },
			{ com = constants.AnimComSetRelativeVelX; param = -2000 },
			{ com = constants.AnimComSetAccX; param = 0 },
			{ dur = 100; num = 0 },
			{ com = constants.AnimComSetAnim; txt = "fly" }
		}
	},
	{
		name = "fly";
		frames =
		{
			{ dur = 100; num = 0; },
			{ com = constants.AnimComPushInt; param = 2000 },
			{ dur = 100; num = 1; com = constants.AnimComAdjustHomingAcc; param = 500 },
			{ dur = 100; num = 2; },
			{ com = constants.AnimComLoop }
		},
	},
	{
		-- ����, ������� �� ��������� ����
		name = "diagdown";
		frames = 
		{
			{ dur = 0 }
		}
	},
	{
		-- ����, ������� �� ��������� �����
		name = "diagup";
		frames = 
		{
			{ dur = 0 }
		}
	},
	{
		--���� ������ � ����������� ������.
		name = "miss";
		frames =
		{
			{ com = constants.AnimComDestroyObject }
		}
	},
	{
		-- ����������� ����
		name = "die";
		frames = 
		{
			{ com = constants.AnimComDestroyObject }
		}
	}
}