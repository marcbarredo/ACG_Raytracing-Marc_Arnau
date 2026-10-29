#ifndef NEEINTEGRATOR_H
#define NEEINTEGRATOR_H

#include "shader.h"

class NEEIntegrator : public Shader
{
public:
    NEEIntegrator();
    NEEIntegrator(Vector3D bgColor_);

    Vector3D computeColor(
        const Ray &r,
        const std::vector<Shape*> &objList,
        const std::vector<LightSource*> &lsList
    ) const override;
};

#endif // NEEINTEGRATOR_H
