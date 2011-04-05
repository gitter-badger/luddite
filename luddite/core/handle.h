#ifndef HANDLE_H
#define HANDLE_H

// based on scott bilas' handle class in
// http://scottbilas.com/publications/gem-resmgr/

#include <EASTL/vector.h>

#include <luddite/core/debug.h>

namespace luddite
{

// ===============================================================
template <typename TAG>
class Handle
{
    enum
    {
        // Sizes to use for bit fields
        MAX_BITS_INDEX = 16,
        MAX_BITS_MAGIC = 16,

        // sizes to compare aganst for asserting derefs
        MAX_INDEX = ( 1 << MAX_BITS_INDEX) - 1,
        MAX_MAGIC = ( 1 << MAX_BITS_MAGIC) - 1
    };
        

    union
    {
        struct
        {
            unsigned m_index : MAX_BITS_INDEX;
            unsigned m_magic : MAX_BITS_MAGIC;            
        };
        
        unsigned int m_handle;
    };
    
    
public:

    // Lifetime
    Handle( void ) : m_handle( 0 ) {}
    void init( unsigned int index );
    
    // Queries
    unsigned int getIndex(void) const { return ( m_index ); }
    unsigned int getMagic(void) const { return ( m_magic ); }
    unsigned int getHandle(void) const { return ( m_handle ); }
    bool isNull(void) const { return ( !m_handle ); }

    operator unsigned int (void) const { return ( m_handle ); }
};

// ---------------------------------------------------------------

template <typename TAG>
void Handle<TAG>::init( unsigned int index )
{
    Assert( isNull(), "Handle already initted" );
    Assert( index <= MAX_INDEX, "Handle table full");
    
    // generate a magic number
    static unsigned int s_autoMagic = 0;
    if ( ++s_autoMagic > MAX_MAGIC)
    {
        // 0 reserved for null handle
        s_autoMagic=1;        
    }
    
    m_index = index;
    m_magic = s_autoMagic;    
}

template <typename TAG>
inline bool operator != ( Handle<TAG> a, Handle<TAG> b )
{
    return ( a.getHandle() != b.getHandle() );    
}

template <typename TAG>
inline bool operator == ( Handle<TAG> a, Handle<TAG> b )
{
    return ( a.getHandle() == b.getHandle() );    
}

// ===============================================================
template <typename DATA, typename HANDLE>
class HandleMgr
{
private:
    typedef eastl::vector<DATA>   UserVec;
    typedef eastl::vector<unsigned int> MagicVec;
    typedef eastl::vector<unsigned int> FreeVec;

    // private data
    UserVec   m_userData;
    MagicVec  m_magicNumbers;
    FreeVec   m_freeSlots;
    
public:

    // Lifetime
    HandleMgr( void ) {}
    ~HandleMgr( void ) {}
    
    // aquisition
    DATA *acquire( HANDLE &handle );
    void release( HANDLE  handle );

    // deref
    DATA *deref( HANDLE handle );
    const DATA *deref( HANDLE handle ) const;
    
    // other queries
    unsigned int getUsedHandleCount( void ) const 
    {
        return ( m_magicNumbers.size() - m_freeSlots.size() );
    }

    bool hasUsedHandles( void ) const
    {
        return ( !!getUsedHandleCount() );
    }
};

template <typename DATA, typename HANDLE>
DATA *HandleMgr< DATA, HANDLE>::acquire( HANDLE &handle )
{
    // if free list is empty, add a new one otherwise use first 
    // one found
    unsigned int index;
    if (m_freeSlots.empty() )
    {
        // No free slots, grow the array
        index = m_magicNumbers.size();
        
        // Initialize the handle
        handle.init( index );
        m_userData.push_back( DATA() );
        m_magicNumbers.push_back( handle.getMagic() );
    }
    else
    {
        // Reuse a handle from the free list
        index = m_freeSlots.back();
        handle.init( index );
        m_freeSlots.pop_back();
        m_magicNumbers[ index ] = handle.getMagic();
    }
    
    return &(m_userData[index]);    
}



template <typename DATA, typename HANDLE>
void HandleMgr<DATA, HANDLE>::release( HANDLE handle )
{
    // which one?
    unsigned int index = handle.getIndex();
    
    // make sure it's valid
    Assert( index < m_userData.size(), "Handle index out of bounds" );
    Assert( m_magicNumbers[ index ] == handle.getMagic(), 
            "Corrupt handle" );
    
    // ok remove it
    m_magicNumbers[ index ] = 0;
    m_freeSlots.push_back( index );    
}

template <typename DATA, typename HANDLE >
inline DATA *HandleMgr<DATA, HANDLE>::deref( HANDLE handle )
{
    if (handle.isNull() ) return NULL;
    
    // check handle validity -- this may be removed for
    // speed in final build
    unsigned int index = handle.getIndex();
    if ( (index >= m_userData.size() ) ||
         ( m_magicNumbers[ index ] != handle.getMagic() ) )
    {
        // No good! invalid handle
        Assert( false, "Invalid Handle" );
        return NULL;        
    }

    return &(m_userData[index]);    
}

template <typename DATA, typename HANDLE>
inline const DATA *HandleMgr<DATA, HANDLE>::deref( HANDLE handle ) const
{
    typedef HandleMgr <DATA, HANDLE> ThisType;
    return ( const_cast<ThisType*>( this )->deref( handle ) );    
}

}; // namespace luddite



#endif


