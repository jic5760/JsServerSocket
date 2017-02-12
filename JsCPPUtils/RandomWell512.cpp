/**
 * @file	RandomWell512.cpp
 * @class	RandomWell512
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/09/30
 * @brief	RandomWell512
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "RandomWell512.h"

#include <time.h>

namespace JsCPPUtils
{

#define W 32
#define P 0
#define M1 13
#define M2 9
#define M3 5

#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define MAT3NEG(t,v) (v<<(-(t)))
#define MAT4NEG(t,b,v) (v ^ ((v<<(-(t))) & b))

#define V0            m_state_data[m_state_i                   ]
#define VM1           m_state_data[(m_state_i+M1) & 0x0000000fU]
#define VM2           m_state_data[(m_state_i+M2) & 0x0000000fU]
#define VM3           m_state_data[(m_state_i+M3) & 0x0000000fU]
#define VRm1          m_state_data[(m_state_i+15) & 0x0000000fU]
#define VRm2          m_state_data[(m_state_i+14) & 0x0000000fU]
#define newV0         m_state_data[(m_state_i+15) & 0x0000000fU]
#define newV1         m_state_data[m_state_i                   ]
#define newVRm1       m_state_data[(m_state_i+14) & 0x0000000fU]
	
	RandomWell512::RandomWell512()
	{
		init2(time(NULL));
	}


	RandomWell512::~RandomWell512()
	{

	}

	void RandomWell512::init(uint32_t *seeds)
	{
		int i;
		m_state_i = 0;
		for(i=0; i<16; i++)
		{
			m_state_data[i] = seeds[i];
		}
	}
	
	void RandomWell512::init2(uint32_t seed)
	{
		int i;
		m_state_i = 0;
		m_state_data[0] = seed;
		for(i=1; i<16; i++)
		{
			uint32_t x = (m_state_data[i-1] + i*i);
			m_state_data[i] = x * x;
		}
	}
	
	uint32_t RandomWell512::_next()
	{
		uint32_t z0, z1, z2;
		z0    = VRm1;
		z1    = MAT0NEG (-16,V0)    ^ MAT0NEG (-15, VM1);
		z2    = MAT0POS (11, VM2)  ;
		newV1 = z1                  ^ z2; 
		newV0 = MAT0NEG (-2,z0)     ^ MAT0NEG(-18,z1)    ^ MAT3NEG(-28,z2) ^ MAT4NEG(-5,0xda442d24U,newV1) ;
		m_state_i = (m_state_i + 15) & 0x0000000fU;
		return m_state_data[m_state_i];
	}

	bool RandomWell512::nextBoolean()
	{
		if(_next() % 2)
			return true;
		else
			return false;
	}
	void RandomWell512::nextBytes(char *bytes, int len)
	{
		int i;
		for(i=0; i<len; i++)
		{
			bytes[i] = ((char)(_next() & 0xFF));
		}
	}
	float RandomWell512::nextFloat()
	{
		return (((float)_next()) / 4294967295.0f);
	}
	double RandomWell512::nextDouble()
	{
		return (((double)_next()) / 4294967295.0f);
	}
	int32_t RandomWell512::nextInt()
	{
		return (int32_t)_next();
	}
	int32_t RandomWell512::nextInt(int32_t n)
	{
		return (_next() % n);
	}

}