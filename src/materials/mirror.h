#ifndef MIRROR_H
#define MIRROR_H

#include "material.h"

class Mirror : public Material
{
public:
    Mirror();

    virtual Vector3D getReflectance(
        const Vector3D &n,
        const Vector3D &wo,
        const Vector3D &wi
    ) const;

    virtual bool hasSpecular() const { return true; }
    virtual bool hasTransmission() const { return false; }
    virtual bool hasDiffuseOrGlossy() const { return false; }
    virtual bool isEmissive() const { return false; }
};

#endif
