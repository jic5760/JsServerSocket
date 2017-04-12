/**
 * @file	HashMap.h
 * @class	HashMap
 * @author	Jichan (jic5760@naver.com)
 * @date	2017/04/12
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <new>
#include <exception>

#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Lockable.h"

namespace JsCPPUtils
{
	/**
	 * TKEY Key의 type
	 * TVALUE Value의 type
	 * _numofbuckets		Key에 대한 Bucket수
	 * _numofinitialblocks	초기 블록 수 (데이터 수)
	 * _incblocksize		지정한 블록 수 보다 데이터가 많아질 때 증가시길 블록 수
	 */
	template<typename TKEY, typename TVALUE, int _numofbuckets, int _numofinitialblocks, int _incblocksize>
		class basic_HashMap : private Lockable
		{
		private:
			struct _tag_block;
		
			typedef struct _tag_block
			{
				int used;
				struct _tag_block *pnext;
				TKEY key;
				TVALUE value;
			} block_t;
		
			block_t *m_blocks;
			int m_blocksize;
			int m_blockcount;
			block_t **m_buckets;
			uint32_t m_poly;
			
			bool m_freed;
		
			int _hash(uint64_t key)
			{
				uint64_t poly = m_poly;
				key *= poly;
				key += ~(key << 15);
				key ^=  (key >> 10);
				key +=  (key << 3);
				key ^=  (key >> 6);
				key += ~(key << 11);
				key ^=  (key >> 16);
				return key % _numofbuckets;
			}

		    // Copy assignment operator.  
			basic_HashMap& operator=(const basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>& other)  
			{  
			}
			
			// Copy constructor.  
			basic_HashMap(const basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>& _ref)
			{
			}
			
		public:
			explicit basic_HashMap()
				: m_poly(0x741B8CD7),
				m_buckets(NULL),
				m_blocks(NULL),
				m_freed(false)
			{
				m_blocksize = _numofinitialblocks;
				m_blockcount = 0;
				
				m_buckets = (block_t**)malloc(sizeof(block_t*) * _numofbuckets); // An exception may occur / std::bad_alloc
				if (m_buckets == NULL)
					throw std::bad_alloc();
				memset(m_buckets, 0, sizeof(block_t*) * _numofbuckets);
				
				m_blocks = (block_t*)malloc(sizeof(block_t) * m_blocksize); // An exception may occur / std::bad_alloc
				if (m_blocks == NULL)
				{
					free(m_buckets); m_buckets = NULL;
					throw std::bad_alloc();
				}
				memset(m_blocks, 0, sizeof(block_t) * m_blocksize);
			}
			
			~basic_HashMap()
			{
				m_freed = true;
				//m_blocksize = 0;
				//m_blockcount = 0;
				if (m_buckets != NULL)
				{
					free(m_buckets);
					m_buckets = NULL;
				}
				if (m_blocks != NULL)
				{
					free(m_blocks);
					m_blocks = NULL;
				}
			}
			
			/*
			// Move constructor.  
			basic_HashMap(const basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>&& _ref)
			{
				m_blocks = _ref.m_blocks;
				m_blocksize = _ref.m_blocksize;
				m_blockcount = _ref.m_blockcount;
				m_buckets = _ref.m_buckets;
				m_poly = _ref.m_poly;
			}
			*/
		
			// std::bad_alloc
			const TVALUE& operator[](const TKEY key) const
			{
				block_t *pblock;
				
				lock();
				
				pblock = _getblock(key);
				const TVALUE& ref_value = pblock->value;
				
				unlock();
				
				return ref_value;
			}
		
			TVALUE& operator[](const TKEY key)
			{
				block_t *pblock;
				
				lock();
				
				pblock = _getblock(key);
				TVALUE& ref_value = pblock->value;
				
				unlock();
				
				return ref_value;
			}
			
			void erase(const TKEY key)
			{
				int hash = _hash(key);
				block_t **pbucket;
				block_t *pblock;
				block_t *pprevblock = NULL;
				
				lock();
				
				pbucket = &m_buckets[hash];
				pblock = _findblock(pbucket, key, &pprevblock);
				if (pblock != NULL)
				{
					if (pprevblock != NULL)
						pprevblock->pnext = pblock->pnext;
					pblock->used = 0;
					m_blockcount--;
				}
				
				unlock();
			}
			
		private:
			block_t *_getblock(const TKEY key)
			{
				int hash = _hash(key);
				
				block_t **pbucket = &m_buckets[hash];
				block_t *pblock = _findblock(pbucket, key);
				
				if (pblock == NULL)
				{
					pblock = _getunusedblock(); // An exception may occur / std::bad_alloc
					
					m_blockcount++;
					
					pblock->used = 1;
					pblock->key = key;
					
					if (*pbucket == NULL)
					{
						*pbucket = pblock;
					}
					else {
						block_t *ptmpblock = *pbucket;
						while (ptmpblock)
						{
							if (ptmpblock->pnext == NULL)
								ptmpblock->pnext = pblock;
							ptmpblock = ptmpblock->pnext;
						}
					}
				}
				
				return pblock;
			}
			
			block_t *_findblock(block_t **pbucket, const TKEY key, block_t **pprevblock = NULL)
			{
				if (*pbucket != NULL)
				{
					block_t *ptmpprevblock = NULL;
					block_t *ptmpblock = *pbucket;
					while (ptmpblock)
					{
						if (ptmpblock->key == key)
						{
							if (pprevblock)
								*pprevblock = ptmpprevblock;
							return ptmpblock;
						}
							return ptmpblock;
						
						ptmpprevblock = ptmpblock;
						ptmpblock = ptmpblock->pnext;
					}
				}
				return NULL;
			}
			
			block_t *_getunusedblock()
			{
				if (m_blockcount == m_blocksize)
				{
					int new_blocksize = m_blocksize + _incblocksize;
					block_t *new_blocks = (block_t*)realloc(m_blocks, sizeof(block_t)*new_blocksize);
					if (new_blocks == NULL)
						throw std::bad_alloc();
					m_blocksize += new_blocksize;
					m_blocks = new_blocks;
					
					memset(m_blocks + m_blockcount, 0, sizeof(block_t)*_incblocksize);
					
					return &new_blocks[m_blockcount];
				}else if (m_blockcount > m_blocksize){
					throw std::exception();
				}else{
					int i;
					block_t *pblock = m_blocks;
					for (i = 0; i < m_blocksize; i++, pblock++)
					{
						if (pblock->used == 0)
						{
							memset(pblock, 0, sizeof(block_t));
							return pblock;
						}
					}
					throw std::exception();
				}
			}
		};
	
	template<typename TKEY, typename TVALUE, int _numofbuckets = 127, int _numofinitialblocks = 256, int _incblocksize = 8>
		class HashMap : public basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>
		{
		};
}
