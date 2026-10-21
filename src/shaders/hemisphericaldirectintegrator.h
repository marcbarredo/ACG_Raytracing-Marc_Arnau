#ifndef HEMISPHERICALDIRECTINTEGRATOR_H
#define HEMISPHERICALDIRECTINTEGRATOR_H

#include "shader.h"
#include "../core/hemisphericalsampler.h"

class HemisphericalDirectIntegrator : public Shader
{
public:
    HemisphericalDirectIntegrator();
    HemisphericalDirectIntegrator(Vector3D bgColor_);

    Vector3D computeColor(
        const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList
    ) const override;
};

#endif // HEMISPHERICALDIRECTINTEGRATOR_H