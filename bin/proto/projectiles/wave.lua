--forbidden

bullet_damage = 40;
bullet_vel = 0;
phys_ghostlike = 1;
ghost_to = 8;

texture = "wave_weapon";

z = -0.001;

animations = 
{
	{
		name = "straight";
		frames = 
		{
			{ com = constants.AnimComSetAnim; txt = "die" }
		}
	},
	{
		name = "diagdown";
		frames = 
		{
			{ com = constants.AnimComSetAnim; txt = "die" }
		}
	},
	{
		name = "diagup";
		frames = 
		{
			{ com = constants.AnimComSetAnim; txt = "die" }
		}
	},
	{
		name = "miss";
		frames =
		{
			{ com = constants.AnimComSetAnim; txt = "die" }
		}
	},
	{
		name = "die";
		frames = 
		{
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = 32; },
			{ com = constants.AnimComRealW; param = 5; },
			{ com = constants.AnimComRealH; param = 3; },
			{ com = constants.AnimComRealX; param = 1; },
			{ com = constants.AnimComRealY; param = 30; },
			{ dur = 25; num = 45; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 8; },
			{ com = constants.AnimComRealH; param = 6; },
			{ com = constants.AnimComRealX; param = 1; },
			{ com = constants.AnimComRealY; param = 28; },
			{ dur = 25; num = 46; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = -2; },
			{ com = constants.AnimComRealW; param = 10; },
			{ com = constants.AnimComRealH; param = 9; },
			{ com = constants.AnimComRealX; param = 2; },
			{ com = constants.AnimComRealY; param = 24; },
			{ dur = 25; num = 47; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 10; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 2; },
			{ com = constants.AnimComRealY; param = 22; },
			{ dur = 25; num = 48; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 10; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 2; },
			{ com = constants.AnimComRealY; param = 21; },
			{ dur = 25; num = 49; },
			{ com = constants.AnimComAdjustX; param = 1; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 9; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 3; },
			{ com = constants.AnimComRealY; param = 21; },
			{ dur = 25; num = 50; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 9; },
			{ com = constants.AnimComRealH; param = 10; },
			{ com = constants.AnimComRealX; param = 3; },
			{ com = constants.AnimComRealY; param = 21; },
			{ dur = 25; num = 51; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 9; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 3; },
			{ com = constants.AnimComRealY; param = 21; },
			{ dur = 25; num = 52; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 2; },
			{ com = constants.AnimComRealW; param = 14; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 3; },
			{ com = constants.AnimComRealY; param = 22; },
			{ dur = 25; num = 53; },
			{ com = constants.AnimComAdjustX; param = -4; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 8; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 2; },
			{ com = constants.AnimComRealY; param = 22; },
			{ dur = 25; num = 54; },
			{ com = constants.AnimComAdjustX; param = 7; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 20; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 3; },
			{ com = constants.AnimComRealY; param = 23; },
			{ dur = 25; num = 55; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 21; },
			{ com = constants.AnimComRealH; param = 14; },
			{ com = constants.AnimComRealX; param = 4; },
			{ com = constants.AnimComRealY; param = 22; },
			{ dur = 25; num = 56; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 23; },
			{ com = constants.AnimComRealH; param = 17; },
			{ com = constants.AnimComRealX; param = 6; },
			{ com = constants.AnimComRealY; param = 19; },
			{ dur = 25; num = 57; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 24; },
			{ com = constants.AnimComRealH; param = 17; },
			{ com = constants.AnimComRealX; param = 8; },
			{ com = constants.AnimComRealY; param = 19; },
			{ dur = 25; num = 58; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 23; },
			{ com = constants.AnimComRealH; param = 17; },
			{ com = constants.AnimComRealX; param = 12; },
			{ com = constants.AnimComRealY; param = 19; },
			{ dur = 25; num = 59; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 22; },
			{ com = constants.AnimComRealH; param = 18; },
			{ com = constants.AnimComRealX; param = 15; },
			{ com = constants.AnimComRealY; param = 18; },
			{ dur = 25; num = 60; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 19; },
			{ com = constants.AnimComRealH; param = 19; },
			{ com = constants.AnimComRealX; param = 19; },
			{ com = constants.AnimComRealY; param = 17; },
			{ dur = 25; num = 61; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = -2; },
			{ com = constants.AnimComRealW; param = 18; },
			{ com = constants.AnimComRealH; param = 19; },
			{ com = constants.AnimComRealX; param = 22; },
			{ com = constants.AnimComRealY; param = 15; },
			{ dur = 25; num = 62; },
			{ com = constants.AnimComAdjustX; param = 1; },
			{ com = constants.AnimComAdjustY; param = -3; },
			{ com = constants.AnimComRealW; param = 18; },
			{ com = constants.AnimComRealH; param = 19; },
			{ com = constants.AnimComRealX; param = 23; },
			{ com = constants.AnimComRealY; param = 12; },
			{ dur = 25; num = 63; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 21; },
			{ com = constants.AnimComRealH; param = 21; },
			{ com = constants.AnimComRealX; param = 21; },
			{ com = constants.AnimComRealY; param = 10; },
			{ dur = 25; num = 64; },
			{ com = constants.AnimComAdjustX; param = -1; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 24; },
			{ com = constants.AnimComRealH; param = 21; },
			{ com = constants.AnimComRealX; param = 18; },
			{ com = constants.AnimComRealY; param = 9; },
			{ dur = 25; num = 65; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 27; },
			{ com = constants.AnimComRealH; param = 22; },
			{ com = constants.AnimComRealX; param = 16; },
			{ com = constants.AnimComRealY; param = 8; },
			{ dur = 25; num = 66; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 30; },
			{ com = constants.AnimComRealH; param = 23; },
			{ com = constants.AnimComRealX; param = 14; },
			{ com = constants.AnimComRealY; param = 7; },
			{ dur = 25; num = 67; },
			{ com = constants.AnimComAdjustX; param = 8; },
			{ com = constants.AnimComAdjustY; param = 2; },
			{ com = constants.AnimComRealW; param = 18; },
			{ com = constants.AnimComRealH; param = 9; },
			{ com = constants.AnimComRealX; param = 28; },
			{ com = constants.AnimComRealY; param = 16; },
			{ dur = 25; num = 68; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 18; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 32; },
			{ com = constants.AnimComRealY; param = 15; },
			{ dur = 25; num = 69; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 16; },
			{ com = constants.AnimComRealH; param = 17; },
			{ com = constants.AnimComRealX; param = 36; },
			{ com = constants.AnimComRealY; param = 13; },
			{ dur = 25; num = 70; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 14; },
			{ com = constants.AnimComRealH; param = 22; },
			{ com = constants.AnimComRealX; param = 41; },
			{ com = constants.AnimComRealY; param = 10; },
			{ dur = 25; num = 71; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 15; },
			{ com = constants.AnimComRealH; param = 23; },
			{ com = constants.AnimComRealX; param = 43; },
			{ com = constants.AnimComRealY; param = 9; },
			{ dur = 25; num = 72; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 15; },
			{ com = constants.AnimComRealH; param = 25; },
			{ com = constants.AnimComRealX; param = 45; },
			{ com = constants.AnimComRealY; param = 8; },
			{ dur = 25; num = 73; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = 8; },
			{ com = constants.AnimComRealW; param = 14; },
			{ com = constants.AnimComRealH; param = 10; },
			{ com = constants.AnimComRealX; param = 47; },
			{ com = constants.AnimComRealY; param = 23; },
			{ dur = 25; num = 74; },
			{ com = constants.AnimComAdjustX; param = -3; },
			{ com = constants.AnimComAdjustY; param = -2; },
			{ com = constants.AnimComRealW; param = 25; },
			{ com = constants.AnimComRealH; param = 13; },
			{ com = constants.AnimComRealX; param = 38; },
			{ com = constants.AnimComRealY; param = 19; },
			{ dur = 25; num = 75; },
			{ com = constants.AnimComAdjustX; param = 6; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 15; },
			{ com = constants.AnimComRealH; param = 17; },
			{ com = constants.AnimComRealX; param = 49; },
			{ com = constants.AnimComRealY; param = 17; },
			{ dur = 25; num = 76; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 15; },
			{ com = constants.AnimComRealH; param = 20; },
			{ com = constants.AnimComRealX; param = 51; },
			{ com = constants.AnimComRealY; param = 14; },
			{ dur = 25; num = 77; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 14; },
			{ com = constants.AnimComRealH; param = 24; },
			{ com = constants.AnimComRealX; param = 53; },
			{ com = constants.AnimComRealY; param = 12; },
			{ dur = 25; num = 78; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 16; },
			{ com = constants.AnimComRealH; param = 27; },
			{ com = constants.AnimComRealX; param = 52; },
			{ com = constants.AnimComRealY; param = 11; },
			{ dur = 25; num = 79; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 17; },
			{ com = constants.AnimComRealH; param = 30; },
			{ com = constants.AnimComRealX; param = 51; },
			{ com = constants.AnimComRealY; param = 10; },
			{ dur = 25; num = 80; },
			{ com = constants.AnimComAdjustX; param = -3; },
			{ com = constants.AnimComAdjustY; param = -6; },
			{ com = constants.AnimComRealW; param = 15; },
			{ com = constants.AnimComRealH; param = 19; },
			{ com = constants.AnimComRealX; param = 49; },
			{ com = constants.AnimComRealY; param = 9; },
			{ dur = 25; num = 81; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = -2; },
			{ com = constants.AnimComRealW; param = 16; },
			{ com = constants.AnimComRealH; param = 17; },
			{ com = constants.AnimComRealX; param = 48; },
			{ com = constants.AnimComRealY; param = 8; },
			{ dur = 25; num = 82; },
			{ com = constants.AnimComAdjustX; param = 1; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 14; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 50; },
			{ com = constants.AnimComRealY; param = 10; },
			{ dur = 25; num = 83; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 14; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 50; },
			{ com = constants.AnimComRealY; param = 11; },
			{ dur = 25; num = 84; },
			{ com = constants.AnimComAdjustX; param = 1; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 16; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 50; },
			{ com = constants.AnimComRealY; param = 11; },
			{ dur = 25; num = 85; },
			{ com = constants.AnimComAdjustX; param = 37; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 90; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 50; },
			{ com = constants.AnimComRealY; param = 11; },
			{ dur = 25; num = 86; },
			{ com = constants.AnimComAdjustX; param = -34; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 21; },
			{ com = constants.AnimComRealH; param = 11; },
			{ com = constants.AnimComRealX; param = 50; },
			{ com = constants.AnimComRealY; param = 11; },
			{ dur = 25; num = 87; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 22; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 51; },
			{ com = constants.AnimComRealY; param = 10; },
			{ dur = 25; num = 88; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 23; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 52; },
			{ com = constants.AnimComRealY; param = 9; },
			{ dur = 25; num = 89; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = -1; },
			{ com = constants.AnimComRealW; param = 21; },
			{ com = constants.AnimComRealH; param = 14; },
			{ com = constants.AnimComRealX; param = 57; },
			{ com = constants.AnimComRealY; param = 7; },
			{ dur = 25; num = 90; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 2; },
			{ com = constants.AnimComRealW; param = 19; },
			{ com = constants.AnimComRealH; param = 13; },
			{ com = constants.AnimComRealX; param = 61; },
			{ com = constants.AnimComRealY; param = 9; },
			{ dur = 25; num = 91; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 19; },
			{ com = constants.AnimComRealH; param = 13; },
			{ com = constants.AnimComRealX; param = 64; },
			{ com = constants.AnimComRealY; param = 10; },
			{ dur = 25; num = 92; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 21; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 67; },
			{ com = constants.AnimComRealY; param = 11; },
			{ dur = 25; num = 93; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 18; },
			{ com = constants.AnimComRealH; param = 10; },
			{ com = constants.AnimComRealX; param = 72; },
			{ com = constants.AnimComRealY; param = 13; },
			{ dur = 25; num = 94; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 19; },
			{ com = constants.AnimComRealH; param = 9; },
			{ com = constants.AnimComRealX; param = 75; },
			{ com = constants.AnimComRealY; param = 14; },
			{ dur = 25; num = 95; },
			{ com = constants.AnimComAdjustX; param = 15; },
			{ com = constants.AnimComAdjustY; param = -4; },
			{ com = constants.AnimComRealW; param = 38; },
			{ com = constants.AnimComRealH; param = 106; },
			{ com = constants.AnimComRealX; param = 80; },
			{ com = constants.AnimComRealY; param = -82; },
			{ dur = 25; num = 96; },
			{ com = constants.AnimComAdjustX; param = -5; },
			{ com = constants.AnimComAdjustY; param = 1; },
			{ com = constants.AnimComRealW; param = 17; },
			{ com = constants.AnimComRealH; param = 7; },
			{ com = constants.AnimComRealX; param = 85; },
			{ com = constants.AnimComRealY; param = 19; },
			{ dur = 25; num = 97; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = 0; },
			{ com = constants.AnimComRealW; param = 15; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 89; },
			{ com = constants.AnimComRealY; param = 16; },
			{ dur = 25; num = 98; },
			{ com = constants.AnimComAdjustX; param = 4; },
			{ com = constants.AnimComAdjustY; param = -4; },
			{ com = constants.AnimComRealW; param = 9; },
			{ com = constants.AnimComRealH; param = 12; },
			{ com = constants.AnimComRealX; param = 96; },
			{ com = constants.AnimComRealY; param = 12; },
			{ dur = 25; num = 99; },
			{ com = constants.AnimComAdjustX; param = 3; },
			{ com = constants.AnimComAdjustY; param = -4; },
			{ com = constants.AnimComRealW; param = 5; },
			{ com = constants.AnimComRealH; param = 10; },
			{ com = constants.AnimComRealX; param = 101; },
			{ com = constants.AnimComRealY; param = 9; },
			{ dur = 25; num = 100; },
			{ com = constants.AnimComAdjustX; param = 2; },
			{ com = constants.AnimComAdjustY; param = -4; },
			{ com = constants.AnimComRealW; param = 3; },
			{ com = constants.AnimComRealH; param = 8; },
			{ com = constants.AnimComRealX; param = 104; },
			{ com = constants.AnimComRealY; param = 6; },
			{ dur = 25; num = 101; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = -3; },
			{ com = constants.AnimComRealW; param = 3; },
			{ com = constants.AnimComRealH; param = 6; },
			{ com = constants.AnimComRealX; param = 104; },
			{ com = constants.AnimComRealY; param = 4; },
			{ dur = 25; num = 102; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = -2; },
			{ com = constants.AnimComRealW; param = 2; },
			{ com = constants.AnimComRealH; param = 5; },
			{ com = constants.AnimComRealX; param = 104; },
			{ com = constants.AnimComRealY; param = 2; },
			{ dur = 25; num = 103; },
			{ com = constants.AnimComAdjustX; param = 0; },
			{ com = constants.AnimComAdjustY; param = -3; },
			{ com = constants.AnimComRealW; param = 1; },
			{ com = constants.AnimComRealH; param = 1; },
			{ com = constants.AnimComRealX; param = 105; },
			{ com = constants.AnimComRealY; param = 1; },
			{ dur = 25; num = 104; },

			{ com = constants.AnimComDestroyObject }
		}
	}
}