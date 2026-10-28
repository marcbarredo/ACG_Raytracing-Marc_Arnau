#include "arealightsource.h"

AreaLightSource::AreaLightSource(Square* areaLightsource_) :
    myAreaLightsource(areaLightsource_)
{ }



Vector3D AreaLightSource::getIntensity() const
{
    return myAreaLightsource->getMaterial().getEmissiveRadiance();
}


Vector3D AreaLightSource::sampleLightPosition()   const
{
    double u = (double)rand() / RAND_MAX;
    double v = (double)rand() / RAND_MAX;

    return myAreaLightsource->corner + u * myAreaLightsource->v1 + v * myAreaLightsource->v2;
}

