#ifndef __VECTORTYPE_H__
#define __VECTORTYPE_H__

// define PODs
template<class T, int leng, bool usePtrRhs = false>
struct Vec {
	// typedefs
	typedef Vec<T, leng, usePtrRhs> ThisType;

	// the data
	T data[leng];

	// operator[]
	inline T& operator[](const int idx) {return data[idx];}
	inline const T& operator[](const int idx) const {return data[idx];}

	// +-*/
#define DEFINE_FUND_ASSIGN(OP)\
	inline ThisType& operator OP##=(const ThisType &r)\
	{\
		for (int i = 0; i < leng; ++i) {\
			data[i] OP##= r[i];\
		}\
		return *this;\
	}
	DEFINE_FUND_ASSIGN(+)
	DEFINE_FUND_ASSIGN(-)
	DEFINE_FUND_ASSIGN(*)
	DEFINE_FUND_ASSIGN(/)

#define DEFINE_FUND_BINARY(OP)\
	inline ThisType operator OP (const ThisType &r) const\
	{\
		ThisType ret = *this;\
		ret OP##= r;\
		return ret;\
	}
	DEFINE_FUND_BINARY(+)
	DEFINE_FUND_BINARY(-)
	DEFINE_FUND_BINARY(*)
	DEFINE_FUND_BINARY(/)

#define DEFINE_FUND_COMPONENT_ASSIGN(OP)\
	inline ThisType& operator OP##=(const T r)\
	{\
		for (int i = 0; i < leng; ++i) {\
			data[i] OP##= r;\
		}\
		return *this;\
	}
	DEFINE_FUND_COMPONENT_ASSIGN(+)
	DEFINE_FUND_COMPONENT_ASSIGN(-)
	DEFINE_FUND_COMPONENT_ASSIGN(*)
	DEFINE_FUND_COMPONENT_ASSIGN(/)
	
#define DEFINE_FUND_COMPONENT_BINARY(OP)\
	inline ThisType operator OP (const T op2) const \
	{\
		ThisType ret = *this;\
		ret OP##= op2;\
		return ret;\
	}
	DEFINE_FUND_COMPONENT_BINARY(+)
	DEFINE_FUND_COMPONENT_BINARY(-)
	DEFINE_FUND_COMPONENT_BINARY(*)
	DEFINE_FUND_COMPONENT_BINARY(/)

	T dot(const ThisType &rhs) const
	{
		T ret = 0;
		for (int i = 0; i < leng; ++i) {
			ret += data[i] * rhs[i];
		}
		return ret;
	}


#undef DEFINE_FUND_ASSIGN
#undef DEFINE_FUND_BINARY
#undef DEFINE_FUND_COMPONENT_ASSIGN
#undef DEFINE_FUND_COMPONENT_BINARY
};

#endif /* end of include guard */
