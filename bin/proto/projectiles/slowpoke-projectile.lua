--forbidden
name = "slowpoke-projectile";

reload_time = 200;
bullets_per_shot = 0;
damage_type = 2;

local diff = (difficulty-1)/5+1;

-- �������� ����
bullet_damage = 25;
bullet_vel = 2;

-- �������� ������� ����
texture = "slowpoke-projectile";

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
			{ com = constants.AnimComRealW; param = 16 },
			{ com = constants.AnimComRealH; param = 16 },
			{ com = constants.AnimComSetLifetime; param = 300 },
			{ dur = 1 },
			{ com = constants.AnimComPlaySound; txt = "slowpoke-shoot.ogg" },
			{ com = constants.AnimComSetAnim; txt = "fly" }
		}
	},
	{
		name = "fly";
		frames =
		{
			{ dur = 100; num = 0; },
			{ dur = 100; num = 1; },
			{ dur = 100; num = 2; },
			{ dur = 100; num = 3; },
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
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComCreateParticles; txt = "pslime"; param = 0 },
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
			{ com = constants.AnimComCreateParticles; txt = "pslime"; param = 0 },
			{ com = constants.AnimComDestroyObject }
		}
	}
}