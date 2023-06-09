--forbidden
parent = "enemies/superbtard"

animations = 
{
	{ 
		-- Создание
		name = "init";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 75 },
			{ com = constants.AnimComSetHealth; param = 40*difficulty },
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComPushInt; param = 255 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComControlledOverlayColor },
			{ dur = 1 },
			{ constants.AnimComCallFunction; txt = "boss_enemy_created" },
			{ com = constants.AnimComSetAnim; txt = "idle" }	
		}
	},
	{ 
		-- Создание
		name = "die";
		frames = 
		{
			{ com = constants.AnimComCallFunction; txt = "boss_enemy_dead" },
			{ com = constants.AnimComSetBulletCollidable; param = 0 },
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 78 },
			{ com = constants.AnimComMapVarAdd; param = difficulty*40; txt = "score" },
			{ com = constants.AnimComMapVarAdd; param = 1; txt = "kills" },
			{ com = constants.AnimComPushInt; param = 220 },
			{ com = constants.AnimComJumpRandom; param = 11 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComCreateItem; txt = "vegetable2" },
			{ dur = 100; num = 18 },
			{ com = constants.AnimComRealH; param = 77 },	
			{ dur = 100; num = 19 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 28 },
			{ dur = 100; num = 20; com = constants.AnimComCreateParticles; txt = "pblood"; param = 2 },
			{ dur = 100; num = 21 },
			{ com = constants.AnimComRealH; param = 75 },
			{ dur = 100; num = 22 },
			{ com = constants.AnimComRealH; param = 68 },
			{ dur = 100; num = 23 },
			{ com = constants.AnimComRealH; param = 55 },
			{ dur = 100; num = 24 },
			{ com = constants.AnimComPushInt; param = -128 },
			{ com = constants.AnimComPushInt; param = -77 },
			{ com = constants.AnimComPlaySound; txt = "foot-right" },
			{ com = constants.AnimComPlaySound; txt = "foot-left" },
			{ com = constants.AnimComRealH; param = 30 },
			{ dur = 100; num = 25, com = constants.AnimComCreateObject; txt = "dust-land" },
			{ com = constants.AnimComRealH; param = 25 },
			{ dur = 100; num = 26 },
			{ com = constants.AnimComPushInt; param = -128 },
			{ com = constants.AnimComPushInt; param = -64 },
			{ com = constants.AnimComRealH; param = 21 },
			{ dur = 5000; num = 27 },
			{ com = constants.AnimComJump; param = 37 },
			{}
		}
	},
	{
		name = "think-cutscene";
		frames = 
		{
			{ dur = 100; num = 0 },
			{ dur = 100; num = 1 },
			{ dur = 100; num = 2 },
			{ dur = 100; num = 3 },
			{ dur = 100; num = 4 },
			{ dur = 100; num = 0 },
			{ dur = 100; num = 1 },
			{ dur = 100; num = 2 },
			{ dur = 100; num = 3 },
			{ dur = 100; num = 4 },
			{ dur = 100; num = 0 },
			{ dur = 100; num = 1 },
			{ dur = 100; num = 2 },
			{ dur = 100; num = 3 },
			{ dur = 100; num = 4 },

			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 52 },
			{ com = constants.AnimComRealH; param = 79 },
			{ dur = 50; num = 28; com = constants.AnimComRealX; param = 0 },
			{ dur = 50; num = 29; com = constants.AnimComRealX; param = 6 },
			{ dur = 50; num = 28; com = constants.AnimComRealX; param = 0 },
			{ dur = 50; num = 29; com = constants.AnimComRealX; param = 6 },
			{ dur = 50; num = 28; com = constants.AnimComRealX; param = 0 },
			{ dur = 50; num = 29; com = constants.AnimComRealX; param = 6 },

			{ com = constants.AnimComRealH; param = 77 },
			{ dur = 50; num = 30; com = constants.AnimComRealX; param = 3 },
			{ dur = 50; num = 31; },
			{ dur = 50; num = 30; },
			{ dur = 50; num = 31; },

			{ dur = 100; num = 32; com = constants.AnimComRealX; param = -3 },
			{ com = constants.AnimComRealH; param = 75 },
			{ dur = 100; num = 33; com = constants.AnimComRealX; param = -6 },
			{ com = constants.AnimComRealH; param = 69 },
			{ dur = 100; num = 34; com = constants.AnimComRealX; param = -9 },

			{ com = constants.AnimComPushInt; param = 20 },
			{ com = constants.AnimComPushInt; param = -20 },
			{ dur = 0; com = constants.AnimComCallFunction; txt = "superbtard_fire" },
			{ dur = 0; num = 34; com = constants.AnimComCreateEnemyRay; txt = "btard-lazor" },

			{ dur = 100; num = 34; com = constants.AnimComRealX; param = -9 },
			{ com = constants.AnimComRealH; param = 75 },
			{ dur = 100; num = 33; com = constants.AnimComRealX; param = -6 },
			{ dur = 100; num = 32; com = constants.AnimComRealX; param = -3 },
			{ com = constants.AnimComRealH; param = 77 },
			{ dur = 100; num = 30; com = constants.AnimComRealX; param = 3 },
			
			{ com = constants.AnimComSetAnim; txt = "move" }
		}
	}	
}



