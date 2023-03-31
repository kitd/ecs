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

    pool._max_entities = 10;
    pool.ComponentId<Vector3D>();
    pool.ComponentId<Vector2D>();
    pool.ComponentId<Health>();
    pool.Build();

    assert(pool.Size() == 10 *(sizeof(Vector2D) + sizeof(Health) + sizeof(Vector3D)));

    Pool::id_type entityId = pool._max_entities;

    new (pool.ComponentAt<Health>(entityId)) Health{25};

    assert(pool.ComponentArray<Health>()[entityId].health == 25);

    pool.Clear();
    assert(pool.Size() == 0);

    return 0;
}