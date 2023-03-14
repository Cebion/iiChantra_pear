--forbidden
name = "slowpoke-snowball";

reload_time = 200;
bullets_per_shot = 0;
damage_type = 2;

local diff = (difficulty-1)/5+1;

-- �������� ����
bullet_damage = 25;
bullet_vel = 2*diff;

-- �������� ������� ����
texture = "slowpoke-snowball";

z = -0.001;

image_width = 64;
image_height = 16;
frames_count = 4;

-- ������ ���������, ����� ���� �����
local sound_shoot = "blaster_shot"

animations = 
{
	{
		-- ����, ������� �����
		name = "straight";
		frames = 
		{
			{ com = constants.AnimComRealW; param = 12 },
			{ com = constants.AnimComRealH; param = 12 },
			{ dur = 100; num = 0 },
			{ dur = 100; num = 1 },
			{ dur = 100; num = 2 },
			{ dur = 100; num = 3 },
			{ com = constants.AnimComLoop }
		}
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
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComCreateParticles; txt = "psnowball"; param = 2 },
			{ com = constants.AnimComDestroyObject }
		}
	},
	{
		-- ����������� ����
		name = "die";
		frames = 
		{
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComCreateParticles; txt = "psnowball"; param = 2 },
			{ com = constants.AnimComDestroyObject }
		}
	}
}