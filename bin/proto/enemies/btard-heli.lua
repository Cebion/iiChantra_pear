--forbidden
name = "btard-heli";

physic = 1;
phys_solid = 0;
phys_bullet_collidable = 1;
phys_max_x_vel = 5;
phys_max_y_vel = 50;
phys_jump_vel = 20;
phys_walk_acc = 3;
phys_one_sided = 0;
mp_count = 1;

FunctionName = "CreateEnemy";

-- �������� �������

texture = "btard-com";

z = -0.002;

image_width = 1024;
image_height = 2048;
frame_width = 256;
frame_height = 128;
frames_count = 28;

overlay = {0};
ocolor = {{1, 0.8, 1, 1}}
local diff = (difficulty-1)/5+1;
local difcom = constants.AnimComNone;
if (difficulty >=1) then difcom = constants.AnimComJump; end

animations = 
{
	{ 
		-- ��������
		name = "init";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 75 },
			{ com = constants.AnimComSetHealth; param = 10*difficulty },
			{ com = constants.AnimComRandomOverlayColor },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 800 },
			{ com = constants.AnimComSetGravity },
			{ com = constants.AnimComSetAnim; txt = "idle" }	
		}
	},
	{ 
		-- ��������
		name = "idle";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 75 },
			{ dur = 100; num = 0; com = constants.AnimComWaitForTarget; param = 3000; txt = "move" },
			{ dur = 100; num = 1; com = constants.AnimComWaitForTarget; param = 3000; txt = "move" },
			{ dur = 100; num = 2; com = constants.AnimComWaitForTarget; param = 3000; txt = "move" },
			{ dur = 100; num = 3; com = constants.AnimComWaitForTarget; param = 3000; txt = "move" },
			{ dur = 100; num = 4; com = constants.AnimComWaitForTarget; param = 3000; txt = "move" },
			--{ dur = 100; num = 4 },
			--{ dur = 100; num = 5 },
			{ com = constants.AnimComLoop }	
		}
	},
	{
		name = "friendlyfire";
		frames =
		{
			{ com = constants.AnimComPop },
			{ com = constants.AnimComRecover }	
		}
	},
	{ 
		-- ��������
		name = "pain";
		frames = 
		{
			{ com = constants.AnimComJumpIfIntEquals; param = 3; txt = "friendlyfire" },
			{ com = constants.AnimComPop },
			{ com = constants.AnimComReduceHealth },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComCreateParticles; txt = "pblood-wound"; param = 2 },
			{ com = constants.AnimComRecover }
		}
	},
	{ 
		-- ��������
		name = "move";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 11 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 75 },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpIfTargetClose; param = 33 },
			{ dur = 100; num = 5; com = constants.AnimComMoveToTargetX; param = 250/diff },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpIfTargetClose; param = 33 },
			{ com = constants.AnimComRealX; param = 4 },
			{ dur = 100; num = 6; com = constants.AnimComMoveToTargetX; param = 250/diff },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpIfTargetClose; param = 33 },
			{ com = constants.AnimComRealX; param = 6 },
			{ dur = 100; num = 7; com = constants.AnimComMoveToTargetX; param = 250/diff },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpIfTargetClose; param = 33 },
			{ com = constants.AnimComRealX; param = 18 },
			{ dur = 100; num = 8; com = constants.AnimComMoveToTargetX; param = 500/diff },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpIfTargetClose; param = 33 },
			{ com = constants.AnimComRealX; param = 12 },
			{ dur = 100; num = 9; com = constants.AnimComMoveToTargetX; param = 500/diff },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpIfTargetClose; param = 33 },
			{ com = constants.AnimComRealX; param = 4 },
			{ dur = 100; num = 10; com = constants.AnimComMoveToTargetX; param = 500/diff },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpIfTargetClose; param = 33 },
			--� ��������� ������������ ������ "������������"
			{ com = constants.AnimComPushInt; param = 200*diff },
			{ com = constants.AnimComJumpRandom; param = 18 },
			{ com = constants.AnimComSetAnim; txt = "think" },
			{ com = constants.AnimComLoop },
			{ com = constants.AnimComSetAnim; txt = "attack" }	
		}
	},
	{
		name = "think";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 1 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 75 },
			{ dur = 200; num = 0; com = constants.AnimComWaitForTarget; param = 100; txt = "move" },
			{ com = constants.AnimComRealY; param = 3 },
			{ dur = 200; num = 1; com = constants.AnimComWaitForTarget; param = 100; txt = "move" },
			{ com = constants.AnimComRealY; param = 0 },
			{ dur = 200; num = 2; com = constants.AnimComWaitForTarget; param = 100; txt = "move" },
			{ com = constants.AnimComRealY; param = 0 },
			{ dur = 200; num = 3; com = constants.AnimComWaitForTarget; param = 100; txt = "move" },
			{ com = constants.AnimComRealY; param = 1 },
			{ dur = 200; num = 4; com = constants.AnimComWaitForTarget; param = 100; txt = "move" },
			--������� ����� ������� ����� � ������������ 0.5
			{ com = constants.AnimComPushInt; param = 128 },
			{ com = constants.AnimComJumpRandom; param = 17 },
			{ com = 0; param = 18 },
			{ com = constants.AnimComSetAnim; txt = "move" },
			{ com = constants.AnimComLoop },
			{ dur = 0 },
			{ com = constants.AnimComPushInt; param = 640 },
			{ com = constants.AnimComJumpIfTargetClose; param = 22 },
			{ com = constants.AnimComSetAnim; txt = "move" },
			{ dur = 0 },
			{ com = constants.AnimComPushInt; param = 400 },
			{ com = constants.AnimComPushInt; param = 410 },
			{ com = constants.AnimComJumpIfCloseToCamera; param = 30 },
			{ com = constants.AnimComDestroyObject },
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 34 },
			{ com = constants.AnimComRealH; param = 81 },
			{ com = constants.AnimComFaceTarget },
			{ dur = 100; num = 11 },
			{ com = constants.AnimComRealW; param = 68 },
			{ com = constants.AnimComRealH; param = 78 },
			{ dur = 100; num = 12 },
			{ com = constants.AnimComRealH; param = 77 },
			{ dur = 100; num = 13 },
			{ com = constants.AnimComRealH; param = 69 },
			{ com = constants.AnimComPushInt; param = 50 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ dur = 100; num = 14; com = constants.AnimComAimedShot; txt = "btard-grenade"; param = 1 },
			{ com = constants.AnimComRealH; param = 68 },
			{ dur = 100; num = 15 },
			{ com = constants.AnimComRealH; param = 78 },
			{ dur = 400; num = 16 }, --����� �������
			{ com = constants.AnimComSetAnim; txt = "move" }
		}
	},
	{ 
		-- ��������
		name = "jump";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 75 },
			{ dur = 100; num = 0 },
			{ com = constants.AnimComSetAnim; txt = "idle" }	
		}
	},
	{ 
		-- ��������
		name = "die";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 78 },
			{ com = constants.AnimComMapVarAdd; param = difficulty*40; txt = "score" },
			{ com = constants.AnimComMapVarAdd; param = 1; txt = "kills" },
			{ com = constants.AnimComPushInt; param = 100 },
			{ com = constants.AnimComJumpRandom; param = 16 },
			{ com = constants.AnimComPushInt; param = 150 },
			{ com = constants.AnimComJumpRandom; param = 13 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComCreateItem; txt = "ammo" },
			{ com = constants.AnimComJump; param = 16 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ com = constants.AnimComCreateItem; txt = "vegetable1" },
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
			{ dur = 5000; num = 27 }
			--{ com = constants.AnimComSetAnim; txt = "idle" }	
		}
	},
	{
		name = "attack";
		frames =
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 34 },
			{ com = constants.AnimComRealH; param = 81 },
			{ com = constants.AnimComFaceTarget },
			{ dur = 100; num = 11 },
			{ com = constants.AnimComRealW; param = 68 },
			{ com = constants.AnimComRealH; param = 78 },
			{ dur = 100; num = 12 },
			{ com = constants.AnimComRealH; param = 77 },
			{ dur = 100; num = 13 },
			{ com = constants.AnimComRealH; param = 69 },
			{ com = constants.AnimComPushInt; param = 50 },
			{ com = constants.AnimComPushInt; param = 0 },
			{ dur = 100; num = 14; com = constants.AnimComCreateEnemyBullet; txt = "btard-punch" },
			--{ dur = 100; num = 14; com = constants.AnimComCreateSprite; txt = "btard-punch" },
			{ com = constants.AnimComRealH; param = 68 },
			{ dur = 100; num = 15 },
			{ com = constants.AnimComRealH; param = 78 },
			{ dur = 400; num = 16 }, --����� �������
			{ com = constants.AnimComSetAnim; txt = "move" }
		}
	},
	{ 
		-- ��������
		name = "land";
		frames = 
		{
			{ com = constants.AnimComRealX; param = 0 },
			{ com = constants.AnimComRealY; param = 0 },
			{ com = constants.AnimComRealW; param = 36 },
			{ com = constants.AnimComRealH; param = 75 },
			{ com = constants.AnimComPlaySound; txt = "foot-right" },
			{ com = constants.AnimComPlaySound; txt = "foot-left" },
			{ com = constants.AnimComPushInt; param = -128 },
			{ com = constants.AnimComPushInt; param = -56 },
			{ com = constants.AnimComCreateObject; txt = "dust-land" },
			{ com = constants.AnimComSetAnim; txt = "idle" }
		}
	}
	
}



