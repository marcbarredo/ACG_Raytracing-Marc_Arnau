#include "hemisphericaldirectintegrator.h"

#include "../core/utils.h"

#include <algorithm>
#include <cmath>


HemisphericalDirectIntegrator::HemisphericalDirectIntegrator() :
    Shader(Vector3D(0.0))
{
}


HemisphericalDirectIntegrator::HemisphericalDirectIntegrator(Vector3D bgColor_) :
    Shader(bgColor_)
{
}


Vector3D HemisphericalDirectIntegrator::computeColor(
    const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    Intersection its;

    // Primary ray misses the scene
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }

    const Material& material = its.shape->getMaterial();

    Vector3D wo = -r.d;


    // If the camera ray directly hits an emissive surface
    if (material.isEmissive())
    {
        return material.getEmissiveRadiance();
    }


    // assume L indirect = Ia * Kd
    double Kd = 0.3;
    //Vector3D color = material.getDiffuseReflectance() * Kd;
    Vector3D color = 0.3;


    // Sample one random direction over the hemisphere
    HemisphericalSampler sampler;

    Vector3D wi = sampler.getSample(its.normal).normalized();


    // Secondary ray along sampled direction
    Ray sampleRay(its.itsPoint, wi, r.depth + 1, Epsilon, INFINITY );


    Intersection lightIts;

    // Only contributes if this random direction reaches geometry
    if (Utils::getClosestIntersection(sampleRay, objList, lightIts))
    {
        const Material& hitMaterial = lightIts.shape->getMaterial();

        // Direct incoming radiance is non-zero only if we hit an emitter
        if (hitMaterial.isEmissive())
        {
            Vector3D Li = hitMaterial.getEmissiveRadiance();

            Vector3D fr = material.getReflectance( its.normal, wo, wi);

            double cosTheta = std::max(dot(wi, its.normal),0.0);

            // Uniform hemisphere PDF = 1 / (2*pi)
            double pdf = 1.0 / (2.0 * 3.1416);

            color += Li * fr * cosTheta / pdf;
        }
    }

    return color;
}