#include <iostream>
#include <assert.h>
#include "ecs.hpp"

int main() {
    std::cout << "Test is running" << std::endl;

    struct Vector2D {
        float x;
        float y;
    };

    struct Vector3D {
        float x;
        float y;
        float z;
    };

    struct Health {
        int val;
    };

    Pool::mem_type buffer[512];
    Pool pool;

    pool.Define<Vector3D,Vector2D,Health>(10);

    assert(pool.SizeBytes() == pool._max_entities * (sizeof(Vector2D) + sizeof(Health) + sizeof(Vector3D)));

    pool.BuildAt(buffer);

    Pool::id_type entityId = pool._max_entities;

    new (pool.GetComponent<Health>(entityId)) Health{25};

    Health& health = pool.GetComponents<Health>()[entityId];
    assert(health.val == 25);

    Pool::id_type target = pool.NewEntity<Vector2D,Health>();
    Vector2D *v2 = pool.GetComponent<Vector2D>(target);
    v2->x = 100;
    v2->y = 200;

    Pool::id_type bullet = pool.NewEntity<>();
    pool.Assign<Vector3D>(bullet);
    new (pool.GetComponent<Vector3D>(bullet)) Vector3D{10, 20, 30};

    pool.ForEachEntityWithAll<Vector2D>([&](Pool::id_type ent) {
        pool.GetComponent<Vector2D>(ent)->x += 10;
        pool.GetComponent<Vector2D>(ent)->y += 20;
    });

    assert( v2->x == 110 
        && v2->y == 220 
        && pool.GetComponent<Vector3D>(bullet)->x == 10 
        && pool.GetComponent<Vector3D>(bullet)->y == 20 
        && pool.GetComponent<Vector3D>(bullet)->z == 30);


    return 0;
}