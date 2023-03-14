name = "bonus-circle";

trajectory_type = constants.pttGlobalSine;
trajectory_param1 = 0.5;
trajectory_param2 = 0.05;

physic = 1;
phys_solid = 0;
phys_bullet_collidable = 0;
phys_max_x_vel = 0;
phys_max_y_vel = 0;

FunctionName = "CreateItem";

-- �������� �������

texture = "weapon_bonuses";
z = -0.001;


animations = 
{
	{ 
		-- ��������
		name = "init";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 22 },
			{ com = constants.AnimComRealH; param = 22 },
			{ com = constants.AnimComSetTouchable; param = 1 },
			{ com = constants.AnimComSetAnim; txt = "idle" }	
		}
	},
	{ 
		name = "idle";
		frames = 
		{
	
			{ dur = 150; num = 0 },
			{ dur = 100; num = 1 },
			{ dur = 100; num = 2 },
			{ dur = 150; num = 3 },
			{ dur = 100; num = 2 },
			{ dur = 100; num = 1 },
			{ dur = 150; num = 0 },
			{ dur = 100; num = 4 },
			{ dur = 100; num = 2 },
			{ dur = 150; num = 5 },
			{ dur = 100; num = 2 },
			{ dur = 100; num = 4 },
			{ com = constants.AnimComLoop }	
		}
	},
	{ 
		-- �����
		name = "touch";
		frames = 
		{
			{ com = constants.AnimComGiveWeapon; txt = "square" },
			{ com = constants.AnimComDestroyObject }
		}
	}
	
}



