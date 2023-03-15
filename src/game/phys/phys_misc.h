#ifndef __PHYSMISC_H_
#define __PHYSMISC_H_

//
//
//		РАЗГРЕСТИ НАХУЙ ВСЕ ЭТО ДЕРЬМО
//		БАРДАК ЖЕ
//
//





#ifdef LINUX
#include <cmath>
#endif
#include <math.h>
#include <float.h>
//#include "sap/OPC_ArraySAP.h"
//#include "sap/SweepAndPrune.h"
#include "sap/ASAP_Types.h"
//using namespace Opcode;

//#pragma warning( disable : 4305 )
//#pragma warning( disable : 4244 )
//#pragma warning( disable : 4996 )
//#pragma warning( disable : 4172 )

// If enabled then optimized version of
// Vector2.Length() used. It costs 5% accuracy.
//#define OPTIMIZE_V2L

#define __DEBUG_PHYSICS_
#undef __DEBUG_PHYSICS_

//#define KERNEL_BUG_PHYSICS
//#define SATANA_PHYSICS
#define USING_SAP_PHYSICS
//#define __DEBUG_SAP_PHYSICS_

#define PI_d180  PI / 180.0f
#define  d180_PI  180.0f / PI

#if defined(WIN32) && !defined(__MINGW32__)
#define __INLINE __forceinline
#else
#define __INLINE inline
#endif

typedef float scalar;

__INLINE scalar DegToRad(scalar degree)
{
	return (scalar)(degree * PI_d180);
}
__INLINE scalar RadToDeg(scalar radian)
{
	return (scalar)(radian * d180_PI);
}
__INLINE scalar Max(scalar a, scalar b)
{
	return a>=b?a:b;
}
__INLINE scalar Min(scalar a, scalar b)
{
	return a<=b?a:b;
}
__INLINE scalar clampf(scalar x , scalar xmin, scalar xmax)
{
	return Min(Max(x, xmin), xmax);
}


class Vector2{
public:
	scalar x, y;
	static const Vector2& Blank()
	{
		static const Vector2 V(0, 0);
		return V;
	}
public:
	__INLINE Vector2(void): x(0.0f), y (0.0f){}
	__INLINE Vector2( scalar Ix, scalar Iy) : x(Ix), y(Iy){}
	__INLINE  Vector2 operator + (const Vector2 &V)const
	{
		return Vector2(x + V.x, y + V.y);
	}
	__INLINE Vector2 operator - (const Vector2 &V)const
	{
		return Vector2(x - V.x, y - V.y);
	}
	__INLINE Vector2 operator * (const scalar a)const
	{
		return Vector2(x * a, y * a);
	}
	__INLINE Vector2 operator / (const scalar a)const
	{
		if (a == 0.0f)
			return Vector2().Blank();
		scalar t = 1.0f/a;
		return Vector2(x*t, y*t);
	}
	friend __INLINE  Vector2 operator * (scalar k, const Vector2& V)
	{
		return Vector2(V.x*k, V.y*k);
	}

	__INLINE Vector2 const &operator +=(const Vector2 &V)
	{
		x += V.x;
		y += V.y;
		return *this;
	}
	__INLINE Vector2 const &operator -=(const Vector2 &V)
	{
		x -= V.x;
		y -= V.y;
		return *this;
	}
	__INLINE Vector2 const  &operator *=(const scalar a)
	{
		x *= a;
		y *= a;
		return *this;
	}
	__INLINE Vector2 const &operator /=(const scalar &a)
	{
		if (a == 0.0f)
			return *this;
		scalar t = 1.0f/a;
		x *= t;
		y *= t;
		return *this;
	}

	__INLINE Vector2 operator -(void) const
	{
		return Vector2( -x, -y);
	}

	// скалярное произведение векторов
	__INLINE scalar operator * (const Vector2 &V) const
	{
		return x * V.x + y * V.y;
	}

	// попытка сделать аналог векторного произведения в двух измеениях
	__INLINE scalar operator ^ (const Vector2 &V) const
	{
		return x * V.y - y * V.x;
	}

	__INLINE bool operator == (const Vector2 &V) const 
	{
		return x == V.x && y == V.y;
	}

	__INLINE bool operator != (const Vector2 &V) const 
	{
		return !(*this == V);
	}

	__INLINE scalar Length(void) const
	{
#ifdef OPTIMIZE_V2L
		scalar dx, dy;
		if ( x < 0 )
			dx = -x;
		else
			dx = x;

		if ( y < 0 )
			dy = -y;
		else
			dy = y;
		if ( dx < dy )
			return 0.961f*dy+0.398f*dx;
		else
			return 0.961f*dx+0.398f*dy;
#else
		return (scalar)sqrt((double)(x*x + y*y));
#endif
	}

	__INLINE Vector2 GetPerp()
	{
		return Vector2(-y, x);
	}

	__INLINE Vector2 Normalized(void)const
	{
		scalar l = Length();
		if ( l == 0.0f )
			return Vector2();
		Vector2 t = (*this) * ( 1.0f / l );
		return t;
	}
	__INLINE scalar Normalize(void)
	{
		scalar l = Length();
		if ( l == 0.0f )
			return 0.0f;
		(*this) *= ( 1.0f / l );
		return l;
	}

};


/**
*	Расстояние от точки до прямой вычисляет эта функция.
*	v1, v2 - координаты любых двух (возможно не совпадающих)
*		точек, принадлежащих прямой.
*	p - та самая точка, расстояние от которой мы ищем.
*	TODO : оптимизировать её нужно.
*/

__INLINE scalar PointLineDistance(Vector2 v1, Vector2 v2, Vector2 p)
{
	scalar dx, dy, D;
	dx = v1.x - v2.x;
	dy = v1.y - v2.y;
	D = dx * (p.y - v1.y) - dy * ( p.x - v1.x );
	return  std::abs( D / sqrt( dx * dx + dy * dy));
}

class CAABB
{
public:
	Vector2 p;
	scalar H;
	scalar W;

	scalar Left() const { return p.x - W; }
	scalar Right() const { return p.x + W; }
	scalar Top() const { return p.y - H; }
	scalar Bottom() const { return p.y + H; }

	scalar GetMin(UINT axis) const { return axis == 0 ? this->Left() : this->Top(); }
	scalar GetMax(UINT axis) const { return axis == 0 ? Right() : Bottom(); }

	ASAP_AABB GetASAP_AABB()
	{
		ASAP_AABB box;
		box.mMax.x = Right();
		box.mMax.y = Bottom();
		box.mMax.z = 1;
		box.mMin.x = Left();
		box.mMin.y = Top();
		box.mMin.z = 0;
		return box;
	}

	__INLINE CAABB(void) : H(0.0f), W(0.0f) { }

	CAABB(const CAABB& a)
	{
		p = a.p;
		H = a.H;
		W = a.W;
	}

	CAABB(scalar x1, scalar y1, scalar x2, scalar y2)
	{
		//H = fabs(y2 - y1) * 0.5f;
		//W = fabs(x2 - x1) * 0.5f;
		W = (max(x1,x2) - min(x1, x2)) * 0.5f;
		H = (max(y1,y2) - min(y1, y2)) * 0.5f;
		p = Vector2(min(x1, x2) + W, min(y1, y2) + H);
	}

	__INLINE bool IsEmpty ()
	{
		return (H == 0 && W == 0);
	}

	void Intersect(const CAABB &a, const CAABB &b)
	{
		if (fabs(a.p.x - b.p.x) <= a.W + b.W &&
			fabs(a.p.y - b.p.y) <= a.H + b.H)
		{
			scalar al, bl, ar, br, at, bt, ab, bb;
			scalar cl, cr, cb, ct;

			al = a.p.x - a.W;
			at = a.p.y - a.H;
			bl = b.p.x - b.W;
			bt = b.p.y - b.W;

			ar = a.p.x + a.W;
			ab = a.p.y + a.H;
			br = b.p.x + b.W;
			bb = b.p.y + b.W;

			cl = al > bl ? al : bl;
			cr = ar < br ? ar : br;
			ct = at > bt ? at : bt;
			cb = ab < bb ? ab : bb;

			W = (cl - cr) / 2;
			H = (cb - ct) / 2;
			p.x = cl + W;
			p.y = ct + H;
		}
		else
		{
			H = 0;
			W = 0;
			p.Blank();
		}
	}

	bool PointInCAABB(scalar x, scalar y)
	{
		return (x >= this->Left() && x <= this->Right() && y >= this->Top() && y <= this->Bottom() );
	}

	__INLINE bool operator == (const CAABB &A) const 
	{
		return p == A.p && W == A.W && H == A.H;
	}

	__INLINE bool operator != (const CAABB &A) const 
	{
		return !(*this == A);
	}
};

#ifdef WIN32
// Фуникция, проверяющая значение на равенство бесконечности
template<typename T>
inline bool isinf(T value)
{
	return std::numeric_limits<T>::has_infinity &&
		value == std::numeric_limits<T>::infinity();
}
#endif // WIN32

// Класс луча, определяет пересечения
class CRay
{
public:
	// Вообще, в основе взято уранвнеие прямой y = kx + b, так его проще хранить и
	// вести вычисления. Отдельно рассматриваются случаи, когда луч вертикален.

	Vector2 p;	// Точка начала луча

	scalar k;	// Тангенс угла междй лучом и осью ОХ. В случае, когда луч вертикален,
				// принимает значения inf (+OУ, вниз) и -inf (-ОУ, вверх).

	bool dir;	// Направление луча. true - вправо, false - влево. Для вертикального
				// луча true - вниз, false - вверх.

	__INLINE CRay(): k(0.0), dir(true) { }

	CRay(const CRay& r)
	{
		p = r.p;
		k = r.k;
		dir = r.dir;
	}

	CRay(scalar x0, scalar y0, scalar k, bool dir)
	{
		p = Vector2(x0, y0);
		this->k = k;
		this->dir = dir;

		// Для вертикального проверяем правильность задания dir.
		assert(!IsVertical() || (k > 0) == dir );
	}

	CRay(scalar x0, scalar y0, scalar x1, scalar y1)
	{
		p = Vector2(x0, y0);
		if (x1 != x0)
		{
			k = (y1-y0)/(x1-x0);
			dir = x1 > x0;
		}
		else
		{
			k = (y1 >= y0) ? 
				numeric_limits<scalar>::infinity() : 
				-numeric_limits<scalar>::infinity();
			dir = k > 0;
		}
	}

	bool IsVertical()
	{
		// TODO: переделать
		return isinf(k) || isinf(-k);
	}

	bool IsHorizontal()
	{
		// TODO: переделать на IsNear
		return fabs(k) < 0.00001;
	}

	// Тест пересечения луча и прямоугольника. 
	bool IsIntersecting(scalar left, scalar right, scalar top, scalar bottom)
	{
		assert(top < bottom);
		assert(left < right);

		// И пошли разные случаи. Навреняка ведь можно проще и быстрее
		if (p.x < left)
		{
			// Начало левее прмоугольника

			if (IsVertical() || !dir)
				return false;

			if ( IsHorizontal() )
				return top <= p.y && p.y <= bottom && dir;

			if (k > 0)
				// Левая и верхняя
				return IsHorLineInters(left, right, top) || IsVertLineInters(top, bottom, left);
			else
				// Левая и нижняя
				return IsHorLineInters(left, right, bottom) || IsVertLineInters(top, bottom, left);
		}
		else if (p.x > right)
		{
			// Начало правее прямоугольника
			if (IsVertical() || dir)
				return false;

			if ( IsHorizontal() )
				return top <= p.y && p.y <= bottom && !dir;

			if (k > 0)
				// Правая и нижняя
				return IsHorLineInters(left, right, bottom) || IsVertLineInters(top, bottom, right);
			else
				// Правая и верхняя
				return IsHorLineInters(left, right, top) || IsVertLineInters(top, bottom, right);
		}
		else
		{
			// Внутри прямоугольника (естественно по оси Х)
			if (p.y < top)
			{
				// Выше прямоугольника
				if (IsHorizontal() || (IsVertical() && !dir))
					return false;
				else if ((dir && k < 0) || (!dir && k > 0))
					return false;
				else
					return IsHorLineInters(left, right, top);
			}
			else if (p.y > bottom)
			{
				// Ниже прямоугольника
				if (IsHorizontal() || (IsVertical() && dir))
					return false;
				else if ((dir && k > 0) || (!dir && k < 0))
					return false;
				else
					return IsHorLineInters(left, right, bottom);
			}
			else
			{
				// Четко внутри, тут уж точно пересечешь
				return true;
			}
		}

		return false;
	}

	// Тест пересечения луча и прямоугольника
	bool IsIntersecting(const CAABB& aabb)
	{
		//scalar left = aabb.Left();
		//scalar right = aabb.Right();
		//scalar top = aabb.Top();
		//scalar bottom = aabb.Bottom();

		return IsIntersecting(aabb.Left(), aabb.Right(), aabb.Top(), aabb.Bottom());
	}

	bool GetIntersectionPoint(CAABB const& aabb, Vector2& p)
	{
		return GetIntersectionPoint(aabb.Left(), aabb.Top(), aabb.Right(), aabb.Bottom(), p);
	}

	bool GetIntersectionPoint(scalar left, scalar top, scalar right, scalar bottom, Vector2& c)
	{
		assert(top < bottom);
		assert(left < right);

		if (p.x < left)
		{
			// Начало левее прмоугольника

			if (IsVertical() || !dir)
				return false;

			if ( IsHorizontal() )
				return dir && GetVertLineInters(top, bottom, left, c);

			if (k > 0)
				// Левая и верхняя
				return GetHorLineInters(left, right, top, c) || GetVertLineInters(top, bottom, left, c);
			else
				// Левая и нижняя
				return GetHorLineInters(left, right, bottom, c) || GetVertLineInters(top, bottom, left, c);
		}
		else if (p.x > right)
		{
			// Начало правее прямоугольника
			if (IsVertical() || dir)
				return false;

			if ( IsHorizontal() )
				return !dir && GetVertLineInters(top, bottom, right, c);

			if (k > 0)
				// Правая и нижняя
				return GetHorLineInters(left, right, bottom, c) || GetVertLineInters(top, bottom, right, c);
			else
				// Правая и верхняя
				return GetHorLineInters(left, right, top, c) || GetVertLineInters(top, bottom, right, c);
		}
		else
		{
			// Внутри прямоугольника (естественно по оси Х)
			if (p.y < top)
			{
				// Выше прямоугольника
				if (IsHorizontal() || (IsVertical() && !dir))
					return false;
				else if ((dir && k < 0) || (!dir && k > 0))
					return false;
				else
					return GetHorLineInters(left, right, top, c);
			}
			else if (p.y > bottom)
			{
				// Ниже прямоугольника
				if (IsHorizontal() || (IsVertical() && dir))
					return false;
				else if ((dir && k > 0) || (!dir && k < 0))
					return false;
				else
					return GetHorLineInters(left, right, bottom, c);
			}
			else
			{
				if (IsVertical())
				{
					if (dir)
						return GetHorLineInters(left, right, top, c);
					else
						return GetHorLineInters(left, right, bottom, c);
				}

				if (dir)
				{
					if (k > 0)
						// Правая и нижняя
						return GetHorLineInters(left, right, bottom, c) || GetVertLineInters(top, bottom, right, c);
					else
						// Правая и верхняя
						return GetHorLineInters(left, right, top, c) || GetVertLineInters(top, bottom, right, c);
				}
				else
				{
					if (k > 0)
						// Левая и верхняя
						return GetHorLineInters(left, right, top, c) || GetVertLineInters(top, bottom, left, c);
					else
						// Левая и нижняя
						return GetHorLineInters(left, right, bottom, c) || GetVertLineInters(top, bottom, left, c);
				}
			}
		}

		return false;

	}

	// Тест пересечения луча и отрезка, заодно возвращает точку пересечения через x и y
	bool GetIntersectionPoint(scalar x1, scalar y1, scalar x2, scalar y2, scalar& x, scalar& y)
	{
		scalar a2 = y2 - y1;
		scalar b2 = x1 - x2;
		scalar c2 = x2*y1 - x1*y2;

		if (IsVertical())
		{
			x = p.x;
			y = - (a2*x + c2) / b2;

			if (min(x1, x2) - x > 0.01 || x - max(x1,x2) > 0.01 || 
				(y < p.y && k > 0) || (y > p.y && k < 0))
				return false;
		}
		else
		{
			y = ( a2 * (p.y - k*p.x) - k * c2 ) / (b2*k + a2);
			if (min(y1, y2) - y > 0.01 || y - max(y1,y2) > 0.01)
				return false;

			if (k != 0)
				x = (y - p.y) / k + p.x;
			else if (a2 != 0)
				x =  ( b2*y - c2 ) / a2;
			else
			{
				x = dir ? min(x1, x2) : max(x1, x2);
			}

			if (min(x1, x2) - x > 0.01 || x - max(x1,x2) > 0.01 || 
				(x < p.x && dir) || (x > p.x && !dir))
				return false;
		}

		return true;
	}

private:
	// Тест пересечениче луча и горизонтального отрезка
	bool IsHorLineInters(scalar x1, scalar x2, scalar y)
	{
		assert(x1 <= x2);
		scalar xn = (y - p.y) / k + p.x;
		return x1 <= xn && xn <= x2;
	}

	// Тест пересечения луча и вертикального отрезка
	bool IsVertLineInters(scalar y1, scalar y2, scalar x)
	{
		assert(y1 <= y2);
		scalar yn = (x - p.x) * k + p.y;
		return y1 <= yn && yn <= y2;
	}

	bool GetHorLineInters(scalar x1, scalar x2, scalar y, Vector2& c)
	{
		assert(x1 <= x2);
		c.x = (y - p.y) / k + p.x;
		c.y = y;
		return x1 <= c.x && c.x <= x2;
	}

	// Тест пересечения луча и вертикального отрезка
	bool GetVertLineInters(scalar y1, scalar y2, scalar x, Vector2& c)
	{
		assert(y1 <= y2);
		c.y = (x - p.x) * k + p.y;
		c.x = x;
		return y1 <= c.y && c.y <= y2;
	}
};


/**
*	Collide - сталкиваются ли 2 бокса.
*	TODO: слишком дохуя сложений\вычитаний в такой простой ф-ии.
*/

__INLINE bool Collide(const CAABB &a, const CAABB &b)
{
	if (a.p.x + a.W < b.p.x - b.W || a.p.x - a.W > b.p.x + b.W)
		return false;
	if (a.p.y + a.H < b.p.y - b.H || a.p.y - a.H > b.p.y + b.H)
		return false;
	return true;
}

/**
*	Дефайны для типов пересечений отрезков проекций боксов.
*	Тоесть: нет пересечения - нам надо знать первый отезок перед вторым или после, если есть пересечение, то хрен с ним есть.
*/

#define SEG_PROJ_STATE_BEFORE		0x01
#define SEG_PROJ_STATE_INTERSECT	0x02
#define SEG_PROJ_STATE_AFTER		0x03

/**
*	Собственно определяет тип пересечения отрезков проекций боксов
*	Вообще не проекция боксов блаблабла, а просто тип пересечения отезков. Но мы передаем в неё обычно проекции боксов.
*	a.x, a.y - 1я и 2я координаты первого отрезка. Вообще, то что они в векторе и названы x y может сбить с мысли. Пусть не сбивает.
*/

__INLINE int SegProjIntersect(const Vector2 &a, const Vector2 &b)
{
	if (a.y <= b.x)
		return SEG_PROJ_STATE_BEFORE;
	if (a.x >= b.y)
		return SEG_PROJ_STATE_AFTER;
	return SEG_PROJ_STATE_INTERSECT;
}

// Хрень. Вроде не используется даже.
// Действительно не используется. Хрень.
__INLINE Vector2 ResolveVector(int x, int y)
{
	if (x == SEG_PROJ_STATE_INTERSECT)
	{
		switch(y)
		{
		case SEG_PROJ_STATE_BEFORE:
			return Vector2(0.0f, -1.0f);
		case SEG_PROJ_STATE_AFTER:
			return Vector2(0.0f, 1.0f);
		}
	}

	if (y == SEG_PROJ_STATE_INTERSECT)
	{
		switch(x)
		{
		case SEG_PROJ_STATE_BEFORE:
			return Vector2(-1.0f, 0.0f);
		case SEG_PROJ_STATE_AFTER:
			return Vector2(1.0f, 0.0f);
		}
	}
	return Vector2(0.0f, 0.0f);

}

/**
*	Это дефайны углов. C - Corner.
*	TL - Top Left
*	BR - Bottom right
*	Ну и по аналогии
*/

#define C_TL 0x00
#define C_TR 0x01
#define C_BL 0x02
#define C_BR 0x03

// Очевидно.

__INLINE void Sort2f(float &f1, float &f2)
{
	if (f1 > f2)
		swap(f1, f2);
}

/**
*	Опередляет тип угла, по типам пересецчения проекций боксов.
*	sX - тип проекции по оси Х, для У аналогично.
*/

__INLINE int DetectCorner(const int sX, const int sY)
{
	if (sY == SEG_PROJ_STATE_BEFORE)
	{
		if (sX == SEG_PROJ_STATE_BEFORE )
			return C_TL;
		if (sX == SEG_PROJ_STATE_AFTER )
			return C_TR;
	}

	if (sY == SEG_PROJ_STATE_AFTER)
	{
		if (sX == SEG_PROJ_STATE_BEFORE )
			return C_BL;
		if (sX == SEG_PROJ_STATE_AFTER )
			return C_BR;
	}
	ASSERT(false);
	return SEG_PROJ_STATE_BEFORE;	// Чисто формально что-то возвращаем, сюда нельзя попадать.
}

/*
*	Ф-я CornerProblemSolve(...) должна решать ту проблему с углами. Она пересекает отрезки и всё такое.
*	Ящитаю больше нечего комментировать, потому что и так в ней хуй разберёшься. Лучше не трогать.
*/

__INLINE bool CornerProblemSolve(int &Sx, int &Sy, const CAABB &dyn, const CAABB &stat, const Vector2 &vel)
{
	float hy0 = 0.0f, hy1 = 0.0f;
	float vx0 = 0.0f, vx1 = 0.0f;
	Vector2 v0;
	Vector2 v1;
	Vector2 x0, x1, p;
	int corner = DetectCorner(Sx, Sy);

	switch(corner)
	{
	case C_TL:
		{
			v0.x = dyn.p.x + dyn.W;
			v0.y = dyn.p.y + dyn.H;
			hy0 = stat.p.y - stat.H;
			vx0 = stat.p.x - stat.W;
			hy1 = stat.p.y + stat.H;
			vx1 = stat.p.x + stat.W;
		}
		break;
	case C_TR:
		{
			v0.x = dyn.p.x - dyn.W;
			v0.y = dyn.p.y + dyn.H;
			hy0 = stat.p.y - stat.H;
			vx0 = stat.p.x + stat.W;
			hy1 = stat.p.y + stat.H;
			vx1 = stat.p.x - stat.W;
		}
		break;
	case C_BL:
		{
			v0.x = dyn.p.x + dyn.W;
			v0.y = dyn.p.y - dyn.H;
			hy0 = stat.p.y + stat.H;
			vx0 = stat.p.x - stat.W;
			hy1 = stat.p.y - stat.H;
			vx1 = stat.p.x + stat.W;
		}
		break;
	case C_BR:
		{
			v0.x = dyn.p.x - dyn.W;
			v0.y = dyn.p.y - dyn.H;
			hy0 = stat.p.y + stat.H;
			vx0 = stat.p.x + stat.W;
			hy1 = stat.p.y - stat.H;
			vx1 = stat.p.x - stat.W;
		}
		break;
	}
	p = Vector2(vx0, hy0);
	v0 -= p;
	v1 = v0 + vel;
	float k = (v1.y - v0.y)/(v1.x - v0.x);
	float b = v1.x*k - v1.y;
	x0.x = -b/k;
	x0.y = 0.0f;
	x1.x = 0.0f;
	x1.y = b;
	x0 += p;
	x1 += p;

  	if (abs(x0.x - vx0) < 0.001f && abs(x1.y - hy0) < 0.001f)
	{
		return false;
	}
	Sort2f(hy0, hy1);
	if (hy0 < x1.y && x1.y < hy1)
	{
		Sy = SEG_PROJ_STATE_INTERSECT;
		if (corner == C_TL || corner == C_BL)
			Sx = SEG_PROJ_STATE_BEFORE;
		else
			Sx = SEG_PROJ_STATE_AFTER;
		return true;
	}
	Sort2f(vx0, vx1);
	if (vx0 < x0.x && x0.x < vx1)
	{
		Sx = SEG_PROJ_STATE_INTERSECT;
		if (corner == C_TL || corner == C_TR)
			Sy = SEG_PROJ_STATE_BEFORE;
		else
			Sy = SEG_PROJ_STATE_AFTER;
		return true;
	}
	return false;
}

__INLINE scalar Minf(const scalar a, const scalar b)
{
	return (a < b)?a:b;
}

__INLINE scalar Maxf(const scalar a, const scalar b)
{
	return (a > b)?a:b;
}


#define SEG_INTERS_B	0x00	// no intersection, first segment before second
#define SEG_INTERS_L	0x01	// left intersection
#define SEG_INTERS_AI	0x02	// a inside b
#define SEG_INTERS_BI	0x03	// b inside a
#define SEG_INTERS_R	0x04	// right intersection
#define SEG_INTERS_A	0x05	// no intersection, first segment after second
#define	SEG_INTERS_ERR	0x06	// Error!

__INLINE scalar Minfex(const scalar a, const scalar b)
{
	return (a < b)?a:-b;
}

__INLINE scalar Maxfex(const scalar a, const scalar b)
{
	return (a > b)?a:-b;
}

/**
*	Эта функция (SegIntersect(...)) определяет тип пересечения 2х отрезков
*	a.x < a.y - первый отрезок
*	аналогично для второго (b)
*	В delta мы возвращаем минимальную дистанцию, на которую надо распихнуть отрезки, чтобы они того...
*	Ф-я возвращает тип пересечение: есть или нет, если есть, то слева, справа или внутри, если нет, то перед и или после.
*	Результат рассматривается положение 1го отрезка относительно второго. Так-то.
*/

__INLINE int SegIntersect(const Vector2 &a, const Vector2 &b, scalar &delta)
{
#ifdef _DEBUG_
	if (a.x > a.y || b.x > b.y)
		return SEG_INTERS_ERR;
#endif
	delta = 0.0f;
	if (a.y < b.x)
		return SEG_INTERS_B;
	if (a.x > b.y)
		return SEG_INTERS_A;
	if (a.x > b.x && a.y < b.y)
	{
		delta = Minfex(b.y-a.x, a.y-b.x);
		return SEG_INTERS_AI;
	}
	if (b.x > a.x && b.y < a.y)
	{
		delta = Minfex(a.y-b.x, b.y-a.x);
		return SEG_INTERS_BI;
	}
	if (a.x > b.x)
	{
		delta = (b.y - a.x);
		return SEG_INTERS_R;
	}
	else
	{
		delta = -(a.y - b.x);
		return SEG_INTERS_L;
	}
	return SEG_INTERS_ERR;
}

static unsigned int g_seed = 152406923;

__INLINE int Random_Int(int min, int max)
{
	g_seed = 214013*g_seed + 2531011;
	return min + (g_seed^g_seed>>15) % (max - min + 1);
}

__INLINE float Random_Float(float min, float max)
{
	g_seed=214013*g_seed+2531011;
	//return min+g_seed*(1.0f/4294967295.0f)*(max-min);
	return min+(g_seed>>16)*(1.0f/65535.0f)*(max-min);
}


#endif
