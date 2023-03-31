#ifndef _ECS_HPP_
#define _ECS_HPP_ 1

#include <vector>
#include <numeric>
#include <functional>

struct Pool {

  using pool_type = unsigned char;
  using id_type = uint16_t;
  using mask_type = uint64_t;

  id_type _max_entities;

  ~Pool() {
    Clear();
  }

  template<typename T>
  inline id_type ComponentId() {
    static id_type id = register_component( sizeof(T) );
    return id;
  }

  template<typename T> 
  inline T* ComponentArray() {
    static T* array = (T*) (_comps[ComponentId<T>()].array);
    return array;
  }

  template<typename T> 
  inline T* ComponentAt( id_type pos ) {
    return (ComponentArray<T>() + pos);
  }

  id_type NewEntity() {
    if( _ents.size() == _max_entities ) {
      return 0xffff;
    }

    id_type id = _ents.size();
    _ents.push_back( {id, 0L} );
    return id;
  }

  template<typename... T>
  void Assign(id_type entId) {
    ent ent = _ents.at(entId);
    ent.mask = mask_for<T...>();
  }

  template<typename... T>
  void ForEachEntityWith( std::function<void (id_type)>& fn ) {
    mask_type mask = mask_for<T...>();
    for( ent& ent: _ents ) {
      if( masks_match(ent.mask, mask) ) {
        fn(ent.id);
      }
    }
  }

  void ForAllEntities( std::function<void (id_type)>& fn ) {
    for( ent& ent: _ents ) {
      fn(ent.id);
    }
  }

  void Build() {
    size_t total_size = std::accumulate( _comps.begin(), _comps.end(), 0, [](size_t acc, struct comp& c){ return acc + c.element_size; } );
    _pool_size = total_size * _max_entities;
    _the_pool = new pool_type[_pool_size];
    pool_type* curr_ptr = _the_pool;
    for( auto &comp : _comps ) {
      comp.array = curr_ptr;
      curr_ptr += comp.element_size * _max_entities;
    }
  }

  size_t Size() {
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
    mask_type mask = 0L;
    id_type compIds[] = { ComponentId<T>()... };
    for( id_type compId : compIds ) {
      mask |= ( 0x01 << compId );
    } 
    return mask;
  }

  inline bool masks_match( mask_type subj, mask_type test ) {
    return (subj & test) == test;
  }

  pool_type* _the_pool = nullptr;
  unsigned long long _pool_size = 0;
};

#endif