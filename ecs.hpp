#ifndef _ECS_HPP_
#define _ECS_HPP_ 1

#include <vector>
#include <numeric>
#include <functional>
#include <limits>

struct Pool {

  using pool_type = unsigned char;
  using id_type = uint16_t;
  using mask_type = uint64_t;

  id_type _max_entities;

  template<typename... T>
  void CreateUsing(size_t max_entities) {
    _max_entities = max_entities;

    id_type ids[] = { component_id<T>()... };
    size_t total_size = std::accumulate( _comps.begin(), _comps.end(), 0, [](size_t acc, comp &c) { return acc + c.element_size; } );
    _pool_size = total_size * _max_entities;
    _the_pool = new pool_type[_pool_size / sizeof(pool_type)];

    pool_type* curr_ptr = _the_pool;
    for( id_type id: ids ) {
      _comps[id].array = curr_ptr;
      curr_ptr += _comps[id].element_size * _max_entities;
    }

  }

  ~Pool() {
    Clear();
  }

  template<typename T> 
  inline T* Component( id_type pos ) {
    static T* array = (T*) (_comps[component_id<T>()].array);
    return (array + pos);
  }

  template<typename T> 
  inline T* Components() {
    return Component<T>(0);
  }

  template<typename... T>
  id_type NewEntity() {
    if( _max_entities > 0 && _ents.size() == _max_entities ) {
      return 0xffff;
    }

    mask_type mask = mask_for<T...>();

    id_type id = _ents.size();
    _ents.push_back( {id, mask} );
    return id;
  }

  template<typename... T>
  void Assign(id_type entId) {
    ent& ent = _ents.at(entId);
    ent.mask |= mask_for<T...>();
  }

  template<typename... T>
  void ForEachEntity( std::function<void (id_type)> fn ) {
    mask_type mask = mask_for<T...>();
    for( ent& ent: _ents ) {
      if( mask == 0 || masks_match(ent.mask, mask) ) {
        fn(ent.id);
      }
    }
  }

  inline size_t SizeBytes() {
    return _pool_size;
  }
  
  void Clear() {
    if( _the_pool != nullptr ) {
      _comps.clear();

      _ents.clear();
      _max_entities = 0;

       delete[] _the_pool;
      _the_pool = nullptr;
      _pool_size = 0;
    }
  }

private:
  struct comp {
    size_t element_size;
    pool_type* array;
  };
  std::vector<comp> _comps{};

  struct ent {
    id_type id;
    mask_type mask;
  };
  std::vector<ent> _ents{};

  template<typename T>
  inline id_type component_id() {
    static id_type id = register_component( sizeof(T) );
    return id;
  }

  id_type register_component( size_t size ) {
    id_type id = _comps.size();
    _comps.push_back( {size, nullptr} );
    return id;
  }

  template<typename... T>
  inline mask_type mask_for() {
    static mask_type mask = create_mask<T...>();
    return mask;
  }

  template<typename... T>
  mask_type create_mask() {
    if( sizeof...(T) == 0 ) {
      return (mask_type)0L;
    }

    mask_type mask = 0L;
    id_type compIds[] = { component_id<T>()... };
    for( id_type compId : compIds ) {
      mask |= ( 0x01 << compId );
    } 
    return mask;
  }

  inline bool masks_match( mask_type subj, mask_type test ) {
    return (subj & test) == test;
  }

  pool_type* _the_pool = nullptr;
  size_t _pool_size;
};

#endif