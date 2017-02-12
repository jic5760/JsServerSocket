/**
 * @file	RandomWell512.h
 * @class	RandomWell512
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/09/30
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Random.h"

namespace JsCPPUtils
{

	class RandomWell512 : public Random
	{
	private:
		uint32_t m_state_i;
		uint32_t m_state_data[16];
		
	public:
		RandomWell512();
		~RandomWell512();
		
		void init(uint32_t *seeds);
		void init2(uint32_t seed);
		
		uint32_t _next();

		bool nextBoolean();
		void nextBytes(char *bytes, int len);
		float nextFloat();
		double nextDouble();
		int32_t nextInt();
		int32_t nextInt(int32_t n);
	};

}

