/*************************************************************************
 *
 * ivi_portmap.c :
 *
 * MAP-T/MAP-E 4to6 Prefix Mapping Kernel Module
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#include "ivi_portmap.h"
#include "ivi_map.h"

typedef struct {
    MapPortmap_t      * htable[ MAPPORTMAP_HTABLE_SIZE ];
    MapPortmap_t        etable[ MAPPORTMAP_MAX_ENTRIES ];

    Dll_t         frlist;
} __attribute__((aligned(16))) MapPortmapping_t;

MapPortmapping_t mapportmapping;

static MapPortmap_t * mapportmap_alloc( void )
{
    MapPortmap_t * pmap_p = MAPPORTMAP_NULL;

    if (unlikely(dll_empty(&mapportmapping.frlist)))
    {
        return pmap_p;
    }

    if (likely(!dll_empty(&mapportmapping.frlist)))
    {
        pmap_p = (MapPortmap_t*)dll_head_p(&mapportmapping.frlist);
        dll_delete(&pmap_p->node);
        dll_init(&pmap_p->node);
    }

    return pmap_p;
}

static inline u32 _hash( u32 hash_val )
{
    hash_val ^= ( hash_val >> 16 );
    hash_val ^= ( hash_val >>  8 );
    hash_val ^= ( hash_val >>  3 );

    return ( hash_val );
}

static inline u32 _mapportmap_hash( u32 port )
{
    u32 hashix;

    hashix = _hash(port);

    return hashix % MAPPORTMAP_HTABLE_SIZE;
}

static inline u32 _mapportmap_match( const MapPortmap_t *pmap_p,
                                  u32 *lanAddr, u32 wanAddr, u32 port,
                                  u32 *intPort, u32 proto, int mode )
{
	if ((mode == MAPPORTMAP_MODE_ADD) || (mode == MAPPORTMAP_MODE_DEL)) {
    	return ((pmap_p->info.lanAddr == *lanAddr) && 
				(pmap_p->info.wanAddr == wanAddr) &&
				(pmap_p->info.intPort == *intPort) &&
				(pmap_p->info.port == port) && (pmap_p->info.proto == proto));
	}
	else {
		u32 test = *intPort ? ((pmap_p->info.intPort == *intPort) ? 1 : 0) : 1;

		if (pmap_p->info.wanAddr != MAPPORTMAP_INVALID_ADDRESS) 
			return ((pmap_p->info.wanAddr == wanAddr) &&
					(pmap_p->info.port == port) && 
					(pmap_p->info.proto & proto) && test);
		else
			return ((pmap_p->info.port == port) && 
					(pmap_p->info.proto & proto) && test);
	}
}

static void mapportmap_hashin( MapPortmap_t * pmap_p, u32 hashix )
{
    pmap_p->chain_p = mapportmapping.htable[ hashix ];
    mapportmapping.htable[ hashix ] = pmap_p;
}

static inline bool port_in_range(u16 _port, u16 _ratio, u16 _adjacent, u16 _offset)
{
	if (_ratio == 1)
		return true;
	else {
		u16 temp;
		_ratio = fls(_ratio) - 1;
		_adjacent = fls(_adjacent) - 1;
		temp = (_port >> _adjacent);
		return (temp - ((temp >> _ratio) << _ratio) == _offset);
	}
}

static u32 mapportmap_new( u32 *lanAddr, u32 wanAddr, u32 port, u32 *intPort, u32 proto, u32 hashix )
{
    MapPortmap_t * pmap_p;

	if (!port_in_range(port, hgw_ratio, hgw_adjacent, hgw_offset)) {
		printk("port<%u> not in MAP-T range\n", port);
        return MAPPORTMAP_IX_INVALID;
	}

    pmap_p = mapportmap_alloc();
    if ( unlikely(pmap_p == MAPPORTMAP_NULL) ) {
        return MAPPORTMAP_IX_INVALID;
    }

    pmap_p->info.lanAddr = *lanAddr;
    pmap_p->info.wanAddr = wanAddr;
    pmap_p->info.port = port;
    pmap_p->info.intPort = *intPort;
    pmap_p->info.proto = proto;
    mapportmap_hashin(pmap_p, hashix);

    return pmap_p->idx;
}

static void mapportmap_unhash(MapPortmap_t * pmap_p, u32 hashix)
{
    register MapPortmap_t * hFrag_p = mapportmapping.htable[hashix];

    if ( unlikely(hFrag_p == MAPPORTMAP_NULL) )
    {
        goto mapportmap_notfound;
    }

    if ( likely(hFrag_p == pmap_p) )                /* At head */
    {
        mapportmapping.htable[ hashix ] = pmap_p->chain_p;  /* Delete at head */
    }
    else
    {
        u32 found = 0;

        /* Traverse the single linked hash collision chain */
        for ( hFrag_p = mapportmapping.htable[ hashix ];
              likely(hFrag_p->chain_p != MAPPORTMAP_NULL);
              hFrag_p = hFrag_p->chain_p )
        {
            if ( hFrag_p->chain_p == pmap_p )
            {
                hFrag_p->chain_p = pmap_p->chain_p;
                found = 1;
                break;
            }
        }

        if ( unlikely(found == 0) )
        {
            goto mapportmap_notfound;
        }
    }

mapportmap_notfound:
    return; /* SUCCESS */
}

static void mapportmap_free( MapPortmap_t * pmap_p )
{
    pmap_p->info.lanAddr = 0;
    pmap_p->info.wanAddr = 0;
    pmap_p->info.port = 0;
    pmap_p->info.proto = 0;
    pmap_p->chain_p = MAPPORTMAP_NULL;

    dll_prepend(&mapportmapping.frlist, &pmap_p->node);
}

void mapportmap_delete( u32 idx, u32 proto )
{
    MapPortmap_t * pmap_p;
    u32 hashix;

    pmap_p = &mapportmapping.etable[idx];
    hashix = _mapportmap_hash(pmap_p->info.port);

    mapportmap_unhash(pmap_p, hashix);
    mapportmap_free(pmap_p);

    if (proto & (1 << MAPPORTMAP_PROTO_TCP))
        refresh_map_list(&tcp_list, idx);

    if (proto & (1 << MAPPORTMAP_PROTO_UDP))
        refresh_map_list(&udp_list, idx);

    if (proto & (1 << MAPPORTMAP_PROTO_ICMP))
        refresh_map_list(&icmp_list, idx);
}

u32 mapportmap_lookup( u32 *lanAddr, u32 wanAddr, u32 port, u32 *intPort, u32 proto, int mode )
{
    MapPortmap_t * pmap_p;
    u32 idx;
    u32 hashix;

    hashix = _mapportmap_hash(port);

    for ( pmap_p = mapportmapping.htable[ hashix ]; pmap_p != MAPPORTMAP_NULL;
          pmap_p = pmap_p->chain_p)
    {
        if (likely( _mapportmap_match(pmap_p, lanAddr, wanAddr, port, intPort, proto, mode) )) {
			if (mode == MAPPORTMAP_MODE_FIND) {
				*lanAddr = pmap_p->info.lanAddr;
				*intPort = pmap_p->info.intPort;
			}

            return pmap_p->idx;
		}
    }

	if (mode == MAPPORTMAP_MODE_ADD)
		idx = mapportmap_new(lanAddr, wanAddr, port, intPort, proto, hashix);
	else
		idx = MAPPORTMAP_IX_INVALID;

    return idx;
}

int mapportmap_port( u16 port, int type )
{
    MapPortmap_t * pmap_p;
    u32 hashix;
	int ret = 0;

    hashix = _mapportmap_hash(port);

    for ( pmap_p = mapportmapping.htable[ hashix ]; pmap_p != MAPPORTMAP_NULL;
          pmap_p = pmap_p->chain_p)
    {
        if (unlikely( (pmap_p->info.port == port) &&
				   	  (pmap_p->info.proto & type) )) {
			ret = 1;
			break;
		}
    }

	return ret;
}

int init_mapportmap_list( void )
{
    register int id;
    MapPortmap_t * pmap_p;

    memset( (void*)&mapportmapping, 0, sizeof(MapPortmapping_t) );

    /* Initialize list */
    dll_init( &mapportmapping.frlist );

    /* Initialize each entry and insert into free list */
    for ( id=MAPPORTMAP_IX_INVALID; id < MAPPORTMAP_MAX_ENTRIES; id++ )
    {
        pmap_p = &mapportmapping.etable[id];
        pmap_p->idx = id;

        if ( unlikely(id == MAPPORTMAP_IX_INVALID) )
            continue;           /* Exclude this entry from the free list */

        dll_append(&mapportmapping.frlist, &pmap_p->node);/* Insert into free list */
    }

    return 0;
}
