#ifndef AREADIRECTINTEGRATOR_H
#define AREADIRECTINTEGRATOR_H

#include "shader.h"

class AreaDirectIntegrator : public Shader
{
public:

    AreaDirectIntegrator();

    AreaDirectIntegrator(Vector3D bgColor_);

    Vector3D computeColor(
        const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList
    ) const override;
};

#endif // AREADIRECTINTEGRATOR_H