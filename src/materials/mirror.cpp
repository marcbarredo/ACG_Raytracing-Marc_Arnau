#include "mirror.h"

Mirror::Mirror()
{ }

Vector3D Mirror::getReflectance(
    const Vector3D &n,
    const Vector3D &wo,
    const Vector3D &wi
) const
{
    return Vector3D(0.0, 0.0, 0.0);
}
