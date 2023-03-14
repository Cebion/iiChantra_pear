name = "bonus-line";

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
			{ com = constants.AnimComRealW; param = 24 },
			{ com = constants.AnimComRealH; param = 7 },
			{ com = constants.AnimComSetTouchable; param = 1 },
			{ com = constants.AnimComSetAnim; txt = "idle" }	
		}
	},
	{ 
		name = "idle";
		frames = 
		{
	
			{ dur = 100; num = 28 },
			{ dur = 100; num = 29 },
			{ dur = 100; num = 30 },
			{ dur = 100; num = 31 },
			{ dur = 100; num = 32 },
			{ dur = 100; num = 33 },
			{ dur = 100; num = 34 },
			{ dur = 100; num = 35 },
			{ dur = 100; num = 36 },
			{ dur = 100; num = 37 },
			{ com = constants.AnimComLoop }	
		}
	},
	{ 
		-- �����
		name = "touch";
		frames = 
		{
			{ com = constants.AnimComGiveWeapon; txt = "line"; num = 28 },
			{ com = constants.AnimComDestroyObject; num = 28 }
		}
	}
	
}



