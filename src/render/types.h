#ifndef __RENDER_TYPES_H_
#define __RENDER_TYPES_H_

typedef float scalar;

// Отключает C4201: nameless struct/union, возникающий в Vector4
#if _MSC_VER > 1000
#pragma warning (disable : 4201)
#endif

union Vector4
{
public:
	scalar v[4];
	struct {scalar x, y, z, w;};
	struct {scalar r, g, b, a;};

	__INLINE scalar operator[](int i)
	{
		if (i<0 || i> 3)
			return 0;
		return v[i];
	}


	Vector4(){x=y=z=0.0f;w=1.0f;}
	Vector4(scalar xv, scalar yv, scalar zv, scalar wv){x=xv;y=yv;z=zv;w=wv;}
	//void operator=(const Vector4 &q){x=q.x;y=q.y;z=q.z;w=q.w;}
	__INLINE Vector4 operator+(Vector4 q){return Vector4( x + q.x , y + q.y, z + q.z, w + q.w );}
	__INLINE Vector4 operator-(Vector4 q){return Vector4( x - q.x , y - q.y, z - q.z, w - q.w );}
	__INLINE Vector4 operator*(scalar s){return Vector4(x * s, y * s, z * s, w * s);}
	__INLINE bool operator!=(Vector4 q){return x != q.x || y != q.y || z != q.z || w != q.w;}
	__INLINE Vector4 operator=(const Vector4& V)
	{
		x = V.x;
		y = V.y;
		z = V.z;
		w = V.w;
		return *this;
	}


	__INLINE Vector4 operator/(scalar s)
	{
		if (s == 0.0f)
			return Vector4();
		scalar is = 1.0f/s; 
		return (*this)*is;
	}
	__INLINE Vector4 operator+=(Vector4 q){(*this) = (*this)+q; return *this;}
	scalar Norm()const{return x * x + y * y + z * z + w * w;}
	scalar Length( void ){return sqrt(x * x + y * y + z * z + w * w );}	
	Vector4 Conjugate(){ return Vector4(-x,-y,-z,w);}
	Vector4 Identity()
	{
		//TODO: Danger chek division by zero
		return (*this).Conjugate()/(*this).Norm();
	};
	scalar InnerProduct(const Vector4& q){return x*q.x+y*q.y+z*q.z+w*q.w;}

	__INLINE Vector4 operator*(const Vector4 &q)
	{
		Vector4 result;

		result.x = w * q.x + x * q.w + y * q.z - z * q.y;
		result.y = w * q.y - x * q.z + y * q.w + z * q.x;
		result.z = w * q.z + x * q.y - y * q.x + z * q.w;
		result.w = w * q.w - x * q.x - y * q.y - z * q.z;

		return result;
	}

	__INLINE void glSet()
	{
		glColor4fv(&r);
	}
};

typedef Vector4 Quaternion;
typedef Vector4 RGBAf;

struct  vertex2f_t{
	float x;
	float y;
	vertex2f_t():x(0),y(0){}
};

struct  vertex3f_t{
	float x;
	float y;
	float z;
	vertex3f_t():x(0),y(0),z(0){}
};

//struct  vertex3d_t{
//	double x;
//	double y;
//	double z;
//	vertex3d_t():x(0),y(0),z(0){}
//};


struct coord2f_t{
	float x;
	float y;
	coord2f_t():x(0),y(0){}
	coord2f_t(float x, float y):x(x),y(y){}
};


enum RenderSpriteMethod { rsmStandart, rsmStretch, rsmCrop, rsmRepeatX, rsmRepeatY, rsmRepeatXY };

#endif // __RENDER_TYPES_H_
