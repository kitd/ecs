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
        int health;
    };

    Pool pool;

    pool.CreateUsing<Vector3D,Vector2D,Health>(10);

    assert(pool.SizeBytes() == pool._max_entities * (sizeof(Vector2D) + sizeof(Health) + sizeof(Vector3D)));

    Pool::id_type entityId = pool._max_entities;

    new (pool.Component<Health>(entityId)) Health{25};

    assert(pool.Components<Health>()[entityId].health == 25);

    Pool::id_type target = pool.NewEntity<Vector2D,Health>();
    Vector2D *v2 = pool.Component<Vector2D>(target);
    v2->x = 100;
    v2->y = 200;

    Pool::id_type bullet = pool.NewEntity<>();
    pool.Assign<Vector3D>(bullet);
    Vector3D *v3 = pool.Component<Vector3D>(bullet);
    v3->x = 10;
    v3->y = 20;
    v3->z = 30;

    pool.ForEachEntity<Vector2D>([&](Pool::id_type ent) {
        v2->x += 10;
        v2->y += 20;
    });

    assert( v2->x == 110 && v2->y == 220 && v3->x == 10 && v3->y == 20 && v3->z == 30);

    pool.Clear();
    assert(pool.SizeBytes() == 0);

    return 0;
}