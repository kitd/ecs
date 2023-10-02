#ifndef _ECS_HPP_
#define _ECS_HPP_ 1

#include <vector>
#include <numeric>
#include <functional>
#include <limits>


struct Pool {

  using mem_type = unsigned char;
  using id_type = uint16_t;
  using mask_type = uint64_t;

  id_type _max_entities;

  ~Pool() {
    if( _the_pool != nullptr ) {
      _comps.clear();

      _ents.clear();
      _max_entities = 0;

      if( !_in_place ){
        delete[] _the_pool;
      }
      _the_pool = nullptr;
    }
  }

  template<typename... T>
  size_t Define(size_t max_entities) {
    _max_entities = max_entities;

    // C++11 trick to bypass `unused variable` warning with paramater pack expansion
    // https://stackoverflow.com/a/25683817
    using unpack = id_type[];
    unpack{ component_id<T>()... };

    return SizeBytes();
  }

  void BuildAt(void* p) {
    _the_pool = new (p) mem_type[SizeBytes() / sizeof(mem_type)];
    _in_place = true;

    assign_component_storage();
  }

  void Build() {
    _the_pool = new mem_type[SizeBytes() / sizeof(mem_type)];
    _in_place = false;

    assign_component_storage();
  }

  size_t SizeBytes() {
    size_t total_size = std::accumulate( _comps.begin(), _comps.end(), 0, [](size_t acc, comp &c) { return acc + c.element_size; } );
    return total_size * _max_entities;
  }

  template<typename T> 
  inline T* GetComponent( id_type pos ) {
    static T* array = (T*) (_comps[component_id<T>()].array); // Component=>array cache
    return (array + pos);
  }

  template<typename T> 
  inline T* GetComponents() {
    return GetComponent<T>(0);
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
  void ForEachEntityWithAll( std::function<void (id_type)> fn ) {
    mask_type mask = mask_for<T...>();
    for( ent& ent: _ents ) {
      if( mask == 0 || all_match(ent.mask, mask) ) {
        fn(ent.id);
      }
    }
  }

  template<typename... T>
  void ForEachEntityWithAny( std::function<void (id_type)> fn ) {
    mask_type mask = mask_for<T...>();
    for( ent& ent: _ents ) {
      if( mask == 0 || any_match(ent.mask, mask) ) {
        fn(ent.id);
      }
    }
  }

private:
  struct comp {
    size_t element_size;
    mem_type* array;
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

  void assign_component_storage() {
    mem_type* curr_ptr = _the_pool;
    for( comp &c : _comps ) {
      c.array = curr_ptr;
      curr_ptr += c.element_size * _max_entities;
    }
  }

  template<typename T>
  inline id_type component_id() {
    static id_type id = register_component( sizeof(T) ); // Component=>id cache
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
    
    if( sizeof...(T) > 0 ) {
      id_type compIds[] = { component_id<T>()... };
      for( id_type compId : compIds ) {
        mask |= ( 0x01 << compId );
      } 
    }
    return mask;
  }

  inline bool all_match( mask_type subj, mask_type test ) {
    return (subj & test) == test;
  }

  inline bool any_match( mask_type subj, mask_type test ) {
    return (subj & test) > 0;
  }

  mem_type* _the_pool = nullptr;
  bool _in_place;
};

template<int MAX_ENTS, typename... T>
struct Pool2 {
  using pool_type = unsigned char;
  using id_type = uint16_t;
  using mask_type = uint64_t;

  id_type ids[sizeof...(T)];


};

#endif